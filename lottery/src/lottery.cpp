#include <lottery/lottery.hpp>

#define COUNT_WINNERS (5)

namespace eosio {

void lottery::create( name lottery )
{
    require_auth( _self );

    active_lottery active_lottery_table(_self, _self.value);
    auto it_lottery_name = active_lottery_table.find(lottery.value);
    eosio_assert(it_lottery_name == active_lottery_table.end(), "lottery with the same name already exists");

    close_lottery close_lottery_table(_self, lottery.value);
    eosio_assert(!close_lottery_table.exists(), "lottery with the same name already exists");


    active_lottery_table.emplace(_self, [&]( auto &item ){
        item.lottery_name = lottery;
    });
}

void lottery::buyticket( name lottery, string addres_wallet, uint64_t count_ticket )
{
    addwallets(lottery, addres_wallet);

    wallets_table wallets( _self, lottery.value );
    auto index_wallet = wallets.get_index<"wallet"_n>();
    auto wallet_index_it = index_wallet.find( get_hash_key(get_checksum256(addres_wallet)) );

    tickets_table tickets(_self, lottery.value);
    for (auto i(0); i<count_ticket; ++i) {
        tickets.emplace(_self, [&]( auto &item ){
            item.id = tickets.available_primary_key();
            item.id_wallet = wallet_index_it->id;
        });
    }
}

void lottery::addwallets( name lottery, string addres_wallet )
{
    require_auth( _self );
    eosio_assert(addres_wallet.size() <= 44, "addres_wallet has more than 44 bytes");

    active_lottery active_lottery_table(_self, _self.value);
    auto it_lottery_name = active_lottery_table.find(lottery.value);
    eosio_assert(it_lottery_name != active_lottery_table.end(), "lottery with the same name already exists");

    wallets_table wallets( _self, lottery.value );
    auto index_wallet = wallets.get_index<"wallet"_n>();
    auto wallet_index_it = index_wallet.find( get_hash_key(get_checksum256(addres_wallet)) );

    if (wallet_index_it == index_wallet.end()) {
        wallets.emplace(_self, [&]( auto &item ){
            item.id = wallets.available_primary_key();
            item.address_wallet = addres_wallet;
        });
    }
}

void lottery::addwalletsr( name lottery, string addres_wallet, uint64_t range )
{
    require_auth( _self );
    eosio_assert(addres_wallet.size() <= 44, "addres_wallet has more than 44 bytes");

    active_lottery active_lottery_table(_self, _self.value);
    auto it_lottery_name = active_lottery_table.find(lottery.value);
    eosio_assert(it_lottery_name != active_lottery_table.end(), "lottery with the same name already exists");

    wallets_table wallets( _self, lottery.value );
    auto index_wallet = wallets.get_index<"wallet"_n>();
    auto wallet_index_it = index_wallet.find( get_hash_key(get_checksum256(addres_wallet)) );

    if (wallet_index_it == index_wallet.end()) {
        wallets.emplace(_self, [&]( auto &item ){
            item.id = range;
            item.address_wallet = addres_wallet;
        });
    }
}

void lottery::apprwint( name lottery, std::vector<win_obj> list_win )
{
    require_auth( _self );

    active_lottery active_lottery_table(_self, _self.value);
    auto it_lottery_name = active_lottery_table.find(lottery.value);
    eosio_assert(it_lottery_name != active_lottery_table.end(), "lottery with the same name already exists");

    tickets_table tickets( _self, lottery.value ); 
    wallets_table wallets( _self, lottery.value );

    std::vector<winner> winners;
    for (auto win_ticket : list_win) {
        auto win_ticket_it = tickets.find(win_ticket.number);
        eosio_assert(win_ticket_it != tickets.end(), "ticket with this number does not exist");
	eosio_assert(win_ticket.prize.size() <= 256, "prize has more than 256 bytes");
        auto wallet = wallets.get(win_ticket_it->id_wallet, "there is no wallet with such id");
	        
        winners.push_back( {win_ticket.number, wallet.address_wallet, win_ticket.id_rng_trx, win_ticket.prize} );
        tickets.erase(win_ticket_it);
    }

    uint8_t count_tickets = 0;
    close_lottery close_lottery_table( _self, lottery.value );
    if (close_lottery_table.exists()) {
        auto obj = close_lottery_table.get();

        for (const auto winner_obj : winners) {
            auto it = std::find_if(obj.winners_list.begin(), obj.winners_list.end(), [&](const winner &obj_it) {
                return obj_it.id_ticket == winner_obj.id_ticket;
            });

            eosio_assert(it == obj.winners_list.end(), "ERROR");
            obj.winners_list.push_back(winner_obj);
        }

        close_lottery_table.set( obj, _self );
        count_tickets = obj.winners_list.size();

    } else {
        close_lottery_table.set( {time_point_sec(now()), winners}, _self );
        count_tickets = close_lottery_table.get().winners_list.size();
    }
}

void lottery::apprwinu( name lottery, std::vector<win_obj> list_win )
{
    require_auth( _self );

    active_lottery active_lottery_table(_self, _self.value);
    auto it_lottery_name = active_lottery_table.find(lottery.value);
    eosio_assert(it_lottery_name != active_lottery_table.end(), "lottery with the same name already exists");

    wallets_table wallets( _self, lottery.value );

    std::vector<winner> winners;
    for (auto win_user : list_win) {
        auto wallet_it = wallets.find(win_user.number);
        eosio_assert(wallet_it != wallets.end(), "wallet with this number does not exist");
        eosio_assert(win_user.prize.size() <= 256, "prize has more than 256 bytes");

        winners.push_back( {win_user.number, wallet_it->address_wallet, win_user.prize} );
        wallets.erase(wallet_it);
    }

    uint8_t count_tickets = 0;
    close_lottery close_lottery_table( _self, lottery.value );
    if (close_lottery_table.exists()) {
        auto obj = close_lottery_table.get();

        for (const auto winner_obj : winners) {
            auto it = std::find_if(obj.winners_list.begin(), obj.winners_list.end(), [&](const winner &obj_it) {
                return obj_it.id_ticket == winner_obj.id_ticket;
            });

            eosio_assert(it == obj.winners_list.end(), "winner exists");
            obj.winners_list.push_back(winner_obj);
        }

        close_lottery_table.set( obj, _self );
        count_tickets = obj.winners_list.size();

    } else {
        close_lottery_table.set( {time_point_sec(now()), winners}, _self );
        count_tickets = close_lottery_table.get().winners_list.size();
    }

    if (count_tickets == COUNT_WINNERS) {
        SEND_INLINE_ACTION( *this, clearwallets, { {_self, "active"_n} }, { lottery } );
        SEND_INLINE_ACTION( *this, clearlottery, { {_self, "active"_n} }, { lottery } );
    }
}

void lottery::apprwinur( name lottery, std::vector<win_obj> list_win )
{
    require_auth( _self );

    active_lottery active_lottery_table(_self, _self.value);
    auto it_lottery_name = active_lottery_table.find(lottery.value);
    eosio_assert(it_lottery_name != active_lottery_table.end(), "lottery with the same name already exists");

    auto lambda = [&]() {
        std::vector<winner> winners;
        wallets_table wallets( _self, lottery.value );
        for (auto win_user : list_win) {
            auto wallet_it = wallets.upper_bound(win_user.number);
            if (wallet_it->id > win_user.number )
                --wallet_it;

	    if (wallet_it == wallets.end())
		--wallet_it;
            eosio_assert(wallet_it != wallets.end(), "wallet with this number does not exist");
            eosio_assert(win_user.id_rng_trx.size() <= 256, "id_rng_trx has more than 256 bytes");
            eosio_assert(win_user.prize.size() <= 256, "prize has more than 256 bytes");

            winners.push_back( {win_user.number, wallet_it->address_wallet, win_user.id_rng_trx, win_user.prize} );
        }
        return winners;
    };

    uint8_t count_tickets = 0;
    close_lottery close_lottery_table( _self, lottery.value );
    if (close_lottery_table.exists()) {
        auto obj = close_lottery_table.get();

        auto winners = lambda();
        for (const auto winner_obj : winners) {
            auto it = std::find_if(obj.winners_list.begin(), obj.winners_list.end(), [&](const winner &obj_it) {
                return obj_it.id_ticket == winner_obj.id_ticket;
            });

            eosio_assert(it == obj.winners_list.end(), "ERROR");
            obj.winners_list.push_back(winner_obj);
        }

        close_lottery_table.set( obj, _self );
        count_tickets = obj.winners_list.size();

    } else {
        close_lottery_table.set( {time_point_sec(now()), lambda()}, _self );
        count_tickets = close_lottery_table.get().winners_list.size();
    }

    if (count_tickets == COUNT_WINNERS) {
        SEND_INLINE_ACTION( *this, clearwallets, { {_self, "active"_n} }, { lottery } );
        SEND_INLINE_ACTION( *this, clearlottery, { {_self, "active"_n} }, { lottery } );
    }
}

void lottery::cleartickets( name lottery )
{
    require_auth( _self );

    auto i = 0;
    tickets_table tickets(_self, lottery.value); 
    auto ticket_it = tickets.begin();
    while ( ticket_it != tickets.end() ) {
        if (i == 250)
            break;

        ticket_it = tickets.erase(ticket_it);
        ++i;
    }
}

void lottery::clearwallets( name lottery )
{
    require_auth( _self );

    wallets_table wallets(_self, lottery.value);
    auto wallet_it = wallets.begin();
    while ( wallet_it != wallets.end() ) {
        wallet_it = wallets.erase(wallet_it);
    }
}

void lottery::clearlottery( name lottery )
{
    require_auth( _self );

    active_lottery active_lottery_table(_self, _self.value);
    auto it_lottery_name = active_lottery_table.find(lottery.value);
    active_lottery_table.erase(it_lottery_name);
}

void lottery::clearwinners( name lottery )
{
    require_auth( _self );

    close_lottery lottery_table(_self, lottery.value);
    lottery_table.remove();
}

}

EOSIO_DISPATCH( eosio::lottery, (create)(buyticket)(addwallets)(addwalletsr)(apprwint)(apprwinu)(apprwinur)(cleartickets)(clearwallets)(clearlottery)(clearwinners) )
