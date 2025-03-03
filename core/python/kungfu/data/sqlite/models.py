'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
from . import *

from sqlalchemy import Column, ForeignKey, Integer, String, Date, Float, Boolean, PrimaryKeyConstraint
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship

Base = declarative_base()


class ModelMixin(object):
    def __init__(self, **kwargs):
        for attr in self.__mapper__.columns.keys():
            if attr in kwargs:
                setattr(self, attr, kwargs[attr])


class Account(Base):
    __tablename__ = 'account_config'
    account_id = Column(String, nullable=False, primary_key=True)
    source_name = Column(String)
    receive_md = Column(Boolean)
    config = Column(Json, nullable=False)


class Holiday(Base):
    __tablename__ = "holidays"
    __table_args__ = (PrimaryKeyConstraint('region', 'holiday'),)
    region = Column(String)
    holiday = Column(Date)


class MarketInfo(Base):
    __tablename__ = "market_info"
    __table_args__ = (PrimaryKeyConstraint(
        'symbol', 'exchange_id', 'instrument_type'),)
    symbol = Column(String)
    exchange_id = Column(String)
    instrument_type = Column(InstrumentType)
    config = Column(String)
    update_time = Column(Integer)


class FutureInstrument(ModelMixin, Base):
    __tablename__ = 'future_instrument'
    __table_args__ = (PrimaryKeyConstraint('symbol', 'exchange_id'),)
    symbol = Column(String)
    exchange_id = Column(String)
    instrument_type = Column(InstrumentType)
    product_id = Column(String)
    contract_multiplier = Column(Integer)
    price_tick = Column(Float)
    open_date = Column(String)
    create_date = Column(String)
    expire_date = Column(String)
    delivery_year = Column(Integer)
    delivery_month = Column(Integer)
    is_trading = Column(Boolean)
    long_margin_ratio = Column(Float)
    short_margin_ratio = Column(Float)


class Commission(Base):
    __tablename__ = 'commission'
    __table_args__ = (PrimaryKeyConstraint(
        'symbol', 'exchange_id', 'instrument_type'),)
    symbol = Column(String)
    exchange_id = Column(String)
    instrument_type = Column(InstrumentType)
    account_id = Column(String)
    broker_id = Column(String)
    mode = Column(Integer)
    open_ratio = Column(Float)
    close_ratio = Column(Float)
    close_today_ratio = Column(Float)
    min_commission = Column(Float)


class Order(ModelMixin, Base):
    __tablename__ = 'orders'
    order_id = Column(UINT64, primary_key=True)
    ex_order_id = Column(String)
    insert_time = Column(Integer)
    update_time = Column(Integer)
    symbol = Column(String)
    exchange_id = Column(String)
    source_id = Column(String)
    account_id = Column(String)
    instrument_type = Column(InstrumentType)
    price = Column(Float)
    stop_price = Column(Float)
    volume = Column(Float)
    volume_traded = Column(Float)
    volume_left = Column(Float)
    fee = Column(Float)
    fee_currency = Column(String)
    status = Column(OrderStatus)
    time_condition = Column(TimeCondition)
    side = Column(Side)
    position_side = Column(Direction)
    order_type = Column(OrderType)
    strategy_id = Column(Integer)


class MyTrade(ModelMixin, Base):
    __tablename__ = 'mytrades'
    trade_id = Column(UINT64, primary_key=True)
    order_id = Column(UINT64, nullable=False)
    trade_time = Column(Integer)
    symbol = Column(String)
    exchange_id = Column(String)
    account_id = Column(String)
    source_id = Column(String)
    instrument_type = Column(InstrumentType)
    side = Column(Side)
    offset = Column(Offset)
    price = Column(Float)
    volume = Column(Integer)
    fee = Column(Float)
    fee_currency = Column(String)
    base_currency = Column(String)
    quote_currency = Column(String)


class AssetMixin(ModelMixin):
    update_time = Column(Integer)
    holder_uid = Column(Integer)
    ledger_category = Column(LedgerCategory)
    coin = Column(String)
    account_id = Column(String)
    exchange_id = Column(String)
    avail = Column(Float)
    margin = Column(Float)
    frozen = Column(Float)
    strategy_id = Column(Integer)


class Asset(AssetMixin, Base):
    __tablename__ = "asset"
    __table_args__ = (PrimaryKeyConstraint(
        'holder_uid', 'coin', 'exchange_id'),)


class AssetSnapshot(AssetMixin, Base):
    __tablename__ = "asset_snapshot"
    __table_args__ = (PrimaryKeyConstraint("holder_uid", 'update_time'),)


class Position(ModelMixin, Base):
    __tablename__ = "position"
    id = Column(UINT64, primary_key=True)
    update_time = Column(Integer)

    symbol = Column(String)
    instrument_type = Column(InstrumentType)
    exchange_id = Column(String)
    strategy_id = Column(Integer)
    holder_uid = Column(Integer)
    ledger_category = Column(LedgerCategory)
    account_id = Column(String)
    source_id = Column(String)

    direction = Column(Direction)
    volume = Column(Integer)
    frozen_total = Column(Integer)
    last_price = Column(Float)
    avg_open_price = Column(Float)
    settlement_price = Column(Float)
    margin = Column(Float)
    realized_pnl = Column(Float)
    unrealized_pnl = Column(Float)
    margin_ratio = Column(Float)
    contract_multiplier = Column(Integer)

    def __init__(self, **kwargs):
        super(Position, self).__init__(**kwargs)
        self.id = self.holder_uid << 32 ^ pyyjj.hash_str_32(
            "{}.{}.{}".format(self.symbol, self.exchange_id, self.direction))


class Location(Base):
    __tablename__ = "location"
    uid = Column(Integer, primary_key=True)
    info = Column(Json)


class AlgoOrder(Base):
    __tablename__ = "algo_order"
    order_id = Column(UINT64, primary_key=True)
    algo_type = Column(String)
    sender_uid = Column(Integer)
    update_time = Column(Integer)
    params = Column(Json)
    status = Column(Json)
    active = Column(Boolean)
