/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#include <utility>
#include <fmt/format.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/strategy/runner.h>
#include <kungfu/wingchun/strategy/metric_controller.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            Runner::Runner(yijinjing::data::locator_ptr locator, const std::string &group, const std::string &name, yijinjing::data::mode m,
                           bool low_latency)
                    : apprentice(location::make(m, category::STRATEGY, group, name, std::move(locator)), low_latency)
            {}

            Context_ptr Runner::make_context()
            {
                return std::make_shared<Context>(*this, events_);
            }

            void Runner::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
            {
                if (context_ and context_->used_account_location(app_location->uid))
                {
                    // std::cout << app_location->uname << ", " << app_location->uid << ", " << static_cast<int>(app_location->category) << std::endl;
                    // std::cout << context_->used_account_location(app_location->uid) << std::endl;
                    throw std::runtime_error("restart due to account[TD] restart");
                }
                apprentice::register_location(trigger_time, app_location);
            }

            void Runner::add_strategy(const Strategy_ptr &strategy, const std::string &path)
            {
                auto uid = yijinjing::util::hash_str_32(path);
                strategy->set_uid(uid);
                strategies_.insert(std::make_pair(uid, strategy));
            }

            void Runner::on_start()
            {
                context_ = make_context();
                context_->react();

                for (const auto &strategy : strategies_)
                {
                    context_->set_current_strategy_index(strategy.first);
                    strategy.second->pre_start(context_);
                }

                events_ | is(msg::type::Depth) |
                $([&](event_ptr event)
                {
                    for (const auto &strategy : strategies_)
                    {
                        context_->set_current_strategy_index(strategy.first);
                        if (context_->is_subscribed("depth", strategy.first, event->data<Depth>())) {
                            strategy.second->on_depth(context_, event->data<Depth>());
                        }
                    }
                });

                events_ | is(msg::type::Ticker) |
                $([&](event_ptr event)
                {
                    for (const auto &strategy : strategies_)
                    {
                        context_->set_current_strategy_index(strategy.first);
                        if (context_->is_subscribed("ticker", strategy.first, event->data<Ticker>())) {
                            strategy.second->on_ticker(context_, event->data<Ticker>());
                        }
                    }
                });

                events_ | is(msg::type::Trade) |
                $([&](event_ptr event)
                {
                    for (const auto &strategy : strategies_)
                    {
                        context_->set_current_strategy_index(strategy.first);
                        if (context_->is_subscribed("trade", strategy.first, event->data<Trade>())) {
                            strategy.second->on_trade(context_, event->data<Trade>());
                        }
                    }
                });

                events_ | is(msg::type::IndexPrice) |
                $([&](event_ptr event)
                {
                    for (const auto &strategy : strategies_)
                    {
                        context_->set_current_strategy_index(strategy.first);
                        if (context_->is_subscribed("index_price", strategy.first, event->data<IndexPrice>())) {
                            strategy.second->on_index_price(context_, event->data<IndexPrice>());
                        }
                    }
                });

                /* 3 */
                // events_ | is(msg::type::Bar) | filter([=](event_ptr event) { return context_->is_subscribed("bar", event->data<Bar>());}) |
                // $([&](event_ptr event)
                //   {
                //       for (const auto &strategy : strategies_)
                //       {
                //           strategy->on_bar(context_, event->data<Bar>());
                //       }
                //   });

                events_ | is(msg::type::Order) | to(context_->app_.get_home_uid()) |
                $([&](event_ptr event)
                {
                    auto order = event->data<Order>();
                    for (const auto &strategy : strategies_)
                    {
                        if (order.strategy_id == strategy.first)
                        {
                            // if ((order.status == OrderStatus::Filled or order.status == OrderStatus::Cancelled) and order.volume_traded * order.avg_price > 0)
                            // {
                            //     MetricController::GetInstance().update_trade_amount(order.strategy_id, order.symbol, order.volume_traded * order.avg_price);
                            // }
                            context_->set_current_strategy_index(strategy.first);
                            strategy.second->on_order(context_, order);
                            break;
                        }
                    }
                });

                // events_ | is(msg::type::OrderActionError) | to(context_->app_.get_home_uid()) |
                // $([&](event_ptr event)
                // {
                //     for (const auto &strategy : strategies_ | indexed(0))
                //     {
                //         if (context_->is_subscribed("order", strategy.first, event->data<Order>())) {
                //             strategy.value()->on_order_action_error(context_, event->data<OrderActionError>());
                //         }
                //     }
                // });

                events_ | is(msg::type::MyTrade) | to(context_->app_.get_home_uid()) |
                $([&](event_ptr event)
                {
                    auto myTrade = event->data<MyTrade>();
                    auto itr = strategies_.find(myTrade.strategy_id);
                    if (itr!= strategies_.end()) {
                        context_->set_current_strategy_index(itr->first);
                        itr->second->on_transaction(context_, myTrade);
                    }
                });

                events_ | is(msg::type::Position) |
                $([&](event_ptr event)
                {
                    auto position = event->data<Position>();
                    for (const auto &strategy : strategies_)
                    {
                        context_->set_current_strategy_index(strategy.first);
                        strategy.second->on_position(context_, position);
                    }
                });

                events_ | is(msg::type::UnionResponse) | to(context_->app_.get_home_uid()) |
                $([&](event_ptr event)
                {
                    nlohmann::json sub_msg = nlohmann::json::parse(event->data_as_string());
                    auto itr = strategies_.find(sub_msg["strategy_id"]);
                    if (itr!= strategies_.end()) {
                        context_->set_current_strategy_index(itr->first);
                        itr->second->on_union_response(context_, sub_msg.dump());
                    }
                });

                apprentice::on_start();

                for (const auto &strategy : strategies_)
                {
                    context_->set_current_strategy_index(strategy.first);
                    strategy.second->post_start(context_);
                }
            }

            void Runner::on_exit()
            {
                for (const auto &strategy : strategies_)
                {
                    context_->set_current_strategy_index(strategy.first);
                    strategy.second->pre_stop(context_);
                }

                apprentice::on_exit();

                for (const auto &strategy : strategies_)
                {
                    context_->set_current_strategy_index(strategy.first);
                    strategy.second->post_stop(context_);
                }
            }

        }
    }
}
