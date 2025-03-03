/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#include <fmt/format.h>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/msg.h>
#include <kungfu/wingchun/strategy/metric_controller.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/utils.h>

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
            Context::Context(practice::apprentice &app, const rx::connectable_observable<yijinjing::event_ptr> &events) :
                app_(app), events_(events), subscribe_all_(false)
            {
                auto home = app.get_io_device()->get_home();
                log::copy_log_settings(home, home->name);
                book_context_ = std::make_shared<book::BookContext>(app, events);
                algo_context_ = std::make_shared<algo::AlgoContext>(app, events);
            }

            void Context::react()
            {
                algo_context_->react();

                // events_ | is(msg::type::Order) | to(app_.get_home_uid()) |
                // $([&](event_ptr event)
                //   {
                //       auto order = event->data<Order>();
                //   });

                // events_ | is(msg::type::MyTrade) |
                // $([&](event_ptr event)
                //   {
                //       auto transaction = event->data<MyTrade>();
                //   });

                // subscribe_instruments();

                auto home = app_.get_io_device()->get_home();
                auto ledger_location = location::make(mode::LIVE, category::SYSTEM, "service", "ledger", home->locator);
                app_.request_write_to(app_.now(), ledger_location->uid);
                app_.request_read_from(app_.now(), ledger_location->uid);
            }

            int64_t Context::now() const
            {
                return app_.now();
            }

            void Context::add_timer(int64_t nanotime, const std::function<void(yijinjing::event_ptr)>& callback)
            {
                auto strategy_id = get_current_strategy_index();
                app_.add_timer(nanotime, [this, callback, strategy_id](yijinjing::event_ptr e){
                    auto prev_strategy_id = get_current_strategy_index();
                    set_current_strategy_index(strategy_id);
                    callback(e);
                    set_current_strategy_index(prev_strategy_id);
                    });
            }

            void Context::add_time_interval(int64_t duration, const std::function<void(yijinjing::event_ptr)>& callback)
            {
                auto strategy_id = get_current_strategy_index();
                app_.add_time_interval(duration, [this, callback, strategy_id](yijinjing::event_ptr e){
                    auto prev_strategy_id = get_current_strategy_index();
                    set_current_strategy_index(strategy_id);
                    callback(e);
                    set_current_strategy_index(prev_strategy_id);
                    });
            }

            void Context::add_account(const std::string &source, const std::string &account)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                if (accounts_.find(account_id) != accounts_.end())
                {
                    // throw wingchun_error(fmt::format("duplicated account {}@{}", account, source));
                    SPDLOG_INFO("duplicated account added {}@{} [{:08x}]", account, source, account_id);
                    return;
                }

                auto home = app_.get_io_device()->get_home();
                auto account_location = location::make(mode::LIVE, category::TD, source, account, home->locator);
                if (home->mode == mode::LIVE and not app_.has_location(account_location->uid))
                {
                    throw wingchun_error(fmt::format("invalid account {}@{}", account, source));
                }

                accounts_[account_id] = account_location;
                account_location_ids_[account_id] = account_location->uid;

                app_.request_write_to(app_.now(), account_location->uid);
                app_.request_read_from(app_.now(), account_location->uid);

                SPDLOG_INFO("added account {}@{} [{:08x}]", account, source, account_id);
            }

            bool Context::used_account_location(uint32_t uid)
            {
                for (auto &it: account_location_ids_)
                {
                    if (it.second == uid) return true;
                }
                return false;
            }

            std::vector<yijinjing::data::location_ptr> Context::list_accounts()
            {
                std::vector<yijinjing::data::location_ptr> acc_locations;
                for (auto item : accounts_)
                {
                    acc_locations.push_back(item.second);
                }
                return acc_locations;
            }

            void Context::set_account_cash_limit(const std::string &account, const std::string &coin, double limit)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                account_cash_limits_[account_id][coin] = limit;
            }

            double Context::get_account_cash_limit(const std::string &account, const std::string &coin)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                auto account_itr = account_cash_limits_.find(account_id);
                if (account_itr == account_cash_limits_.end())
                {
                    throw wingchun_error(fmt::format("invalid account {}", account));
                }
                auto coin_itr = account_itr->second.find(coin);
                if (coin_itr == account_itr->second.end())
                {
                    throw wingchun_error(fmt::format("invalid coin {} of {}", coin.c_str(), account));
                }
                return coin_itr->second;
            }

            void Context::subscribe_instruments()
            {
                auto home = app_.get_io_device()->get_home();
                auto ledger_location = location::make(mode::LIVE, category::SYSTEM, "service", "ledger", home->locator);
                if (home->mode == mode::LIVE and not app_.has_location(ledger_location->uid))
                {
                    throw wingchun_error("has no location for ledger service");
                }
                if (not app_.has_writer(ledger_location->uid))
                {
                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
                    filter([=](yijinjing::event_ptr e)
                           {
                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                               return data.dest_id == ledger_location->uid;
                           }) | first() |
                           $([=](event_ptr e)
                           {
                               auto writer = app_.get_writer(ledger_location->uid);
                               writer->mark(0, msg::type::InstrumentRequest);
                               SPDLOG_INFO("instrument requested");
                           });
                } else
                {
                    auto writer = app_.get_writer(ledger_location->uid);
                    writer->mark(0, msg::type::InstrumentRequest);
                    SPDLOG_INFO("instrument requested");
                }
            }

            uint32_t Context::add_marketdata(const std::string &source)
            {
                if (market_data_.find(source) == market_data_.end())
                {
                    auto home = app_.get_io_device()->get_home();
                    auto md_location = source == "bar" ? location::make(mode::LIVE, category::SYSTEM, "service", source, home->locator) :
                                       location::make(mode::LIVE, category::MD, source, source, home->locator);
                    if (home->mode == mode::LIVE and not app_.has_location(md_location->uid))
                    {
                        throw wingchun_error(fmt::format("invalid md {}", source));
                    }
                    app_.request_read_from(app_.now(), md_location->uid, true);
                    app_.request_write_to(app_.now(), md_location->uid);
                    market_data_[source] = md_location->uid;
                    SPDLOG_INFO("added md {} [{:08x}]", source, md_location->uid);
                }
                return market_data_[source];
            }

            void Context::adjust_leverage(const std::string &account, const std::string &symbol, Direction &position_side, const int leverage)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                nlohmann::json data{};
                data["symbol"] = symbol;
                data["position_side"] = static_cast<int>(position_side);
                data["leverage"] = leverage;
                data["strategy_uid"] = get_current_strategy_index();
                auto msg = data.dump();
                auto &&frame = writer->open_frame(app_.now(), msg::type::AdjustLeverage, msg.length());
                memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                writer->close_frame(msg.length());
                SPDLOG_TRACE("written adjust leverage msg for {}/{}@{}:{}", symbol, static_cast<int>(position_side), account, leverage);
            }

            void Context::merge_positions(const std::string &account, const std::string &symbol, const int amount, InstrumentType inst_type)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                nlohmann::json data{};
                data["strategy_uid"] = get_current_strategy_index();
                data["symbol"] = symbol;
                data["amount"] = amount;
                data["account"] = account;
                data["instrument_type"] = static_cast<int>(inst_type);
                auto msg = data.dump();
                auto &&frame = writer->open_frame(app_.now(), msg::type::MergePosition, msg.length());
                memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                writer->close_frame(msg.length());
                SPDLOG_TRACE("written merge position msg for {}@{}:{}", symbol, account, amount);
            }

            void Context::subscribe_all(const std::string &source)
            {
                subscribe_all_ = true;
                auto md_source = add_marketdata(source);
                SPDLOG_INFO("strategy subscribe all from {} [{:08x}]", source, md_source);
                if (not app_.has_writer(md_source))
                {
                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
                    filter([=](yijinjing::event_ptr e)
                           {
                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                               return data.dest_id == md_source;
                           }) | first() |
                    $([=](event_ptr e)
                      {
                            auto writer = app_.get_writer(md_source);
                            writer->mark(0, msg::type::SubscribeAll);
                      });
                } else
                {
                    auto writer = app_.get_writer(md_source);
                    writer->mark(0, msg::type::SubscribeAll);
                }
            }

            void Context::_subscribe(const std::string &sub_type, const std::string &source, const std::vector<std::string> &symbols, InstrumentType inst_type, const std::string &exchange)
            {
                for (const auto& symbol: symbols)
                {
                    SPDLOG_TRACE("_subscribe: instrument_type({}), strategy_id({})", static_cast<int>(inst_type), current_strategy_idx);
                    auto symbol_id = get_symbol_id(symbol, sub_type, inst_type, exchange, current_strategy_idx);
                    subscribed_symbols_[symbol_id] = symbol_id;
                }

                auto md_source = add_marketdata(source);
                SPDLOG_INFO("strategy subscribe {} from {} [{:08x}]", sub_type, source, md_source);
                if (not app_.has_writer(md_source))
                {
                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
                    filter([=](yijinjing::event_ptr e)
                           {
                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                               return data.dest_id == md_source;
                           }) | first() |
                    $([=](event_ptr e)
                      {
                          request_subscribe(md_source, symbols, exchange, sub_type, inst_type);
                      });
                } else
                {
                    request_subscribe(md_source, symbols, exchange, sub_type, inst_type);
                }
            }

            void Context::_unsubscribe(const std::string &sub_type, const std::string &source, const std::vector<std::string> &symbols, InstrumentType inst_type, const std::string &exchange)
            {
                for (const auto& symbol: symbols)
                {
                    SPDLOG_TRACE("_unsubscribe: instrument_type({}), strategy_id({})", static_cast<int>(inst_type), current_strategy_idx);
                    auto symbol_id = get_symbol_id(symbol, sub_type, inst_type, exchange, current_strategy_idx);
                    subscribed_symbols_[symbol_id] = symbol_id;
                }

                auto md_source = add_marketdata(source);
                SPDLOG_INFO("strategy unsubscribe {} from {} [{:08x}]", sub_type, source, md_source);
//                if (not app_.has_writer(md_source))
//                {
//                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
//                    filter([=](yijinjing::event_ptr e)
//                           {
//                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
//                               return data.dest_id == md_source;
//                           }) | first() |
//                    $([=](event_ptr e)
//                      {
//                          request_subscribe(md_source, symbols, exchange, sub_type, inst_type);
//                      });
//                } else
//                {
                    request_unsubscribe(md_source, symbols, exchange, sub_type, inst_type);
//                }
            }

            void Context::subscribe(const std::string &source, const std::vector<std::string> &symbols, InstrumentType inst_type, const std::string &exchange)
            {
                _subscribe("depth", source, symbols, inst_type, exchange);
            }
            void Context::unsubscribe(const std::string &source, const std::vector<std::string> &symbols, InstrumentType inst_type, const std::string &exchange)
            {
                _unsubscribe("depth", source, symbols, inst_type, exchange);
            }
            void Context::subscribe_trade(const std::string &source, const std::vector<std::string> &symbols, InstrumentType inst_type, const std::string &exchange)
            {
                _subscribe("trade", source, symbols, inst_type, exchange);
            }


            void Context::subscribe_ticker(const std::string &source, const std::vector<std::string> &symbols, InstrumentType inst_type, const std::string &exchange)
            {
                _subscribe("ticker", source, symbols, inst_type, exchange);
            }

            void Context::subscribe_index_price(const std::string &source, const std::vector<std::string> &symbols, InstrumentType inst_type, const std::string &exchange)
            {
                if (inst_type != InstrumentType::FFuture) return;
                _subscribe("index_price", source, symbols, inst_type, exchange);
            }

            void Context::request_subscribe(uint32_t source, const std::vector<std::string> &symbols, const std::string &exchange, const std::string &sub_type, InstrumentType inst_type)
            {
                for (const auto &symbol : symbols)
                {
                    write_subscribe_msg(app_.get_writer(source), app_.now(), sub_type, exchange, symbol, inst_type);
                }
            }

            void Context::request_unsubscribe(uint32_t source, const std::vector<std::string> &symbols, const std::string &exchange, const std::string &sub_type, InstrumentType inst_type)
            {
                for (const auto &symbol : symbols)
                {
                    write_unsubscribe_msg(app_.get_writer(source), app_.now(), sub_type, exchange, symbol, inst_type);
                }
            }

            uint64_t Context::insert_order(
                const std::string &symbol,
                InstrumentType inst_type,
                const std::string &exchange,
                const std::string &account,
                double limit_price,
                double volume,
                OrderType type,
                Side side,
                TimeCondition time,
                Direction position_side,
                bool reduce_only)
            {
                //auto &controller = MetricController::GetInstance();
                //if (not controller.check_order_amount(symbol, limit_price * volume) /* or not controller.check_trade_amount(current_strategy_idx) */)
                //{
                //    SPDLOG_CRITICAL("Invalid order blocked by hard limit: (symbol){}, (amount){}", symbol, limit_price*volume);
                //    return 0;
                //}
                auto symbol_id = get_symbol_id(symbol, "order", inst_type, exchange, current_strategy_idx);
                subscribed_symbols_[symbol_id] = symbol_id;
                auto writer = app_.get_writer(lookup_account_location_id(account));
                SPDLOG_DEBUG("{:08x} insert order with account location {:08x}", app_.get_home_uid(), lookup_account_location_id(account));
                msg::data::OrderInput &input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);
                input.strategy_id = current_strategy_idx;
                input.order_id = writer->current_frame_uid();
                strcpy(input.symbol, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.instrument_type = inst_type;
                input.price = limit_price;
                input.stop_price = limit_price;
                input.volume = volume;
                input.order_type = type;
                input.time_condition = time;
                input.side = side;
                input.position_side = position_side;
                input.reduce_only = reduce_only;
                writer->close_data();
                SPDLOG_TRACE("insert order with reduce_only {}@{}", symbol, reduce_only);
                return input.order_id;
            }

            uint64_t Context::query_order(const std::string &account, uint64_t order_id, std::string &ex_order_id, InstrumentType inst_type, const std::string &symbol)
            {
                auto symbol_id = get_symbol_id(symbol, "order", InstrumentType::Spot, "", current_strategy_idx);
                subscribed_symbols_[symbol_id] = symbol_id;
                auto writer = app_.get_writer(lookup_account_location_id(account));
                SPDLOG_DEBUG("{:08x} query order {:016x} with account location {:08x}", app_.get_home_uid(), order_id, lookup_account_location_id(account));
                msg::data::OrderAction &action = writer->open_data<msg::data::OrderAction>(0, msg::type::OrderAction);
                action.set_symbol(symbol);
                action.order_action_id = writer->current_frame_uid();
                action.strategy_id = current_strategy_idx;
                action.order_id = order_id;
                action.instrument_type = inst_type;
                action.set_ex_order_id(ex_order_id);
                action.action_flag = OrderActionFlag::Query;
                writer->close_data();
                return action.order_action_id;
            }

            bool Context::query_positions(const std::string &account, const std::string &symbol, InstrumentType inst_type) {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                nlohmann::json data{};
                data["strategy_uid"] = get_current_strategy_index();
                data["symbol"] = symbol;
                data["account"] = account;
                data["instrument_type"] = static_cast<int>(inst_type);
                auto msg = data.dump();
                auto &&frame = writer->open_frame(app_.now(), msg::type::QueryPosition, msg.length());
                memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                writer->close_frame(msg.length());
                SPDLOG_TRACE("written query position msg for {}@{}", symbol, account);
                return true;
            }

            uint64_t Context::cancel_order(const std::string &account, uint64_t order_id, std::string &symbol, std::string &ex_order_id, InstrumentType inst_type)
            {
                auto symbol_id = get_symbol_id("", "order", InstrumentType::Spot, "", current_strategy_idx);
                subscribed_symbols_[symbol_id] = symbol_id;
                auto writer = app_.get_writer(lookup_account_location_id(account));
                SPDLOG_DEBUG("{:08x} cancel order {} with account account {}", app_.get_home_uid(), order_id, account);
                // auto writer = app_.get_writer(account_location_id);
                msg::data::OrderAction &action = writer->open_data<msg::data::OrderAction>(0, msg::type::OrderAction);

                action.order_action_id = writer->current_frame_uid();
                action.strategy_id = current_strategy_idx;
                action.order_id = order_id;
                action.set_symbol(symbol);
                action.set_ex_order_id(ex_order_id);
                action.instrument_type = inst_type;
                action.action_flag = OrderActionFlag::Cancel;
                writer->close_data();
                return action.order_action_id;
            }

            uint32_t Context::lookup_account_location_id(const std::string &account)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                if (account_location_ids_.find(account_id) == account_location_ids_.end())
                {
                    throw wingchun_error("invalid account " + account);
                }
                return account_location_ids_[account_id];
            }

            //! replaced with db query
            const std::string Context::get_market_info(const std::string &symbol, const std::string &exchange)
            {
                return "";
            }
        }
    }
}
