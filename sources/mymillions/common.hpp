#pragma once
#include <eosiolib/eosio.hpp>

namespace millions
{
namespace common
{
    constexpr auto one_day_in_seconds = 24 * 60 * 60;
    constexpr auto one_week_in_seconds = one_day_in_seconds * 7;
    constexpr auto one_month_in_seconds = one_day_in_seconds * 30;
    constexpr auto one_year_in_seconds = one_day_in_seconds * 365;

    enum ResourceType: uint8_t
    {
        WRONG_TYPE  = 0,
        WOOD        = 1,
        METAL       = 2,
        OIL         = 3,
        JEWEL       = 4
    };

    enum Levels: uint8_t
    {
        WRONG_LVL   = 0,
        LVL_1       = 1,
        LVL_2       = 2,
        LVL_3       = 3,
        LVL_4       = 4,
        LVL_5       = 5,
        LVL_6       = 6,
        LVL_7       = 7,
    };

    ResourceType to_resource_type(uint32_t value)
    {
        if (ResourceType::WOOD == value)return ResourceType::WOOD;
        if (ResourceType::METAL == value)return ResourceType::METAL;
        if (ResourceType::OIL == value)return ResourceType::OIL;
        if (ResourceType::JEWEL == value)return ResourceType::JEWEL;
        return ResourceType::WRONG_TYPE;
    }

    Levels to_level(uint32_t value)
    {
        if (Levels::LVL_1 == value)return Levels::LVL_1;
        if (Levels::LVL_2 == value)return Levels::LVL_2;
        if (Levels::LVL_3 == value)return Levels::LVL_3;
        if (Levels::LVL_4 == value)return Levels::LVL_4;
        if (Levels::LVL_5 == value)return Levels::LVL_5;
        if (Levels::LVL_6 == value)return Levels::LVL_6;
        if (Levels::LVL_7 == value)return Levels::LVL_7;
        return Levels::WRONG_LVL;
    }

#define DISPATCH_ME(MEMBER, NAME) \
if(code == receiver) \
{ \
    if (eosio::name(BOOST_PP_STRINGIZE(NAME)).value == action) \
    { \
        eosio::execute_action(eosio::name(receiver), eosio::name(code), &MEMBER); \
        return; \
    } \
}

#define DISPATCH_EXTERNAL(OTHER_CONTRACT, OTHER_ACTION, MEMBER, NAME) \
    if((code == eosio::name(BOOST_PP_STRINGIZE(OTHER_CONTRACT)).value) && (action == eosio::name(BOOST_PP_STRINGIZE(OTHER_ACTION)).value)) \
    { \
        eosio::execute_action(eosio::name(receiver), eosio::name(code), &MEMBER); \
        return; \
    }


}//namespace config
}//namespace millions