'''
This is source code under the Apache License 2.0.
Original Author: kx@godzilla.dev
Original date: March 3, 2025
'''
import math
import json
import pyyjj
from kungfu.wingchun.constants import *
from pywingchun.constants import Side, InstrumentType, OrderType


SECOND_IN_NANO = int(1e9)

md_source = Source.BINANCE
td_source = Source.BINANCE
exchange = Exchange.BINANCE


class TriangularArbitrage(object):

    def __init__(self, context, account, max_investment_pct, base, quote, currency, diff=0.004):
        '''context'''
        self.ctx = context
        location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.STRATEGY, self.ctx.group, self.ctx.name, self.ctx.locator)
        self.cache_file = location.locator.layout_file(location, pyyjj.layout.SQLITE, "inspect_cache")
        self.inspect_stat = {}
        self.account = account
        self.max_investment_pct = max_investment_pct
        '''the inspect diff'''
        self.diff_ratio = diff
        '''aave'''
        self.base = base
        '''usdt'''
        self.quote = quote
        '''eth/btc/bnb/xt'''
        self.currency = currency
        '''base currency pair'''
        self.base_currency_pair = merge(self.base, self.currency)
        '''currency quote pair'''
        self.currency_quote_pair = merge(self.currency, self.quote)
        '''base quote pair'''
        self.base_quote_pair = merge(self.base, self.quote)
        '''aave_usdt depth'''
        self.base_quote_depth = None
        '''aave_eth depth'''
        self.base_currency_depth = None
        '''eth_usdt depth'''
        self.currency_quote_depth = None
        '''market config'''
        self.market_config = {}
        '''order_ids'''
        self.order_ids = None
        '''order status'''
        self.order_status = {}
        '''last inspect time'''
        self.last_inspect_time = self.ctx.now()
        '''last log alive time'''
        self.last_log_alive_time = self.ctx.now()

    def update_market_config(self, exchange):
        symbols = [self.base_currency_pair, self.currency_quote_pair, self.base_quote_pair]
        for symbol in symbols:
            info_str = self.ctx.get_market_info(symbol, exchange, InstrumentType.Spot)
            info = json.loads(info_str)
            self.ctx.log().info(info)
            self.ctx.log().info(symbol)
            self.ctx.log().info(exchange)
            a = 1
            b = 1
            for filter in info['filters']:
                self.ctx.log().info(filter)
                if filter['filterType'] == 'PRICE_FILTER':
                    a = math.log10(1/float(filter.get('tickSize', a)))
                if filter['filterType'] == 'LOT_SIZE':
                    b = math.log10(1/float(filter.get('stepSize', b)))
            self.market_config.update({symbol: {'price_precision': a, 'quantity_precesion': b}})
        self.ctx.log().info(self.market_config)

    def set_order_status(self, order_id, order_status):
        if self.order_ids:
            if order_id in self.order_ids:
                self.order_status[order_id] = order_status

    def reset_order_status(self):
        self.order_ids = None
        self.order_status = {}

    def tick_size_rounddown(self, vol, tick_size):
        if tick_size <= 1:
            return vol
        if vol < tick_size:
            return tick_size
        return int(vol/tick_size)*tick_size

    def pre_inspect_check(self):
        '''如果产生过订单'''
        if self.order_ids:
            try:
                self.ctx.log().info(f"order_ids: {self.order_ids}")
                self.ctx.log().info(f"order_status: {self.order_status}")
                time_delta_in_seconds = (self.ctx.now() - self.last_inspect_time) / SECOND_IN_NANO
                if len(self.order_ids) == 3:
                    order_status_list = [self.order_status[order_id] for order_id in self.order_ids]
                    '''如果全部filled'''
                    if order_status_list == [OrderStatus.Filled, OrderStatus.Filled, OrderStatus.Filled]:
                        self.ctx.log().info(f"time_delta_in_seconds: {time_delta_in_seconds}")
                        '''如果全部filled并且超过10秒，通过检查'''
                        if time_delta_in_seconds > 10:
                            '''清空当前batch的订单'''
                            self.reset_order_status()
                            return True

                    '''如果全部error'''
                    if order_status_list == [OrderStatus.Error, OrderStatus.Error, OrderStatus.Error]:
                        self.ctx.log().info(f"time_delta_in_seconds: {time_delta_in_seconds}")
                        '''如果全部error并且超过1小时，通过检查'''
                        if time_delta_in_seconds > 60 * 60:
                            '''清空当前batch的订单'''
                            self.reset_order_status()
                            return True
                return False
            except:
                return False
        else:
            '''如果没有产生过任何订单，默认通过检查'''
            '''清空当前batch的订单'''
            self.reset_order_status()
            return True

    def inspect(self, depth):
        '''日志中60s更新depth'''
        time_delta_in_seconds = (self.ctx.now() - self.last_log_alive_time) / SECOND_IN_NANO
        if time_delta_in_seconds > 60:
            self.last_log_alive_time = self.ctx.now()
            self.ctx.log().info('TriangularArbitrage: update and I am Alive')
            self.ctx.log().info(depth)

        if depth.symbol == self.base_currency_pair:
            self.base_currency_depth = depth
        if depth.symbol == self.currency_quote_pair:
            self.currency_quote_depth = depth
        if depth.symbol == self.base_quote_pair:
            self.base_quote_depth = depth

        '''状态检查'''
        if not self.pre_inspect_check():
            return None

        '''正式开始inspect: 记录本次inspect时间'''
        self.last_inspect_time = self.ctx.now()

        '''info updated and trigger the calc'''
        sync_bid = 0
        sync_ask = 0
        if self.base_currency_depth and self.currency_quote_depth and self.base_quote_depth:
            '''sync bid/ask'''
            if self.base_currency_depth.bid_price and self.currency_quote_depth.bid_price and self.base_currency_depth.ask_price and self.currency_quote_depth.ask_price:
                sync_bid = self.base_currency_depth.bid_price[0] * self.currency_quote_depth.bid_price[0]
                sync_ask = self.base_currency_depth.ask_price[0] * self.currency_quote_depth.ask_price[0]

        '''inspect'''
        if sync_ask != 0 and sync_bid != 0:
            '''正向leg'''
            inspect_leg_1 = (sync_bid - self.base_quote_depth.ask_price[0]) / sync_bid
            '''反向leg'''
            inspect_leg_2 = (self.base_quote_depth.bid_price[0] - sync_ask) / sync_ask

            if inspect_leg_1 > self.diff_ratio:
                self.ctx.log().info(f"inspect_leg_1: {inspect_leg_1}")
                self.ctx.log().info(f"inspect_leg_2: {inspect_leg_2}")
                self.ctx.log().info(f"Got inspect_leg_1 for {self.base_currency_pair}")
                self.ctx.log().info(self.base_currency_depth)
                self.ctx.log().info(self.currency_quote_depth)
                self.ctx.log().info(self.base_quote_depth)
                return {"type": "inspect_leg_1"}
            elif inspect_leg_2 > self.diff_ratio:
                self.ctx.log().info(f"inspect_leg_1: {inspect_leg_1}")
                self.ctx.log().info(f"inspect_leg_2: {inspect_leg_2}")
                self.ctx.log().info(f"Got inspect_leg_2 for {self.base_currency_pair}")
                self.ctx.log().info(self.base_currency_depth)
                self.ctx.log().info(self.currency_quote_depth)
                self.ctx.log().info(self.base_quote_depth)
                return {"type": "inspect_leg_2"}
            else:
                return None

    def optimized_volume(self, triangular_arbitrage):
        try:
            if triangular_arbitrage:
                if triangular_arbitrage['type'] == 'inspect_leg_1':
                    bid_base_currency_in_usdt = self.base_currency_depth.bid_price[0] * self.base_currency_depth.bid_volume[0] * self.currency_quote_depth.bid_price[0]
                    bid_currency_quote_in_usdt = self.currency_quote_depth.bid_price[0] * self.currency_quote_depth.bid_volume[0]
                    ask_base_quote_in_usdt = self.base_quote_depth.ask_price[0] * self.base_quote_depth.ask_volume[0]
                    min_money = min(bid_base_currency_in_usdt, bid_currency_quote_in_usdt, ask_base_quote_in_usdt)
                    self.ctx.log().info(f"bid_base_currency_in_usdt: {bid_base_currency_in_usdt} bid_currency_quote_in_usdt: {bid_currency_quote_in_usdt} ask_base_quote_in_usdt: {ask_base_quote_in_usdt} min_money: {min_money}")
                    vols = None
                    '''根据最小盘口决定开始执行的最优vols'''
                    if min_money == bid_base_currency_in_usdt:
                        self.ctx.log().info(f"type: inspect_leg_1 case: a")
                        vols = [self.base_currency_depth.bid_volume[0],
                                self.base_currency_depth.bid_price[0] * self.base_currency_depth.bid_volume[0],
                                self.base_currency_depth.bid_price[0] * self.base_currency_depth.bid_volume[0] * self.currency_quote_depth.bid_price[0] / self.base_quote_depth.ask_price[0]]
                    if min_money == bid_currency_quote_in_usdt:
                        self.ctx.log().info(f"type: inspect_leg_1 case: b")
                        vols = [self.currency_quote_depth.bid_price[0] * self.currency_quote_depth.bid_volume[0] / self.base_quote_depth.ask_price[0],
                                self.currency_quote_depth.bid_volume[0],
                                self.currency_quote_depth.bid_price[0] * self.currency_quote_depth.bid_volume[0] / self.base_quote_depth.ask_price[0]]
                    if min_money == ask_base_quote_in_usdt:
                        self.ctx.log().info(f"type: inspect_leg_1 case: c")
                        vols = [self.base_quote_depth.ask_volume[0],
                                self.base_quote_depth.ask_volume[0] * self.base_currency_depth.bid_price[0],
                                self.base_quote_depth.ask_volume[0]]
                    '''vol的执行前检查'''
                    if len(vols) == 3:
                        '''调整量精度'''
                        vols[0] = int(round(vols[0], self.market_config[self.base_currency_pair]['quantity_precesion'])) if self.market_config[self.base_currency_pair]['quantity_precesion'] <= 0 else round(vols[0], self.market_config[self.base_currency_pair]['quantity_precesion'])
                        vols[1] = int(round(vols[1], self.market_config[self.currency_quote_pair]['quantity_precesion'])) if self.market_config[self.currency_quote_pair]['quantity_precesion'] <= 0 else round(vols[1], self.market_config[self.currency_quote_pair]['quantity_precesion'])
                        vols[2] = int(round(vols[2], self.market_config[self.base_quote_pair]['quantity_precesion'])) if self.market_config[self.base_quote_pair]['quantity_precesion'] <= 0 else round(vols[2], self.market_config[self.base_quote_pair]['quantity_precesion'])
                        self.ctx.log().info(f"optimized_volume: {vols}")
                        '''分别做1)quantity_precesion, 2)quote_qty还有3)可用资金的检查'''
                        constraints = [
                            vols[0] < 1 / 10 ** self.market_config[self.base_currency_pair]['quantity_precesion'],
                            vols[1] < 1 / 10 ** self.market_config[self.currency_quote_pair]['quantity_precesion'],
                            vols[2] < 1 / 10 ** self.market_config[self.base_quote_pair]['quantity_precesion'],
                            self.base_currency_depth.bid_price[0] * vols[0] < self.market_config[self.base_currency_pair]['quote_qty'],
                            self.currency_quote_depth.bid_price[0] * vols[1] < self.market_config[self.currency_quote_pair]['quote_qty'],
                            self.base_quote_depth.ask_price[0] * vols[2] < self.market_config[self.base_quote_pair]['quote_qty'],
                            vols[0] > self.ctx.get_account_cash_limit(self.account, self.base) * self.max_investment_pct,
                            vols[1] > self.ctx.get_account_cash_limit(self.account, self.currency) * self.max_investment_pct,
                            vols[2] > self.ctx.get_account_cash_limit(self.account, self.quote) / self.base_quote_depth.ask_price[0] * self.max_investment_pct]
                        self.ctx.log().info(f"constraints: {constraints}")
                        if True in constraints:
                            return None
                        else:
                            self.ctx.log().info(f"optimized_volume: {vols}")
                            '''step size round down'''
                            rounddown_vols = [self.tick_size_rounddown(vols[0], self.market_config[self.base_currency_pair]['tick_size']),
                                              vols[1],
                                              self.tick_size_rounddown(vols[2], self.market_config[self.base_quote_pair]['tick_size'])]
                            self.ctx.log().info(f"rounddown_volume: {rounddown_vols}")
                            return rounddown_vols

                if triangular_arbitrage['type'] == 'inspect_leg_2':
                    ask_base_currency_in_usdt = self.base_currency_depth.ask_price[0] * self.base_currency_depth.ask_volume[0] * self.currency_quote_depth.ask_price[0]
                    ask_currency_quote_in_usdt = self.currency_quote_depth.ask_price[0] * self.currency_quote_depth.ask_volume[0]
                    bid_base_quote_in_usdt = self.base_quote_depth.bid_price[0] * self.base_quote_depth.bid_volume[0]
                    min_money = min(ask_base_currency_in_usdt, ask_currency_quote_in_usdt, bid_base_quote_in_usdt)
                    self.ctx.log().info(f"ask_base_currency_in_usdt: {ask_base_currency_in_usdt} ask_currency_quote_in_usdt: {ask_currency_quote_in_usdt} bid_base_quote_in_usdt: {bid_base_quote_in_usdt} min_money: {min_money}")
                    vols = None
                    '''根据最小盘口决定开始执行的最优vols'''
                    if min_money == ask_base_currency_in_usdt:
                        self.ctx.log().info(f"type: inspect_leg_2 case: a")
                        vols = [self.base_currency_depth.ask_volume[0],
                                self.base_quote_depth.bid_price[0] * self.base_currency_depth.ask_volume[0] / self.currency_quote_depth.ask_price[0],
                                self.base_currency_depth.ask_volume[0]]
                    if min_money == ask_currency_quote_in_usdt:
                        self.ctx.log().info(f"type: inspect_leg_2 case: b")
                        vols = [self.currency_quote_depth.ask_volume[0] / self.base_currency_depth.ask_price[0],
                                self.currency_quote_depth.ask_volume[0],
                                self.currency_quote_depth.ask_volume[0] / self.base_currency_depth.ask_price[0]]
                    if min_money == bid_base_quote_in_usdt:
                        self.ctx.log().info(f"type: inspect_leg_2 case: c")
                        vols = [self.base_quote_depth.bid_volume[0] * self.base_quote_depth.bid_price[0] / self.currency_quote_depth.ask_price[0] / self.base_currency_depth.ask_price[0],
                                self.base_quote_depth.bid_volume[0] * self.base_quote_depth.bid_price[0] / self.currency_quote_depth.ask_price[0],
                                self.base_quote_depth.bid_volume[0]]
                    '''vol的执行前检查'''
                    if len(vols) == 3:
                        '''调整量精度'''
                        vols[0] = int(round(vols[0], self.market_config[self.base_currency_pair]['quantity_precesion'])) if self.market_config[self.base_currency_pair]['quantity_precesion'] <= 0 else round(vols[0], self.market_config[self.base_currency_pair]['quantity_precesion'])
                        vols[1] = int(round(vols[1], self.market_config[self.currency_quote_pair]['quantity_precesion'])) if self.market_config[self.currency_quote_pair]['quantity_precesion'] <= 0 else round(vols[1], self.market_config[self.currency_quote_pair]['quantity_precesion'])
                        vols[2] = int(round(vols[2], self.market_config[self.base_quote_pair]['quantity_precesion'])) if self.market_config[self.base_quote_pair]['quantity_precesion'] <= 0 else round(vols[2], self.market_config[self.base_quote_pair]['quantity_precesion'])
                        self.ctx.log().info(f"optimized_volume: {vols}")
                        '''分别做1)quantity_precesion, 2)quote_qty还有3)可用资金的检查'''
                        constraints = [
                            vols[0] < 1 / 10 ** self.market_config[self.base_currency_pair]['quantity_precesion'],
                            vols[1] < 1 / 10 ** self.market_config[self.currency_quote_pair]['quantity_precesion'],
                            vols[2] < 1 / 10 ** self.market_config[self.base_quote_pair]['quantity_precesion'],
                            self.base_currency_depth.ask_price[0] * vols[0] < self.market_config[self.base_currency_pair]['quote_qty'],
                            self.currency_quote_depth.ask_price[0] * vols[1] < self.market_config[self.currency_quote_pair]['quote_qty'],
                            self.base_quote_depth.bid_price[0] * vols[2] < self.market_config[self.base_quote_pair]['quote_qty'],
                            vols[0] > self.ctx.get_account_cash_limit(self.account, self.currency) / self.base_currency_depth.ask_price[0] * self.max_investment_pct,
                            vols[1] > self.ctx.get_account_cash_limit(self.account, self.quote) / self.currency_quote_depth.ask_price[0] * self.max_investment_pct,
                            vols[2] > self.ctx.get_account_cash_limit(self.account, self.base) * self.max_investment_pct]
                        self.ctx.log().info(f"constraints: {constraints}")
                        if True in constraints:
                            return None
                        else:
                            self.ctx.log().info(f"optimized_volume: {vols}")
                            '''step size round down'''
                            rounddown_vols = [self.tick_size_rounddown(vols[0], self.market_config[self.base_currency_pair]['tick_size']),
                                              vols[1],
                                              self.tick_size_rounddown(vols[2], self.market_config[self.base_quote_pair]['tick_size'])]
                            self.ctx.log().info(f"rounddown_volume: {rounddown_vols}")
                            return rounddown_vols
        except ZeroDivisionError:
            self.ctx.log().info(f"ZeroDivisionError in optimized_volume")
            return None

    def execute(self, triangular_arbitrage):
        if triangular_arbitrage:
            if triangular_arbitrage['type'] == 'inspect_leg_1':
                vols = self.optimized_volume(triangular_arbitrage)
                if vols:
                    self.order_ids = list(map(self.trade,
                                              [self.base_currency_pair,
                                               self.currency_quote_pair,
                                               self.base_quote_pair],
                                              [self.base_currency_depth.bid_price[0],
                                               self.currency_quote_depth.bid_price[0],
                                               self.base_quote_depth.ask_price[0]],
                                              vols,
                                              [0, 0, 1]))
                    self.ctx.log().info(f"Exe inspect_leg_1 for {self.base_currency_pair}")
                    self.ctx.log().info(f"order_ids: {self.order_ids}")
            if triangular_arbitrage['type'] == 'inspect_leg_2':
                vols = self.optimized_volume(triangular_arbitrage)
                if vols:
                    self.order_ids = list(map(self.trade,
                                              [self.base_currency_pair,
                                               self.currency_quote_pair,
                                               self.base_quote_pair],
                                              [self.base_currency_depth.ask_price[0],
                                               self.currency_quote_depth.ask_price[0],
                                               self.base_quote_depth.bid_price[0]],
                                              vols,
                                              [1, 1, 0]))
                    self.ctx.log().info(f"Exe inspect_leg_2 for {self.base_currency_pair}")
                    self.ctx.log().info(f"order_ids: {self.order_ids}")

    def trade(self, market, price, vol,side_in_num):
        '''只调整价格精度 在optimized_volume中调整量的精度'''
        price = int(round(price, self.market_config[market]['price_precision'])) if self.market_config[market]['price_precision'] <= 0 else round(price, self.market_config[market]['price_precision'])
        '''下单'''
        side = Side.Buy if side_in_num else Side.Sell
        inst_type = InstrumentType.Spot
        order_type = OrderType.Limit
        order_id = self.ctx.insert_order(market, inst_type, exchange, self.account, price, vol, order_type, side)
        self.ctx.log().info(f'insert_order: [order_id]{order_id} [market]{market} [price]{price} [vol]{vol} [order_type]{order_type} [side]{side}')
        return order_id


def merge(currency1, currency2):
    return currency1 + '_' + currency2


def pre_start(context):
    #x = input('Enter your name:')
    '''加载配置'''
    str_para = context.get_config()
    account = str_para.get('account')
    max_investment_pct = str_para.get('max_investment_pct')
    base_asset = str_para.get('assets').get('base').get('asset')
    quote_asset = str_para.get('assets').get('quote').get('asset')
    currency_asset = str_para.get('assets').get('currency').get('asset')
    base_cash_limit = str_para.get('assets').get('base').get('cash_limit')
    quote_cash_limit = str_para.get('assets').get('quote').get('cash_limit')
    currency_cash_limit = str_para.get('assets').get('currency').get('cash_limit')
    base_currency_pair = merge(base_asset, currency_asset)
    currency_quote_pair = merge(currency_asset, quote_asset)
    base_quote_pair = merge(base_asset, quote_asset)

    '''td添加account md订阅行情'''
    context.add_account(td_source, account)
    context.subscribe(md_source, [base_currency_pair, currency_quote_pair, base_quote_pair], InstrumentType.Spot, exchange)
    context.set_account_cash_limit(td_source, exchange, account, base_asset, base_cash_limit)
    context.set_account_cash_limit(td_source, exchange, account, quote_asset, quote_cash_limit)
    context.set_account_cash_limit(td_source, exchange, account, currency_asset, currency_cash_limit)

    context.log().info(
        f"{base_asset} limit: {context.get_account_cash_limit(account, base_asset)}, "
        f"{quote_asset} limit: {context.get_account_cash_limit(account, quote_asset)}, "
        f"{currency_asset} limit: {context.get_account_cash_limit(account, currency_asset)}"
    )

    for k, v in context.books.items():
        context.log().info(f"k: {k}, assets: {v}")

    '''注册策略'''
    arbitrager = TriangularArbitrage(context, account=account, max_investment_pct=max_investment_pct, base=base_asset, quote=quote_asset, currency=currency_asset)
    arbitrager.update_market_config(exchange)
    context.set_object('arbitrager', arbitrager)


def on_depth(context, depth):
    arbitrager = context.get_object('arbitrager')
    triangular_arbitrage = arbitrager.inspect(depth)
    if triangular_arbitrage:
        #arbitrager.execute(triangular_arbitrage)
        context.log().info(triangular_arbitrage)


def on_order(context, order):
    context.log().info(f'order received: [id]{order.order_id} [symbol]{order.symbol} [volume]{order.volume} [price]{order.price} [status]{order.status} [error_code]{order.error_code}')
    arbitrager = context.get_object('arbitrager')
    arbitrager.set_order_status(order.order_id, order.status)

