#include <boost/test/unit_test.hpp>
#include "common.hpp"

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo = fc::mutable_variant_object;

static std::vector<uint8_t> millions_wasm()
{
      return read_wasm("mymillions/mymillions.wasm");
}

//static std::string millions_wast()
//{
//      return read_wast("mymillions.wast");
//}

static std::vector<char> millions_abi()
{
      return read_abi("mymillions/mymillions.abi");
}



//class custom_tester : public TESTER {
//public:
//
//   custom_tester()
//   {
//      produce_blocks( 2 );
//
//      create_accounts( { N(alice), N(bob), N(carol), N(eosio.token) } );
//
//      produce_blocks( 2 );
//
//      set_code( N(eosio.token), contracts::token_wasm() );
//
//      set_abi( N(eosio.token), contracts::token_abi().data() );
//
//      produce_blocks();
//
//      const auto& accnt = control->db().get<account_object,by_name>( N(eosio.token) );
//      abi_def abi;
//      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
//
//      abi_ser.set_abi(abi, abi_serializer_max_time);
//   }
//
//   action_result push_action(const account_name& signer, const action_name &name, const variant_object &data)
//   {
//      string action_type_name = abi_ser.get_action_type(name);
//
//      action act;
//      act.account = N(eosio.token);
//      act.name    = name;
//      act.data    = abi_ser.variant_to_binary( action_type_name, data,abi_serializer_max_time );
//
//      return base_tester::push_action( std::move(act), uint64_t(signer));
//   }
//
//   fc::variant get_stats( const string& symbolname )
//   {
//      auto symb = eosio::chain::symbol::from_string(symbolname);
//      auto symbol_code = symb.to_symbol_code().value;
//      vector<char> data = get_row_by_account( N(eosio.token), symbol_code, N(stat), symbol_code );
//      return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "currency_stats", data, abi_serializer_max_time );
//   }
//
//   fc::variant get_account( account_name acc, const string& symbolname)
//   {
//      auto symb = eosio::chain::symbol::from_string(symbolname);
//      auto symbol_code = symb.to_symbol_code().value;
//      vector<char> data = get_row_by_account( N(eosio.token), acc, N(accounts), symbol_code );
//      return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "account", data, abi_serializer_max_time );
//   }
//
//   action_result create( account_name issuer,
//                asset        maximum_supply ) {
//
//      return push_action( N(eosio.token), N(create), mvo()
//           ( "issuer", issuer)
//           ( "maximum_supply", maximum_supply)
//      );
//   }
//
//   action_result issue( account_name issuer, account_name to, asset quantity, string memo ) {
//      return push_action( issuer, N(issue), mvo()
//           ( "to", to)
//           ( "quantity", quantity)
//           ( "memo", memo)
//      );
//   }
//
//   action_result retire( account_name issuer, asset quantity, string memo ) {
//      return push_action( issuer, N(retire), mvo()
//           ( "quantity", quantity)
//           ( "memo", memo)
//      );
//
//   }
//
//   action_result transfer( account_name from,
//                  account_name to,
//                  asset        quantity,
//                  string       memo ) {
//      return push_action( from, N(transfer), mvo()
//           ( "from", from)
//           ( "to", to)
//           ( "quantity", quantity)
//           ( "memo", memo)
//      );
//   }
//
//   action_result open( account_name owner,
//                       const string& symbolname,
//                       account_name ram_payer    ) {
//      return push_action( ram_payer, N(open), mvo()
//           ( "owner", owner )
//           ( "symbol", symbolname )
//           ( "ram_payer", ram_payer )
//      );
//   }
//
//   action_result close( account_name owner,
//                        const string& symbolname ) {
//      return push_action( owner, N(close), mvo()
//           ( "owner", owner )
//           ( "symbol", "0,CERO" )
//      );
//   }
//
//   abi_serializer abi_ser;
//};

BOOST_AUTO_TEST_SUITE(millions_tests)

BOOST_FIXTURE_TEST_CASE( create_tests, TESTER )
try
{
      const char* name = "adminco";

      produce_blocks(2);
      create_accounts( {::string_to_name(name)} );
      produce_blocks(2);

      set_code( ::string_to_name(name), millions_wasm());
      set_abi( ::string_to_name(name), millions_abi().data());

      produce_blocks(1);

      abi_serializer abi_ser(json::from_string(millions_abi().data()).as<abi_def>(), abi_serializer_max_time);

      signed_transaction trx1;
      {
//            auto& trx = trx1;
//            action trigger_act;
//            trigger_act.account = N(multitest);
//            trigger_act.name = N(trigger);
//            trigger_act.authorization = vector<permission_level>{{N(multitest), config::active_name}};
//            trigger_act.data = abi_ser.variant_to_binary("trigger", mutable_variant_object()("what", 0),
//                                                         abi_serializer_max_time);
//            trx.actions.emplace_back(std::move(trigger_act));
//            set_transaction_headers(trx);
//            trx.sign(get_private_key(N(multitest), "active"), control->get_chain_id());
//            push_transaction(trx);
      }

      signed_transaction trx2;
      {
//            auto& trx = trx2;
//
//            action trigger_act;
//            trigger_act.account = N(multitest);
//            trigger_act.name = N(trigger);
//            trigger_act.authorization = vector<permission_level>{{N(multitest), config::active_name}};
//            trigger_act.data = abi_ser.variant_to_binary("trigger", mutable_variant_object()
//                                                                 ("what", 1),
//                                                         abi_serializer_max_time
//            );
//            trx.actions.emplace_back(std::move(trigger_act));
//            set_transaction_headers(trx);
//            trx.sign(get_private_key(N(multitest), "active"), control->get_chain_id());
//            push_transaction(trx);
      }

      produce_block();
//      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx1.id()));
//      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx2.id()));
}
FC_LOG_AND_RETHROW()


BOOST_AUTO_TEST_SUITE_END()
