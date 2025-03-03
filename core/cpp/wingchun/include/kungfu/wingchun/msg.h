/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#ifndef WINGCHUN_EVENT_H
#define WINGCHUN_EVENT_H

#include <cinttypes>
#include <cmath>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <kungfu/wingchun/common.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace msg
        {
            namespace type
            {
                enum MsgType
                {
                    Depth = 101,
                    Ticker = 102,
                    Trade = 103,
                    IndexPrice = 104,
                    Bar = 110,

                    OrderInput = 201,
                    OrderAction = 202,
                    Order = 203,
                    MyTrade = 204,
                    Position = 205,
                    Asset = 206,
                    AssetSnapshot = 207,
                    Instrument = 209,
                    AlgoOrderInput = 210,
                    AlgoOrderReport = 211,
                    AlgoOrderModify = 212,
                    OrderActionError = 213,

                    Subscribe = 302,
                    SubscribeAll = 303,
                    Unsubscribe = 304,

                    AdjustLeverage = 352,
                    NewOrderSingle = 353,
                    CancelOrder = 354,
                    CancelAllOrder = 355,
                    InstrumentRequest = 356,
                    UnionResponse = 357,
                    MergePosition = 358,
                    QueryPosition = 359,


                    BrokerStateRefresh = 400,
                    BrokerState = 401,
                    QryAsset = 402,
                    PublishAllAssetInfo = 403,
                    RemoveStrategy = 404,

                    InstrumentEnd = 802,
                };
            }

            namespace data
            {
                enum class BrokerState : int
                {
                    Unknown = 0,
                    Idle = 1,
                    DisConnected = 2,
                    Connected = 3,
                    LoggedIn = 4,
                    LoggedInFailed = 5,
                    Ready = 100
                };

#ifdef _WIN32
    #pragma pack(push, 1)
#endif
                struct Instrument
                {
                    char symbol[SYMBOL_LEN];                    //交易品种
                    char exchange_id[EXCHANGE_ID_LEN];          //交易所ID
                    InstrumentType instrument_type;             //交易品种类型
                    char product_id[PRODUCT_ID_LEN];            //产品ID

                    int contract_multiplier;                    //合约乘数
                    double price_tick;                          //最小变动价位
                    char open_date[DATE_LEN];                   //上市日
                    char create_date[DATE_LEN];                 //创建日
                    char expire_date[DATE_LEN];                 //到期日

                    int delivery_year;                          //交割年份
                    int delivery_month;                         //交割月
                    bool is_trading;                            //当前是否交易
                    double long_margin_ratio;                   //多头保证金率
                    double short_margin_ratio;                  //空头保证金率

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    {strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN); }

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string& exchange_id)
                    {strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN); }

                    const std::string get_product_id() const
                    { return std::string(product_id); }

                    void set_product_id(const std::string& product_id)
                    {strncpy(this->product_id, product_id.c_str(), PRODUCT_ID_LEN);}

                    const std::string get_open_date() const
                    { return std::string(open_date); }

                    void set_open_date(const std::string& open_date)
                    {strncpy(this->open_date, open_date.c_str(), DATE_LEN);}

                    const std::string get_create_date() const
                    { return std::string(create_date); }

                    void set_create_date(const std::string& create_date)
                    {strncpy(this->create_date, create_date.c_str(), DATE_LEN);}

                    const std::string get_expire_date() const
                    { return std::string(expire_date); }

                    void set_expire_date(const std::string& expire_date)
                    {strncpy(this->expire_date, expire_date.c_str(), DATE_LEN);}

                    bool operator ==(const Instrument & obj) const
                    {
                        return strcmp(this->symbol, obj.symbol) == 0 && strcmp(this->exchange_id, obj.exchange_id) == 0;
                    }

                    bool operator <(const Instrument & obj) const
                    {
                        return get_symbol_id(this->get_symbol(), this->get_exchange_id()) < get_symbol_id(obj.get_symbol(), obj.get_exchange_id());
                    }

#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const Instrument &instrument)
                {
                    j["exchange_id"] = std::string(instrument.exchange_id);
                    j["symbol"] = std::string(instrument.symbol);
                    j["instrument_type"] = instrument.instrument_type;
                    j["product_id"] = std::string(instrument.product_id);
                    j["contract_multiplier"] = instrument.contract_multiplier;
                    j["price_tick"] = instrument.price_tick;
                    j["open_date"] = std::string(instrument.open_date);
                    j["create_date"] = std::string(instrument.create_date);
                    j["expire_date"] = std::string(instrument.expire_date);
                    j["delivery_year"] = instrument.delivery_year;
                    j["delivery_month"] = instrument.delivery_month;
                    j["long_margin_ratio"] = instrument.long_margin_ratio;
                    j["short_margin_ratio"] = instrument.short_margin_ratio;
                }

                struct Ticker
                {
                    char source_id[SOURCE_ID_LEN];              //柜台ID
                    char symbol[SYMBOL_LEN];                    //交易品种
                    char exchange_id[EXCHANGE_ID_LEN];          //交易所ID
                    int64_t data_time;                          //数据生成时间
                    InstrumentType instrument_type;             //交易品种类型
                    double bid_price;                           //买单最优挂单价格
                    double bid_volume;                          //买单最优挂单数量
                    double ask_price;                           //卖单最优挂单价格
                    double ask_volume;                          //卖单最优挂单数量

                    const std::string get_source_id() const
                    { return std::string(source_id); }

                    void set_source_id(const std::string& source_id)
                    {strncpy(this->source_id, source_id.c_str(), SOURCE_ID_LEN);}

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN);}

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string& exchange_id)
                    {strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN);}

#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const Ticker &ticker)
                {
                    j["data_time"] = ticker.data_time;
                    j["instrument_type"] = ticker.instrument_type;
                    j["source_id"] = ticker.get_source_id();
                    j["symbol"] = ticker.get_symbol();
                    j["exchange_id"] = ticker.get_exchange_id();

                    j["bid_price"] = ticker.bid_price;
                    j["ask_price"] = ticker.ask_price;
                    j["bid_volume"] = ticker.bid_volume;
                    j["ask_volume"] = ticker.ask_volume;
                }

                inline void from_json(const nlohmann::json &j, Ticker &ticker)
                {
                    ticker.data_time = j["data_time"];
                    ticker.instrument_type = j["instrument_type"];
                    ticker.set_source_id(j["source_id"].get<std::string>());
                    ticker.set_symbol(j["symbol"].get<std::string>());
                    ticker.set_exchange_id(j["exchange_id"].get<std::string>());

                    ticker.bid_price = j["bid_price"];
                    ticker.ask_price = j["ask_price"];
                    ticker.bid_volume = j["bid_volume"];
                    ticker.ask_volume = j["ask_volume"];
                }


                //Depth10
                struct Depth
                {
                    char source_id[SOURCE_ID_LEN];              //柜台ID
                    char symbol[SYMBOL_LEN];                    //交易品种
                    char exchange_id[EXCHANGE_ID_LEN];          //交易所ID

                    int64_t data_time;                          //数据生成时间
                    InstrumentType instrument_type;             //交易品种类型

                    double bid_price[10];                       //申买价
                    double ask_price[10];                       //申卖价
                    double bid_volume[10];                      //申买量
                    double ask_volume[10];                      //申卖量

                    const std::string get_source_id() const
                    { return std::string(source_id); }

                    void set_source_id(const std::string& source_id)
                    {strncpy(this->source_id, source_id.c_str(), SOURCE_ID_LEN);}

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN);}

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string& exchange_id)
                    {strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN);}

                    std::vector<double> get_bid_price() const
                    { return std::vector<double>(bid_price, bid_price + 10); }

                    void set_bid_price(const std::vector<double> &bp)
                    { memcpy(bid_price, (const void*) bp.data(), sizeof(double) * std::min(10, int(bp.size())));}

                    std::vector<double> get_ask_price() const
                    { return std::vector<double>(ask_price, ask_price + 10); }

                    void set_ask_price(const std::vector<double> &ap)
                    { memcpy(ask_price, (const void*) ap.data(), sizeof(double) * std::min(10, int(ap.size())));}

                    std::vector<double> get_bid_volume() const
                    { return std::vector<double>(bid_volume, bid_volume + 10); }

                    void set_bid_volume(const std::vector<double> &bv)
                    { memcpy(bid_volume, (const void*) bv.data(), sizeof(double) * std::min(10, int(bv.size())));}

                    std::vector<double> get_ask_volume() const
                    { return std::vector<double>(ask_volume, ask_volume + 10); }

                    void set_ask_volume(const std::vector<double> &av)
                    { memcpy(ask_volume, (const void*) av.data(), sizeof(double) * std::min(10, int(av.size())));}

#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const Depth &depth)
                {
                    j["data_time"] = depth.data_time;
                    j["instrument_type"] = depth.instrument_type;
                    j["source_id"] = depth.get_source_id();
                    j["symbol"] = depth.get_symbol();
                    j["exchange_id"] = depth.get_exchange_id();

                    j["bid_price"] = depth.get_bid_price();
                    j["ask_price"] = depth.get_ask_price();
                    j["bid_volume"] = depth.get_bid_volume();
                    j["ask_volume"] = depth.get_ask_volume();
                }

                inline void from_json(const nlohmann::json &j, Depth &depth)
                {
                    depth.data_time = j["data_time"];
                    depth.set_symbol(j["symbol"].get<std::string>());
                    depth.set_exchange_id(j["exchange_id"].get<std::string>());
                    depth.instrument_type = j["instrument_type"];

                    depth.set_bid_price(j["bid_price"].get<std::vector<double>>());
                    depth.set_ask_price(j["ask_price"].get<std::vector<double>>());
                    depth.set_bid_volume(j["bid_volume"].get<std::vector<double>>());
                    depth.set_ask_volume(j["ask_volume"].get<std::vector<double>>());
                }

                struct Trade
                {
                    char client_id[CLIENT_ID_LEN];              //客户端ID
                    char symbol[SYMBOL_LEN];                    //交易品种
                    char exchange_id[EXCHANGE_ID_LEN];          //交易所ID
                    InstrumentType instrument_type;             //交易品种类型

                    int64_t trade_id;                           //交易ID
                    int64_t ask_id;                             //卖方订单ID
                    int64_t bid_id;                             //买方订单ID
                    double price;                               //成交价
                    double volume;                              //成交量
                    Side side;                                  //主动成交方向
                    Direction position_side;                    //仓位方向
                    int64_t trade_time;                         //成交时间

                    const std::string get_client_id() const
                    { return std::string(client_id); }

                    void set_client_id(const std::string& client_id)
                    {strncpy(this->client_id, client_id.c_str(), CLIENT_ID_LEN);}

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN);}

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string& exchange_id)
                    {strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN);}

#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const Trade &trade)
                {
                    j["trade_time"] = trade.trade_time;
                    j["symbol"] = trade.get_symbol();
                    j["exchange_id"] = trade.get_exchange_id();
                    j["client_id"] = trade.get_client_id();
                    j["instrument_type"] = trade.instrument_type;
                    j["trade_id"] = trade.trade_id;
                    j["price"] = trade.price;
                    j["volume"] = trade.volume;
                    j["side"] = trade.side;
                    j["position_side"] = trade.position_side;
                    j["ask_id"] = trade.ask_id;
                    j["bid_id"] = trade.bid_id;
                }

                inline void from_json(const nlohmann::json &j, Trade &trade)
                {
                    trade.trade_time = j["trade_time"];
                    trade.set_symbol(j["symbol"].get<std::string>());
                    trade.set_exchange_id(j["exchange_id"].get<std::string>());
                    trade.set_client_id(j["client_id"].get<std::string>());
                    trade.instrument_type = j["instrument_type"];
                    trade.trade_id = j["trade_id"];
                    trade.price = j["price"];
                    trade.volume = j["volume"];
                    trade.side = j["side"];
                    trade.position_side = j["position_side"];
                    trade.ask_id = j["ask_id"];
                    trade.bid_id = j["bid_id"];
                }


                //指数价格信息
                struct IndexPrice
                {
                    char symbol[SYMBOL_LEN];                //交易品种
                    char exchange_id[EXCHANGE_ID_LEN];      //交易所代码
                    InstrumentType instrument_type;         //交易品种类型

                    double price;                           //指数价格

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN);}

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string& exchange_id)
                    {strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN);}
#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const IndexPrice &ip)
                {
                    j["symbol"] = ip.get_symbol();
                    j["exchange_id"] = ip.get_exchange_id();
                    j["instrument_type"] = ip.instrument_type;
                    j["price"] = ip.price;
                }

                inline void from_json(const nlohmann::json &j, IndexPrice &ip)
                {
                    ip.set_symbol(j["symbol"].get<std::string>());
                    ip.set_exchange_id(j["exchange_id"].get<std::string>());
                    ip.instrument_type = j["instrument_type"];
                    ip.price = j["price"];
                }

                struct Bar
                {
                    char symbol[SYMBOL_LEN];                //交易品种
                    char exchange_id[EXCHANGE_ID_LEN];      //交易所代码

                    int64_t start_time;                     //开始时间
                    int64_t end_time;                       //结束时间
                    double open;                            //开
                    double close;                           //收
                    double low;                             //低
                    double high;                            //高
                    double volume;                          //区间交易量
                    double start_volume;                    //初始总交易量
                    int32_t trade_count;                    //区间交易笔数
                    int interval;                           //周期(秒数)

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN);}

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string& exchange_id)
                    {strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN);}
#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif


                inline void to_json(nlohmann::json &j, const Bar &bar)
                {
                    j["symbol"] = std::string(bar.symbol);
                    j["exchange_id"] = std::string(bar.exchange_id);
                    j["start_time"] = bar.start_time;
                    j["end_time"] = bar.end_time;
                    j["open"] = bar.open;
                    j["close"] = bar.close;
                    j["low"] = bar.low;
                    j["high"] = bar.high;
                    j["volume"] = bar.volume;
                    j["start_volume"] = bar.start_volume;
                    j["trade_count"] = bar.trade_count;
                }

                //订单输入
                struct OrderInput
                {
                    uint32_t strategy_id;                   //strategy instance id
                    uint64_t order_id;                      //客户端订单ID
                    char symbol[SYMBOL_LEN];                //交易品种
                    char exchange_id[EXCHANGE_ID_LEN];      //交易所代码
                    char source_id[SOURCE_ID_LEN];          //Source ID
                    char account_id[ACCOUNT_ID_LEN];        //账号ID

                    InstrumentType instrument_type;         //交易品种类型
                    double price;                           //价格
                    double stop_price;                      //冻结价格
                    double volume;                          //数量
                    Side side;                              //买卖方向
                    Direction position_side;                //仓位方向
                    OrderType order_type;                   //价格类型
                    TimeCondition time_condition;           //时间条件
                    bool reduce_only;                       //只减仓

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN);}

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string& exchange_id)
                    {strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN);}

                    const std::string get_account_id() const
                    { return std::string(account_id); }

                    void set_account_id(const std::string &account_id)
                    { strncpy(this->account_id, account_id.c_str(), ACCOUNT_ID_LEN);}

                    const std::string get_source_id() const
                    { return std::string(source_id); }

                    void set_source_id(const std::string& source_id)
                    { strncpy(this->source_id, source_id.c_str(), SOURCE_ID_LEN);}
#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const OrderInput &input)
                {
                    j["strategy_id"] = input.strategy_id;
                    j["order_id"] = input.order_id;
                    j["symbol"] = std::string(input.symbol);
                    j["exchange_id"] = std::string(input.exchange_id);
                    j["account_id"] = std::string(input.account_id);
                    j["source_id"] = std::string(input.source_id);
                    j["instrument_type"] = input.instrument_type;
                    j["volume"] = input.volume;
                    j["price"] = input.price;
                    j["stop_price"] =  input.stop_price;
                    j["side"] = input.side;
                    j["position_side"] = input.position_side;
                    j["order_type"] = input.order_type;
                    j["time_condition"] = input.time_condition;
                    j["reduce_only"] = input.reduce_only;
                }

                inline void from_json(const nlohmann::json &j, OrderInput &input)
                {
                    input.strategy_id = j["strategy_id"].get<uint32_t>();
                    input.order_id = j["client_order_id"].get<uint64_t>();
                    strncpy(input.symbol, j["symbol"].get<std::string>().c_str(), SYMBOL_LEN);
                    strncpy(input.exchange_id, j["exchange_id"].get<std::string>().c_str(), EXCHANGE_ID_LEN);
                    strncpy(input.account_id, j["account_id"].get<std::string>().c_str(), ACCOUNT_ID_LEN);
                    strncpy(input.source_id, j["source_id"].get<std::string>().c_str(), SOURCE_ID_LEN);
                    input.instrument_type = j["instrument_type"];
                    input.price = j["price"].get<double>();
                    input.stop_price = j["stop_price"].get<double>();
                    input.volume = j["volume"].get<double>();
                    input.side = j["side"];
                    input.position_side = j["position_side"];
                    input.order_type = j["order_type"];
                    input.time_condition = j["time_condition"];
                    input.reduce_only = j["reduce_only"].get<bool>();
                }

                //订单操作
                struct OrderAction
                {
                    uint32_t strategy_id;                   //strategy instance id
                    uint64_t order_id;                      //订单ID
                    uint64_t order_action_id;               //订单操作ID
                    char symbol[SYMBOL_LEN];                //交易对名称
                    char ex_order_id[ORDER_ID_LEN];         //交易所订单ID
                    OrderActionFlag action_flag;            //订单操作类型
                    InstrumentType instrument_type;         //交易品种类型

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN); }

                    const std::string get_ex_order_id() const
                    { return std::string(ex_order_id); }

                    void set_ex_order_id(const std::string& ex_order_id)
                    { strncpy(this->ex_order_id, ex_order_id.c_str(), ORDER_ID_LEN); }

#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const OrderAction &action)
                {
                    j["strategy_id"] = action.strategy_id;
                    j["order_id"] = action.order_id;
                    j["order_action_id"] = action.order_action_id;
                    j["symbol"] = action.get_symbol();
                    j["ex_order_id"] = action.get_ex_order_id();
                    j["instrument_type"] = action.instrument_type;
                    j["action_flag"] = action.action_flag;
                }

                inline void from_json(const nlohmann::json &j, OrderAction &action)
                {
                    action.strategy_id = j["strategy_id"].get<uint32_t>();
                    action.order_id = j["order_id"].get<uint64_t>();
                    action.order_action_id = j["order_action_id"].get<uint64_t>();
                    action.set_symbol(j["symbol"].get<std::string>());
                    action.set_ex_order_id(j["ex_order_id"].get<std::string>());
                    action.instrument_type = j["instrument_type"];
                    action.action_flag = j["action_flag"];
                }

                //订单操作错误
                struct OrderActionError
                {
                    uint64_t order_id;                       //客户端订单ID
                    uint64_t order_action_id;                //订单操作ID
                    int32_t error_id;                        //错误ID
                    char error_msg[ERROR_MSG_LEN];           //错误信息

                    const std::string get_error_msg() const { return std::string(error_msg); }
                    void set_error_msg(const std::string &msg) { strncpy(this->error_msg, msg.c_str(), ERROR_MSG_LEN); }
#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const OrderActionError &error)
                {
                    j["order_id"] = error.order_id;
                    j["order_action_id"] = error.order_action_id;
                    j["error_id"] = error.error_id;
                    j["error_msg"] = error.error_msg;
                }

                inline void from_json(const nlohmann::json &j, OrderActionError &error)
                {
                    error.order_id = j["order_id"];
                    error.order_action_id = j["order_action_id"].get<uint64_t>();
                    error.error_id = j["error_id"];
                    error.set_error_msg(j["error_msg"].get<std::string>());
                }

                //订单消息
                struct Order
                {
                    uint32_t strategy_id;                   //strategy instance id
                    uint64_t order_id;                      //客户端订单ID
                    char ex_order_id[ORDER_ID_LEN];         //交易所订单ID

                    char symbol[SYMBOL_LEN];                //交易品种
                    InstrumentType instrument_type;         //交易品种类型
                    char exchange_id[EXCHANGE_ID_LEN];      //交易所ID
                    char account_id[ACCOUNT_ID_LEN];        //账号ID
                    char source_id[SOURCE_ID_LEN];          //Source ID

                    double price;                           //价格
                    double volume;                          //原始数量
                    double volume_traded;                   //成交数量
                    double volume_left;                     //剩余数量
                    double stop_price;                      //止损价格
                    double close_pnl;                       //已实现盈亏(目前只有币安有)

                    double avg_price;                       //平均成交价格
                    double fee;                             //手续费
                    char fee_currency[SYMBOL_LEN];          //手续费币种
                    OrderStatus status;                     //订单状态
                    TimeCondition time_condition;           //时间条件

                    Side side;                              //买卖方向
                    Direction position_side;                //仓位方向
                    OrderType order_type;                   //订单类型 LIMIT, MARTET
                    char error_code[ERROR_CODE_LEN];        //错误码，每个交易所不同
                    int64_t insert_time;                    //订单写入时间
                    int64_t update_time;                    //订单更新时间

                    const std::string get_fee_currency() const
                    { return std::string(fee_currency); }

                    void set_fee_currency(const std::string& currency)
                    { strncpy(this->fee_currency, currency.c_str(), SYMBOL_LEN); }

                    const std::string get_ex_order_id() const
                    { return std::string(ex_order_id); }

                    void set_ex_order_id(const std::string& ex_order_id)
                    { strncpy(this->ex_order_id, ex_order_id.c_str(), ORDER_ID_LEN); }

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN); }

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string& exchange_id)
                    { strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN); }

                    const std::string get_account_id() const
                    { return std::string(account_id); }

                    void set_account_id(const std::string& account_id)
                    { strncpy(this->account_id, account_id.c_str(), ACCOUNT_ID_LEN); }

                    const std::string get_source_id() const
                    { return std::string(source_id); }

                    void set_source_id(const std::string& source_id)
                    { strncpy(this->source_id, source_id.c_str(), SOURCE_ID_LEN); }

                    const std::string get_error_code() const
                    { return std::string(error_code); }

                    void set_error_code(const std::string& error_code)
                    { strncpy(this->error_code, error_code.c_str(), ERROR_CODE_LEN); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#endif

                inline void to_json(nlohmann::json &j, const Order &order)
                {
                    j["strategy_id"] = order.strategy_id;
                    j["order_id"] = order.order_id;
                    j["ex_order_id"] = std::string(order.ex_order_id);
                    j["insert_time"] = order.insert_time;
                    j["update_time"] = order.update_time;
                    j["symbol"] = std::string(order.symbol);
                    j["instrument_type"] = order.instrument_type;
                    j["exchange_id"] = std::string(order.exchange_id);
                    j["account_id"] = std::string(order.account_id);
                    j["source_id"] = std::string(order.source_id);
                    j["price"] = order.price;
                    j["stop_price"] = order.stop_price;
                    j["volume"] = order.volume;
                    j["volume_traded"] = order.volume_traded;
                    j["volume_left"] = order.volume_left;
                    j["close_pnl"] = order.close_pnl;
                    j["fee_currency"] = std::string(order.fee_currency);
                    j["fee"] = order.fee;
                    j["status"] = order.status;
                    j["time_condition"] = order.time_condition;
                    j["side"] = order.side;
                    j["position_side"] = order.position_side;
                    j["order_type"] = order.order_type;
                    j["error_code"] = std::string(order.error_code);

                }

                inline void from_json(const nlohmann::json &j, Order &order)
                {
                    order.strategy_id = j["strategy_id"].get<uint32_t>();
                    order.order_id = j["order_id"].get<uint64_t>();
                    order.set_ex_order_id(j["ex_order_id"].get<std::string>());
                    order.insert_time = j["insert_time"];
                    order.update_time = j["update_time"];
                    order.set_symbol(j["symbol"].get<std::string>());
                    order.set_exchange_id(j["exchange_id"].get<std::string>());
                    order.set_account_id(j["account_id"].get<std::string>());
                    order.set_source_id(j["source_id"].get<std::string>());
                    order.instrument_type = j["instrument_type"];
                    order.price = j["price"].get<double>();
                    order.stop_price = j["stop_price"].get<double>();
                    order.volume = j["volume"].get<double>();
                    order.volume_traded = j["volume_traded"].get<double>();
                    order.volume_left = j["volume_left"].get<double>();
                    order.close_pnl = j["close_pnl"].get<double>();
                    order.fee = j["fee"].get<double>();
                    order.set_fee_currency(j["fee_currency"].get<std::string>());
                    order.status = j["status"];
                    order.time_condition = j["time_condition"];
                    order.side = j["side"];
                    order.position_side = j["position_side"];
                    order.order_type = j["order_type"];
                    order.set_error_code(j["error_code"].get<std::string>());
                }

                inline void order_from_input(const msg::data::OrderInput &input, msg::data::Order &order)
                {
                    order.strategy_id = input.strategy_id;
                    order.order_id = input.order_id;
                    strcpy(order.symbol, input.symbol);
                    strcpy(order.exchange_id, input.exchange_id);
                    strcpy(order.account_id, input.account_id);
                    strcpy(order.source_id, input.source_id);
                    order.instrument_type = input.instrument_type;
                    order.price = input.price;
                    order.stop_price = input.stop_price;
                    order.volume = input.volume;
                    order.volume_traded = 0;
                    order.volume_left = input.volume;
                    order.close_pnl = 0;
                    order.status = OrderStatus::PreSend;
                    order.time_condition = input.time_condition;
                    order.side = input.side;
                    order.position_side = input.position_side;
                    order.order_type = input.order_type;
                }

                //成交信息
                struct MyTrade
                {
                    uint32_t strategy_id;                   //strategy instance id
                    int64_t trade_time;                     //成交时间
                    uint64_t trade_id;                      //成交ID
                    uint64_t order_id;                      //客户端订单ID
                    char ex_order_id[ORDER_ID_LEN];         //交易所订单ID
                    char symbol[SYMBOL_LEN];                //交易品种
                    char exchange_id[EXCHANGE_ID_LEN];      //交易所ID
                    char account_id[ACCOUNT_ID_LEN];        //账号ID
                    char source_id[SOURCE_ID_LEN];          //Source ID
                    InstrumentType instrument_type;         //交易品种类型
                    Side side;                              //买卖方向
                    Offset offset;                          //开平方向
                    double price;                           //成交价格
                    double volume;                          //成交量
                    double fee;                             //手续费
                    char fee_currency[SYMBOL_LEN];          //手续费币种
                    char base_currency[SYMBOL_LEN];         //标的币种
                    char quote_currency[SYMBOL_LEN];        //报价币种

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    const std::string get_account_id() const
                    { return std::string(account_id); }

                    const std::string get_fee_currency() const
                    { return std::string(fee_currency); }

                    const std::string get_base_currency() const
                    { return std::string(base_currency); }

                    const std::string get_ex_order_id() const
                    { return std::string(ex_order_id); }

                    void set_ex_order_id(const std::string& ex_order_id)
                    { strncpy(this->ex_order_id, ex_order_id.c_str(), ORDER_ID_LEN); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN); }

                    void set_exchange_id(const std::string& exchange_id)
                    { strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN); }

                    void set_account_id(const std::string& account_id)
                    { strncpy(this->account_id, account_id.c_str(), ACCOUNT_ID_LEN); }

                    const std::string get_source_id() const
                    { return std::string(source_id); }

                    void set_source_id(const std::string& source_id)
                    { strncpy(this->source_id, source_id.c_str(), SOURCE_ID_LEN); }

                    void set_fee_currency(const std::string& currency)
                    { strncpy(this->fee_currency, currency.c_str(), SYMBOL_LEN); }

                    void set_base_currency(const std::string& currency)
                    { strncpy(this->base_currency, currency.c_str(), SYMBOL_LEN); }

                    const std::string get_quote_currency() const
                    { return std::string(quote_currency); }

                    void set_quote_currency(const std::string& currency)
                    { strncpy(this->quote_currency, currency.c_str(), SYMBOL_LEN); }

#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#endif

                inline void to_json(nlohmann::json &j, const MyTrade &trade)
                {
                    j["strategy_id"] = trade.strategy_id;
                    j["trade_time"] = trade.trade_time;
                    j["trade_id"] = trade.trade_id;
                    j["order_id"] = trade.order_id;
                    j["ex_order_id"] = std::string(trade.ex_order_id);
                    j["symbol"] = std::string(trade.symbol);
                    j["exchange_id"] = std::string(trade.exchange_id);
                    j["account_id"] = std::string(trade.account_id);
                    j["source_id"] = std::string(trade.source_id);
                    j["instrument_type"] = trade.instrument_type;
                    j["side"] = trade.side;
                    j["offset"] = trade.offset;
                    j["price"] = trade.price;
                    j["volume"] = trade.volume;
                    j["fee"] = trade.fee;
                    j["fee_currency"] = std::string(trade.fee_currency);
                    j["base_currency"] = std::string(trade.base_currency);
                    j["quote_currency"] = std::string(trade.quote_currency);
                }

                inline void from_json(const nlohmann::json &j, MyTrade &trade)
                {
                    trade.strategy_id = j["strategy_id"].get<uint32_t>();
                    trade.trade_time = j["trade_time"];
                    trade.trade_id = j["trade_id"];
                    trade.order_id = j["order_id"].get<uint64_t>();
                    strncpy(trade.ex_order_id, j["ex_order_id"].get<std::string>().c_str(), ORDER_ID_LEN);
                    strncpy(trade.symbol, j["symbol"].get<std::string>().c_str(), SYMBOL_LEN);
                    strncpy(trade.exchange_id, j["exchange_id"].get<std::string>().c_str(), EXCHANGE_ID_LEN);
                    strncpy(trade.account_id, j["account_id"].get<std::string>().c_str(), ACCOUNT_ID_LEN);
                    strncpy(trade.source_id, j["source_id"].get<std::string>().c_str(), SOURCE_ID_LEN);
                    trade.instrument_type = j["instrument_type"];
                    trade.side = static_cast<Side>(j["side"].get<int>());
                    trade.offset = j["offset"];
                    trade.price = j["price"].get<double>();
                    trade.volume = j["volume"].get<double>();
                    trade.fee = j["fee"].get<double>();
                    strncpy(trade.fee_currency, j["fee_currency"].get<std::string>().c_str(), SYMBOL_LEN);
                    strncpy(trade.base_currency, j["base_currency"].get<std::string>().c_str(), SYMBOL_LEN);
                    strncpy(trade.quote_currency, j["quote_currency"].get<std::string>().c_str(), SYMBOL_LEN);
                }

                //资产信息
                struct Asset
                {
                    int64_t update_time;                //更新时间
                    uint32_t holder_uid;
                    LedgerCategory ledger_category;

                    char coin[SYMBOL_LEN];              //币种名称
                    char account_id[ACCOUNT_ID_LEN];    //账号ID
                    char exchange_id[EXCHANGE_ID_LEN];  //交易所ID

                    double avail;                       //可用资金
                    double margin;                      //保证金余额
                    double frozen;                      //冻结资金

                    const std::string get_coin() const
                    { return std::string(coin); }

                    const std::string get_account_id() const
                    { return std::string(account_id); }

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_coin(const std::string& coin)
                    { strncpy(this->coin, coin.c_str(), SYMBOL_LEN); }

                    void set_account_id(const std::string& account_id)
                    { strncpy(this->account_id, account_id.c_str(), ACCOUNT_ID_LEN); }

                    void set_exchange_id(const std::string& exchange_id)
                    { strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#endif

                inline void to_json(nlohmann::json &j, const Asset &asset_info)
                {
                    j["update_time"] = asset_info.update_time;
                    j["holder_uid"] = asset_info.holder_uid;
                    j["ledger_category"] = asset_info.ledger_category;
                    j["coin"] = std::string(asset_info.coin);
                    j["account_id"] = std::string(asset_info.account_id);
                    j["exchange_id"] = std::string(asset_info.exchange_id);
                    j["avail"] = asset_info.avail;
                    j["margin"] = asset_info.margin;
                    j["frozen"] = asset_info.frozen;
                }

                //持仓信息
                struct Position
                {
                    uint32_t strategy_id;                   //strategy instance id
                    int64_t update_time;                    //更新时间
                    char symbol[SYMBOL_LEN];                //交易品种
                    InstrumentType instrument_type;         //交易品种类型
                    char exchange_id[EXCHANGE_ID_LEN];      //交易所ID
                    uint32_t holder_uid;
                    LedgerCategory ledger_category;
                    char source_id[SOURCE_ID_LEN];          //柜台ID
                    char account_id[ACCOUNT_ID_LEN];        //账号ID

                    Direction direction;                    //持仓方向
                    double volume;                          //持仓数量
                    int64_t frozen_total;                   //冻结数量
                    double last_price;                      //最新价
                    double avg_open_price;                  //开仓均价
                    double settlement_price;                //结算价(交割合约)
                    double margin;                          //保证金
                    double realized_pnl;                    //已实现盈亏
                    double unrealized_pnl;                  //未实现盈亏

                    const std::string get_symbol() const
                    { return std::string(symbol); }

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    const std::string get_source_id() const
                    { return std::string(source_id); }

                    const std::string get_account_id() const
                    { return std::string(account_id); }

                    void set_symbol(const std::string& symbol)
                    { strncpy(this->symbol, symbol.c_str(), SYMBOL_LEN); }

                    void set_exchange_id(const std::string& exchange_id)
                    { strncpy(this->exchange_id, exchange_id.c_str(), EXCHANGE_ID_LEN); }

                    void set_source_id(const std::string& source_id)
                    { strncpy(this->source_id, source_id.c_str(), SOURCE_ID_LEN); }

                    void set_account_id(const std::string& account_id)
                    { strncpy(this->account_id, account_id.c_str(), ACCOUNT_ID_LEN); }


#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#endif

                inline void to_json(nlohmann::json &j, const Position &position)
                {
                    j["strategy_id"] = position.strategy_id;
                    j["update_time"] = position.update_time;
                    j["symbol"] = std::string(position.symbol);
                    j["instrument_type"] = position.instrument_type;
                    j["exchange_id"] = std::string(position.exchange_id);
                    j["account_id"] = std::string(position.account_id);
                    j["direction"] = position.direction;
                    j["volume"] = position.volume;
                    j["frozen_total"] = position.frozen_total;
                    j["last_price"] = position.last_price;
                    j["avg_open_price"] = position.avg_open_price;
                    j["settlement_price"] = position.settlement_price;
                    j["margin"] = position.margin;
                    j["realized_pnl"] = position.realized_pnl;
                    j["unrealized_pnl"] = position.unrealized_pnl;
                }

            template<typename T> std::string to_string(const T &ori)
            {
                nlohmann::json j;
                to_json(j, ori);
                return j.dump(-1, ' ', false, nlohmann::json::error_handler_t::ignore);;
            }
            }
        }
    }
}

#endif //WINGCHUN_EVENT_H
