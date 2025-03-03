/**
 * This is source code under the Apache License 2.0.
 * Original Author: kx@godzilla.dev
 * Original date: March 3, 2025
 */

#ifndef GODZILLA_BINANCE_EXT_TYPE_CONVERT_H
#define GODZILLA_BINANCE_EXT_TYPE_CONVERT_H

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <binapi/enums.hpp>
#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace binance
        {
            using namespace kungfu::wingchun::msg::data;

            inline OrderType from_binance(const binapi::e_type &binance_order_type)
            {
                if (binance_order_type == binapi::e_type::limit)
                    return OrderType::Limit;
                else if (binance_order_type == binapi::e_type::market)
                    return OrderType::Market;
                else
                    return OrderType::UnKnown;
            }

            inline binapi::e_type to_binance(const OrderType &order_type)
            {
                if (order_type == OrderType::Limit)
                    return binapi::e_type::limit;
                else if (order_type == OrderType::Market)
                    return binapi::e_type::market;
                else
                    throw wingchun_error("unknown wingchun order type");
            }

            inline OrderStatus from_binance(const binapi::e_status &status)
            {
                if (status == binapi::e_status::newx) {
                    return OrderStatus::Submitted;
                } else if (status == binapi::e_status::partially_filled) {
                    return OrderStatus::PartialFilledActive;
                } else if (status == binapi::e_status::filled) {
                    return OrderStatus::Filled;
                } else if (status == binapi::e_status::canceled) {
                    return OrderStatus::Cancelled;
                } else if (status == binapi::e_status::pending_cancel) {
                    return OrderStatus::Pending;
                } else if (status == binapi::e_status::rejected) {
                    return OrderStatus::Error;
                } else if (status == binapi::e_status::expired) {
                    return OrderStatus::Error;
                } else {
                    return OrderStatus::Unknown;
                }
            }

            inline Side from_binance(const binapi::e_side &binance_side)
            {
                if (binance_side == binapi::e_side::buy)
                {
                    return Side::Buy;
                } else if (binance_side == binapi::e_side::sell)
                {
                    return Side::Sell;
                }
                else
                {
                    throw wingchun_error("unknown binance side");
                }
            }

            inline binapi::e_side to_binance(const Side &side)
            {
                if (side == Side::Buy)
                {
                    return binapi::e_side::buy;
                } else if (side == Side::Sell)
                {
                    return binapi::e_side::sell;
                }
                else
                {
                    throw wingchun_error("unknown wingchun side");
                }
            }

            inline binapi::e_position_side to_binance(const Direction &direction)
            {
                if (direction == Direction::Long)
                {
                    return binapi::e_position_side::p_long;
                } else if (direction == Direction::Short)
                {
                    return binapi::e_position_side::p_short;
                }
                else
                {
                    throw wingchun_error("unknown wingchun direction");
                }
            }

            inline const std::string to_binance_symbol(const std::string &symbol)
            {
                std::vector<std::string> result;
                std::string res;
                boost::split(result, symbol, boost::is_any_of("_"));
                for (auto &coin: result)
                {
                    std::transform(coin.begin(), coin.end(), coin.begin(), [](unsigned char c){ return std::toupper(c); });
                    res.append(coin);
                }
                return res;
            }

            inline const std::string from_binance_symbol(const std::string &symbol)
            {
                std::vector<std::string> quote_coins{"BTC", "USDT", "BNB", "BUSD", "ETH"};
                std::vector<std::string> result;
                std::string res = symbol;
                bool converted = false;
                for (auto &quote: quote_coins)
                {
                    if (symbol.ends_with(quote))
                    {
                        res.insert(res.rfind(quote), "_");
                        converted = true;
                        break;
                    }
                }
                if (converted)
                    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c){ return std::tolower(c); });
                return res;
            }

        }
    }
}
#endif //GODZILLA_BINANCE_EXT_TYPE_CONVERT_H
