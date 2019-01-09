#pragma once
#include <eosiolib/name.hpp>  // type eosio::name
#include <eosiolib/multi_index.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/time.hpp>

namespace tables
{
using namespace eosio;

/*
 * struct to catch "eosio.token".transfer(...) action
 * */
struct TokenTransfer
{
    name from;
    name to;
    asset quantity;
    std::string memo;
};

/*
 * Table to store main config.
 */
struct [[eosio::table("main.cfg"), eosio::contract("mymillions")]] Config
{
    int  revision;                  // revision number of config
    name owner;                     // name of the account who called init on this contract
    name admin;                     // can change wallets

    name developer_wallet;          // account to collect developers fee
    name main_wallet;               // account to collect fee
    double developers_percent;      // default: 0.15 to developers account

    double referral_bonus_1st_lvl;  // default: 0.07
    double referral_bonus_2nd_lvl;  // default: 0.03

    double price_for_1000_wood;     // exchange: eos = (1000 WOOD * price_for_1000_wood)
    double price_for_1000_metal;    // exchange: eos = (1000 METAL * price_for_1000_metal)
    double price_for_1000_oil;      // exchange: eos = (1000 OIL * price_for_1000_oil)
    double price_for_1000_jewel;    // exchange: eos = (1000 JEWEL * price_for_1000_jewel)

    bool enable_withdraw;           // set false to disable withdraw operation

    void print() const
    {
            print_f("Config[rev='%';owner='%';admin='%';developer_wallet='%';main_wallet='%';" \
                    "developers_percent='%';referral_bonus_1st_lvl='%';referral_bonus_2nd_lvl='%';" \
                    "price_for_1000_wood='%';price_for_1000_metal='%';" \
                    "price_for_1000_oil='%';price_for_1000_jewel='%']\n",
                    revision, owner, admin, developer_wallet, main_wallet,
                    developers_percent, referral_bonus_1st_lvl, referral_bonus_2nd_lvl,
                    price_for_1000_wood, price_for_1000_metal, price_for_1000_oil, price_for_1000_jewel
            );
    }

    EOSLIB_SERIALIZE(Config,
        (revision)
        (owner)
        (admin)
        (developer_wallet)(main_wallet)(developers_percent)
        (referral_bonus_1st_lvl)(referral_bonus_2nd_lvl)
        (price_for_1000_wood)(price_for_1000_metal)(price_for_1000_oil)(price_for_1000_jewel)
        (enable_withdraw)
    )
};

/*
 * Table to store factory configuration.
 */
struct [[eosio::table("factorycfg"), eosio::contract("mymillions")]] FactoryConfig
{
    uint8_t type;                       // wood | metal | oil | jewel
    uint8_t level;                      // level
    uint64_t produce;                   // example: 100
    double produce_bonus;               // example: 5.3
    uint32_t price_for_next_lvl;        // example: 10 EOS

    uint64_t primary_key() const
    {
        return make_primary_key(type, level);
    }

    static uint64_t make_primary_key(uint8_t resource_type, uint8_t factory_level)
    {
        return (resource_type * 100) + factory_level;
    }

    void print() const
    {
        print_f("FactoryConfig[id='%';type='%';level='%';produce='%';price_for_next_lvl='%';produce_bonus='%']\n",
                primary_key(), (int)type, (int)level, produce, price_for_next_lvl, produce_bonus);
    }

    EOSLIB_SERIALIZE(FactoryConfig,
            (type)
            (level)
            (produce)
            (produce_bonus)(price_for_next_lvl)
    )
};

/*
 * Table to store information about factories.
 */
struct [[eosio::table("factory"), eosio::contract("mymillions")]] Factory
{
    eosio::name player_name;            // i.e.: bob / uint64_t
    uint8_t type;                       // wood | metal | oil | jewel
    uint8_t level;                      // level
    uint32_t count = 1;                 // count of this factory type
    eosio::time_point last_collected;   // time when factories was collected

    uint64_t primary_key()const
    {
        return make_primary_key(player_name, type, level);
    }

    uint64_t by_player_name()const
    {
        return player_name.value;
    }

    uint64_t by_player_and_resource()const
    {
        return make_index2_key(player_name, type);
    }

    static uint64_t make_primary_key(name user, uint8_t resource_type, uint8_t factory_level)
    {
        return  (user.value * 1000) + (100 * resource_type) + factory_level;
    }

    static uint64_t make_index2_key(name user, uint8_t resource_type)
    {
        return  (user.value * 1000) + (100 * resource_type);
    }

    void print() const
    {
        print_f("Factory[id='%';player_name='%';type='%';level='%';count='%';last_collected='%';]\n", primary_key(),
                player_name, (unsigned int)type, (unsigned int)level, count, last_collected.sec_since_epoch());
    }

    EOSLIB_SERIALIZE(Factory,
        (player_name)(type)(level)(count)(last_collected)
    )
};

/*
 * Table to store information about registered accounts.
 */
struct [[eosio::table, eosio::contract("mymillions")]] Account
{
    eosio::name player_name;            // i.e.: bob
    eosio::time_point created_at;       // time of registration, will use as referral id
    uint64_t referrer_id;
    //resources
    uint64_t wood;
    uint64_t metal;
    uint64_t oil;
    uint64_t jewel;
    //EOS
    asset payment_balance;
    asset withdraw_balance;

    uint64_t primary_key()const
    {
        return player_name.value;
    }

    uint64_t referrer_key()const
    {
        return player_name.value;
    }

    void print() const
    {
        print_f("Account[id='%';player_name='%';created_at='%';referrer_id='%';wood='%';metal='%';oil='%';jewel='%';payment_balance='%']\n",
                primary_key(), player_name, created_at.sec_since_epoch(), referrer_id, wood, metal, oil, jewel,
                payment_balance.to_string().c_str());
    }

    EOSLIB_SERIALIZE(Account,
        (player_name)(created_at)(referrer_id)
        (wood)(metal)(oil)(jewel)
        (payment_balance)(withdraw_balance)
    )
};

typedef eosio::singleton<"main.cfg"_n, Config> ContractConfig;

typedef eosio::multi_index<"factory.cfg"_n, FactoryConfig> FactoriesConfig;

typedef eosio::multi_index<"factories"_n, Factory,
            eosio::indexed_by<"index1"_n, eosio::const_mem_fun<Factory, uint64_t, &Factory::by_player_name>>,
            eosio::indexed_by<"index2"_n, eosio::const_mem_fun<Factory, uint64_t, &Factory::by_player_and_resource>>
        > Factories;

typedef eosio::multi_index<"accounts"_n, Account,
            eosio::indexed_by< "referrer"_n, eosio::const_mem_fun<Account, uint64_t, &Account::referrer_key>>
        > RegisteredAccounts;

}//namespace tables