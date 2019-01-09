#MyMillions EOS smart contract. 

##Necessary documentation
* https://developers.eos.io/eosio-home/docs/introduction
* https://developers.eos.io/eosio-home/docs/your-first-contract
* https://github.com/tokenika/eosfactory/blob/master/docs/tutorials/01.InstallingEOSFactory.md

##Dependencies
1. https://github.com/EOSIO/eos.git 
2. https://github.com/EOSIO/eosio.cdt.git
3. https://github.com/tokenika/eosfactory.git

##Build
1. Install all dependencies. 
2. ```cd scripts```
3. ```./build.sh```

##Deploy of a contract
Replace \<owner\> with valid account.

 * deploy contract ```cleos set contract <owner> ./build/output/mymillions -p <owner>@active```
 * give permissions ```cleos set account permission <owner> active --add-code```
  

##Main Actions

### admin.set(name caller, name admin)

### Parameters
* `caller` - User who want to call this action
* `admin` - New admin account

### wallets.set(name caller, name main_wallet, name dev_wallet)
```cleos push action <owner> wallets.set '["<owner_or_admin>", "<main>", "<dev>"]' -p <owner_or_admin>@active```
#### Parameters
* `caller` - User who want to call this action
* `main_wallet` - Account to store main fee 85%
* `dev_wallet` - Account to store dev fee 15%

### account.regr(name user, uint64_t referral_id)
```cleos push action <owner> account.regr '["<player_name>", 123456]' -p <player_name>@active```
#### Parameters
* `user` - User who want to call this action
* `referral_id` - Referral id of another user, who will receive referral bonus.

### account.reg(name user)
```cleos push action <owner> account.reg '["<player_name>"]' -p <player_name>@active```
#### Parameters
* `user` - User who want to call this action

### factory.buy(name user, uint8_t resource_type)
```cleos push action <owner> factory.buy '["<player_name>", 1]' -p <player_name>@active```
#### Parameters
* `user` - User who want to call this action
* `resource_type` - one of the values 1 2 3 4

### level.up(name user, uint8_t resource_type, uint8_t level, uint32_t count)
```cleos push action <owner> level.up '["<player_name>", 1, 1, 1]' -p <player_name>@active```
#### Parameters
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4
* `level` - level of factory which you want to level up (use one of values 1 2 3 4 5 6 7)
* `count` - How many factories you want to level up.

### collect.all(name user)
```cleos push action <owner> collect.all '["<player_name>"]' -p <player_name>@active```
#### Parameters
Input parameters:
* `user` - User who want to call this action

### collect.one(name user, uint8_t resource_type)```
```cleos push action <owner> collect.one '["<player_name>", 1]' -p <player_name>@active```
#### Parameters
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

### sell(name user, uint8_t resource_type)
```cleos push action <owner> sell '["<player_name>", 1]' -p <player_name>@active```
#### Parameters
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

### sell.all(name user)
```cleos push action <owner> sell '["<player_name>"]' -p <player_name>@active```
#### Parameters
* `user` - User who want to call this action

### collect.one(name user, uint8_t resource_type)
```cleos push action <owner> collect.one '["<player_name>", 1]' -p <player_name>@active```
#### Parameters
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

### woodf.buy(name user)

#### Parameters
* `user` - User who want to call this action

### metalf.buy(name user)

#### Parameters
* `user` - User who want to call this action

### oilf.buy(name user)

#### Parameters
* `user` - User who want to call this action

###jewelf.buy(name user)

#### Parameters
* `user` - User who want to call this action



## Debug Actions

### printcfg(name user)

#### Parameters
* `user` - User who want to call this action

### woods(name user)

#### Parameters
* `user` - User who want to call this action

### metals(name user)

#### Parameters
* `user` - User who want to call this action

### oils(name user)

#### Parameters
* `user` - User who want to call this action

### jewels(name user)

#### Parameters
* `user` - User who want to call this action

### resources(name user)

### Parameters
* `user` - User who want to call this action

### factories(name user)

#### Parameters
* `user` - User who want to call this action

### factories(name user, uint8_t resource_type)

#### Parameters
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

### referral.id(name user)```

#### Parameters
* `user` - User who want to call this action

###resource(name user, uint8_t resource_type)

#### Parameters
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

### account(name user)

#### Parameters
* `user` - User who want to call this action

###withdraw(name user, double value)

#### Parameters
* `user` - User who want to call this action
* `value` - Amount of EOS to withdraw

### withdraw.set(name user, bool value)

#### Parameters
* `user` - User who want to call this action
* `value` - True | False. False disable any withdraw requests.  


## How to run unittest? 
Build contract first.
1. ```cd sources/tests/eosfactory/```
2. ```python3 mymillions.py --etoken <path to eosio.token contract> --mymillions <path to mymillions compiled contract>```  
It could be something like this
```
python3 mymillions.py --etoken="<path to eos repo>/build/contracts/eosio.token/" --mymillions="<path to my-millions repo>/scripts/build/output/mymillions/"
```
