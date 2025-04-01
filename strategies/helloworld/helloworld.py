'''
This is source code under the Apache License 2.0.
Original Author: kx@godzilla.dev
Original date: March 3, 2025
'''
from kungfu.wingchun.constants import *
from pywingchun.constants import Side, InstrumentType, OrderType

exchange = Exchange.BINANCE
instrument_type = InstrumentType.Spot

def pre_start(context):
    config = context.get_config()
    context.subscribe(config["md_source"], [config["symbol"]], instrument_type, exchange)

def on_depth(context, depth):
    breakpoint()
    context.log().info(depth)
