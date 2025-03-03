/**
 * This is source code under the Apache License 2.0.
 * Original Author: kx@godzilla.dev
 * Original date: March 3, 2025
 */

#ifndef GODZILLA_BINANCE_EXT_MARKET_DATA_H
#define GODZILLA_BINANCE_EXT_MARKET_DATA_H

#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <list>
#include <boost/asio/io_context.hpp>
#include <binapi/websocket.hpp>
#include <binapi/api.hpp>
#include <binapi/double_type.hpp>

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/marketdata.h>

#include "common.h"

namespace kungfu {
    namespace wingchun {
        namespace binance {
            struct OrderBook
            {
                struct level_t
                {
                    binapi::double_type price;
                    binapi::double_type amount;

                    friend std::ostream& operator<<(std::ostream& os, const level_t& s) {
                        os << "price: " << s.price << ", amount: " << s.amount;
                        return os;
                    }

                    bool operator<(const struct level_t& r) const {
                        return this->price < r.price;
                    }
                };

                struct greater {
                    bool operator () (const level_t& l, const level_t& r) const {
                        return l.price > r.price;
                    }
                };

                std::string symbol;
                std::size_t lastUpdateId;
                std::set<level_t, greater> bids;
                std::set<level_t> asks;

                // static OrderBook construct(const flatjson::fjson &json);
                friend std::ostream& operator<<(std::ostream& os, const OrderBook& book) {
                    os << "orderbook: " << book.symbol << ", lastUpdateId: " << book.lastUpdateId << std::endl;
                    os << "asks: " << std::endl;
                    for (auto& a : book.asks) {
                        os << a << std::endl;
                    }
                    os << "bids: " << std::endl;
                    for (auto& b : book.bids) {
                        os << b << std::endl;
                    }
                    return os;
                }

                void merge() {}
            };

            struct ChannelInfo
            {
                std::string symbol;
                std::string sub_type;
                InstrumentType inst_type;
                binapi::ws::websockets::handle handle;
                bool ready;
            };

            class MarketDataBinance: public broker::MarketData
            {
            public:
                MarketDataBinance(bool low_latency, yijinjing::data::locator_ptr locator, const std::string& json_config);

                ~MarketDataBinance() override;

                bool subscribe(const std::vector<wingchun::msg::data::Instrument>& instruments) override;

                bool subscribe_trade(const std::vector<wingchun::msg::data::Instrument>& instruments) override;

                bool subscribe_ticker(const std::vector<wingchun::msg::data::Instrument>& instruments) override;

                bool subscribe_index_price(const std::vector<wingchun::msg::data::Instrument> &instruments) override;

                bool subscribe_all() override;

                bool unsubscribe(const std::vector<wingchun::msg::data::Instrument>& instruments) override;

            protected:

                void on_start() override;
                void _check_status(kungfu::yijinjing::event_ptr);

            private:
                Configuration config_;
                std::string get_runtime_folder() const;
                boost::asio::io_context ioctx_;
                std::shared_ptr<binapi::ws::websockets> ws_ptr_;
                std::shared_ptr<binapi::ws::websockets> fws_ptr_;
                std::shared_ptr<binapi::ws::websockets> dws_ptr_;
                std::shared_ptr<std::thread> task_thread_;
                std::shared_ptr<binapi::rest::api> rest_ptr_;
                std::shared_ptr<binapi::rest::api> frest_ptr_;
                std::map<std::string, OrderBook> depths_cache_;
                std::list<binapi::ws::diff_depths_t> diff_depths_cache_;
                std::map<uint32_t, ChannelInfo> channel_cache_;
            };
        }
    }
}

#endif //GODZILLA_BINANCE_EXT_MARKET_DATA_H
