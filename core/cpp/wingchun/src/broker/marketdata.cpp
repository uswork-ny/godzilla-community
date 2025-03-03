/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#include <utility>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/marketdata.h>

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
            MarketData::MarketData(bool low_latency, locator_ptr locator, const std::string &source) :
                    apprentice(location::make(mode::LIVE, category::MD, source, source, std::move(locator)), low_latency)
            {
                log::copy_log_settings(get_io_device()->get_home(), source);
            }

            void MarketData::on_start()
            {
                apprentice::on_start();

                events_ | is(msg::type::SubscribeAll) |
                $([&](event_ptr event)
                {
                    SPDLOG_INFO("subscribe all request");
                    subscribe_all();
                });

                events_ | is(msg::type::Subscribe) |
                $([&](event_ptr event)
                  {
                      std::vector<Instrument> symbols;
                      auto json_str = event->data_as_string();
                      SPDLOG_TRACE("subscribe request: {}", json_str);
                      nlohmann::json sub_msg = nlohmann::json::parse(json_str);
                      std::string symbol = sub_msg["symbol"];
                      std::string exchange = sub_msg["exchange"];
                      std::string sub_type = sub_msg["sub_type"];
                      InstrumentType inst_type = static_cast<InstrumentType>(sub_msg["inst_type"].get<int>());
                      Instrument instrument{};
                      strcpy(instrument.symbol, symbol.c_str());
                      strcpy(instrument.exchange_id, exchange.c_str());
                      instrument.instrument_type = inst_type;
                      symbols.push_back(instrument);
                      if (sub_type == "depth") {
                        subscribe(symbols);
                      } else if (sub_type == "trade") {
                        subscribe_trade(symbols);
                      } else if (sub_type == "ticker") {
                        subscribe_ticker(symbols);
                      } else if (sub_type == "index_price") {
                        subscribe_index_price(symbols);
                      } else {
                        SPDLOG_WARN("unknown sub_type: {}", sub_type);
                      }

                  });

                events_ | is(msg::type::Unsubscribe) |
                $([&](event_ptr event)
                  {
                      std::vector<Instrument> symbols;
                      auto json_str = event->data_as_string();
                      SPDLOG_TRACE("unsubscribe request: {}", json_str);
                      nlohmann::json sub_msg = nlohmann::json::parse(json_str);
                      std::string symbol = sub_msg["symbol"];
                      std::string exchange = sub_msg["exchange"];
                      std::string sub_type = sub_msg["sub_type"];
                      InstrumentType inst_type = static_cast<InstrumentType>(sub_msg["inst_type"].get<int>());
                      Instrument instrument{};
                      strcpy(instrument.symbol, symbol.c_str());
                      strcpy(instrument.exchange_id, exchange.c_str());
                      instrument.instrument_type = inst_type;
                      symbols.push_back(instrument);
                      if (sub_type == "depth") {
                        unsubscribe(symbols);
//                      } else if (sub_type == "trade") {
//                        subscribe_trade(symbols);
//                      } else if (sub_type == "ticker") {
//                        subscribe_ticker(symbols);
//                      } else if (sub_type == "index_price") {
//                        subscribe_index_price(symbols);
                      } else {
                        SPDLOG_WARN("unknown sub_type: {}", sub_type);
                      }

                  });
            }
        }
    }
}
