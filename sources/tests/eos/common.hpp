#pragma once
#include <fstream>

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>

#include <eosio/chain/name.hpp>

#include "Runtime/Runtime.h"

#include <fc/variant_object.hpp>

using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

#ifndef TESTER
    #ifdef NON_VALIDATING_TEST
        #define TESTER tester
    #else
        #define TESTER validating_tester
    #endif
#endif
