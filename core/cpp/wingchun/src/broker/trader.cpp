/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#include <utility>

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/trader.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace broker
        {
            Trader::Trader(bool low_latency, locator_ptr locator, const std::string &source, const std::string &account_id) :
                    apprentice(location::make(mode::LIVE, category::TD, source, account_id, std::move(locator)), low_latency),
                    source_(source), account_id_(account_id)
            {
                log::copy_log_settings(get_io_device()->get_home(), account_id);
            }

            void Trader::on_start()
            {
                apprentice::on_start();

                events_ | is(msg::type::OrderInput) |
                $([&](event_ptr event)
                  {
                      SPDLOG_DEBUG("insert_order in trader");
                      insert_order(event);
                  });

                events_ | is(msg::type::AdjustLeverage) |
                $([&](event_ptr event)
                  {
                      adjust_leverage(event);
                  });

                events_ | is(msg::type::MergePosition) |
                $([&](event_ptr event)
                  {
                      merge_position(event);
                  });

                events_ | is(msg::type::QueryPosition) |
                $([&](event_ptr event)
                  {
                      query_position(event);
                  });

                events_ | is(msg::type::OrderAction) |
                $([&](event_ptr event)
                  {
                      const auto& action = event->data<OrderAction>();
                      if (action.action_flag == OrderActionFlag::Cancel)
                      {
                        SPDLOG_DEBUG("cancel order in trader");
                        cancel_order(event);
                      }
                      else
                      {
                        SPDLOG_DEBUG("query order in trader");
                        query_order(event);
                      }
                  });
            }
        }
    }
}
