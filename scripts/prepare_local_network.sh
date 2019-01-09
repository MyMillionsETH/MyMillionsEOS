#!/bin/bash 

printf "\t=========== Create accounts ===========\n\n"

yes PW5JeGEGHdjVhz8ySRKAf5TViZDfQ1AvgeUm69oY5GXnf311g1sKb | cleos wallet unlock --name svan
cleos create account eosio eosio.token EOS6AxTkLTc7T8gCiW8VTtCcZRJkFDz9yuMEGhRTffbP2fNWSs6vX
cleos create account eosio contractor EOS6AxTkLTc7T8gCiW8VTtCcZRJkFDz9yuMEGhRTffbP2fNWSs6vX
cleos create account eosio acc1 EOS6AxTkLTc7T8gCiW8VTtCcZRJkFDz9yuMEGhRTffbP2fNWSs6vX
cleos create account eosio acc2 EOS6AxTkLTc7T8gCiW8VTtCcZRJkFDz9yuMEGhRTffbP2fNWSs6vX
cleos create account eosio acc3 EOS6AxTkLTc7T8gCiW8VTtCcZRJkFDz9yuMEGhRTffbP2fNWSs6vX
cleos create account eosio acc4 EOS6AxTkLTc7T8gCiW8VTtCcZRJkFDz9yuMEGhRTffbP2fNWSs6vX
cleos create account eosio acc5 EOS6AxTkLTc7T8gCiW8VTtCcZRJkFDz9yuMEGhRTffbP2fNWSs6vX
cleos set account permission contractor active '{"threshold" : 1, "keys" : [{"key": "EOS6AxTkLTc7T8gCiW8VTtCcZRJkFDz9yuMEGhRTffbP2fNWSs6vX", "weight": 1}], "accounts" : [{"permission":{"actor":"contractor","permission":"eosio.code"},"weight":1}], "waits":[]}}' owner -p contractor@owner

cleos set contract eosio.token ~/eos/eos/build/contracts/eosio.token/ -p eosio.token@active
cleos push action eosio.token create '[ "eosio", "1000000000.0000 EOS"]' -p eosio.token@active
cleos push action eosio.token issue '[ "contractor", "10000000.0000 EOS", "memo" ]' -p eosio@active
cleos push action eosio.token transfer '[ "contractor", "acc1", "10000.0000 EOS", "memo" ]' -p contractor@active
cleos push action eosio.token transfer '[ "contractor", "acc2", "10000.0000 EOS", "memo" ]' -p contractor@active
cleos push action eosio.token transfer '[ "contractor", "acc3", "10000.0000 EOS", "memo" ]' -p contractor@active
cleos get currency balance eosio.token contractor EOS
cleos get currency balance eosio.token acc1 EOS

cleos set contract contractor ~/eos/my-millions/scripts/build/output/mymillions -p contractor@active
cleos push action contractor account.reg '["acc1"]' -p acc1@active
cleos push action contractor account.reg '["acc2"]' -p acc2@active
