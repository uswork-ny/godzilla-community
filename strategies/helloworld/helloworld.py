'''
This is source code under the Apache License 2.0.
Original Author: kx@godzilla.dev
Original date: March 3, 2025
'''
import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *
from pywingchun.constants import Side, InstrumentType, OrderType

exchange = Exchange.BINANCE
instrument_type = InstrumentType.Spot

def pre_start(context):
    context.subscribe("binance", ["eth_usdt"], instrument_type, exchange)

def on_depth(context, depth):
    context.log().info(depth)
