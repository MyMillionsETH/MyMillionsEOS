#pragma once
#include <eosiolib/contract.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/types.h>

#include <string>

#include <mymillions/config.hpp>
#include <mymillions/common.hpp>
#include <mymillions/tables.hpp>
#include "tables.hpp"

namespace millions
{
using eosio::name;
using eosio::symbol;
using eosio::asset;

/*
 * contract name should be equal to file name
 * otherwise you will receive empty abi file
 * */
class [[eosio::contract("mymillions")]] MyMillions: public eosio::contract
{
protected:
    tables::Config _stateConfig;
    tables::ContractConfig _globalConfig;
    tables::FactoriesConfig _factoriesConfig;
    tables::RegisteredAccounts _userProfiles;
    tables::Factories _factories;

    void print_user_factories(name user);
    void print_profiles();

    void pay_for_factory(name user, common::ResourceType resource_type, common::Levels factory_level, uint32_t count);
    void pay_from_user_balance(tables::RegisteredAccounts::const_iterator& profile_it, name to, asset quantity,
            const std::string& msg);
    void add_referrer_bonus(tables::RegisteredAccounts::const_iterator& profile_it, uint64_t amount);

    void add_payment_balance(name user, asset quantity);
    void add_withdraw_balance(name user, asset quantity);

    bool is_registered(name who);
    bool check_referrer(uint64_t referrer_id);

    uint64_t calculate_resource_count(const tables::Factory& factory, eosio::time_point now_collected);
    void add_factory(name user, common::ResourceType resource_type, common::Levels factory_level, uint32_t count);
    void dec_factory(name user, common::ResourceType type, common::Levels level, uint32_t count);

    bool filter_incoming_transactions(const tables::TokenTransfer& transfer);
public:

    MyMillions(name receiver, name code, eosio::datastream<const char*> ds);
    ~MyMillions();

    [[eosio::action("admin.set")]] void setAdmin(name caller, name admin);
    [[eosio::action("wallets.set")]] void setWallets(name caller, name main_wallet, name dev_wallet);

    // Paid functions
    [[eosio::action("woodf.buy")]] void buyWoodFactory(name user);
    [[eosio::action("metalf.buy")]] void buyMetalFactory(name user);
    [[eosio::action("oilf.buy")]] void buyOilFactory(name user);
    [[eosio::action("jewelf.buy")]] void buyPreciousMetalFactory(name user);
    [[eosio::action("factory.buy")]] void buyFactory(name user, uint8_t resource_type);
    [[eosio::action("level.up")]] void levelUp(name user, uint8_t resource_type, uint8_t level, uint32_t count);

    // Costless functions
    [[eosio::action("account.regr")]] void registerUserWithReferrer(name user, uint64_t referral_id);
    [[eosio::action("account.reg")]] void registerUser(name user);
    [[eosio::action("collect.all")]] void collectAll(name user);
    [[eosio::action("collect.one")]] void collectFactory(name user, uint8_t resource_type);
    [[eosio::action("sell")]] void sell(name user, uint8_t resource_type);
    [[eosio::action("sell.all")]] void sellAll(name user);

    //catched events
    void on_transfer();

    //withdraw money
    [[eosio::action("withdraw")]] void withdraw(name user, double value);
    [[eosio::action("withdraw.set")]] void enableWithdraw(name user, bool value);
};

/*
 * Derived contract for debug purposes
 * */
class [[eosio::contract("mymillions")]] MyMillionsDebug: public MyMillions
{
protected:

public:
    MyMillionsDebug(name receiver, name code, eosio::datastream<const char*> ds);
    ~MyMillionsDebug();
    // Public readonly functions
    [[eosio::action("printcfg")]] void printConfig( name user );
    [[eosio::action("woods")]] void printCollectedWoods(name user);
    [[eosio::action("metals")]] void printCollectedMetals(name user);
    [[eosio::action("oils")]] void printCollectedOils(name user);
    [[eosio::action("jewels")]] void printCollectedPreciousMetals(name user);
    [[eosio::action("resource")]] void printCollectedResource(name user, uint8_t resource_type);
    [[eosio::action("resources")]] void printCollectedResources(name user);
    [[eosio::action("factories")]] void printFactoriesInfo(name user);
    [[eosio::action("factory")]] void printFactoryInfo(name user, uint8_t resource_type);
    [[eosio::action("referral.id")]] void printReferralId(name user);
    [[eosio::action("account")]] void printAccountInfo(name user);
};

} /// namespace millions


extern "C"
void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
#ifdef DEBUG_CONTRACT
    DISPATCH_ME(millions::MyMillionsDebug::printConfig, printcfg)
    DISPATCH_ME(millions::MyMillionsDebug::printFactoriesInfo, factories)
    DISPATCH_ME(millions::MyMillionsDebug::printFactoryInfo, factory)
    DISPATCH_ME(millions::MyMillionsDebug::printReferralId, referralid)
    DISPATCH_ME(millions::MyMillionsDebug::printAccountInfo, account)
    DISPATCH_ME(millions::MyMillionsDebug::printCollectedResources, resources)
    DISPATCH_ME(millions::MyMillionsDebug::printCollectedWoods, woods)
    DISPATCH_ME(millions::MyMillionsDebug::printCollectedMetals, metals)
    DISPATCH_ME(millions::MyMillionsDebug::printCollectedOils, oils)
    DISPATCH_ME(millions::MyMillionsDebug::printCollectedPreciousMetals, jewels)
    DISPATCH_ME(millions::MyMillionsDebug::printCollectedResource, resource)
#endif
    DISPATCH_ME(millions::MyMillions::setAdmin, admin.set)
    DISPATCH_ME(millions::MyMillions::setWallets, wallets.set)
    DISPATCH_ME(millions::MyMillions::buyWoodFactory, woodf.buy)
    DISPATCH_ME(millions::MyMillions::buyMetalFactory, metalf.buy)
    DISPATCH_ME(millions::MyMillions::buyOilFactory, oilf.buy)
    DISPATCH_ME(millions::MyMillions::buyPreciousMetalFactory, jewelf.buy)
    DISPATCH_ME(millions::MyMillions::buyFactory, factory.buy)
    DISPATCH_ME(millions::MyMillions::levelUp, level.up)
    DISPATCH_ME(millions::MyMillions::registerUser, account.reg)
    DISPATCH_ME(millions::MyMillions::registerUserWithReferrer, account.regr)
    DISPATCH_ME(millions::MyMillions::collectFactory, collect.one)
    DISPATCH_ME(millions::MyMillions::collectAll, collect.all)
    DISPATCH_ME(millions::MyMillions::sell, sell)
    DISPATCH_ME(millions::MyMillions::sellAll, sell.all)
    DISPATCH_ME(millions::MyMillions::withdraw, withdraw)
    DISPATCH_ME(millions::MyMillions::enableWithdraw, withdraw.set)

    DISPATCH_EXTERNAL(eosio.token, transfer, millions::MyMillions::on_transfer, ontransfer)
}

