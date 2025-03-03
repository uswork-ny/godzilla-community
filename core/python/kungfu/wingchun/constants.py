'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import pywingchun

class Source:
    SIM = "sim"
    XT = 'xt'
    XTC = 'xtc'
    XTS = 'xts'
    BINANCE = "binance"
    BINANCEPM = "binancepm"
    KUCOIN = "kucoin"
    GATEC = 'gatec'
    OKX = 'okx'
    BYBIT = 'bybit'
    CPT = "cpt"

class Exchange:
    XT = "xt"
    BINANCE = "binance"
    KUCOIN = "kucoin"
    GATE = "gate"
    OKX = "okx"
    BYBIT = "bybit"
    COINW = "coinw"

class Region:
    CN = 'CN'
    HK = 'HK'

InstrumentType = pywingchun.constants.InstrumentType
ExecType = pywingchun.constants.ExecType
Side = pywingchun.constants.Side
Offset = pywingchun.constants.Offset
BsFlag = pywingchun.constants.BsFlag
OrderStatus = pywingchun.constants.OrderStatus
Direction = pywingchun.constants.Direction
OrderType = pywingchun.constants.OrderType
VolumeCondition = pywingchun.constants.VolumeCondition
TimeCondition = pywingchun.constants.TimeCondition
OrderActionFlag = pywingchun.constants.OrderActionFlag
LedgerCategory = pywingchun.constants.LedgerCategory

AllFinalOrderStatus = [OrderStatus.Filled, OrderStatus.Error, OrderStatus.PartialFilledNotActive, OrderStatus.Cancelled]

InstrumentTypeInStockAccount = [InstrumentType.Swap,
                                InstrumentType.Spot,
                                InstrumentType.Index,
                                InstrumentType.Etf]

ENUM_TYPES = [InstrumentType,
              ExecType,
              Side,
              Offset,
              BsFlag,
              OrderStatus,
              Direction,
              OrderType,
              VolumeCondition,
              TimeCondition,
              OrderActionFlag,
              LedgerCategory]
