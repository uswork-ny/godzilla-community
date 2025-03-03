'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import pywingchun
from kungfu.msg import Registry
from kungfu.msg.utils import underscore

Depth = 101
Ticker = 102
Trade = 103
IndexPrice = 104
Bar = 110

OrderInput = 201
OrderAction = 202
Order = 203
MyTrade = 204
Position = 205
Asset = 206
AssetSnapshot = 207
Instrument = 209
AlgoOrderInput = 210
AlgoOrderReport = 211
AlgoOrderModify = 212
OrderActionError = 213
MarketInfo = 214

Subscribe = 302
SubscribeAll = 303,
AdjustLeverage = 352,
NewOrderSingle = 353
CancelOrder = 354
CancelAllOrder = 355
InstrumentRequest = 356,
UnionResponse = 357,
BrokerStateRefresh = 400
BrokerState = 401
QryAsset = 402
PublishAllAssetInfo = 403
RemoveStrategy = 404
Calendar = 601
InstrumentEnd = 802

Registry.register(Depth, underscore(pywingchun.Depth.__name__), pywingchun.Depth)
Registry.register(Ticker, underscore(pywingchun.Ticker.__name__), pywingchun.Ticker)
Registry.register(Trade, underscore(pywingchun.Trade.__name__), pywingchun.Trade)
Registry.register(Bar, underscore(pywingchun.Bar.__name__), pywingchun.Bar)
Registry.register(OrderInput, underscore(pywingchun.OrderInput.__name__), pywingchun.OrderInput)
Registry.register(OrderAction, underscore(pywingchun.OrderAction.__name__), pywingchun.OrderAction)
Registry.register(OrderActionError, underscore(pywingchun.OrderActionError.__name__), pywingchun.OrderActionError)
Registry.register(Order, underscore(pywingchun.Order.__name__), pywingchun.Order)
Registry.register(MyTrade, underscore(pywingchun.MyTrade.__name__), pywingchun.MyTrade)
Registry.register(Position, underscore(pywingchun.Position.__name__), pywingchun.Position)
Registry.register(Asset, underscore(pywingchun.Asset.__name__), pywingchun.Asset)
Registry.register(Instrument, underscore(pywingchun.Instrument.__name__), pywingchun.Instrument)
Registry.register(AlgoOrderInput, "algo_order_input", str)






