/**
 * This is source code under the Apache License 2.0.
 * Original Author: kx@godzilla.dev
 * Original date: March 3, 2025
 */

#ifndef GODZILLA_BINANCE_EXT_TRADER_H
#define GODZILLA_BINANCE_EXT_TRADER_H

#include <iostream>
#include <boost/asio/io_context.hpp>
#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <binapi/websocket.hpp>
#include <kungfu/wingchun/broker/trader.h>
#include <binapi/api.hpp>

#include "common.h"
#include "order_mapper.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace binance
        {
            class TraderBinance : public broker::Trader
            {
            public:
                TraderBinance(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &account_id, const std::string &json_config);

                ~TraderBinance() override;

                AccountType get_account_type() const override
                { return AccountType::Stock; }

                bool insert_order(const yijinjing::event_ptr &event) override;

                bool query_order(const yijinjing::event_ptr &event) override;

                bool cancel_order(const yijinjing::event_ptr &event) override;

                bool req_position() override;

                bool req_account() override;

                bool adjust_leverage(const yijinjing::event_ptr &event);

            protected:

                void on_start() override;
                void _check_status(kungfu::yijinjing::event_ptr);
                void _start_userdata(const InstrumentType type);

            private:
                Configuration config_;
                std::string get_runtime_folder() const;
                boost::asio::io_context ioctx_;
                std::shared_ptr<std::thread> task_thread_;
                std::shared_ptr<binapi::rest::api> rest_ptr_;
                std::shared_ptr<binapi::rest::api> frest_ptr_;
                std::shared_ptr<binapi::ws::websockets> ws_ptr_;
                std::shared_ptr<binapi::ws::websockets> fws_ptr_;
                std::map<std::size_t, order_map_record> order_data_;
                std::list<std::string> listenKeys;
                std::mutex order_mtx_;
            };
        }
    }
}
#endif //GODZILLA_BINANCE_EXT_TRADER_H
