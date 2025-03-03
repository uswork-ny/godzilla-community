'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
from .position import FuturePosition, Position
from .position import get_uid as get_position_uid
import kungfu.wingchun.utils as wc_utils
import kungfu.wingchun.constants as wc_constants
import kungfu.wingchun.msg as wc_msg
import datetime
import pyyjj
import sys
import traceback
from collections import namedtuple
import pywingchun
from pywingchun.constants import OrderType
from rx.subject import Subject
import json
import kungfu.msg.utils as msg_utils
import kungfu.yijinjing.time as kft

DATE_FORMAT = "%Y%m%d"
DEFAULT_CASH = 1e7

class AccountBookTags(namedtuple('AccountBookTags', 'holder_uid ledger_category source_id account_id client_id')):
    def __new__(cls, holder_uid, ledger_category, source_id="", account_id="", client_id=""):
        return super(AccountBookTags, cls).__new__(cls, holder_uid, ledger_category, source_id, account_id, client_id)
    @classmethod
    def make_from_location(cls, location):
        if location.category == pyyjj.category.TD:
            return cls(**{"holder_uid": location.uid,  "source_id": location.group, "account_id": location.name, "ledger_category": wc_constants.LedgerCategory.Account})
        elif location.category == pyyjj.category.STRATEGY:
            return cls(**{"holder_uid": location.uid , "client_id": location.name, "ledger_category": wc_constants.LedgerCategory.Strategy})
        else:
            raise ValueError('invalid location category {}'.format(location.category))

class BookEvent:
    def __init__(self, msg_type, data):
        self.msg_type = msg_type
        self.data = data

    def as_dict(self):
        return {"msg_type": self.msg_type, "data": msg_utils.object_as_dict(self.data)}

    def __repr__(self):
        return str(self.as_dict())

class AccountBook(pywingchun.Book):
    def __init__(self, ctx, location, **kwargs):
        pywingchun.Book.__init__(self)
        ctx.logger.info(f"init AccountBook: location - [{location.uid}]{location.uname}")
        self.ctx = ctx
        self.strategy_id = None
        self.backtest = False
        self.location = location
        self.tags = AccountBookTags.make_from_location(self.location)
        self.subject = Subject()
        self._assets = {}
        self._active_orders = {}
        self._positions = {}
        assets = kwargs.pop("assets", [])
        for asset in assets:
            if isinstance(asset, pywingchun.Asset):
                a = msg_utils.object_as_dict(asset)
            else:
                a = asset
            self._assets[a["coin"]] = a
        orders = kwargs.pop("orders", [])
        for order in orders:
            self._active_orders[order["order_id"]] = order
        ctx.logger.info(f"orders: [{len(orders)}], assets: [{assets}]")
        # positions = kwargs.pop("positions", [])
        # for pos in positions:
        #     if isinstance(pos, pywingchun.Position):
        #         pos = msg_utils.object_as_dict(pos)
        #     if isinstance(pos, dict):
        #         try:
        #             pos = Position.factory(ctx=self.ctx, book=self, **pos)
        #         except Exception as err:
        #             exc_type, exc_obj, exc_tb = sys.exc_info()
        #             self.ctx.logger.error('init position from dict %s, error [%s] %s', pos, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
        #             continue
        #     if isinstance(pos, Position):
        #         self._positions[pos.uid] = pos
        #     else:
        #         raise TypeError("Position object required, but {} provided".format(type(pos)))
        self._last_check = 0

    def _on_interval_check(self, now: int):
        """定时保存Asset信息到数据库

        Args:
            now (int): 当前时间，用于和上次时间对比
        """
        if self._last_check + int(1e9) * 30 < now:
            for asset in self._assets.values():
                self.subject.on_next(self.make_asset_event(asset))
            self._last_check = now

    def set_strategy_id(self, strategy_id: int) -> None:
        self.strategy_id = strategy_id

    def set_backtest(self) -> None:
        self.backtest = True

    def on_order_input(self, input: pywingchun.OrderInput) -> None:
        """响应OrderInput事件的回调函数
        新增订单，并更新资产信息。
        Args:
            event (event): 事件对象
            input (pywingchun.OrderInput): 订单委托对象
        """
        if not self.backtest and self.strategy_id and input.strategy_id != self.strategy_id:
            return
        if input.order_type == OrderType.Mock:
            return
        self.ctx.logger.debug("{} received order input event: {}".format(self.location.uname, input))
        self._active_orders[input.order_id] = msg_utils.object_as_dict(pywingchun.utils.order_from_input(input))
        splited = input.symbol.split("_")
        base_coin = splited[0]
        quote_coin = splited[1]
        self.ctx.logger.debug(self._assets)
        if base_coin not in self._assets or quote_coin not in self._assets:
            return
        if input.side == wc_constants.Side.Buy:
            self._assets[quote_coin]["avail"] -= input.price * input.volume
            self._assets[quote_coin]['frozen'] += input.price * input.volume
            self.subject.on_next(self.make_asset_event(self._assets[quote_coin]))
        else:
            self._assets[base_coin]['avail'] -= input.volume
            self._assets[base_coin]['frozen'] += input.volume
            self.subject.on_next(self.make_asset_event(self._assets[base_coin]))

    def on_order(self, event, order: pywingchun.Order) -> None:
        """响应Order事件的回调函数
        更新缓存的订单，删除已完成的订单。返回的订单有几种情况：
        1. 正常活跃(active)订单
        2. 正常终止(not active)订单
        3. 错误订单
        3.1 错误订单包含订单信息
        3.2 错误订单不包含订单信息（通常是交易所查不到订单相关信息）
        Args:
            event (event): 事件对象
            order (pywingchun.Order): 订单对象
        """
        if not self.backtest and self.strategy_id and order.strategy_id != self.strategy_id:
            return
        self.ctx.logger.debug("{} received order event: {}".format(self.location.uname, order))
        if order.active:
            self._active_orders[order.order_id] = msg_utils.object_as_dict(order)
        elif order.order_id in self._active_orders:
            orig_order = self._active_orders[order.order_id]
            splited = orig_order["symbol"].split("_")
            base_coin = splited[0]
            quote_coin = splited[1]
            if base_coin not in self._assets or quote_coin not in self._assets:
                self._active_orders.pop(order.order_id)
                return
            self.ctx.logger.debug(f"{base_coin}, {quote_coin}, {self._assets}")
            if order.status == wc_constants.OrderStatus.Submitted:  # assets computed in order_input
                pass
                # if order.side == wc_constants.Side.Buy:
                #     self._assets[quote_coin]["avail"] -= order.volume * order.price
                #     self._assets[quote_coin]['frozen'] += order.volume * order.price
                # else:
                #     self._assets[base_coin]['avail'] -= order.volume
                #     self._assets[base_coin]['frozen'] += order.volume
            elif order.symbol == "":    # no order information from exchange
                if orig_order["side"] == wc_constants.Side.Buy:
                    self._assets[quote_coin]['frozen'] -= orig_order["volume"] * orig_order["price"]
                    self._assets[quote_coin]['avail'] += orig_order["volume"] * orig_order["price"]
                else:
                    self._assets[base_coin]['frozen'] -= orig_order["volume"]
                    self._assets[base_coin]['avail'] += orig_order["volume"]
            elif not order.active:
                if orig_order["side"] == wc_constants.Side.Buy:
                    self._assets[base_coin]["avail"] += order.volume_traded
                    self._assets[quote_coin]['frozen'] -= order.volume * order.price
                    self._assets[quote_coin]['avail'] += order.volume_left * order.price
                else:
                    self._assets[quote_coin]['avail'] += order.volume_traded * order.avg_price
                    self._assets[base_coin]['frozen'] -= order.volume
                    self._assets[base_coin]['avail'] += order.volume_left
            self._active_orders.pop(order.order_id)

    def on_trade(self, event, trade: pywingchun.MyTrade):
        """响应成交事件的回调函数

        Args:
            event (event): 事件对象
            trade (pywingchun.MyTrade): 成交信息对象
        """
        if not self.backtest and self.strategy_id and trade.strategy_id != self.strategy_id:
            return
        self.ctx.logger.debug("{} received trade event: {}".format(self.location.uname, trade))
        # direction = wc_utils.get_position_effect(trade.instrument_type, trade.side, trade.offset)
        # try:
        #     self._get_position(trade.symbol, trade.exchange_id, direction).apply_trade(trade)
        # except Exception as err:
        #     exc_type, exc_obj, exc_tb = sys.exc_info()
        #     self.ctx.logger.error('apply trade error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
        # coins = trade.symbol.split('_')
        # for coin in coins:
        #     if coin in self._assets and self._assets[coin]["exchange_id"] == trade.exchange_id:
        #         self.subject.on_next(self.make_asset_event(self._assets[coin]))
        #     else:
        #         self.ctx.logger.error(f"No asset info: {coin}")


    def on_depth(self, event, depth: pywingchun.Depth):
        """响应Depth事件的回调函数
        此函数根据Depth消息的数据更新持仓信息。
        Args:
            event (_type_): _description_
            depth (pywingchun.Depth): _description_
        """
        # self.ctx.logger.debug('{} received depth event: {}'.format(self.location.uname, depth))
        # long_pos_uid = get_position_uid(depth.symbol, depth.exchange_id, wc_constants.Direction.Long)
        # short_pos_uid = get_position_uid(depth.symbol, depth.exchange_id, wc_constants.Direction.Short)
        # if long_pos_uid in self._positions:
        #     position = self._positions[long_pos_uid]
        #     position.apply_quote(depth)
        # if short_pos_uid in self._positions:
        #     position = self._positions[short_pos_uid]
        #     position.apply_quote(depth)
        self._on_interval_check(event.gen_time)

    def on_asset(self, event, asset: pywingchun.Asset):
        """响应Asset事件的回调函数
        此函数转发event给订阅者。
        Args:
            event (event): 事件消息对象
            asset (pywingchun.Asset): 资产对象
        """
        self.ctx.logger.info("{} [{:08x}] asset report received, msg_type: {}, data: {}".
                             format(self.location.uname, self.location.uid, event.msg_type, asset))
        self.subject.on_next(event)

    def on_position(self, position):
        self.ctx.logger.debug("{} [{:08x}] position report received".
                             format(self.location.uname, self.location.uid))
        # self._positions = {}
        # if isinstance(position, pywingchun.Position):
        #     pos = msg_utils.object_as_dict(position)
        # if isinstance(pos, dict):
        #     try:
        #         pos = Position.factory(ctx=self.ctx, book=self, **position)
        #     except Exception as err:
        #         exc_type, exc_obj, exc_tb = sys.exc_info()
        #         self.ctx.logger.error('init position from dict %s, error [%s] %s', position, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
        # if isinstance(pos, Position):
        #     self._positions[pos.uid] = pos
        # else:
        #     raise TypeError("Position object required, but {} provided".format(type(pos)))
        # if self.ctx.name == "ledger":
        #     for pos in self._positions:
        #         self.subject.on_next(pos.event)
        #     self.ctx.db.dump_book(self)
        #     self.ctx.logger.info("book {} [{:08x}] saved in database".format(self.location.uname, self.location.uid))

    @property
    def assets(self):
        return list(self._assets.values())

    @property
    def positions(self):
        return list(self._positions.values())

    @property
    def active_orders(self):
        return list(self._active_orders.values())

    @property
    def margin(self):
        return sum([position.margin for position in self._positions.values()])

    @property
    def dynamic_equity(self):
        total_value = self.avail
        for pos in self.positions:
            if isinstance(pos, FuturePosition):
                total_value += (pos.margin + pos.position_pnl)
        return total_value

    @property
    def unrealized_pnl(self):
        return sum([position.unrealized_pnl for position in self._positions.values()])

    def make_asset_event(self, asset):
        data = pywingchun.Asset()
        data.avail = asset["avail"]
        data.frozen = asset["frozen"]
        data.coin = asset["coin"]
        data.margin = asset["margin"]
        data.exchange_id = asset["exchange_id"]
        data.update_time = self.ctx.now()
        data.account_id = self.tags.account_id
        data.holder_uid = self.tags.holder_uid
        data.ledger_category = self.tags.ledger_category
        event = BookEvent(wc_msg.Asset, data)
        return event

    def make_event(self, msg_type, data):
        data.trading_day = self.trading_day.strftime(DATE_FORMAT)
        data.update_time = self.ctx.now()
        data.source_id  = self.tags.source_id
        data.client_id = self.tags.client_id
        data.account_id = self.tags.account_id
        data.holder_uid = self.tags.holder_uid
        data.ledger_category = self.tags.ledger_category
        event = BookEvent(msg_type, data)
        return event

    def get_asset(self, coin):
        return self._assets.get(coin, None)

    def set_asset(self, strategy_id, exchange_id, account, coin, avail, frozen):
        asset = {
            "avail": avail,
            "frozen": frozen,
            "coin": coin,
            "margin": 0,
            "account_id": account,
            "exchange_id": exchange_id,
            "strategy_id": strategy_id
        }
        self._assets[coin] = asset
        self.subject.on_next(self.make_asset_event(asset))

    def get_position(self, symbol, exchange_id, direction = wc_constants.Direction.Long):
        uid = get_position_uid(symbol, exchange_id, direction)
        return self._positions.get(uid, None)

    def _get_position(self, symbol, exchange_id, direction = wc_constants.Direction.Long):
        uid = get_position_uid(symbol, exchange_id, direction)
        if uid not in self._positions:
            position = Position.factory(ctx = self.ctx, book = self, symbol = symbol, exchange_id = exchange_id, direction = direction)
            self._positions[uid] = position
        return self._positions[uid]

