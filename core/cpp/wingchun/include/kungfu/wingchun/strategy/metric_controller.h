/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#ifndef WINGCHUN_METRIC_CONTROLLER_H
#define WINGCHUN_METRIC_CONTROLLER_H

#include <memory>
#include <list>
#include <map>
#include <boost/pool/object_pool.hpp>

#define TIME_PERIOD_CONTROL_5MINUTES kungfu::yijinjing::time_unit::NANOSECONDS_PER_MINUTE * 5

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            struct MetricTradeAmount {
                int64_t time;
                double value;
            };

            template<typename T>
            struct QuotaEnforcer {
                QuotaEnforcer() = default;

                ~QuotaEnforcer()
                {
                    for (auto &t: data)
                        my_pool.destroy(t);
                }

                bool check_trade_quota(uint32_t sid);

                bool update_data(uint32_t sid, double value);

                void clean_up_data(uint32_t sid, int64_t now)
                {
                    while (true)
                    {
                        if (data.size() == 0) break;
                        auto it = data.front();
                        if (now - it->time > TIME_PERIOD_CONTROL_5MINUTES)
                        {
                            stats[sid] -= it->value;
                            my_pool.destroy(it);
                            data.pop_front();
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                std::list<T *> data;
                std::map<std::size_t, double> stats;
                boost::object_pool<T> my_pool;
            };

            class MetricController : public std::enable_shared_from_this<MetricController>
            {
            public:
                static MetricController& GetInstance()
                {
                    if (instance_ == nullptr) {
                        std::lock_guard<std::mutex> lock(mutex_);
                        if (instance_ == nullptr)
                        {
                            instance_ = new MetricController();
                        }
                    }
                    return *instance_;
                }

                virtual ~MetricController() = default;

            public:

                bool update_trade_amount(uint32_t sid, const std::string &symbol, double value);
                bool check_trade_amount(uint32_t sid);
                bool check_order_amount(const std::string &symbol, double value);

            private:
                MetricController() = default;
                int _get_multiplier(const std::string &symbol) {
                    if (symbol.find("_btc") != std::string::npos) return 30000;
                    else if (symbol.find("_eth") != std::string::npos) return 3000;
                    else if (symbol.find("_xt") != std::string::npos) return 3;
                    else return 1;
                }

            private:
                QuotaEnforcer<MetricTradeAmount> trades_;

                static MetricController* instance_;
                static std::mutex mutex_;
            };
        }
    }
}

#endif //WINGCHUN_METRIC_CONTROLLER_H
