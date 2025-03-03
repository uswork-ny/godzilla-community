/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#ifndef WINGCHUN_COMMON_H
#define WINGCHUN_COMMON_H

#include <cmath>
#include <string>
#include <fmt/format.h>
#include <kungfu/yijinjing/util/util.h>

#define REGION_CN "CN"
#define REGION_HK "HK"

#define INST_TYPE_SPOT "SPOT"
#define INST_TYPE_FUTURE "FUTURE"
#define INST_TYPE_MARGIN "MARGIN"

#define EXCHANGE_BINANCE "binance"
#define EXCHANGE_XT "xt"
#define EXCHANGE_KUCOIN "kucoin"
#define EXCHANGE_GATE "gate"
#define EXCHANGE_MEXC "mexc"
#define EXCHANGE_OKX "okx"
#define EXCHANGE_BYBIT "bybit"
#define EXCHANGE_COINW "coinw"

#define SOURCE_SIM "sim"
#define SOURCE_XT "xt"
#define SOURCE_XTC "xtc"
#define SOURCE_XTS "xts"
#define SOURCE_BINANCE "binance"
#define SOURCE_BINANCEPM "binancepm"
#define SOURCE_KUCOIN "kucoin"
#define SOURCE_GATE "gatec"
#define SOURCE_MEXC "mexc"
#define SOURCE_OKX "okx"
#define SOURCE_BYBIT "bybit"
#define SOURCE_CPT "cpt"

#define EPSILON (1e-6)
#define DOUBLEMAX (1e16) // 一亿亿, 2018年A股总市值不到50万亿

namespace kungfu
{
    namespace wingchun
    {
        const int ORDER_ID_LEN = 32;
        const int SYMBOL_LEN = 32;
        const int PRODUCT_ID_LEN = 32;
        const int DATE_LEN = 9;
        const int EXCHANGE_ID_LEN = 16;
        const int ACCOUNT_ID_LEN = 32;
        const int CLIENT_ID_LEN = 32;
        const int EXEC_ID_LEN = 32;
        const int SOURCE_ID_LEN = 16;
        const int BROKER_ID_LEN = 32;
        const int ERROR_MSG_LEN = 128;
        const int ERROR_CODE_LEN = 16;

        enum class InstrumentType : int8_t
        {
            Unknown, // 未知
            FFuture, // U本位合约
            DFuture, // 币本位合约
            Future,  // 交割合约
            Etf,     // ETF
            Spot,    // 现货
            Index,   // 指数
            Swap     // 永续合约
        };

        enum class ExecType : int8_t
        {
            Unknown,
            Cancel,
            Trade
        };

        enum class BsFlag : int8_t
        {
            Unknown,
            Buy,
            Sell
        };

        enum class Side : int8_t
        {
            Buy,
            Sell,
            Lock,
            Unlock,
            Exec,
            Drop
        };

        enum class Offset : int8_t
        {
            Open,
            Close,
            CloseToday,
            CloseYesterday
        };

        enum class OrderActionFlag : int8_t
        {
            Cancel,
            Query
        };

        enum class OrderType : int8_t
        {
            Limit,       // 限价
            Market,      // 市价
            Mock,        // 自成交
            UnKnown
        };

        enum class VolumeCondition : int8_t
        {
            Any,
            Min,
            All
        };

        enum class TimeCondition : int8_t
        {
            IOC,
            FOK,
            GTC,
            GTX,
            POC
        };

        enum class OrderStatus : int8_t
        {
            Unknown,
            Submitted,
            Pending,
            Cancelled,
            Error,
            Filled,
            PartialFilledNotActive,
            PartialFilledActive,
            PreSend
        };

        enum class Direction : int8_t
        {
            Long,
            Short
        };

        enum class AccountType : int8_t
        {
            Stock,
            Credit,
            Future
        };

        enum class CommissionRateMode : int8_t
        {
            ByAmount,
            ByVolume
        };

        enum class LedgerCategory : int8_t
        {
            Account,
            Strategy,
        };

        class wingchun_error : public std::runtime_error
        {
        public:
            explicit wingchun_error(const std::string &__s) : std::runtime_error(__s)
            {
            }

            explicit wingchun_error(const char *__s) : std::runtime_error(__s)
            {
            }

            virtual ~wingchun_error() noexcept = default;
        };

        inline bool is_greater(double x, double y)
        {
            return std::isgreater(x - y, EPSILON);
        }

        inline bool is_less(double x, double y)
        {
            return std::isless(x - y, EPSILON * -1);
        }

        inline bool is_equal(double x, double y)
        {
            return std::abs(x - y) <= EPSILON * std::abs(x);
        }

        inline bool is_greater_equal(double x, double y)
        {
            return is_greater(x, y) || is_equal(x, y);
        }

        inline bool is_less_equal(double x, double y)
        {
            return is_less(x, y) || is_equal(x, y);
        }

        inline bool is_zero(double x)
        {
            return is_equal(x, 0.0);
        }

        inline bool is_too_large(double x)
        {
            return is_greater(x, DOUBLEMAX);
        }

        inline bool is_valid_price(double price)
        {
            return !is_less_equal(price, 0.0) && !is_too_large(price);
        }

        inline double rounded(double x, int n)
        {
            if (is_too_large(x) || is_zero(x) || is_too_large(std::abs(x)))
            {
                return 0.0;
            }
            else
            {
                char out[64];
                double xrounded;
                sprintf(out, "%.*f", n, x);
                xrounded = strtod(out, 0);
                return xrounded;
            }
        }

        inline bool string_equals(const std::string &s1, const std::string &s2)
        {
            return std::strcmp(s1.c_str(), s2.c_str()) == 0;
        }

        inline bool string_equals_n(const std::string &s1, const std::string &s2, size_t l)
        {
            return std::strncmp(s1.c_str(), s2.c_str(), l) == 0;
        }

        inline bool endswith(const std::string &str, const std::string &suffix)
        {
            return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
        }

        inline bool startswith(const std::string &str, const std::string &prefix)
        {
            return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
        }

        inline void to_upper(std::string &data)
        {
            std::for_each(data.begin(), data.end(), [](char &c)
                          { c = ::toupper(c); });
        }

        inline std::string to_upper_copy(const std::string &data)
        {
            std::string rtn = data;
            to_upper(rtn);
            return rtn;
        }

        inline void to_lower(std::string &data)
        {
            std::for_each(data.begin(), data.end(), [](char &c)
                          { c = ::tolower(c); });
        }

        inline std::string to_lower_copy(const std::string &data)
        {
            std::string rtn = data;
            to_lower(rtn);
            return rtn;
        }

        inline bool is_final_status(const OrderStatus &status)
        {
            switch (status)
            {
            case OrderStatus::Submitted:
            case OrderStatus::Pending:
            case OrderStatus::PartialFilledActive:
            case OrderStatus::Unknown:
            case OrderStatus::PreSend:
                return false;
            default:
                return true;
            }
        }

        inline std::string get_shm_db() {
            return std::string("/dev/shm/godzilla_runtime.db");
        }

        inline std::string str_from_instrument_type(InstrumentType type)
        {
            switch (type)
            {
            case InstrumentType::Unknown:
                return "Unknown";
            case InstrumentType::FFuture:
                return "FFuture";
            case InstrumentType::Future:
                return "Future";
            case InstrumentType::DFuture:
                return "DFuture";
            case InstrumentType::Swap:
                return "SWAP";
            case InstrumentType::Spot:
                return "SPOT";
            case InstrumentType::Index:
                return "Index";
            case InstrumentType::Etf:
                return "ETF";
            default:
                return "Unknown";
            }
        }

        inline Direction get_future_direction(Side side, Offset offset)
        {
            if ((side == Side::Buy && offset == Offset::Open) || (side == Side::Sell && offset != Offset::Open))
            {
                return Direction::Long;
            }
            else
            {
                return Direction::Short;
            }
        }

        inline uint32_t get_symbol_id(const std::string &symbol, const std::string &exchange)
        {
            return yijinjing::util::hash_str_32(symbol) ^ yijinjing::util::hash_str_32(exchange);
        }

        inline uint32_t get_symbol_id(const std::string &symbol, const std::string &sub_type, InstrumentType type, const std::string &exchange, uint32_t index)
        {
            if (sub_type == "order") {
                return yijinjing::util::hash_str_32(std::to_string(index));
            } else {
                return yijinjing::util::hash_str_32(symbol) \
                    ^ yijinjing::util::hash_str_32(sub_type) \
                    ^ yijinjing::util::hash_str_32(std::to_string(static_cast<int>(type))) \
                    ^ yijinjing::util::hash_str_32(exchange) \
                    ^ yijinjing::util::hash_str_32(std::to_string(index));
            }
        }
    }
}

#endif // WINGCHUN_COMMON_H
