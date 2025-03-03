'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import random
import time
import datetime
from pydantic_settings import BaseSettings
import pywingchun
import kungfu.wingchun.constants as wc_constants


def nano_to_str(nano):
    seconds = nano / 1e9
    dt = datetime.datetime.fromtimestamp(seconds)
    return dt

class BacktestSettings(BaseSettings):
    match_mode: str = "fill"
    capital: float = 10000


class BacktestEngine:
    def __init__(self) -> None:
        self.settings = BacktestSettings()
        self.orders = {}
        self.balance = self.settings.capital
        self.order_queue = []
        self.spot = 0.0
        self.equity = []
        self.order_id = 0
        self.begin_time = 0
        self.end_time = 0
        self.quote_count = 0

    def update_depth(self, depth):
        self.quote_count += 1
        if self.begin_time == 0:
            self.begin_time = depth.data_time
        self.end_time = depth.data_time
        self.depth = depth
        self.equity.append(self.balance + self.spot * depth.bid_price[0])

    def update_ticker(self, ticker):
        self.quote_count += 1
        if self.begin_time == 0:
            self.begin_time = ticker.data_time
        self.end_time = ticker.data_time
        self.ticker = ticker
        self.equity.append(self.balance + self.spot * ticker.bid_price)

    def insert_order(self, symbol, inst_type, exchange, account, limit_price, volume,
                     order_type, side, time_condition, position_side) -> int:
        # order_id = random.randint(0, 2**64 - 1)
        if (side == wc_constants.Side.Buy and self.balance < limit_price * volume) or \
                (side == wc_constants.Side.Sell and self.spot < volume):
            return 0
        if side == wc_constants.Side.Buy:
            self.balance -= limit_price * volume
            self.spot += volume
        else:
            self.balance += limit_price * volume
            self.spot -= volume
        self.order_id += 1
        self.orders[self.order_id] = {
            "symbol": symbol,
            "inst_type": inst_type,
            "exchange": exchange,
            "account": account,
            "limit_price": limit_price,
            "volume": volume,
            "order_type": order_type,
            "side": side,
            "time_condition": time_condition,
            "position_side": position_side,
            "status": wc_constants.OrderStatus.Filled if self.settings.match_mode == "fill" else wc_constants.OrderStatus.Submitted,  # noqa: E501
            "insert_time": int(time.time() * 1000)
        }
        self.order_queue.append(self.order_id)
        return self.order_id

    def cancel_order(self, account, order_id, symbol, ex_order_id, instrument_type):
        if order_id in self.orders and self.orders[order_id]["status"] == "submitted":
            self.orders[order_id]["status"] = "canceled"

    def query_order(self, account, order_id, ex_order_id, instrument_type, symbol):
        return self.orders[order_id]

    def report_order(self):
        orders = []
        for oid in self.order_queue:
            o = self.orders[oid]
            data = pywingchun.Order()
            data.order_id = oid
            data.ex_order_id = str(oid + 100000000)
            data.symbol = o['symbol']
            data.instrument_type = o['inst_type']
            data.exchange_id = o['exchange']
            data.account_id = o['account']
            data.price = o['limit_price']
            data.volume = o['volume']
            data.volume_traded = o['volume']
            data.volume_left = 0
            data.avg_price = o['limit_price']
            data.fee = 0
            data.time_condition = o['time_condition']
            data.side = o['side']
            data.position_side = o['position_side']
            data.status = o['status']
            data.order_type = o['order_type']
            data.insert_time = o['insert_time']
            data.update_time = int(time.time() * 1000)
            orders.append(data)
        self.order_queue = []
        return orders

    def report_pnl(self):
        print(f"Balance: {self.balance}")
        print(f"Spot: {self.spot}")
        print(
            f"Profit: {self.balance + self.spot * self.depth.bid_price[0] - self.settings.capital}")
        print(f"Begin_time: {nano_to_str(self.begin_time)}")
        print(f"end_time: {nano_to_str(self.end_time)}")
        print(f"quote_count: {self.quote_count}")
