/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#ifndef WINGCHUN_TRADER_H
#define WINGCHUN_TRADER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace broker
        {
            class Trader : public practice::apprentice
            {
            public:
                explicit Trader(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source, const std::string &account_id);

                virtual ~Trader() = default;

                const std::string &get_account_id() const
                { return account_id_; }

                const std::string &get_source() const
                { return source_; }

                virtual AccountType get_account_type() const = 0;

                virtual bool insert_order(const yijinjing::event_ptr &event) = 0;

                virtual bool cancel_order(const yijinjing::event_ptr &event) = 0;

                virtual bool query_order(const yijinjing::event_ptr &event) = 0;

                virtual bool adjust_leverage(const yijinjing::event_ptr &event) {
                    boost::ignore_unused(event);
                    throw wingchun_error(fmt::format("not implemented"));
                }

                virtual bool merge_position(const yijinjing::event_ptr &event) {
                    boost::ignore_unused(event);
                    throw wingchun_error(fmt::format("not implemented"));
                }

                virtual bool query_position(const yijinjing::event_ptr &event) {
                    boost::ignore_unused(event);
                    throw wingchun_error(fmt::format("not implemented"));
                }

                virtual bool req_position() = 0;

                virtual bool req_account() = 0;

                virtual void on_start() override;

            protected:

                void publish_state(msg::data::BrokerState state)
                {
                    auto s = static_cast<int32_t>(state);
                    write_to(0, msg::type::BrokerState, s);
                }

            private:
                std::string source_;
                std::string account_id_;
            };
        }
    }
}

#endif //WINGCHUN_TRADER_H
