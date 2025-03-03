/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#include <kungfu/wingchun/service/bar.h>
#include <regex>
#include <kungfu/wingchun/utils.h>
#include <kungfu/yijinjing/log/setup.h>

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::rx;

namespace kungfu
{
    namespace wingchun
    {
        namespace service
        {
            static int64_t parse_time_interval(const std::string &s)
            {
                std::regex r("[0-9]+");
                std::smatch m;
                std::regex_search(s, m, r);
                if (m.empty())
                {
                    throw std::runtime_error("invalid time interval: " + s);
                }
                int n = std::stoi(m[0]);
                if (endswith(s, "s"))
                {
                    return n * time_unit::NANOSECONDS_PER_SECOND;
                }
                else if (endswith(s, "m"))
                {
                    return n * time_unit::NANOSECONDS_PER_MINUTE;
                }
                else if (endswith(s, "h"))
                {
                    return n * time_unit::NANOSECONDS_PER_HOUR;
                }
                else if (endswith(s, "d"))
                {
                    return n * time_unit::NANOSECONDS_PER_DAY;
                }
                else
                {
                    throw std::runtime_error("invalid time_interval: " + s);
                }
            }

            BarGenerator::BarGenerator(locator_ptr locator, mode m, bool low_latency, const std::string &json_config) : practice::apprentice(location::make(m, category::SYSTEM, "service", "bar", std::move(locator)), low_latency),
                                                                                                                        time_interval_(kungfu::yijinjing::time_unit::NANOSECONDS_PER_MINUTE)
            {
                log::copy_log_settings(get_io_device()->get_home(), "bar");
                auto config = nlohmann::json::parse(json_config);
                std::string source = config["source"];
                auto home = get_io_device()->get_home();
                source_location_ = location::make(mode::LIVE, category::MD, source, source, home->locator);
                if (config.find("time_interval") != config.end())
                {
                    time_interval_ = parse_time_interval(config["time_interval"]);
                }
            }

            bool BarGenerator::subscribe(const std::vector<Instrument> &instruments)
            {
                if (not has_writer(source_location_->uid))
                {
                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
                        filter([=](yijinjing::event_ptr e)
                               {
                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                               return data.dest_id == source_location_->uid; }) |
                        first() |
                        $([=](event_ptr e)
                          {
                          for (const auto& inst: instruments)
                          {
                              auto symbol_id = get_symbol_id(inst.get_symbol(), inst.get_exchange_id());
                              if(bars_.find(symbol_id) ==  bars_.end())
                              {
                                //   write_subscribe_msg(get_writer(source_location_->uid), 0, inst.get_exchange_id(), inst.get_symbol(), "bar");
                                  auto now_in_nano = now();
                                  auto start_time = now_in_nano - now_in_nano % time_interval_ + time_interval_;
                                  SPDLOG_INFO("subscribe {}.{}", inst.symbol, inst.exchange_id);
                                  Bar bar = {};
                                  strncpy(bar.symbol, inst.symbol, SYMBOL_LEN);
                                  strncpy(bar.exchange_id, inst.exchange_id, EXCHANGE_ID_LEN);
                                  bar.start_time = start_time;
                                  bar.end_time = start_time + time_interval_;
                                  bars_[symbol_id] = bar;
                              }
                          } });
                }
                else
                {
                    for (const auto &inst : instruments)
                    {
                        auto symbol_id = get_symbol_id(inst.get_symbol(), inst.get_exchange_id());
                        if (bars_.find(symbol_id) == bars_.end())
                        {
                            // write_subscribe_msg(get_writer(source_location_->uid), 0, inst.get_exchange_id(), inst.get_symbol(), "bar");
                            auto now_in_nano = now();
                            auto start_time = now_in_nano - now_in_nano % time_interval_ + time_interval_;
                            SPDLOG_INFO("subscribe {}.{}", inst.symbol, inst.exchange_id);
                            Bar bar = {};
                            strncpy(bar.symbol, inst.symbol, SYMBOL_LEN);
                            strncpy(bar.exchange_id, inst.exchange_id, EXCHANGE_ID_LEN);
                            bar.start_time = start_time;
                            bar.end_time = start_time + time_interval_;
                            bars_[symbol_id] = bar;
                        }
                    }
                }
                return true;
            }

            void BarGenerator::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location)
            {
                if (has_location(location->uid))
                {
                    return;
                }
                apprentice::register_location(trigger_time, location);
                if (location->uid == source_location_->uid)
                {
                    request_read_from(now(), source_location_->uid, true);
                    request_write_to(now(), source_location_->uid);
                    SPDLOG_INFO("added md {} [{:08x}]", source_location_->uname, source_location_->uid);
                }
            }

            void BarGenerator::on_start()
            {
                apprentice::on_start();

                events_ | is(msg::type::Trade) |
                    $([&](event_ptr event)
                      {
                    const auto& trade = event->data<Trade>();
                    auto symbol_id = get_symbol_id(trade.get_symbol(), trade.get_exchange_id());
                    if (bars_.find(symbol_id) != bars_.end())
                    {
                        SPDLOG_TRACE("{}.{} at {} vol {} price {}", trade.symbol, trade.exchange_id, time::strftime(trade.trade_time), trade.volume, trade.price);
                        auto& bar = bars_[symbol_id];
                        if (trade.trade_time >= bar.start_time && trade.trade_time <= bar.end_time)
                        {
                            if(bar.trade_count == 0)
                            {
                                bar.high = trade.price;
                                bar.low = trade.price;
                                bar.open = trade.price;
                                bar.close = trade.price;
                                bar.start_volume = trade.volume;
                            }
                            bar.trade_count++;
                            bar.volume = trade.volume - bar.start_volume;
                            bar.high = std::max(bar.high, trade.price);
                            bar.low = std::min(bar.low, trade.price);
                            bar.close = trade.price;
                        }
                        if (trade.trade_time >= bar.end_time)
                        {
                            this->get_writer(0)->write(0, msg::type::Bar, bar);
                            SPDLOG_INFO("{}.{} [o:{},c:{},h:{},l:{}] from {} to {}",bar.symbol, bar.exchange_id,
                                    bar.open, bar.close, bar.high, bar.low, time::strftime(bar.start_time), time::strftime(bar.end_time));
                            bar.start_time = bar.end_time;
                            while(bar.start_time + time_interval_ < trade.trade_time)
                            {
                                bar.start_time += time_interval_;
                            }
                            bar.end_time = bar.start_time + time_interval_;
                            if (bar.start_time <= trade.trade_time)
                            {
                                bar.trade_count = 1;
                                bar.start_volume = trade.volume;
                                bar.volume = 0;
                                bar.high = trade.price;
                                bar.low = trade.price;
                                bar.open = trade.price;
                                bar.close = trade.price;
                            }
                            else
                            {
                                bar.trade_count = 0;
                                bar.start_volume = 0;
                                bar.volume = 0;
                                bar.high = 0;
                                bar.low = 0;
                                bar.open = 0;
                                bar.close = 0;
                            }
                        }
                    } });

                events_ | is(msg::type::Subscribe) |
                    $([&](event_ptr event)
                      {
                      std::vector<Instrument> symbols;
                      auto json_str = event->data_as_string();
                      SPDLOG_TRACE("subscribe bar: {}", json_str);
                      nlohmann::json sub_msg = nlohmann::json::parse(json_str);
                      std::string symbol = sub_msg["symbol"];
                      std::string exchange = sub_msg["exchange"];
                      std::string sub_type = sub_msg["sub_type"];
                      Instrument instrument{};
                      strcpy(instrument.symbol, symbol.c_str());
                      strcpy(instrument.exchange_id, exchange.c_str());
                      symbols.push_back(instrument);
                      if (sub_type == "bar") {
                        subscribe(symbols);
                      } });
            }

        }
    }
}
