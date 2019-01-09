#include <mymillions/mymillions.hpp>
#include <mymillions/logger.hpp>
#include <utility>
#include "logger.hpp"
#include "config.hpp"
#include "tables.hpp"

namespace {
    using namespace eosio;
    using namespace millions::config;
    using namespace millions::common;
    using tables::FactoryConfig;

    void init_factories_config(name bill_to_account, tables::FactoriesConfig& config)
    {
        for(const auto& item: factories_config)
        {
            log(item);
            config.emplace(bill_to_account, [&](auto& cfg)
            {
                cfg.type = item.type;
                cfg.level = item.level;
                cfg.price_for_next_lvl = item.price_for_next_lvl;
                cfg.produce_bonus = item.produce_bonus;
                cfg.produce = item.produce;
            });
        }
    }
}

namespace millions
{
using common::ResourceType;
using common::Levels;
using config::DEFAULT_SYMBOL;

MyMillions::MyMillions(name receiver, name code, eosio::datastream<const char*> ds)
    : contract(receiver, code, ds),
    _globalConfig(_self, _self.value),
    _factoriesConfig(_self, _self.value),
    _userProfiles(_self, _self.value),
    _factories(_self, _self.value)
{
    log("MyMillions Constructor started\n");
    if (!_globalConfig.exists())
    {
        /*
         * IMPORTANT: Initialize once on first call!
         * */
        _stateConfig = get_default_config(_self);
        init_factories_config(_self, _factoriesConfig);
    }
    else
    {
        _stateConfig = _globalConfig.get();
    }

    log("MyMillions Constructor finished\n");
}

MyMillions::~MyMillions()
{
    log("MyMillions destructor started\n");
    _globalConfig.set(_stateConfig, _self);
    log("MyMillions destructor finished\n");
}

void MyMillions::setAdmin(name caller, name admin)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot change admin.");
    eosio_assert(is_account(admin), "Unregistered 'admin' account.");
    _stateConfig.admin = admin;
}

void MyMillions::setWallets(name caller, name main_wallet, name dev_wallet)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot set developers wallet.");
    eosio_assert(is_account(main_wallet), "Unregistered 'main' account.");
    eosio_assert(is_account(dev_wallet), "Unregistered 'dev' account.");
    _stateConfig.main_wallet = main_wallet;
    _stateConfig.developer_wallet = dev_wallet;
}

void MyMillions::pay_from_user_balance(tables::RegisteredAccounts::const_iterator& profile_it, name to, asset quantity,
        const std::string& msg)
{
    if(_stateConfig.owner != _stateConfig.main_wallet)
    {
        action(
                permission_level{_self, "active"_n},
                "eosio.token"_n,
                "transfer"_n,
                std::make_tuple(
                        _stateConfig.owner,
                        to,
                        quantity,
                        msg
                )
        ).send();
    }
    _userProfiles.modify(profile_it, _self, [&](auto& profile)
    {
        profile.payment_balance -= quantity;
    });
}

void MyMillions::enableWithdraw(name user, bool value)
{
    require_auth(user);
    eosio_assert(_stateConfig.admin == user || _stateConfig.owner == user, "You cannot enable/disable withdraw.");
    _stateConfig.enable_withdraw = value;
}

void MyMillions::withdraw(name user, double value)
{
    log("User request withdraw %\n", value, user);
    eosio_assert(_stateConfig.enable_withdraw, "Withdraw operation temporarily disabled to fix vulnerability.");
    require_auth(user);
    auto quantity = asset{int64_t(DEFAULT_MULT * value), DEFAULT_SYMBOL};
    auto profile_it = _userProfiles.find(user.value);
    eosio_assert(_userProfiles.end() != profile_it, "Logic error: profile not found.");
    eosio_assert(profile_it->withdraw_balance >= quantity, "Not enough 'withdraw' balance.");
    action(
            permission_level{get_self(), "active"_n},
            "eosio.token"_n,
            "transfer"_n,
            std::make_tuple(
                    _stateConfig.owner,
                    user,
                    quantity,
                    std::string("MyMillions: withdraw.")
            )
    ).send();
    _userProfiles.modify(profile_it, _self, [&](auto& profile)
    {
        profile.withdraw_balance -= quantity;
    });
}

void MyMillions::pay_for_factory(name user, ResourceType resource_type, Levels factory_level, uint32_t count)
{
    log("pay_for_factory(%, %, %, %)\n", user, (int)resource_type, (int)factory_level, count);
    require_auth(user);
    eosio_assert(count >= 1, "Parameters error: count < 1.");
    auto config_key = FactoryConfig::make_primary_key(resource_type, factory_level);
    auto factory_it = _factoriesConfig.find(config_key);
    eosio_assert(factory_it != _factoriesConfig.end(), "Logic error: Config for factory not found.");
    auto profile_it = _userProfiles.find(user.value);
    eosio_assert(_userProfiles.end() != profile_it, "Logic error: Profile not found.");
    auto base_quantity = asset(DEFAULT_MULT * count * factory_it->price_for_next_lvl, DEFAULT_SYMBOL);
    eosio_assert(base_quantity.is_valid(), "Logic error: Invalid main asset.");
    eosio_assert(profile_it->payment_balance >= base_quantity, "User error: Not enough 'payment' balance.");
    log("pay fee % for % factories of type % \n", base_quantity.amount, count, (int)resource_type);

    auto quantity = asset(base_quantity.amount * (1 - _stateConfig.developers_percent), DEFAULT_SYMBOL);
    std::string msg = "From 'payment' balance of user " + user.to_string() +
                      " to main wallet for " + std::to_string(count) +
                      " factory(ies) of " + std::to_string(factory_level) + " level.";
    pay_from_user_balance(profile_it, _stateConfig.main_wallet, quantity, msg);

    quantity = asset(base_quantity.amount * _stateConfig.developers_percent, DEFAULT_SYMBOL);
    msg = "From 'payment' balance of user " + user.to_string() +
          " to dev wallet for " + std::to_string(count) +
          " factory(ies) of " + std::to_string(factory_level) + " level.";
    pay_from_user_balance(profile_it, _stateConfig.developer_wallet, quantity, msg);

    add_referrer_bonus(profile_it, base_quantity.amount);
}

void MyMillions::add_payment_balance(name user, asset quantity)
{
    eosio_assert(quantity.is_valid(), "Logic error: Invalid asset.");
    if(quantity.amount > 0)
    {
        auto profile_it = _userProfiles.find(user.value);
        _userProfiles.modify(profile_it, _self, [&](auto& profile)
        {
            profile.payment_balance += quantity;
        });
    }
}

void MyMillions::add_withdraw_balance(name user, asset quantity)
{
    eosio_assert(quantity.is_valid(), "Logic error: Invalid asset.");
    if(quantity.amount > 0)
    {
        auto profile_it = _userProfiles.find(user.value);
        _userProfiles.modify(profile_it, _self, [&](auto& profile)
        {
            profile.withdraw_balance += quantity;
        });
    }
}

uint64_t MyMillions::calculate_resource_count(const tables::Factory& factory, time_point now_collected)
{
    log("calculate_resource_count()\n");
    auto config_key = FactoryConfig::make_primary_key(factory.type, factory.level);
    auto it = _factoriesConfig.find(config_key);
    eosio_assert(it != _factoriesConfig.end(), "Config for factory not found.");
    /*
        Formula:
        (base_produce + produce_bonus ) * factory count * minutes from prev collecting
    */
    uint64_t count = (it->produce + it->produce_bonus) * factory.count *
        time_point(now_collected - factory.last_collected).sec_since_epoch() / 60; // minutes
    log("calculate_resource_count() = %\n", count);
    return count;
}

bool MyMillions::is_registered(name who)
{
    auto it = _userProfiles.find(who.value);
    return !(it == _userProfiles.end());
}

void MyMillions::print_profiles()
{
    log(_userProfiles);
}

void MyMillions::print_user_factories(name user)
{
    eosio_assert(is_registered(user), "User is not registered.");
    log("Print available factories for given user:\n");
    auto index = _factories.get_index<"index1"_n>();
    auto start = index.lower_bound(user.value);
    while(start != index.end())
    {
        log(*start);
        ++start;
    }
}

void MyMillions::buyWoodFactory(name user)
{
    log("buyWoodFactory(%)\n", user);
    eosio_assert(is_registered(user), "User is not registered.");
    buyFactory(user, ResourceType::WOOD);
}

void MyMillions::buyMetalFactory(name user)
{
    log("buyMetalFactory(%)\n", user);
    eosio_assert(is_registered(user), "User is not registered.");
    buyFactory(user, ResourceType::METAL);
}

void MyMillions::buyOilFactory(name user)
{
    log("buyOilFactory(%)\n", user);
    eosio_assert(is_registered(user), "User is not registered.");
    buyFactory(user, ResourceType::OIL);
}

void MyMillions::buyPreciousMetalFactory(name user)
{
    log("buyPreciousMetalFactory(%)\n", user);
    eosio_assert(is_registered(user), "User is not registered.");
    buyFactory(user, ResourceType::JEWEL);
}

void MyMillions::buyFactory(name user, uint8_t resource_type)
{
    log("buyFactory(%)\n", user);
    auto type = common::to_resource_type(resource_type);
    eosio_assert(ResourceType::WRONG_TYPE != type, "Wrong resource type.");
    eosio_assert(is_registered(user), "User is not registered.");

    pay_for_factory(user, type, Levels::LVL_1, 1);

    auto factory_key = tables::Factory::make_primary_key(user, type, Levels::LVL_1);
    auto it = _factories.find(factory_key);

    if(_factories.end() == it)
    {
        // add new
        _factories.emplace(user, [&](auto& f)
        {
            f.player_name = user;
            f.type = type;
            f.level = Levels::LVL_1;
            f.count = 1;
            f.last_collected = time_point(seconds(now()));
        });
    }
    else
    {
        // increase count of this type factory
        _factories.modify(it, user, [&](auto& f)
        {
            ++f.count;
        });
    }

    print_user_factories(user);
}

void MyMillions::add_factory(name user, ResourceType type, Levels level, uint32_t count)
{
    eosio_assert(is_registered(user), "User is not registered.");
    eosio_assert(ResourceType::WRONG_TYPE != type, "Wrong factory type.");
    eosio_assert(Levels::WRONG_LVL != level, "Wrong factory level.");
    auto factory_key = tables::Factory::make_primary_key(user, type, level);
    auto it = _factories.find(factory_key);
    if(_factories.end() == it)
    {
        _factories.emplace(user, [&](auto& f)
        {
            f.player_name = user;
            f.type = type;
            f.level = level;
            f.count = count;
            f.last_collected = time_point(seconds(now()));
        });
    }
    else
    {
        _factories.modify(it, user, [&](auto& f)
        {
            f.count += count;
        });
    }
}

void MyMillions::dec_factory(name user, ResourceType type, Levels level, uint32_t count)
{
    eosio_assert(is_registered(user), "User is not registered.");
    eosio_assert(ResourceType::WRONG_TYPE != type, "Wrong factory type.");
    eosio_assert(Levels::WRONG_LVL != level, "Wrong factory level.");
    auto factory_key = tables::Factory::make_primary_key(user, type, level);
    auto it = _factories.find(factory_key);

    eosio_assert(_factories.end() != it, "Account don't have factories of this level");
    eosio_assert(it->count >= count, "Not enough factories of this type");

    if(it->count == count)
    {
        _factories.erase(it);
    }
    else
    {
        _factories.modify(it, user, [&](auto& f)
        {
            f.count -= count;
        });
    }
}

void MyMillions::levelUp(name user, uint8_t resource_type, uint8_t factory_level, uint32_t count)
{
    log("levelUp(%)\n", user);
    auto type = to_resource_type(resource_type);
    auto level = to_level(factory_level);
    auto next_level = to_level(level + 1);
    eosio_assert(ResourceType::WRONG_TYPE != type, "Wrong resource type.");
    eosio_assert(Levels::WRONG_LVL != level, "Wrong level.");
    eosio_assert(level < Levels::LVL_7, "Impossible to do levelup for this level.");
    eosio_assert(is_registered(user), "User is not registered.");
    pay_for_factory(user, type, next_level, count);

    collectFactory(user, type);
    dec_factory(user, type, level, count);
    add_factory(user, type, next_level, count);

    print_user_factories(user);
}

bool MyMillions::check_referrer(uint64_t referrer_id)
{
    auto index = _userProfiles.get_index<"referrer"_n>();
    auto itr = index.find(referrer_id);
    return itr != index.end();
}

void MyMillions::add_referrer_bonus(tables::RegisteredAccounts::const_iterator& profile_it, uint64_t amount)
{
    log("add_referrer_bonus(%, %)\n", profile_it->player_name, amount);
    auto first = profile_it->referrer_id;
    if(0 == first)
        return;
    auto index = _userProfiles.get_index<"referrer"_n>();
    auto referrer_iter = index.find(first);
    if(index.end() == referrer_iter)
        return;
    auto bonus_1st = asset(amount * _stateConfig.referral_bonus_1st_lvl, DEFAULT_SYMBOL);
    auto bonus_2nd = asset(amount * _stateConfig.referral_bonus_2nd_lvl, DEFAULT_SYMBOL);
    log("try to add 1 referrer bonus % to %\n", bonus_1st, referrer_iter->player_name);
    add_payment_balance(referrer_iter->player_name, bonus_1st);
    auto second = referrer_iter->referrer_id;
    if(0 == second )
        return;
    referrer_iter = index.find(second);
    if(index.end() == referrer_iter)
        return;
    log("try to add 2 referrer bonus % to %\n", bonus_2nd, referrer_iter->player_name);
    add_payment_balance(referrer_iter->player_name, bonus_2nd);
}

void MyMillions::registerUserWithReferrer(name user, uint64_t referral_id)
{
    log("registerUserWithRefferal(%, %) started.\n", user, referral_id);
    eosio_assert(is_account(user), "account does not exist. ");
    require_auth(user);
    print_profiles();
    eosio_assert(!is_registered(user), "Account already registered. ");
    eosio_assert(check_referrer(referral_id), "Referral id not registered. ");
    auto ref = _userProfiles.emplace(user, [&](auto& profile)
    {
        profile.player_name = user;
        profile.created_at = time_point(seconds(now()));
        profile.referrer_id = referral_id;
        profile.wood = 0;
        profile.metal = 0;
        profile.jewel = 0;
        profile.oil = 0;
        profile.payment_balance = eosio::asset(0, DEFAULT_SYMBOL);
        profile.withdraw_balance = eosio::asset(0, DEFAULT_SYMBOL);
    });
    log("New registered user\n");
    log(*ref);

    log("registerUserWithRefferal(%) finished.\n", user);
}

void MyMillions::registerUser(name user)
{
    log("registerUser(%) started.\n", user);
    eosio_assert(is_account(user), "account does not exist. ");
    require_auth(user);
    print_profiles();
    eosio_assert(!is_registered(user), "Account already registered. ");
    auto ref = _userProfiles.emplace(user, [&](auto& profile)
    {
        profile.player_name = user;
        profile.created_at = time_point(seconds(now()));
        profile.referrer_id = 0;
        profile.wood = 0;
        profile.metal = 0;
        profile.jewel = 0;
        profile.oil = 0;
        profile.payment_balance = eosio::asset(0, DEFAULT_SYMBOL);
        profile.withdraw_balance = eosio::asset(0, DEFAULT_SYMBOL);
    });
    log("New registered user\n");
    log(*ref);

    log("registerUser(%) finished.\n", user);
}

void MyMillions::collectAll(name user)
{
    log("collectAll(%)\n", user);
    eosio_assert(is_registered(user), "User is not registered.");
    collectFactory(user, ResourceType::WOOD);
    collectFactory(user, ResourceType::METAL);
    collectFactory(user, ResourceType::OIL);
    collectFactory(user, ResourceType::JEWEL);
}

void MyMillions::collectFactory(name user, uint8_t resource_type)
{
    eosio_assert(is_registered(user), "User is not registered.");
    auto type = to_resource_type(resource_type);
    eosio_assert(ResourceType::WRONG_TYPE != type, "Wrong resource type.");

    //key = user + type
    auto factory_key = tables::Factory::make_index2_key(user, type);
    auto index = _factories.get_index<"index2"_n>();
    auto factory_it = index.lower_bound(factory_key);

    auto profile_it = _userProfiles.find(user.value);

    while(factory_it != index.end())
    {
        if(type == factory_it->type)
        {
            auto prev_collected = factory_it->last_collected.sec_since_epoch();
            auto now_collected = eosio::time_point(seconds(now()));
            auto count = calculate_resource_count(*factory_it, now_collected);
            log("Collected % of type %\n", (int)count, (int)resource_type);
            _userProfiles.modify(profile_it, user, [&](auto& f)
            {
                if(type == ResourceType::WOOD)
                {
                    f.wood += count;
                }
                else if(type == ResourceType::METAL)
                {
                    f.metal += count;
                }
                else if(type == ResourceType::OIL)
                {
                    f.oil += count;
                }
                else if(type == ResourceType::JEWEL)
                {
                   f.jewel += count;
                }
            });

            index.modify(factory_it, user, [&](auto& f)
            {
                f.last_collected = time_point(now_collected);
            });
        }
        ++factory_it;
    }
}

void MyMillions::sell(name user, uint8_t resource_type)
{
    log("sell(%, %)\n", user, (unsigned int)resource_type);
    require_auth(user);
    eosio_assert(is_registered(user), "User is not registered.");
    auto type = to_resource_type(resource_type);
    eosio_assert(ResourceType::WRONG_TYPE != type, "Wrong resource type.");

    auto it = _userProfiles.find(user.value);
    if(_userProfiles.end() != it)
    {
        _userProfiles.modify(it, user, [&](auto& f)
        {
            if(type == ResourceType::WOOD)
            {
                auto quantity = asset(DEFAULT_MULT * f.wood * _stateConfig.price_for_1000_wood / 1000, DEFAULT_SYMBOL);
                add_withdraw_balance(user, quantity);
                f.wood = 0;
            }
            else if(type == ResourceType::METAL)
            {
                auto quantity = asset(DEFAULT_MULT * f.metal * _stateConfig.price_for_1000_metal / 1000, DEFAULT_SYMBOL);
                add_withdraw_balance(user, quantity);
                f.metal = 0;
            }
            else if(type == ResourceType::OIL)
            {
                auto quantity = asset(DEFAULT_MULT * f.oil * _stateConfig.price_for_1000_oil / 1000, DEFAULT_SYMBOL);
                add_withdraw_balance(user, quantity);
                f.oil = 0;
            }
            else if(type == ResourceType::JEWEL)
            {
                auto quantity = asset(DEFAULT_MULT * f.jewel * _stateConfig.price_for_1000_jewel / 1000, DEFAULT_SYMBOL);
                add_withdraw_balance(user, quantity);
                f.jewel = 0;
            }
        });
    }
}

void MyMillions::sellAll(name user)
{
    log("sellAll(%)\n", user);
    sell(user, ResourceType::WOOD);
    sell(user, ResourceType::METAL);
    sell(user, ResourceType::OIL);
    sell(user, ResourceType::JEWEL);
}

MyMillionsDebug::MyMillionsDebug(name receiver, name code, eosio::datastream<const char*> ds)
        : MyMillions(receiver, code, ds)
{
    log("MyMillionsDebug Constructor started\n");
    log("MyMillionsDebug Constructor finished\n");
}

MyMillionsDebug::~MyMillionsDebug()
{
    log("MyMillionsDebug destructor started\n");
    log("MyMillionsDebug destructor finished\n");
}

void MyMillionsDebug::printConfig(name user)
{
    require_auth(user);
    eosio_assert(_stateConfig.admin == user || _stateConfig.owner == user || is_registered(user),
                 "Allowed only for registered user.");

    log(_stateConfig);
    log(_factoriesConfig);
}

void MyMillionsDebug::printFactoriesInfo(name user)
{
    log("printFactoriesInfo(%)\n", user);
    log("Print all factories for user %\n", user);
    printFactoryInfo(user, ResourceType::WOOD);
    printFactoryInfo(user, ResourceType::METAL);
    printFactoryInfo(user, ResourceType::OIL);
    printFactoryInfo(user, ResourceType::JEWEL);
}

void MyMillionsDebug::printFactoryInfo(name user, uint8_t resource_type)
{
    log("printFactoryInfo(%)\n", user);
    eosio_assert(is_registered(user), "User is not registered.");
    auto type = to_resource_type(resource_type);
    eosio_assert(ResourceType::WRONG_TYPE != type, "Wrong resource type.");
    auto index = _factories.get_index<"index1"_n>();
    auto start = index.lower_bound(user.value);
    log("Print all factories of type % for user %\n", (int)type, user);
    log("--------------------------\n");
    while(start != index.end())
    {
        if(type == start->type)
        {
            log(*start);
        }
        ++start;
    }
    log("--------------------------\n");
}

void MyMillionsDebug::printCollectedResources(name user)
{
    log("printCollectedResources(%)\n", user);
    printCollectedResource(user, ResourceType::WOOD);
    printCollectedResource(user, ResourceType::METAL);
    printCollectedResource(user, ResourceType::OIL);
    printCollectedResource(user, ResourceType::JEWEL);
}

void MyMillionsDebug::printCollectedWoods(name user)
{
    log("printCollectedWoods(%)\n", user);
    printCollectedResource(user, ResourceType::WOOD);
}

void MyMillionsDebug::printCollectedMetals(name user)
{
    log("printCollectedMetals(%)\n", user);
    printCollectedResource(user, ResourceType::METAL);
}

void MyMillionsDebug::printCollectedOils(name user)
{
    log("printCollectedOils(%)\n", user);
    printCollectedResource(user, ResourceType::OIL);
}

void MyMillionsDebug::printCollectedPreciousMetals(name user)
{
    log("printCollectedPreciousMetals(%)\n", user);
    printCollectedResource(user, ResourceType::JEWEL);
}

void MyMillionsDebug::printCollectedResource(name user, uint8_t resource_type)
{
    log("printCollectedResource(%)\n", user);
    eosio_assert(is_registered(user), "User is not registered.");
    auto type = to_resource_type(resource_type);
    eosio_assert(ResourceType::WRONG_TYPE != type, "Wrong resource type.");

    auto it = _userProfiles.find(user.value);
    if(_userProfiles.end() != it)
    {
        if(type == ResourceType::WOOD)
        {
            log("User % collected % WOOD\n", user, it->wood);
        }
        else if(type == ResourceType::METAL)
        {
            log("User % collected % METAL\n", user, it->metal);
        }
        else if(type == ResourceType::OIL)
        {
            log("User % collected % OIL\n", user, it->oil);
        }
        else if(type == ResourceType::JEWEL)
        {
            log("User % collected % JEWEL\n", user, it->jewel);
        }
    }
}


void MyMillionsDebug::printReferralId(name user)
{
    log("printReferralId(%)\n", user);
    auto it = _userProfiles.find(user.value);
    eosio_assert(it != _userProfiles.end(), "User not found.");
    log("User % has referral id '%'", user, it->created_at.sec_since_epoch());
}

void MyMillionsDebug::printAccountInfo(name user)
{
    log("printAccountInfo(%)\n", user);
    auto it = _userProfiles.find(user.value);
    eosio_assert(it != _userProfiles.end(), "User not found.");
    log(*it);
}

bool MyMillions::filter_incoming_transactions(const tables::TokenTransfer& transfer)
{
    if(transfer.from == _self
        || transfer.from == _stateConfig.owner
        || transfer.from == _stateConfig.admin
        || transfer.from == _stateConfig.main_wallet
        || transfer.from == _stateConfig.developer_wallet
        )
    {
        return false;
    }
    if(transfer.to != _stateConfig.owner)
    {
        return false;
    }
    auto& ref = transfer.quantity;
    if(!ref.is_valid() || ref.symbol != DEFAULT_SYMBOL || ref.amount == 0)
    {
        return false;
    }
    return true;
}

void MyMillions::on_transfer()
{
    log("MyMillions::on_transfer\n");
    eosio_assert(_code == "eosio.token"_n, "Stupid check failed.");
    auto data = unpack_action_data<tables::TokenTransfer>();
    if(!filter_incoming_transactions(data))
        // skip transactions
        return;
    if(!is_registered(data.from))
        return;

    if(data.to != _stateConfig.owner)
        return;

    auto profile_it = _userProfiles.find(data.from.value);
    _userProfiles.modify(profile_it, _self, [&](auto& profile)
    {
        profile.payment_balance += data.quantity;
    });
}

} /// namespace millions
