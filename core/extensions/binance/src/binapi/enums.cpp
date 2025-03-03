
// ----------------------------------------------------------------------------
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
//
// This file is part of binapi(https://github.com/niXman/binapi) project.
//
// Copyright (c) 2019-2021 niXman (github dot nixman dog pm.me). All rights reserved.
// ----------------------------------------------------------------------------

#include <binapi/enums.hpp>
#include <binapi/fnv1a.hpp>

#include <cassert>

namespace binapi {

/*************************************************************************************************/

e_side e_side_from_string(const char *str) {
    const auto hash = fnv1a(str);
    switch ( hash ) {
        case fnv1a("BUY"): return e_side::buy;
        case fnv1a("SELL"): return e_side::sell;
    }

    assert(!"unreachable");
}

const char* e_side_to_string(e_side side) {
    switch ( side ) {
        case e_side::buy: return "BUY";
        case e_side::sell: return "SELL";
    }

    assert(!"unreachable");

    return nullptr;
}

/*************************************************************************************************/

e_position_side e_position_side_from_string(const char *str) {
    const auto hash = fnv1a(str);
    switch ( hash ) {
        case fnv1a("LONG"): return e_position_side::p_long;
        case fnv1a("SHORT"): return e_position_side::p_short;
    }

    assert(!"unreachable");
}
const char* e_position_side_to_string(e_position_side side) {
    switch ( side ) {
        case e_position_side::p_long: return "LONG";
        case e_position_side::p_short: return "SHORT";
    }

    assert(!"unreachable");

    return nullptr;
}

/*************************************************************************************************/

e_status e_status_from_string(const char *str) {
    const auto hash = fnv1a(str);
    switch ( hash ) {
        case fnv1a("NEW"): return e_status::newx;
        case fnv1a("PARTIALLY_FILLED"): return e_status::partially_filled;
        case fnv1a("FILLED"): return e_status::filled;
        case fnv1a("CANCELED"): return e_status::canceled;
        case fnv1a("PENDING_CANCEL"): return e_status::pending_cancel;
        case fnv1a("REJECTED"): return e_status::rejected;
        case fnv1a("EXPIRED"): return e_status::expired;
    }

    assert(!"unreachable");
}

const char* e_status_to_string(e_status status) {
    switch ( status ) {
        case e_status::newx: return "NEW";
        case e_status::partially_filled: return "PARTIALLY_FILLED";
        case e_status::filled: return "FILLED";
        case e_status::canceled: return "CANCELED";
        case e_status::pending_cancel: return "PENDING_CANCEL";
        case e_status::rejected: return "REJECTED";
        case e_status::expired: return "EXPIRED";
    }

    assert(!"unreachable");

    return nullptr;
}

/*************************************************************************************************/

e_type e_type_from_string(const char *str) {
    const auto hash = fnv1a(str);
    switch ( hash ) {
        case fnv1a("LIMIT"): return e_type::limit;
        case fnv1a("MARKET"): return e_type::market;
        case fnv1a("STOP_LOSS"): return e_type::stop_loss;
        case fnv1a("STOP_LOSS_LIMIT"): return e_type::stop_loss_limit;
        case fnv1a("TAKE_PROFIT"): return e_type::take_profit;
        case fnv1a("TAKE_PROFIT_LIMIT"): return e_type::take_profit_limit;
        case fnv1a("LIMIT_MAKER"): return e_type::limit_maker;
    }

    assert(!"unreachable");
}

const char* e_type_to_string(e_type type) {
    switch ( type ) {
        case e_type::limit: return "LIMIT";
        case e_type::market: return "MARKET";
        case e_type::stop_loss: return "STOP_LOSS";
        case e_type::stop_loss_limit: return "STOP_LOSS_LIMIT";
        case e_type::take_profit: return "TAKE_PROFIT";
        case e_type::take_profit_limit: return "TAKE_PROFIT_LIMIT";
        case e_type::limit_maker: return "LIMIT_MAKER";
    }

    assert(!"unreachable");

    return nullptr;
}

/*************************************************************************************************/

e_time e_time_from_string(const char *str) {
    const auto hash = fnv1a(str);
    switch ( hash ) {
        case fnv1a("GTC"): return e_time::GTC;
        case fnv1a("IOC"): return e_time::IOC;
        case fnv1a("FOK"): return e_time::FOK;
    }

    assert(!"unreachable");
}

const char* e_time_to_string(e_time time) {
    switch ( time ) {
        case e_time::GTC: return "GTC";
        case e_time::IOC: return "IOC";
        case e_time::FOK: return "FOK";
    }

    assert(!"unreachable");

    return nullptr;
}

/*************************************************************************************************/
e_trade_resp_type e_trade_resp_type_from_string(const char *str) {
    const auto hash = fnv1a(str);
    switch ( hash ) {
        case fnv1a("ACK"): return e_trade_resp_type::ACK;
        case fnv1a("RESULT"): return e_trade_resp_type::RESULT;
        case fnv1a("FULL"): return e_trade_resp_type::FULL;
        case fnv1a("TEST"): return e_trade_resp_type::TEST;
        case fnv1a("UNKNOWN"): return e_trade_resp_type::UNKNOWN;
    }

    assert(!"unreachable");
}

const char* e_trade_resp_type_to_string(e_trade_resp_type resp) {
    switch ( resp ) {
        case e_trade_resp_type::ACK: return "ASK";
        case e_trade_resp_type::RESULT: return "RESULT";
        case e_trade_resp_type::FULL: return "FULL";
        case e_trade_resp_type::TEST: return "TEST";
        case e_trade_resp_type::UNKNOWN: return "UNKNOWN";
    }

    assert(!"unreachable");

    return nullptr;
}

/*************************************************************************************************/

} // ns binapi
