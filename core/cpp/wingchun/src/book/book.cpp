/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#include <kungfu/wingchun/book/book.h>
#include <kungfu/yijinjing/log/setup.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace book
        {
            BookContext::BookContext(practice::apprentice &app, const rx::connectable_observable<yijinjing::event_ptr> &events):
                app_(app), events_(events)
            {
                auto home = app.get_io_device()->get_home();
                log::copy_log_settings(home, home->name);
                this->monitor_instruments();
                service_location_ = location::make(mode::LIVE, category::SYSTEM, "service", "ledger", app.get_io_device()->get_home()->locator);
            }

            const msg::data::Instrument& BookContext::get_inst_info(const std::string &symbol, const std::string &exchange_id) const
            {
                auto id = yijinjing::util::hash_str_32(std::string(symbol).append(exchange_id));
                if (this->instruments_.find(id) == this->instruments_.end())
                {
                    throw wingchun_error(fmt::format("no instrument info found for {}", symbol));
                }
                return this->instruments_.at(id);
            }

            std::vector<msg::data::Instrument> BookContext::all_inst_info() const
            {
                std::vector<msg::data::Instrument> res(instruments_.size());
                transform(instruments_.begin(), instruments_.end(), res.begin(), [](auto kv){return kv.second;});
                return res;
            }

            void BookContext::monitor_instruments()
            {
                auto insts = events_ | to(app_.get_live_home_uid()) | take_while([&](event_ptr event) {
                    return event->msg_type() != msg::type::InstrumentEnd;
                }) | is(msg::type::Instrument) | reduce(std::vector<Instrument>(),
                        [](std::vector<Instrument> res, event_ptr event)
                        {
                            res.push_back(event->data<msg::data::Instrument>());
                            return res;
                        }) | as_dynamic();

                insts.subscribe([&](std::vector<Instrument> res)
                {
                    if (! this->app_.is_live()) { return;}
                    SPDLOG_INFO("instrument info updated, size: {}", res.size());
                    this->instruments_.clear();
                    for (const auto& inst: res)
                    {
                        auto id = yijinjing::util::hash_str_32(std::string(inst.symbol).append(inst.exchange_id));
                        this->instruments_[id] = inst;
                    }
                    this->monitor_instruments();
                });
            }

            void BookContext::pop_book(uint32_t location_uid)
            {
                books_.erase(location_uid);
            }

            void BookContext::add_book(const yijinjing::data::location_ptr& location, const Book_ptr& book)
            {
                if(location->category != category::TD and location->category != category::STRATEGY)
                {
                    throw wingchun_error(fmt::format("invalid book location category: {}", get_category_name(location->category)));
                }

                books_[location->uid].insert(book);

                // this->monitor_positions(location, book);

                for(const auto& item: app_.get_channels())
                {
                    const auto &channel = item.second;
                    if (channel.source_id == location->uid)
                    {
                        std::string dest_uname = app_.has_location(channel.dest_id) ? app_.get_location(
                                channel.dest_id)->uname : "unknown";
                        app_.get_reader()->join(location, channel.dest_id, app_.now());
                        SPDLOG_INFO("book {} [{:08x}] read {} from {} [{:08x}] to {} [{:08x}]",
                                    location->uname, location->uid, location->category == category::TD ? "order/trades": "input",
                                    location->uname, location->uid, dest_uname, channel.dest_id);
                    }
                    else if(channel.dest_id == location->uid)
                    {
                        if (app_.has_location(channel.source_id))
                        {
                            auto src_location = app_.get_location(channel.source_id);
                            app_.get_reader()->join(src_location, channel.dest_id, app_.now());
                            SPDLOG_INFO("book {} [{:08x}] read {} from {} [{:08x}] to {} [{:08x}]",
                                        location->uname, location->uid, location->category == category::TD ? "input": "order/trades",
                                        src_location->uname,src_location->uid, location->uname, location->uid);
                        }
                        else
                        {
                            SPDLOG_WARN("location {} not exist", channel.source_id);
                        }
                    }
                }

                events_ | is(yijinjing::msg::type::Channel) |
                $([=](event_ptr event)
                {
                    const auto& channel = event->data<yijinjing::msg::data::Channel>();
                    if (channel.source_id == location->uid)
                    {
                        std::string dest_uname = app_.has_location(channel.dest_id) ? app_.get_location(
                                channel.dest_id)->uname : "unknown";
                        app_.get_reader()->join(location, channel.dest_id, app_.now());
                        SPDLOG_INFO("book {} [{:08x}] read {} from {} [{:08x}] to {} [{:08x}]",
                                    location->uname, location->uid, location->category == category::TD ? "order/trades": "input",
                                    location->uname, location->uid, dest_uname,channel.dest_id);
                    }
                    else if(channel.dest_id == location->uid)
                    {
                        if (app_.has_location(channel.source_id))
                        {
                            auto src_location = app_.get_location(channel.source_id);
                            app_.get_reader()->join(src_location, channel.dest_id, app_.now());
                            SPDLOG_INFO("book {} [{:08x}] read {} from {} [{:08x}] to {} [{:08x}]",
                                        location->uname, location->uid, location->category == category::TD ? "input": "order/trades",
                                        src_location->uname, src_location->uid, location->uname, location->uid);
                        }
                        else
                        {
                            SPDLOG_WARN("location {} not exist", channel.source_id);
                        }
                    }
                });

                events_ | is(msg::type::Depth) |
                $([=](event_ptr event)
                {
                    try
                    {
                        book->on_depth(event, event->data<msg::data::Depth>());
                    }
                    catch (const std::exception &e)
                    {
                        SPDLOG_ERROR("Unexpected exception {}", e.what());
                    }
                });

                events_ | is(msg::type::Position) | filter([=](yijinjing::event_ptr e)
                {
                    return location->category == category::TD ? location->uid == e->source() : location->uid == e->dest();
                }) |
                $([=](event_ptr event)
                {
                    try
                    {
                        book->on_position(event, event->data<Position>());
                    }
                    catch (const std::exception &e)
                    {
                        SPDLOG_ERROR("Unexpected exception {}", e.what());
                    }
                });

                events_ | is(msg::type::MyTrade) | filter([=](yijinjing::event_ptr e)
                {
                    return location->category == category::TD ? location->uid == e->source() : location->uid == e->dest();
                }) |
                $([=](event_ptr event)
                {
                    try
                    {
                        book->on_trade(event, event->data<MyTrade>());
                    }
                    catch (const std::exception &e)
                    {
                        SPDLOG_ERROR("Unexpected exception {}", e.what());
                    }
                });

                events_ | is(msg::type::Order) | filter([=](yijinjing::event_ptr e)
                {
                    return location->category == category::TD ? location->uid == e->source() : location->uid == e->dest();
                }) |
                $([=](event_ptr event)
                  {
                      try
                      {
                          book->on_order(event, event->data<Order>());
                      }
                      catch (const std::exception &e)
                      {
                          SPDLOG_ERROR("Unexpected exception {}", e.what());
                      }
                  });

                // events_ | is(msg::type::OrderInput) | filter([=](yijinjing::event_ptr e)
                // {
                //     return location->category == category::TD ? location->uid == e->dest() : location->uid == e->source();
                // }) |
                // $([=](event_ptr event)
                //   {
                //       try
                //       {
                //           const auto& data = event->data<OrderInput>();
                //           book->on_order_input(event, event->data<OrderInput>());
                //       }
                //       catch (const std::exception &e)
                //       {
                //           SPDLOG_ERROR("Unexpected exception {}", e.what());
                //       }
                //   });

                events_ | is(msg::type::Asset) | filter([=](yijinjing::event_ptr e)
                {
                    const auto& asset = e->data<Asset>();
                    SPDLOG_DEBUG("Asset event from: {} in {}", asset.holder_uid, location->uid);
                    return asset.holder_uid == location->uid;
                })  | $([=](event_ptr event)
                {
                    try
                    {
                        book->on_asset(event, event->data<Asset>());
                    }
                    catch (const std::exception &e)
                    {
                        SPDLOG_ERROR("Unexpected exception {}", e.what());
                    }
                });

                auto home = app_.get_io_device()->get_home();
                if (home->uid != service_location_->uid)
                {
                    if (home->mode == mode::LIVE and not app_.has_location(service_location_->uid))
                    {
                        throw wingchun_error("has no location for ledger service");
                    }

                    if (not app_.has_writer(service_location_->uid))
                    {
                        events_ | is(yijinjing::msg::type::RequestWriteTo) |
                        filter([=](yijinjing::event_ptr e)
                               {
                                   const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                                   return data.dest_id == service_location_->uid;
                               }) | first() |
                        $([=](event_ptr e)
                          {
                              nlohmann::json location_desc{};
                              location_desc["mode"] = location->mode;
                              location_desc["category"] = location->category;
                              location_desc["group"] = location->group;
                              location_desc["name"] = location->name;
                              location_desc["uname"] = location->uname;
                              location_desc["uid"] = location->uid;
                              auto msg = location_desc.dump();
                              auto writer = app_.get_writer(service_location_->uid);
                              auto &&frame = writer->open_frame(0, msg::type::QryAsset, msg.length());
                              memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                              writer->close_frame(msg.length());
                              SPDLOG_INFO("{}[{:08x}] asset requested", location->uname, location->uid);
                          });
                    } else
                    {
                        nlohmann::json location_desc{};
                        location_desc["mode"] = location->mode;
                        location_desc["category"] = location->category;
                        location_desc["group"] = location->group;
                        location_desc["name"] = location->name;
                        location_desc["uname"] = location->uname;
                        location_desc["uid"] = location->uid;
                        auto msg = location_desc.dump();
                        auto writer = app_.get_writer(service_location_->uid);
                        auto &&frame = writer->open_frame(0, msg::type::QryAsset, msg.length());
                        memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                        writer->close_frame(msg.length());
                        SPDLOG_INFO("{}[{:08x}] asset requested", location->uname, location->uid);
                    }

                    app_.request_write_to(app_.now(), service_location_->uid);
                    app_.request_read_from(app_.now(), service_location_->uid);
                }
            }
        }
    }
}