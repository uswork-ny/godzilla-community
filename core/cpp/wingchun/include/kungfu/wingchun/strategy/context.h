/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#ifndef WINGCHUN_CONTEXT_H
#define WINGCHUN_CONTEXT_H

#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/wingchun/book/book.h>
#include <kungfu/wingchun/algo/algo.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            class Runner;

            class Context : public std::enable_shared_from_this<Context>
            {
            public:
                explicit Context(practice::apprentice &app, const rx::connectable_observable<yijinjing::event_ptr> &events);

                ~Context() = default;

                // 获取当前纳秒时间
                //@return            当前纳秒时间
                int64_t now() const;

                virtual void add_timer(int64_t nanotime, const std::function<void(yijinjing::event_ptr)> &callback);

                virtual void add_time_interval(int64_t duration, const std::function<void(yijinjing::event_ptr)> &callback);

                inline uint32_t get_current_strategy_index() {
                    return current_strategy_idx;
                }

                inline void set_current_strategy_index(uint32_t index) {
                    current_strategy_idx = index;
                }

                // 添加策略使用的交易账户
                //@param source_id   柜台ID
                //@param account_id  账户ID
                virtual void add_account(const std::string &source, const std::string &account);

                // 获取交易账户列表
                virtual std::vector<yijinjing::data::location_ptr> list_accounts();

                // 设置交易账户可用资金限制
                //@param account_id  账户ID
                //@param coin 资产名称
                //@param limit 可用资金限制
                virtual void set_account_cash_limit(const std::string &account, const std::string &coin, double limit);

                // 获取交易账户可用资金限制
                //@param account_id  账户ID
                virtual double get_account_cash_limit(const std::string &account, const std::string &coin);

                // 订阅行情
                //@param source_id   柜台ID
                //@param instruments 合约列表
                //@param exchange_id 交易所ID
                virtual void subscribe(const std::string &source, const std::vector<std::string> &instruments, InstrumentType inst_type = InstrumentType::Spot, const std::string &exchange = "");
                virtual void unsubscribe(const std::string &source, const std::vector<std::string> &instruments, InstrumentType inst_type = InstrumentType::Spot, const std::string &exchange = "");

                // 订阅成交
                //@param source_id   柜台ID
                //@param instruments 合约列表
                //@param exchange_id 交易所ID
                virtual void subscribe_trade(const std::string &source, const std::vector<std::string> &instruments, InstrumentType inst_type = InstrumentType::Spot, const std::string &exchange = "");

                // 订阅实时ticker
                //@param source_id   柜台ID
                //@param instruments 合约列表
                //@param exchange_id 交易所ID
                virtual void subscribe_ticker(const std::string &source, const std::vector<std::string> &instruments, InstrumentType inst_type = InstrumentType::Spot, const std::string &exchange = "");

                // 订阅实时指数价格
                virtual void subscribe_index_price(const std::string &source, const std::vector<std::string> &instruments, InstrumentType inst_type = InstrumentType::Spot, const std::string &exchange = "");

                virtual void adjust_leverage(const std::string &account, const std::string &symbol, Direction &position_side, const int leverage);
                virtual void merge_positions(const std::string &account, const std::string &symbol, const int amount, InstrumentType inst_type = InstrumentType::FFuture);
                virtual bool query_positions(const std::string &account, const std::string &symbol, InstrumentType inst_type = InstrumentType::FFuture);
                virtual void subscribe_all(const std::string &source);

                // 报单
                //@param symbol        交易对名称
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param type          报单价格类型
                //@param side          买卖方向
                //@param time          TimeCondition
                //@param position_side Direction
                //@param reduce_only   只减仓
                //@return              订单ID
                virtual uint64_t insert_order(
                    const std::string &symbol,
                    const InstrumentType inst_type,
                    const std::string &exchange,
                    const std::string &account,
                    double limit_price,
                    double volume,
                    OrderType type,
                    Side side,
                    TimeCondition time = TimeCondition::GTC,
                    Direction position_side = Direction::Long,
                    bool reduce_only = false);

                // 撤单
                //@param order_id      订单ID
                //@return              撤单操作ID
                virtual uint64_t cancel_order(const std::string &account, uint64_t order_id,
                    std::string &symbol, std::string &ex_order_id, InstrumentType inst_type=InstrumentType::Spot);

                virtual uint64_t query_order(const std::string &account, uint64_t order_id, std::string &ex_order_id, InstrumentType inst_type, const std::string &symbol={});

                virtual const std::string get_market_info(const std::string &symbol, const std::string &exchange);

                virtual book::BookContext_ptr get_book_context() const { return book_context_; };

                virtual algo::AlgoContext_ptr get_algo_context() const { return algo_context_; }

            protected:
                virtual void react();

                practice::apprentice &app_;
                const rx::connectable_observable<yijinjing::event_ptr> &events_;

            private:
                book::BookContext_ptr book_context_;
                algo::AlgoContext_ptr algo_context_;

                uint32_t lookup_account_location_id(const std::string &account);
                bool used_account_location(uint32_t uid);

                void _subscribe(
                    const std::string &sub_type,
                    const std::string &source, const std::vector<std::string> &instruments,
                    InstrumentType inst_type = InstrumentType::Spot, const std::string &exchange = "");
                void _unsubscribe(
                    const std::string &sub_type,
                    const std::string &source, const std::vector<std::string> &instruments,
                    InstrumentType inst_type = InstrumentType::Spot, const std::string &exchange = "");

                void subscribe_instruments();

                uint32_t add_marketdata(const std::string &source);

                void request_subscribe(uint32_t source, const std::vector<std::string> &symbols, const std::string &exchange, const std::string &sub_type, InstrumentType inst_type=InstrumentType::Spot);
                void request_unsubscribe(uint32_t source, const std::vector<std::string> &symbols, const std::string &exchange, const std::string &sub_type, InstrumentType inst_type=InstrumentType::Spot);

                template <class T>
                bool is_subscribed(const std::string &sub_type, uint32_t hash_id, const T &data)
                {
                    // std::cout << sub_type << "," << static_cast<int>(data.instrument_type) << "," << data.exchange_id << "," << hash_id << std::endl;
                    if (subscribe_all_) {
                        return true;
                    } else {
                        auto symbol_id = get_symbol_id(data.symbol, sub_type, data.instrument_type, data.exchange_id, hash_id);
                        return subscribed_symbols_.find(symbol_id) != subscribed_symbols_.end();
                    }
                }

            private:
                std::unordered_map<uint32_t, uint32_t> account_location_ids_;
                std::unordered_map<uint32_t, yijinjing::data::location_ptr> accounts_;
                std::unordered_map<uint32_t, std::unordered_map<std::string, double>> account_cash_limits_;
                std::unordered_map<std::string, uint32_t> market_data_;
                bool subscribe_all_;
                std::unordered_map<uint32_t, uint32_t> subscribed_symbols_;
                uint32_t current_strategy_idx;
                std::unordered_map<uint32_t, std::vector<msg::data::Bar>> bars_;

                friend class Runner;
            };
        }
    }
}

#endif // WINGCHUN_CONTEXT_H
