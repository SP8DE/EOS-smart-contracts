#pragma once

#include <eosiolib/crypto.h>
#include <eosiolib/time.hpp>
#include <eosiolib/eosio.hpp>
#include<eosiolib/singleton.hpp>

#include <string>

namespace eosiosystem {
   class system_contract;
}

namespace eosio {

   using std::string;

   class [[eosio::contract("lottery")]] lottery : public contract {
     private:
         struct win_obj {
             uint64_t number;
             string id_rng_trx;
             string prize;

             EOSLIB_SERIALIZE( win_obj, (number)(id_rng_trx)(prize) )
         };

         struct [[eosio::table]] activelot {
             name lottery_name;

             auto primary_key()const { return lottery_name.value; }

             EOSLIB_SERIALIZE( activelot, (lottery_name) )
        };

         struct winner {
             uint64_t id_ticket;
             string address_wallet;
             string id_rng_trx;
             string award_description;

             EOSLIB_SERIALIZE( winner, (id_ticket)(address_wallet)(id_rng_trx)(award_description) )
         };

         struct [[eosio::table]] winners {
             time_point_sec date_close;
             std::vector<winner> winners_list;

             EOSLIB_SERIALIZE( winners, (date_close)(winners_list) )
         };

         struct [[eosio::table]] ticket {
            uint64_t id;
            uint64_t id_wallet;

            auto primary_key()const { return id; }

            EOSLIB_SERIALIZE( ticket, (id)(id_wallet) )
         };

         struct [[eosio::table]] wallet {
            uint64_t id;
            string address_wallet;

            auto primary_key()const { return id; }
            key256 secondary_key()const {
                capi_checksum256 hash;
                sha256(address_wallet.c_str(), sizeof(address_wallet), &hash);
                const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&hash);
                return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);

            }

            EOSLIB_SERIALIZE( wallet, (id)(address_wallet) )
         };

         using tickets_table  = eosio::multi_index< "tickets"_n, ticket >;
         using wallets_table  = eosio::multi_index< "wallets"_n, wallet,
            indexed_by<"wallet"_n, const_mem_fun<wallet, key256, &wallet::secondary_key>>
            >;
         using close_lottery  = eosio::singleton< "winners"_n, winners >;
         using active_lottery = eosio::multi_index< "activelots"_n, activelot >;

      public:
         using contract::contract;

         [[eosio::action]]
         void create( name lottery );

         [[eosio::action]]
         void buyticket( name lottery, string addres_wallet, uint64_t count_ticket );

         [[eosio::action]]
         void addwallets( name lottery, string addres_wallet );

         [[eosio::action]]
         void addwalletsr( name lottery, string addres_wallet, uint64_t range );

         [[eosio::action]]
         void apprwint( name lottery, std::vector<win_obj> list_win );

         [[eosio::action]]
         void apprwinu( name lottery, std::vector<win_obj> list_win );

         [[eosio::action]]
         void apprwinur( name lottery, std::vector<win_obj> list_win );

         [[eosio::action]]
         void cleartickets( name lottery );

         [[eosio::action]]
         void clearwallets( name lottery );

         [[eosio::action]]
         void clearlottery( name lottery );

         [[eosio::action]]
         void clearwinners( name lottery );

         inline key256 get_hash_key(const capi_checksum256& hash_str) {
             const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&hash_str);
             return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
         }

         inline capi_checksum256 get_checksum256(std::string &str)const {
             capi_checksum256 hash;
             sha256(str.c_str(), sizeof(str), &hash);
             return hash;
         }
   };

}
