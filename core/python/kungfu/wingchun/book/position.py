'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
from pyyjj import hash_str_32
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import *
import datetime
import kungfu.wingchun.msg as wc_msg

DATE_FORMAT = "%Y%m%d"

def get_uname(symbol, instrument_type, exchange_id, direction):
    return "{}.{}.{}.{}".format(symbol, instrument_type, exchange_id, int(direction))

def get_uid(symbol, instrument_type, exchange_id, direction):
    uname = get_uname(symbol, instrument_type, exchange_id, direction)
    return hash_str_32(uname)

class Position:
    registry = {}
    def __init__(self, ctx, book, **kwargs):
        self.ctx = ctx
        self.book = book
        self.symbol = kwargs["symbol"]
        self.instrument_type = kwargs["instrument_type"]
        self.exchange_id = kwargs["exchange_id"]
        self.direction = kwargs.get("direction", Direction.Long)
        if isinstance(self.direction, int):
            self.direction = Direction(self.direction)
        self.uname = get_uname(self.symbol, self.instrument_type, self.exchange_id, self.direction)
        self.uid = get_uid(self.symbol, self.instrument_type, self.exchange_id, self.direction)
        self._last_check = 0

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        for inst_type in cls._INSTRUMENT_TYPES:
            cls.registry[inst_type] = cls

    def __repr__(self):
        return self.event.data.__repr__()

    def _on_interval_check(self, now):
        if self._last_check + int(1e9) * 30 < now:
            self.subject.on_next(self.event)
            self._last_check = now

    @classmethod
    def factory(cls, inst_type, ctx, book, **kwargs):
        return cls.registry[inst_type](ctx, book, **kwargs)

    @property
    def subject(self):
        return self.book.subject

    def apply_trade(self, trade):
        raise NotImplementedError

    def apply_quote(self, quote):
        raise NotImplementedError


class FuturePosition(Position):
    _INSTRUMENT_TYPES = [InstrumentType.Future]
    def __init__(self, ctx, book, **kwargs):
        super(FuturePosition, self).__init__(ctx, book, **kwargs)
        inst_info = self.ctx.get_inst_info(self.symbol, self.exchange_id)
        self.contract_multiplier = inst_info["contract_multiplier"]
        self.margin_ratio = inst_info["long_margin_ratio"] if self.direction == Direction.Long else inst_info["short_margin_ratio"]
        self.settlement_price = kwargs.pop("settlement_price", 0.0)
        self.last_price = kwargs.pop("last_price", 0.0)
        self.avg_open_price = kwargs.pop("avg_open_price", 0.0)
        self.volume = kwargs.pop("volume", 0)
        self.margin = kwargs.pop("margin", 0.0)
        self.realized_pnl = kwargs.pop("realized_pnl", 0.0)

    @property
    def unrealized_pnl(self):
        if not is_valid_price(self.last_price):
            return 0.0
        else:
            return (self.last_price - self.avg_open_price) * self.volume * \
                   self.contract_multiplier * (1 if self.direction == Direction.Long else -1)
    @property
    def position_pnl(self):
        if is_valid_price(self.last_price):
            return self.contract_multiplier * self.last_price * self.volume * self.margin_ratio - self.margin
        else:
            return 0.0

    @property
    def event(self):
        data = pywingchun.Position()
        data.symbol = self.symbol
        data.exchange_id = self.exchange_id
        data.instrument_type = self.instrument_type
        data.last_price = self.last_price
        data.settlement_price = self.settlement_price
        data.avg_open_price = self.avg_open_price
        data.realized_pnl = self.realized_pnl
        data.unrealized_pnl = self.unrealized_pnl
        data.position_pnl = self.position_pnl
        data.volume = self.volume
        data.direction = self.direction
        data.margin = self.margin
        return self.book.make_event(wc_msg.Position, data)

    def apply_trade(self, trade):
        if trade.offset == Offset.Open:
            self._apply_open(trade)
        elif trade.offset == Offset.Close or trade.offset == Offset.CloseToday or trade.offset == Offset.CloseYesterday:
            self._apply_close(trade)
        self.book.subject.on_next(self.event)

    def apply_quote(self, quote):
            # pre_margin = self.margin
            # self.margin = self.contract_multiplier * self.settlement_price * self.volume * self.margin_ratio
            # self.book.avail -= (self.margin - pre_margin)
        if is_valid_price(quote.last_price):
            self.last_price = quote.last_price
        self._on_interval_check(self.ctx.now())

    def _apply_close(self, trade):
        if self.volume < trade.volume:
            raise Exception("{} over close position, current volume is {}, {} to close".format(self.uname, self.volume, trade.volume))
        if trade.offset == Offset.CloseToday:
            raise Exception("{} over close today position, current volume is {} to close".format(self.uname, trade.volume))
        margin = self.contract_multiplier * trade.price * trade.volume * self.margin_ratio
        self.margin -= margin
        self.book.avail += margin
        self.volume -= trade.volume
        realized_pnl = (trade.price - self.avg_open_price) * trade.volume * \
                       self.contract_multiplier * (1 if self.direction == Direction.Long else -1)
        self.realized_pnl += realized_pnl
        self.book.realized_pnl += realized_pnl

    def _apply_open(self, trade):
        margin = self.contract_multiplier * trade.price * trade.volume * self.margin_ratio
        self.margin += margin
        self.book.avail -= margin
        self.avg_open_price = (self.avg_open_price * self.volume + trade.volume * trade.price) / (self.volume + trade.volume)
        self.volume += trade.volume

