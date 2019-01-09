import datetime
import unittest
import argparse
import os
from unittest import TestLoader
from time import sleep
from eosfactory.eosf import *
from eosfactory.core import setup

# UNCOMMENT to see commands
# setup.is_print_command_line = True


verbosity([Verbosity.INFO, Verbosity.OUT, Verbosity.TRACE, Verbosity.DEBUG])


def gt(dt_str):
    return datetime.datetime.strptime(dt_str, "%Y-%m-%dT%H:%M:%S.%f").replace(
        tzinfo=datetime.timezone.utc)


def filter_by_value(contract, table, key, value):
    tbl = contract.table(table, contract).json["rows"]
    for row in tbl:
        if row[key] == value:
            yield row


def name_to_value(s):
    value = 0
    n = min(len(s), 12)
    for i in range(n):
        value <<= 5
        value |= char_to_value(s[i])
    value <<= (4 + 5 * (12 - n))
    if len(s) == 13:
        v = char_to_value(s[12])
        if v > int(0x0F):
            raise RuntimeError(false, "thirteenth character in name cannot be a letter that comes after j")
        value |= v
    return value


def char_to_value(c):
    c = ord(c)
    if c == '.':
        return 0
    elif ord('5') >= c >= ord('1'):
        return (c - ord('1')) + 1
    elif ord('z') >= c >= ord('a'):
        return (c - ord('a')) + 6
    else:
        raise RuntimeError("wrong character")


def to_ref_id(name, time):
    return name_to_value(name)
    # return acc << 32 | int(gt(time).timestamp())


def withdraw_balance(acc):
    profiles = list(filter_by_value(contractor, "accounts", "player_name", acc.name))
    result = profiles[0]["withdraw_balance"]
    result_float = float(result.split(" ")[0])
    return result, result_float


def float_to_balance(balance):
    import decimal
    myothercontext = decimal.Context(rounding=decimal.ROUND_DOWN)
    decimal.setcontext(myothercontext)
    return round(decimal.Decimal(balance), 4)


class ContractTest(unittest.TestCase):
    """
        Contract must be compiled before you will run this tests
    """
    etoken_dir = "/home/user/eos/eos/build/contracts/eosio.token/"
    mymillions_dir = "/home/user/eos/my-millions/scripts/build/output/mymillions/"

    def run(self, result=None):
        super().run(result)

    @classmethod
    def setUpEosioToken(self):
        create_account("etoken", master, account_name="eosio.token")
        abi_file = "eosio.token.abi"
        wasm_file = "eosio.token.wasm"

        self.assertTrue(os.path.isfile(os.path.join(ContractTest.etoken_dir, abi_file)), f"{abi_file} not found")
        self.assertTrue(os.path.isfile(os.path.join(ContractTest.etoken_dir, wasm_file)), f"{wasm_file} not found")

        COMMENT('''Create, build and deploy eosio.token contract''')
        contract = Contract(etoken, ContractTest.etoken_dir, abi_file=abi_file, wasm_file=wasm_file)
        contract.deploy()

        COMMENT('''Create tokens''')
        etoken.push_action(
            "create",
            {
                "issuer": etoken,
                "maximum_supply": "1000000000.0000 EOS",
                "can_freeze": "0",
                "can_recall": "0",
                "can_whitelist": "0"
            },
            permission=[(etoken, Permission.ACTIVE)])
        etoken.push_action("issue", {"to": contractor, "quantity": "10000000.0000 EOS", "memo": "issue"},
                           permission=(etoken, Permission.ACTIVE))
        etoken.push_action(
            "transfer",
            {
                "from": contractor, "to": unregistered, "quantity": "1000.0000 EOS", "memo": "transfer"
            },
            permission=(contractor, Permission.ACTIVE),  # json=True
        )

    @classmethod
    def setUpMyMillions(self):
        abi_file = "mymillions.abi"
        wasm_file = "mymillions.wasm"
        self.assertTrue(os.path.isfile(os.path.join(ContractTest.mymillions_dir, abi_file)), f"{abi_file} not found")
        self.assertTrue(os.path.isfile(os.path.join(ContractTest.mymillions_dir, wasm_file)), f"{wasm_file} not found")
        COMMENT('''deploy mymillions contract''')
        contract = Contract(contractor, ContractTest.mymillions_dir, abi_file=abi_file, wasm_file=wasm_file)
        contract.deploy()

    @classmethod
    def setUpPermissions(self):
        from eosfactory.core.account_set import set_account_permission
        p = interface.key_arg(master, is_owner_key=True, is_private_key=False)
        x = f"""{{"threshold" : 1, "keys" : [{{"key": "{p}", "weight": 1}}], "accounts" : [{{"permission":{{"actor":"
                       {contractor.name}","permission":"eosio.code"}},"weight":1}}], "waits":[]}}}}"""
        set_account_permission(contractor,
                               "active",
                               x,
                               "owner", contractor.name + "@owner")

    @classmethod
    def setUpClass(cls):
        SCENARIO('''
                    Create a contract from template, then build and deploy it.
                    Also, initialize the token and run a couple of transfers between different accounts.
                ''')
        reset()
        create_master_account("master")

        COMMENT('''Create test accounts:''')
        create_account("contractor", master)
        create_account("unregistered", master)
        create_account("admin", master)
        create_account("main", master)
        create_account("dev", master)

        cls.setUpPermissions()

        cls.setUpEosioToken()

        cls.setUpMyMillions()

    def tearDown(self):
        pass

    @classmethod
    def tearDownClass(cls):
        stop()

    def test_configs(self):
        create_account("player333", master)

        # call any action to init configs
        contractor.push_action(
            "account.reg",
            {
                "user": player333,
            },
            permission=[(player333, Permission.ACTIVE)]
        )
        tbl_сonfig = contractor.table("main.cfg", contractor)
        tbl_factories_сonfig = contractor.table("factory.cfg", contractor)

        self.assertEqual(10, len(tbl_factories_сonfig.json["rows"]), '''Not enough configs''')
        cfg = tbl_сonfig.json["rows"][0]
        self.assertEqual(bool(cfg["enable_withdraw"]), True, '''enable_withdraw must be enabled by default''')
        self.assertEqual(cfg["owner"], contractor.name, '''config.owner != contract publisher''')
        self.assertEqual(cfg["admin"], contractor.name, '''config.admin != contract publisher ''')
        self.assertEqual(cfg["developer_wallet"], contractor.name, '''config.developer_wallet != contract publisher''')
        self.assertEqual(cfg["main_wallet"], contractor.name, '''config.main_wallet != contract publisher ''')

        self.assertEqual(float(cfg["developers_percent"]), 0.15, '''wrong config value''')
        self.assertEqual(float(cfg["referral_bonus_1st_lvl"]), 0.07, '''wrong config value''')
        self.assertEqual(float(cfg["referral_bonus_2nd_lvl"]), 0.03, '''wrong config value''')
        self.assertEqual(float(cfg["price_for_1000_wood"]), 2.315, '''wrong config value''')
        self.assertEqual(float(cfg["price_for_1000_metal"]), 3.646, '''wrong config value''')
        self.assertEqual(float(cfg["price_for_1000_oil"]), 4.244, '''wrong config value''')
        self.assertEqual(float(cfg["price_for_1000_jewel"]), 6.655, '''wrong config value''')

    def test_set_admin(self):
        COMMENT('''Test action 'admin.set':''')
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(contractor.name, cfg["owner"], 'wrong owner name')
        self.assertEqual(contractor.name, cfg["admin"], 'admin must be equal owner')
        action = contractor.push_action
        # 1. wrong caller
        self.assertRaises(Error, action, "admin.set", {"caller": player401, "admin": admin},
                          permission=[(player401, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(contractor.name, cfg["admin"], 'admin must be equal owner')
        # 2. wrong permissions
        self.assertRaises(Error, action, "admin.set", {"caller": contractor, "admin": admin},
                          permission=[(player401, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(contractor.name, cfg["admin"], 'admin must be equal owner')
        # 3. valid call
        action("admin.set", {"caller": contractor, "admin": admin}, permission=[(contractor, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(admin.name, cfg["admin"], 'wrong admin')
        # it is necessary to avoid duplicate transaction error
        sleep(1)
        # 4. repeat call
        action("admin.set", {"caller": contractor, "admin": admin}, permission=[(contractor, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(admin.name, cfg["admin"], 'wrong admin')
        # 5. return admin rights back
        action("admin.set", {"caller": admin, "admin": contractor}, permission=[(admin, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(contractor.name, cfg["admin"], 'wrong admin')

    def test_set_wallets(self):
        COMMENT('''Test action 'wallets.set':''')
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(contractor.name, cfg["owner"], 'wrong owner name')
        self.assertEqual(contractor.name, cfg["admin"], 'admin must be equal owner')
        action = contractor.push_action
        # 1. wrong caller
        self.assertRaises(Error, action, "wallets.set",
                          {"caller": player401, "main_wallet": admin, "dev_wallet": admin},
                          permission=[(contractor, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(contractor.name, cfg["main_wallet"], 'admin != owner')
        self.assertEqual(contractor.name, cfg["developer_wallet"], 'admin != owner')
        # 2. wrong permissions
        self.assertRaises(Error, action, "wallets.set",
                          {"caller": contractor, "main_wallet": admin, "dev_wallet": admin},
                          permission=[(player401, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(contractor.name, cfg["main_wallet"], 'wrong main wallet')
        self.assertEqual(contractor.name, cfg["developer_wallet"], 'wrong dev wallet')
        # 3. owner call wallets.set
        action("wallets.set", {"caller": contractor, "main_wallet": main, "dev_wallet": dev},
               permission=[(contractor, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(main.name, cfg["main_wallet"], 'wrong main wallet')
        self.assertEqual(dev.name, cfg["developer_wallet"], 'wrong dev wallet')
        # 3. admin call wallets.set
        # sleep is necessary to avoid error about duplicate transaction
        sleep(0.5)
        action("admin.set", {"caller": contractor, "admin": admin}, permission=[(contractor, Permission.ACTIVE)])
        action("wallets.set", {"caller": admin, "main_wallet": contractor, "dev_wallet": contractor},
               permission=[(admin, Permission.ACTIVE)])
        cfg = contractor.table("main.cfg", contractor).json["rows"][0]
        self.assertEqual(contractor.name, cfg["main_wallet"], 'wrong main wallet')
        self.assertEqual(contractor.name, cfg["developer_wallet"], 'wrong dev wallet')
        action("admin.set", {"caller": admin, "admin": contractor}, permission=[(admin, Permission.ACTIVE)])

    def test_account_register(self):
        COMMENT('''Test action 'account.reg':''')
        create_account("player444", master)

        # 1. register wrong user
        self.assertRaises(Error, contractor.push_action,
                          "account.reg",
                          {
                              "user": "unknown",
                          },
                          permission=[("unknown", Permission.ACTIVE)]
                          )
        accounts = list(filter_by_value(contractor, "accounts", "player_name", player444.name))
        self.assertEqual(0, len(accounts), "logic error")

        # 2. register correct user
        contractor.push_action(
            "account.reg",
            {
                "user": player444,
            },
            permission=[(player444, Permission.ACTIVE)]
        )
        accounts = list(filter_by_value(contractor, "accounts", "player_name", player444.name))

        self.assertEqual(1, len(accounts), "logic error")
        profile = accounts[0]
        self.assertEqual(profile["player_name"], player444.name, "logic error")
        self.assertEqual(profile["referrer_id"], 0, "logic error")
        self.assertEqual(profile["wood"], 0, "logic error")
        self.assertEqual(profile["metal"], 0, "logic error")
        self.assertEqual(profile["jewel"], 0, "logic error")
        self.assertEqual(profile["oil"], 0, "logic error")
        self.assertEqual(profile["payment_balance"], '0.0000 EOS', "logic error")
        self.assertEqual(profile["withdraw_balance"], '0.0000 EOS', "logic error")

        # 3. try to register again
        self.assertRaises(Error, contractor.push_action,
                          "account.reg",
                          {
                              "user": player444,
                          },
                          permission=[(player444, Permission.ACTIVE)]
                          )
        accounts = list(filter_by_value(contractor, "accounts", "player_name", player444.name))
        self.assertEqual(1, len(accounts), "logic error")

    def test_account_register_with_ref(self):
        COMMENT('''Test action 'account.regr':''')
        create_account("player401", master)
        create_account("player402", master)
        action = contractor.push_action
        action("account.reg", {"user": player401}, permission=[(player401, Permission.ACTIVE)])
        profiles = list(filter_by_value(contractor, "accounts", "player_name", player401.name))
        ref = to_ref_id(profiles[0]["player_name"], profiles[0]["created_at"])
        action("account.regr", {"user": player402, "referral_id": ref}, permission=[(player402, Permission.ACTIVE)])
        profiles = list(filter_by_value(contractor, "accounts", "player_name", player402.name))
        self.assertEqual(str(ref), profiles[0]["referrer_id"], "logic error")

    def test_on_transfer(self):
        COMMENT('''Test event 'on_transfer':''')
        create_account("player404", master)
        contractor.push_action("account.reg", {"user": player404}, permission=[(player404, Permission.ACTIVE)])
        # 1. transaction from unregistered to contract
        etoken.push_action(
            "transfer",
            {
                "from": unregistered, "to": contractor, "quantity": "10.0000 EOS", "memo": "transfer"
            },
            permission=(unregistered, Permission.ACTIVE),  # json=True
        )
        profiles = list(filter_by_value(contractor, "accounts", "player_name", "unregistered"))
        self.assertEqual(0, len(profiles), "Found unregistered account.")

        # 2. transaction from contract to account
        etoken.push_action(
            "transfer",
            {
                "from": contractor, "to": player404, "quantity": "1000.0000 EOS", "memo": "transfer"
            },
            permission=(contractor, Permission.ACTIVE),  # json=True
        )

        profiles = list(filter_by_value(contractor, "accounts", "player_name", player404.name))
        self.assertEqual(1, len(profiles), "Found unregistered account.")
        self.assertEqual("0.0000 EOS", profiles[0]["payment_balance"], "Wrong balance.")
        self.assertEqual("0.0000 EOS", profiles[0]["withdraw_balance"], "Wrong balance.")

        # 3. transaction from account to contract
        etoken.push_action(
            "transfer",
            {
                "from": player404, "to": contractor, "quantity": "100.0000 EOS", "memo": "transfer"
            },
            permission=(player404, Permission.ACTIVE),  # json=True
        )
        profiles = list(filter_by_value(contractor, "accounts", "player_name", player404.name))
        self.assertEqual(1, len(profiles), "account not found.")
        self.assertEqual("100.0000 EOS", profiles[0]["payment_balance"], "Wrong balance.")
        self.assertEqual("0.0000 EOS", profiles[0]["withdraw_balance"], "Wrong balance.")

    def test_withdraw(self):
        COMMENT('''Test action 'withdraw':''')
        create_account("player405", master)
        contractor.push_action("account.reg", {"user": player405}, permission=[(player405, Permission.ACTIVE)])
        etoken.push_action(
            "transfer",
            {
                "from": contractor, "to": player405, "quantity": "100000.0000 EOS", "memo": "transfer"
            },
            permission=(contractor, Permission.ACTIVE),  # json=True
        )
        etoken.push_action(
            "transfer",
            {
                "from": player405, "to": contractor, "quantity": "100000.0000 EOS", "memo": "transfer"
            },
            permission=(player405, Permission.ACTIVE),  # json=True
        )
        sleep(2)
        profiles = list(filter_by_value(contractor, "accounts", "player_name", player405.name))
        self.assertEqual(1, len(profiles), "account not found.")
        self.assertEqual("100000.0000 EOS", profiles[0]["payment_balance"], "Wrong balance.")
        action = contractor.push_action
        # it is necessary to collect some resource to sell and receive some money for this resource
        for x in range(5):
            action("factory.buy", {"user": player405, "resource_type": 4},
                   permission=[(player405, Permission.ACTIVE)])
            action("level.up", {"user": player405, "resource_type": 4, "level": 1, "count": 1},
                   permission=[(player405, Permission.ACTIVE)])
            action("level.up", {"user": player405, "resource_type": 4, "level": 2, "count": 1},
                   permission=[(player405, Permission.ACTIVE)])
            action("level.up", {"user": player405, "resource_type": 4, "level": 3, "count": 1},
                   permission=[(player405, Permission.ACTIVE)])
            action("level.up", {"user": player405, "resource_type": 4, "level": 4, "count": 1},
                   permission=[(player405, Permission.ACTIVE)])
            action("level.up", {"user": player405, "resource_type": 4, "level": 5, "count": 1},
                   permission=[(player405, Permission.ACTIVE)])
            action("level.up", {"user": player405, "resource_type": 4, "level": 6, "count": 1},
                   permission=[(player405, Permission.ACTIVE)])
            sleep(0.6)
        sleep(5)

        action("collect.one", {"user": player405, "resource_type": 4}, permission=[(player405, Permission.ACTIVE)])
        action("sell", {"user": player405, "resource_type": 4}, permission=[(player405, Permission.ACTIVE)])

        balance_before, balance_before_float = withdraw_balance(player405)

        self.assertTrue(balance_before_float > 0, "not enough withdraw balance")

        # 1. without permission
        self.assertRaises(Error, action, "withdraw", {"user": player405, "value": 1.0}, permission=[])
        profiles = list(filter_by_value(contractor, "accounts", "player_name", player405.name))
        self.assertEqual(balance_before, profiles[0]["withdraw_balance"], "Wrong balance.")

        # 2. wrong permission
        self.assertRaises(Error, action, "withdraw",
                          {"user": player405, "value": 1.0}, permission=[(player403, Permission.ACTIVE)])
        profiles = list(filter_by_value(contractor, "accounts", "player_name", player405.name))

        self.assertEqual(balance_before, profiles[0]["withdraw_balance"], "Wrong balance.")

        # 3. correct request
        _, balance_before_float = withdraw_balance(player405)

        action("withdraw", {"user": player405, "value": 1.0}, permission=[(player405, Permission.ACTIVE)])

        _, balance_after_float = withdraw_balance(player405)
        self.assertEqual(balance_before_float - 1.0, balance_after_float, "Wrong balance.")

        # 4. correct request with minimal value
        _, balance_before_float = withdraw_balance(player405)

        action("withdraw", {"user": player405, "value": 0.0001}, permission=[(player405, Permission.ACTIVE)])

        _, balance_after_float = withdraw_balance(player405)
        self.assertEqual(balance_before_float - 0.0001, balance_after_float, "Wrong balance.")

        # 5. attempt to withdraw more then exists on balance
        _, balance_before_float = withdraw_balance(player405)

        self.assertRaises(Error, action, "withdraw", {"user": player405, "value": 10000}, permission=[])
        _, balance_after_float = withdraw_balance(player405)
        self.assertEqual(balance_before_float, balance_after_float, "Wrong balance.")

        # 6. withdraw all balance
        _, balance_before_float = withdraw_balance(player405)

        action("withdraw",
               {"user": player405, "value": balance_before_float},
               permission=[(player405, Permission.ACTIVE)])

        profiles = list(filter_by_value(contractor, "accounts", "player_name", player405.name))
        self.assertEqual("0.0000 EOS", profiles[0]["withdraw_balance"], "Wrong balance.")

    def test_withdraw_set(self):
        COMMENT('''Test action 'withdraw.set':''')
        create_account("player407", master)
        contractor.push_action("account.reg", {"user": player407}, permission=[(player407, Permission.ACTIVE)])

        sleep(1)

        etoken.push_action(
            "transfer",
            {
                "from": contractor, "to": player407, "quantity": "100000.0000 EOS", "memo": "transfer"
            },
            permission=(contractor, Permission.ACTIVE),
        )
        etoken.push_action(
            "transfer",
            {
                "from": player407, "to": contractor, "quantity": "100000.0000 EOS", "memo": "transfer"
            },
            permission=(player407, Permission.ACTIVE),
        )
        sleep(3)
        profiles = list(filter_by_value(contractor, "accounts", "player_name", player407.name))
        self.assertEqual(1, len(profiles), "account not found.")
        self.assertEqual("100000.0000 EOS", profiles[0]["payment_balance"], "Wrong balance.")
        action = contractor.push_action
        # it is necessary to collect some resource to sell and receive some money for this resource
        for x in range(5):
            action("factory.buy", {"user": player407, "resource_type": 4},
                   permission=[(player407, Permission.ACTIVE)])
            action("level.up", {"user": player407, "resource_type": 4, "level": 1, "count": 1},
                   permission=[(player407, Permission.ACTIVE)])
            action("level.up", {"user": player407, "resource_type": 4, "level": 2, "count": 1},
                   permission=[(player407, Permission.ACTIVE)])
            action("level.up", {"user": player407, "resource_type": 4, "level": 3, "count": 1},
                   permission=[(player407, Permission.ACTIVE)])
            action("level.up", {"user": player407, "resource_type": 4, "level": 4, "count": 1},
                   permission=[(player407, Permission.ACTIVE)])
            action("level.up", {"user": player407, "resource_type": 4, "level": 5, "count": 1},
                   permission=[(player407, Permission.ACTIVE)])
            action("level.up", {"user": player407, "resource_type": 4, "level": 6, "count": 1},
                   permission=[(player407, Permission.ACTIVE)])
            sleep(0.6)
        sleep(5)

        action("collect.one", {"user": player407, "resource_type": 4}, permission=[(player407, Permission.ACTIVE)])
        action("sell", {"user": player407, "resource_type": 4}, permission=[(player407, Permission.ACTIVE)])

        balance_before, balance_before_float = withdraw_balance(player407)
        self.assertTrue(balance_before_float > 0, "not enough withdraw balance")

        # 1. correct request
        _, balance_before_float = withdraw_balance(player407)
        action("withdraw", {"user": player407, "value": 1.0}, permission=[(player407, Permission.ACTIVE)])
        _, balance_after_float = withdraw_balance(player407)
        self.assertEqual(balance_before_float - 1.0, balance_after_float, "Wrong balance.")

        # 2. disable & correct request
        _, balance_before_float = withdraw_balance(player407)
        action("withdraw.set", {"user": contractor, "value": 0}, permission=[(contractor, Permission.ACTIVE)])
        self.assertRaises(Error, action, "withdraw", {"user": player407, "value": 1.0},
                          permission=[(player407, Permission.ACTIVE)])
        _, balance_after_float = withdraw_balance(player407)
        self.assertEqual(balance_before_float, balance_after_float, "Wrong balance.")

        # 3. enable & correct request
        sleep(1)
        action("withdraw.set", {"user": contractor, "value": 1}, permission=[(contractor, Permission.ACTIVE)])
        _, balance_before_float = withdraw_balance(player407)
        action("withdraw", {"user": player407, "value": 1.0}, permission=[(player407, Permission.ACTIVE)])
        _, balance_after_float = withdraw_balance(player407)
        self.assertEqual(balance_before_float - 1.0, balance_after_float, "Wrong balance.")

    def test_buy_factory(self):
        COMMENT('''Test action 'factory.buy':''')
        create_account("player403", master)
        action = contractor.push_action

        user_factories = list(filter_by_value(contractor, "factories", "player_name", player403.name))
        self.assertEqual(0, len(user_factories), "No factories must be exists here.")

        # 1. try to buy for unregistered user
        self.assertRaises(Error, action, "factory.buy", {"user": player403, "resource_type": 1},
                          permission=[(player403, Permission.ACTIVE)])

        user_factories = list(filter_by_value(contractor, "factories", "player_name", player403.name))
        self.assertEqual(0, len(user_factories), "No factories must be exists here.")

        # 2. try to buy if not enough balance
        action("account.reg", {"user": player403}, permission=[(player403, Permission.ACTIVE)])
        self.assertRaises(Error, action, "factory.buy", {"user": player403, "resource_type": 1},
                          permission=[(player403, Permission.ACTIVE)])

        user_factories = list(filter_by_value(contractor, "factories", "player_name", player403.name))
        self.assertEqual(0, len(user_factories), "No factories must be exists here.")

        # 3. try to buy if enough balance
        etoken.push_action("transfer",
                           {"from": contractor, "to": player403, "quantity": "1000.0000 EOS", "memo": "transfer"},
                           permission=(contractor, Permission.ACTIVE),
                           )
        etoken.push_action("transfer",
                           {"from": player403, "to": contractor, "quantity": "100.0000 EOS", "memo": "transfer"},
                           permission=(player403, Permission.ACTIVE),
                           )
        profiles = list(filter_by_value(contractor, "accounts", "player_name", player403.name))
        self.assertEqual("100.0000 EOS", profiles[0]["payment_balance"], "Wrong balance.")

        action("factory.buy", {"user": player403, "resource_type": 1}, permission=[(player403, Permission.ACTIVE)])
        user_factories = list(filter_by_value(contractor, "factories", "player_name", player403.name))
        self.assertEqual(1, len(user_factories), "No factories must be exists here.")
        self.assertEqual(player403.name, user_factories[0]["player_name"], "Wrong factory.")
        self.assertEqual(1, user_factories[0]["type"], "Wrong factory.")
        self.assertEqual(1, user_factories[0]["level"], "Wrong factory.")
        self.assertEqual(1, user_factories[0]["count"], "Wrong factory.")

        # sleep is necessary to avoid ERROR
        # Error 3040008: Duplicate transaction
        # You can try embedding eosio nonce action inside your transaction to ensure uniqueness.
        # Error Details:
        # duplicate transaction 1caa7ec150a775072a9221bb0386ffdc01f68adcf44aa5ead4d60971fe8f1e5c
        sleep(1)

        # 4. buy another one of this type
        action("factory.buy", {"user": player403, "resource_type": 1}, permission=[(player403, Permission.ACTIVE)])
        user_factories = list(filter_by_value(contractor, "factories", "player_name", player403.name))
        self.assertEqual(1, len(user_factories), "No factories must be exists here.")
        self.assertEqual(player403.name, user_factories[0]["player_name"], "Wrong factory.")
        self.assertEqual(1, user_factories[0]["type"], "Wrong factory.")
        self.assertEqual(1, user_factories[0]["level"], "Wrong factory.")
        self.assertEqual(2, user_factories[0]["count"], "Wrong factory.")

        # 5. buy another factories of other types
        for x in (2, 3, 4):
            action("factory.buy", {"user": player403, "resource_type": x}, permission=[(player403, Permission.ACTIVE)])

        user_factories = list(filter_by_value(contractor, "factories", "player_name", player403.name))
        self.assertEqual(4, len(user_factories), "Wrong amount of factories.")

        self.assertEqual(player403.name, user_factories[0]["player_name"], "Wrong factory.")
        self.assertEqual(1, user_factories[0]["type"], "Wrong factory.")
        self.assertEqual(1, user_factories[0]["level"], "Wrong factory.")
        self.assertEqual(2, user_factories[0]["count"], "Wrong factory.")

        self.assertEqual(player403.name, user_factories[1]["player_name"], "Wrong factory.")
        self.assertEqual(2, user_factories[1]["type"], "Wrong factory.")
        self.assertEqual(1, user_factories[1]["level"], "Wrong factory.")
        self.assertEqual(1, user_factories[1]["count"], "Wrong factory.")

        self.assertEqual(player403.name, user_factories[1]["player_name"], "Wrong factory.")
        self.assertEqual(3, user_factories[2]["type"], "Wrong factory.")
        self.assertEqual(1, user_factories[2]["level"], "Wrong factory.")
        self.assertEqual(1, user_factories[2]["count"], "Wrong factory.")

        self.assertEqual(player403.name, user_factories[1]["player_name"], "Wrong factory.")
        self.assertEqual(4, user_factories[3]["type"], "Wrong factory.")
        self.assertEqual(1, user_factories[3]["level"], "Wrong factory.")
        self.assertEqual(1, user_factories[3]["count"], "Wrong factory.")

        # 6. try to buy wrong type
        self.assertRaises(Error, action,
                          "factory.buy",
                          {"user": player403, "resource_type": 5}, permission=[(player403, Permission.ACTIVE)])
        user_factories = list(filter_by_value(contractor, "factories", "player_name", player403.name))
        self.assertEqual(4, len(user_factories), "Wrong amount of factories.")

    def test_level_up(self):
        COMMENT('''Test action 'level.up':''')
        create_account("player406", master)
        create_account("main1", master)
        create_account("dev1", master)
        action = contractor.push_action

        action("wallets.set", {"caller": contractor, "main_wallet": main1, "dev_wallet": dev1},
               permission=[(contractor, Permission.ACTIVE)])
        try:
            user_factories = list(filter_by_value(contractor, "factories", "player_name", player406.name))
            self.assertEqual(0, len(user_factories), "No factories must be exists at this point.")

            # 1. try to level up for unregistered user
            self.assertRaises(Error, action,
                              "level.up",
                              {"user": player406, "resource_type": 1, "factory_level": 1, "count": 1},
                              permission=[(player406, Permission.ACTIVE)])

            user_factories = list(filter_by_value(contractor, "factories", "player_name", player406.name))
            self.assertEqual(0, len(user_factories), "No factories must be exists here.")
            contractor.push_action("account.reg", {"user": player406}, permission=[(player406, Permission.ACTIVE)])
            # 2. try to level up with wrong permissions
            etoken.push_action(
                "transfer",
                {
                    "from": contractor, "to": player406, "quantity": "1000.0000 EOS", "memo": "transfer"
                },
                permission=(contractor, Permission.ACTIVE),
            )
            etoken.push_action(
                "transfer",
                {
                    "from": player406, "to": contractor, "quantity": "19.0000 EOS", "memo": "transfer"
                },
                permission=(player406, Permission.ACTIVE),
            )
            for x in (1, 2, 3, 4):
                action("factory.buy", {"user": player406, "resource_type": x},
                       permission=[(player406, Permission.ACTIVE)])
            self.assertRaises(Error, action,
                              "level.up",
                              {"user": player406, "resource_type": 1, "level": 1, "count": 1},
                              permission=[(admin, Permission.ACTIVE)])
            # 3. try to level up with wrong resource type
            self.assertRaises(Error, action,
                              "level.up",
                              {"user": player406, "resource_type": 34, "level": 1, "count": 1},
                              permission=[(player406, Permission.ACTIVE)])
            # 4. try to level up with wrong level
            self.assertRaises(Error, action,
                              "level.up",
                              {"user": player406, "resource_type": 1, "level": 11, "count": 1},
                              permission=[(player406, Permission.ACTIVE)])
            # 5. try to level up with wrong count
            self.assertRaises(Error, action,
                              "level.up",
                              {"user": player406, "resource_type": 1, "level": 1, "count": 2},
                              permission=[(player406, Permission.ACTIVE)])
            # 6. try to level up if not enough money on balance
            self.assertRaises(Error, action,
                              "level.up",
                              {"user": player406, "resource_type": 1, "level": 1, "count": 1},
                              permission=[(player406, Permission.ACTIVE)])
            user_factories = list(filter_by_value(contractor, "factories", "player_name", player406.name))
            self.assertEqual(4, len(user_factories), "No factories must be exists here.")
            # 7. level up 1 factory of lvl1
            etoken.push_action(
                "transfer",
                {
                    "from": player406, "to": contractor, "quantity": "858.0000 EOS", "memo": "transfer"
                },
                permission=(player406, Permission.ACTIVE),
            )
            profiles = list(filter_by_value(contractor, "accounts", "player_name", player406.name))
            self.assertEqual("858.0000 EOS", profiles[0]["payment_balance"], "Wrong balance.")

            action("level.up", {"user": player406, "resource_type": 1, "level": 1, "count": 1},
                   permission=[(player406, Permission.ACTIVE)])
            user_factories = list(filter_by_value(contractor, "factories", "player_name", player406.name))
            self.assertEqual(4, len(user_factories), "Wrong amount.")
            self.assertEqual(1, user_factories[0]["type"], "Wrong resource.")
            self.assertEqual(2, user_factories[0]["level"], "Wrong level.")
            self.assertEqual(1, user_factories[0]["count"], "Wrong count.")

            self.assertEqual(2, user_factories[1]["type"], "Wrong resource.")
            self.assertEqual(1, user_factories[1]["level"], "Wrong level.")
            self.assertEqual(1, user_factories[1]["count"], "Wrong count.")

            self.assertEqual(3, user_factories[2]["type"], "Wrong resource.")
            self.assertEqual(1, user_factories[2]["level"], "Wrong level.")
            self.assertEqual(1, user_factories[2]["count"], "Wrong count.")

            self.assertEqual(4, user_factories[3]["type"], "Wrong resource.")
            self.assertEqual(1, user_factories[3]["level"], "Wrong level.")
            self.assertEqual(1, user_factories[3]["count"], "Wrong count.")

            profiles = list(filter_by_value(contractor, "accounts", "player_name", player406.name))
            self.assertEqual("853.0000 EOS", profiles[0]["payment_balance"], "Wrong balance.")

            # 7. level up 1 factory of 2 lvl to 7 lvl
            action("level.up", {"user": player406, "resource_type": 1, "level": 2, "count": 1},
                   permission=[(player406, Permission.ACTIVE)])
            action("level.up", {"user": player406, "resource_type": 1, "level": 3, "count": 1},
                   permission=[(player406, Permission.ACTIVE)])
            action("level.up", {"user": player406, "resource_type": 1, "level": 4, "count": 1},
                   permission=[(player406, Permission.ACTIVE)])
            action("level.up", {"user": player406, "resource_type": 1, "level": 5, "count": 1},
                   permission=[(player406, Permission.ACTIVE)])
            action("level.up", {"user": player406, "resource_type": 1, "level": 6, "count": 1},
                   permission=[(player406, Permission.ACTIVE)])

            user_factories = list(filter_by_value(contractor, "factories", "player_name", player406.name))
            self.assertEqual(4, len(user_factories), "Wrong amount.")
            self.assertEqual(1, user_factories[0]["type"], "Wrong resource.")
            self.assertEqual(7, user_factories[0]["level"], "Wrong level.")
            self.assertEqual(1, user_factories[0]["count"], "Wrong count.")

            self.assertEqual(2, user_factories[1]["type"], "Wrong resource.")
            self.assertEqual(1, user_factories[1]["level"], "Wrong level.")
            self.assertEqual(1, user_factories[1]["count"], "Wrong count.")

            self.assertEqual(3, user_factories[2]["type"], "Wrong resource.")
            self.assertEqual(1, user_factories[2]["level"], "Wrong level.")
            self.assertEqual(1, user_factories[2]["count"], "Wrong count.")

            self.assertEqual(4, user_factories[3]["type"], "Wrong resource.")
            self.assertEqual(1, user_factories[3]["level"], "Wrong level.")
            self.assertEqual(1, user_factories[3]["count"], "Wrong count.")

            profiles = list(filter_by_value(contractor, "accounts", "player_name", player406.name))
            self.assertEqual("1.0000 EOS", profiles[0]["payment_balance"], "Wrong balance.")
            # check balance
            row = etoken.table("accounts", main1).json["rows"][0]
            payments = (5 + 12 + 30 + 90 + 180 + 540) + (1 + 3 + 5 + 10)
            self.assertEqual(f"{payments * 0.85:.4f} EOS", row["balance"], "Wrong balance.")
            row = etoken.table("accounts", dev1).json["rows"][0]
            self.assertEqual(f"{payments * 0.15:.4f} EOS", row["balance"], "Wrong balance.")
        finally:
            action("wallets.set", {"caller": contractor, "main_wallet": contractor, "dev_wallet": contractor},
                   permission=[(contractor, Permission.ACTIVE)])

    def test_collect_one_and_sell(self):
        COMMENT('''Test action 'collect.one':''')
        create_account("player408", master)
        create_account("main2", master)
        create_account("dev2", master)
        action = contractor.push_action
        action("wallets.set", {"caller": contractor, "main_wallet": main2, "dev_wallet": dev2},
               permission=[(contractor, Permission.ACTIVE)])
        try:
            user_factories = list(filter_by_value(contractor, "factories", "player_name", player408.name))
            self.assertEqual(0, len(user_factories), "No factories must be exists at this point.")

            contractor.push_action("account.reg", {"user": player408}, permission=[(player408, Permission.ACTIVE)])
            etoken.push_action(
                "transfer",
                {
                    "from": contractor, "to": player408, "quantity": "1000000.0000 EOS", "memo": "transfer"
                },
                permission=(contractor, Permission.ACTIVE),
            )
            etoken.push_action(
                "transfer",
                {
                    "from": player408, "to": contractor, "quantity": "1000000.0000 EOS", "memo": "transfer"
                },
                permission=(player408, Permission.ACTIVE),
            )
            # buy 10 factories of type 4 and 7 level
            for x in range(5):
                action("factory.buy", {"user": player408, "resource_type": 4},
                       permission=[(player408, Permission.ACTIVE)])
                action("level.up", {"user": player408, "resource_type": 4, "level": 1, "count": 1},
                       permission=[(player408, Permission.ACTIVE)])
                action("level.up", {"user": player408, "resource_type": 4, "level": 2, "count": 1},
                       permission=[(player408, Permission.ACTIVE)])
                action("level.up", {"user": player408, "resource_type": 4, "level": 3, "count": 1},
                       permission=[(player408, Permission.ACTIVE)])
                action("level.up", {"user": player408, "resource_type": 4, "level": 4, "count": 1},
                       permission=[(player408, Permission.ACTIVE)])
                action("level.up", {"user": player408, "resource_type": 4, "level": 5, "count": 1},
                       permission=[(player408, Permission.ACTIVE)])
                action("level.up", {"user": player408, "resource_type": 4, "level": 6, "count": 1},
                       permission=[(player408, Permission.ACTIVE)])
                sleep(0.6)

            # we have 4 factories
            # get last collected time
            user_factories = list(filter_by_value(contractor, "factories", "player_name", player408.name))
            self.assertEqual(1, len(user_factories), "Wrong amount.")
            before = {x["type"]: x["last_collected"] for x in user_factories}

            accounts = list(filter_by_value(contractor, "accounts", "player_name", player408.name))
            self.assertEqual(0, accounts[0]["wood"], "found collected resources at this point.")
            self.assertEqual(0, accounts[0]["metal"], "found collected resources at this point.")
            self.assertEqual(0, accounts[0]["oil"], "found collected resources at this point.")
            count_before = accounts[0]["jewel"]

            sleep(5)

            action("collect.one", {"user": player408, "resource_type": 4}, permission=[(player408, Permission.ACTIVE)])

            user_factories = list(filter_by_value(contractor, "factories", "player_name", player408.name))
            after = {x["type"]: x["last_collected"] for x in user_factories}

            # formula:
            # uint32_t count = (it->produce + it->produce_bonus) *
            #          factory.count * time_point(now_collected - factory.last_collected).sec_since_epoch() / 60
            count = int((6776 + 3535.45) * 5 * (gt(after[4]).timestamp() - gt(before[4]).timestamp()) / 60)

            accounts = list(filter_by_value(contractor, "accounts", "player_name", player408.name))
            self.assertEqual(count_before + count, accounts[0]["jewel"], "Wrong collected resources.")

            balance_before = accounts[0]["withdraw_balance"]
            balance_before = float(balance_before.split(" ")[0])
            balance_after = balance_before + accounts[0]["jewel"] * 0.006655

            action("sell", {"user": player408, "resource_type": 4}, permission=[(player408, Permission.ACTIVE)])

            accounts = list(filter_by_value(contractor, "accounts", "player_name", player408.name))
            self.assertEqual(0, accounts[0]["jewel"], "Wrong collected resources.")

            import decimal
            myothercontext = decimal.Context(rounding=decimal.ROUND_DOWN)
            decimal.setcontext(myothercontext)
            balance_after = round(decimal.Decimal(balance_after), 4)
            self.assertEqual(f"{balance_after} EOS", accounts[0]["withdraw_balance"], "Wrong balance.")
        finally:
            action("wallets.set", {"caller": contractor, "main_wallet": contractor, "dev_wallet": contractor},
                   permission=[(contractor, Permission.ACTIVE)])

    def test_referral_bonuses(self):
        COMMENT('''Test referral bonuses':''')
        create_account("player409", master)
        create_account("player410", master)
        create_account("player411", master)
        action = contractor.push_action

        contractor.push_action("account.reg", {"user": player409}, permission=[(player409, Permission.ACTIVE)])

        accounts = list(filter_by_value(contractor, "accounts", "player_name", player409.name))
        self.assertEqual("0.0000 EOS", accounts[0]["payment_balance"], "Wrong balance.")
        ref_id_1 = to_ref_id(player409.name, accounts[0]["created_at"])
        contractor.push_action("account.regr", {"user": player410, "referral_id": ref_id_1},
                               permission=[(player410, Permission.ACTIVE)])
        accounts = list(filter_by_value(contractor, "accounts", "player_name", player410.name))
        self.assertEqual("0.0000 EOS", accounts[0]["payment_balance"], "Wrong balance.")
        ref_id_2 = to_ref_id(player410.name, accounts[0]["created_at"])
        contractor.push_action("account.regr", {"user": player411, "referral_id": ref_id_2},
                               permission=[(player411, Permission.ACTIVE)])
        sleep(3)
        accounts = list(filter_by_value(contractor, "accounts", "player_name", player411.name))
        self.assertEqual("0.0000 EOS", accounts[0]["payment_balance"], "Wrong balance.")

        etoken.push_action("transfer", {"from": contractor, "to": player411, "quantity": "100.0000 EOS",
                                        "memo": "transfer"}, permission=(contractor, Permission.ACTIVE))
        etoken.push_action("transfer", {"from": player411, "to": contractor, "quantity": "100.0000 EOS",
                                        "memo": "transfer"}, permission=(player411, Permission.ACTIVE))
        accounts = list(filter_by_value(contractor, "accounts", "player_name", player411.name))
        self.assertEqual("100.0000 EOS", accounts[0]["payment_balance"], "Wrong balance.")

        action("factory.buy", {"user": player411, "resource_type": 4}, permission=[(player411, Permission.ACTIVE)])

        sleep(1)

        accounts = list(filter_by_value(contractor, "accounts", "player_name", player411.name))
        self.assertEqual("90.0000 EOS", accounts[0]["payment_balance"], "Wrong balance.")
        self.assertEqual(str(ref_id_2), accounts[0]["referrer_id"], "Wrong referrer.")

        accounts = list(filter_by_value(contractor, "accounts", "player_name", player410.name))
        self.assertEqual("0.7000 EOS", accounts[0]["payment_balance"], "Wrong balance.")
        self.assertEqual(str(ref_id_1), accounts[0]["referrer_id"], "Wrong referrer.")

        accounts = list(filter_by_value(contractor, "accounts", "player_name", player409.name))
        self.assertEqual("0.3000 EOS", accounts[0]["payment_balance"], "Wrong balance.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='MyMillions contract test')
    parser.add_argument('--etoken', dest='etoken_dir', help='Path to eosio.token compiled contract.')
    parser.add_argument('--mymillions', dest='mymillions_dir', help='Path to mymillions compiled contract.')
    args = parser.parse_args()

    if args.etoken_dir:
        ContractTest.etoken_dir = args.etoken_dir
    if args.mymillions_dir:
        ContractTest.mymillions_dir = mymillions_dir

    suite = unittest.TestSuite()
    loader = TestLoader()
    tests = loader.loadTestsFromTestCase(ContractTest)
    suite.addTests(tests)
    unittest.TextTestRunner(verbosity=2).run(suite)
