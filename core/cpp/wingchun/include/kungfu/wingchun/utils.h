/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#ifndef KUNGFU_WINGCHUN_UTILS_H
#define KUNGFU_WINGCHUN_UTILS_H

#include <spdlog/spdlog.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/common.h>

using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        inline void write_subscribe_msg(const yijinjing::journal::writer_ptr &writer, int64_t trigger_time,
                const std::string &sub_type, const std::string &exchange, const std::string &symbol, InstrumentType inst_type)
        {
            nlohmann::json data{};
            data["symbol"] = symbol;
            data["exchange"] = exchange;
            data["sub_type"] = sub_type;
            data["inst_type"] = static_cast<int>(inst_type);
            auto msg = data.dump();
            auto &&frame = writer->open_frame(trigger_time, msg::type::Subscribe, msg.length());
            memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
            writer->close_frame(msg.length());
            SPDLOG_TRACE("written subscribe msg for {}/{}@{}:{}", symbol, data["inst_type"].get<int>(), exchange, sub_type);
        }

        inline void write_unsubscribe_msg(const yijinjing::journal::writer_ptr &writer, int64_t trigger_time,
                const std::string &sub_type, const std::string &exchange, const std::string &symbol, InstrumentType inst_type)
        {
            nlohmann::json data{};
            data["symbol"] = symbol;
            data["exchange"] = exchange;
            data["sub_type"] = sub_type;
            data["inst_type"] = static_cast<int>(inst_type);
            auto msg = data.dump();
            auto &&frame = writer->open_frame(trigger_time, msg::type::Unsubscribe, msg.length());
            memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
            writer->close_frame(msg.length());
            SPDLOG_TRACE("written unsubscribe msg for {}/{}@{}:{}", symbol, data["inst_type"].get<int>(), exchange, sub_type);
        }

        inline void write_union_response(const yijinjing::journal::writer_ptr &writer, int64_t trigger_time, const std::string &msg) {
            auto &&frame = writer->open_frame(trigger_time, msg::type::UnionResponse, msg.length());
            memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
            writer->close_frame(msg.length());
            SPDLOG_TRACE("written union response: {}", msg);
        }

        inline double calc_amount_by_symbol(const std::string &symbol, double price, double volume) {
            double amount = price * volume;
            if (yijinjing::util::ends_with(symbol, "_btc")) amount *= 30000;
            else if (yijinjing::util::ends_with(symbol, "_xt")) amount *= 3;
            else if (yijinjing::util::ends_with(symbol, "_eth")) amount *= 2000;
            else if (yijinjing::util::ends_with(symbol, "_bnb")) amount *= 400;
            return amount;
        }
    }
}

#endif //KUNGFU_WINGCHUN_UTILS_H
