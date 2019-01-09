#pragma once
#include <mymillions/config.hpp>
#include <mymillions/tables.hpp>
#include <eosiolib/print.hpp>

#ifdef DEBUG_CONTRACT

template<class T, class ... TArgs>
void log(const T& var, const TArgs& ... args)
{
    eosio::print_f(var, args ...);
}

template<>
void log(const tables::RegisteredAccounts& profiles)
{
    log("User profiles:\n");
    for(auto item : profiles)
    {
        item.print();
    }
}

template<>
void log(const tables::Config& cfg)
{
    log("Current config");
    cfg.print();
}

template<>
void log(const tables::FactoriesConfig& factories)
{
    log("Current factories config\n");
    for(const auto& item : factories)
    {
        item.print();
    }
}

template<>
void log(const tables::Account& acc)
{
    log("Account info: \n");
    acc.print();
}

template<>
void log(const tables::FactoryConfig& cfg)
{
    cfg.print();
}

template<>
void log(const tables::Factory& data)
{
    data.print();
}

#else
template<class T, class ... TArgs>
void log(const T& var, const TArgs& ... args)
{
    //empty
}
#endif
