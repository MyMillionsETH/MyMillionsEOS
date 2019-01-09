#!/usr/bin/env bash
##############################################################################################
#  Use local wallet
#  name = jungle
#  password PW5K4QDdRJkC4vyaQ5yTJuX4V5ESJyD42kTYWKW5k7poSjLdD87z4
#  Accounts registered in jungle network:
#  1. bb2333bmopew
#     public  key "EOS5w7D9RF3dCKtbtmh8iwu7HQK2JMckX777oxkt7Zox4tvN58jn4"
#     private key "5JPbt8iT1UZ62tuLHshJhNFMej6Sb46MZMUS9vrrx3ju172Hqvr"
#  2. bb2334bmopew
#     public  key "EOS5eVc9zGT8jGrvDUaP4HTHEk3Hbz7ZPgefmGj9BwntQjE51Wtid"
#     private key "5JjJJCBGGnU2Ko9EyjqyTpkmxw5bHe9BmDfuuZQw8Cny9Rj99E9"
#
#
##############################################################################################
# Some commands:
#   ./cleos.sh wallet create -n jungle --to-console
#   yes 5JPbt8iT1UZ62tuLHshJhNFMej6Sb46MZMUS9vrrx3ju172Hqvr | ./cleos.sh wallet import -n jungle
#   yes 5JjJJCBGGnU2Ko9EyjqyTpkmxw5bHe9BmDfuuZQw8Cny9Rj99E9 | ./cleos.sh wallet import -n jungle
#
#   ./cleos.sh system buyram bb2334bmopew bb2334bmopew --kbytes 200000
#   ./cleos.sh create account eosio bb2333bmopew EOS5w7D9RF3dCKtbtmh8iwu7HQK2JMckX777oxkt7Zox4tvN58jn4
#   ./cleos.sh push action bb2334bmopew account.reg '["bb2333bmopew"]' -p bb2333bmopew@active
#   ./cleos.sh get table bb2334bmopew bb2334bmopew main.cfg
#   ./cleos.sh get table bb2334bmopew bb2334bmopew factories.cfg
#   ./cleos.sh get currency balance eosio.token bb2334bmopew EOS
#   ./cleos.sh push action eosio.token transfer '[ "bb2334bmopew", "bb2333bmopew", "181.0000 EOS", "" ]' -p bb2334bmopew@active
#   ./cleos.sh push action eosio.token transfer '[ "bb2333bmopew", "bb2334bmopew", "181.0000 EOS", "" ]' -p bb2333bmopew@active
#
##############################################################################################
wallet_host="127.0.0.1"
wallet_port="3000"

cleos --url http://jungle2.cryptolions.io:80 --wallet-url http://$wallet_host:$wallet_port $@