/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#include <memory>
#include <list>
#include <map>
#include <boost/pool/object_pool.hpp>
#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/strategy/metric_controller.h>

#define HARD_LIMIT_AMOUNT_5MINUTES 4000000  //USDT
#define HARD_LIMIT_ORDER_AMOUNT 1000000  //USDT

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            template<typename T>
            bool QuotaEnforcer<T>::check_trade_quota(uint32_t sid)
            {
                auto iter = stats.find(sid);
                if (iter == stats.end()) {
                    return true;
                }
                double value = iter->second;
                return (value < HARD_LIMIT_AMOUNT_5MINUTES);
            }

            template<typename T>
            bool QuotaEnforcer<T>::update_data(uint32_t sid, double value)
            {
                auto now = kungfu::yijinjing::time::now_in_nano();
                clean_up_data(sid, now);
                T* obj = static_cast<T*>(my_pool.construct());
                obj->time = now;
                obj->value = value;
                data.push_back(obj);
                stats[sid] += value;
                return true;
            }

            bool MetricController::update_trade_amount(uint32_t sid, const std::string &symbol, double value)
            {
                return trades_.update_data(sid, value * _get_multiplier(symbol));
            }

            bool MetricController::check_trade_amount(uint32_t sid)
            {
                return trades_.check_trade_quota(sid);
            }

            bool MetricController::check_order_amount(const std::string &symbol, double value)
            {
                return value < HARD_LIMIT_ORDER_AMOUNT;
            }

            MetricController* MetricController::instance_ = nullptr;
            std::mutex MetricController::mutex_;
        }
    }
}

