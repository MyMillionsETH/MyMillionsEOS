<h1 class="contract">
   printcfg
</h1>

## Virtual signature
```printcfg(name user)```

### Parameters
* `user` - User who want to call this action

<h1 class="contract">
    woodf.buy
</h1>

## Virtual signature
```woodf.buy(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract">
    metalf.buy
</h1>

## Virtual signature
```metalf.buy(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    oilf.buy 
</h1>

## Virtual signature
```oilf.buy(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract">
    jewelf.buy 
</h1>

## Virtual signature
```jewelf.buy(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    factory.buy 
</h1>

## Virtual signature
```factory.buy(name user, uint8_t resource_type)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `resource_type` - one of the values 1 2 3 4

<h1 class="contract"> 
    level.up 
</h1>

## Virtual signature
```level.up(name user, uint8_t resource_type, uint8_t level, uint32_t count)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4
* `level` - level of factory which you want to level up (use one of values 1 2 3 4 5 6 7)
* `count` - How many factories you want to level up.

<h1 class="contract"> 
    account.regr
</h1>

## Virtual signature
```account.regr(name user, uint64_t referral_id)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `referral_id` - Referral id of another user, who will receive referral bonus.

<h1 class="contract"> 
    account.reg 
</h1>

## Virtual signature
```account.regr(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    collect.all 
</h1>

## Virtual signature
```collect.all(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    collect.one 
</h1>

## Virtual signature
```collect.one(name user, uint8_t resource_type)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

<h1 class="contract"> 
    sell
</h1>

## Virtual signature
```sell(name user, uint8_t resource_type)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

<h1 class="contract"> 
    sell.all 
</h1>

## Virtual signature
```sell.all(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    collect.one
</h1>

## Virtual signature
```collect.one(name user, uint8_t resource_type)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

<h1 class="contract"> 
    woods    
</h1>

## Virtual signature
```woods(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    metals 
</h1>

## Virtual signature
```metals(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    oils 
</h1>

## Virtual signature
```oils(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    jewels 
</h1>

## Virtual signature
```jewels(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    resources 
</h1>

## Virtual signature
```resources(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    factories 
</h1>

## Virtual signature
```factories(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    factory 
</h1>

## Virtual signature
```factories(name user, uint8_t resource_type)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

<h1 class="contract"> 
    referral.id 
</h1>

## Virtual signature
```referral.id(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    admin.set
</h1>

## Virtual signature
```admin.set(name caller, name admin)```
* `caller` - User who want to call this action
* `admin` - New admin account

### Parameters
Input parameters:

<h1 class="contract"> 
    wallets.set
</h1>

## Virtual signature
```wallets.set(name caller, name main_wallet, name dev_wallet)```

### Parameters
Input parameters:
* `caller` - User who want to call this action
* `main_wallet` - Account to store main fee 85%
* `dev_wallet` - Account to store dev fee 15%

<h1 class="contract"> 
    resource
</h1>

## Virtual signature
```resource(name user, uint8_t resource_type)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `resource_type` - use one of values 1 2 3 4

<h1 class="contract"> 
    account
</h1>

## Virtual signature
```account(name user)```

### Parameters
Input parameters:
* `user` - User who want to call this action

<h1 class="contract"> 
    withdraw
</h1>

## Virtual signature
```withdraw(name user, double value)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `value` - Amount of EOS to withdraw

<h1 class="contract"> 
    withdraw.set
</h1>

## Virtual signature
```withdraw.set(name user, bool value)```

### Parameters
Input parameters:
* `user` - User who want to call this action
* `value` - True | False. False disable any withdraw requests. 