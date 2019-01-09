#pragma once
#include <eosiolib/eosio.hpp>
#include <mymillions/common.hpp>
#include <mymillions/tables.hpp>

#define DEBUG_CONTRACT

namespace millions
{
namespace config
{
    using tables::FactoryConfig;
    using common::ResourceType;
    using common::Levels;

    constexpr uint32_t DEFAULT_MULT = 10000;
    constexpr eosio::symbol DEFAULT_SYMBOL = eosio::symbol("EOS", 4);

    constexpr FactoryConfig factories_config[] =
    {
        {ResourceType::WOOD, Levels::LVL_1, 2, 0, 1},
        {ResourceType::WOOD, Levels::LVL_2, 12, 1.2, 5},
        {ResourceType::WOOD, Levels::LVL_3, 36, 5.4, 12},
        {ResourceType::WOOD, Levels::LVL_4, 96, 24, 30},
        {ResourceType::WOOD, Levels::LVL_5, 276, 96.6, 90},
        {ResourceType::WOOD, Levels::LVL_6, 636, 254.4, 180},
        {ResourceType::WOOD, Levels::LVL_7, 1716, 858, 540},

        {ResourceType::METAL, Levels::LVL_1, 4, 0, 3},
        {ResourceType::METAL, Levels::LVL_2, 16, 1.38, 9},
        {ResourceType::METAL, Levels::LVL_3, 52, 8.66, 27},
        {ResourceType::METAL, Levels::LVL_4, 152, 39.8, 75},
        {ResourceType::METAL, Levels::LVL_5, 436, 155.68, 213},
        {ResourceType::METAL, Levels::LVL_6, 1144, 490.22, 531},
        {ResourceType::METAL, Levels::LVL_7, 2984, 1581.2, 1389},

        {ResourceType::OIL, Levels::LVL_1, 6, 0, 5},
        {ResourceType::OIL, Levels::LVL_2, 24, 1.64, 15},
        {ResourceType::OIL, Levels::LVL_3, 66, 10.5, 35},
        {ResourceType::OIL, Levels::LVL_4, 204, 50.99, 115},
        {ResourceType::OIL, Levels::LVL_5, 564, 179.43, 300},
        {ResourceType::OIL, Levels::LVL_6, 1440, 556.29, 730},
        {ResourceType::OIL, Levels::LVL_7, 4374, 2186.74, 2445},

        {ResourceType::JEWEL, Levels::LVL_1, 8, 0, 10},
        {ResourceType::JEWEL, Levels::LVL_2, 48, 4.18, 50},
        {ResourceType::JEWEL, Levels::LVL_3, 128, 16.7, 100},
        {ResourceType::JEWEL, Levels::LVL_4, 408, 115.31, 350},
        {ResourceType::JEWEL, Levels::LVL_5, 968, 315.67, 700},
        {ResourceType::JEWEL, Levels::LVL_6, 2336, 964.92, 1710},
        {ResourceType::JEWEL, Levels::LVL_7, 6776, 3535.45, 5550},
    };


    constexpr tables::Config get_default_config(eosio::name user)
    {
#ifdef DEBUG_CONTRACT
        return tables::Config
                {
                        1,          /*revision*/
                        user,       /*owner*/
                        user,       /*admin*/
                        user,       /*developer_wallet*/
                        user,       /*main_wallet*/
                        0.15,       /*developers_percent*/
                        0.07,       /*referral_bonus_1st_lvl*/
                        0.03,       /*referral_bonus_2nd_lvl*/
                        2.315,      /*price_for_1000_wood*/
                        3.646,      /*price_for_1000_metal*/
                        4.244,      /*price_for_1000_oil*/
                        6.655,      /*price_for_1000_jewel*/
                        true,       /*enable_withdraw*/
                };
#else
        return tables::Config
        {
                1,          /*revision*/
                user,       /*owner*/
                user,       /*admin*/
                user,       /*developer_wallet*/
                user,       /*main_wallet*/
                0.15,       /*developers_percent*/
                0.07,       /*referral_bonus_1st_lvl*/
                0.03,       /*referral_bonus_2nd_lvl*/
                0.002315,   /*price_for_1000_wood*/
                0.003646,   /*price_for_1000_metal*/
                0.004244,   /*price_for_1000_oil*/
                0.006655,   /*price_for_1000_jewel*/
                true,       /*enable_withdraw*/
        };
#endif
    }
}//namespace config
}//namespace millions