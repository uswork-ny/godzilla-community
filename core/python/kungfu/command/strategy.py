'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import pyyjj
import click
import pathlib
import json
import time
from importlib import util
from pywingchun import Context
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import Runner, replay_setup
from kungfu.wingchun.strategy import Strategy
from kungfu.yijinjing.log import create_logger
from kungfu.data.sqlite.data_proxy import LedgerDB, AccountsDB
import kungfu.wingchun.constants as wc_constants
from xt4.spot import XTApi


@kfc.command(help_priority=4)
@click.option('-g', '--group', type=str, default='default', help='group')
@click.option('-n', '--name', type=str, required=True, help='name')
@click.option('-p', '--path', type=str, required=True, help='path of strategy py file')
@click.option('-c', '--config', type=str, default=[], multiple=True, help='config of strategy (one strategy instance with different config)')
@click.option('-d', '--cancel', is_flag=True, help='cancel active orders')
@click.option('-s', '--symbol', type=str, default="", help='cancel active orders by symbol')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('-r', '--replay', is_flag=True, help='run in replay mode')
@click.option('-i', '--session_id', type=int, help='replay session id, MUST be specified if replay is set')
@click.option('-t', '--backtest', is_flag=True, help='run in backtest mode')
@click.option('-b', '--begin_time', type=str, required=False, help='begin time for backtest')
@click.option('-e', '--end_time', type=str, required=False, help='end time for backtest')
@click.pass_context
def strategy(ctx, group, name, path, config, cancel, symbol, low_latency, replay, session_id, backtest, begin_time, end_time):
    pass_ctx_from_parent(ctx)
    ctx.group = group
    ctx.name = name
    ctx.paths = path
    ctx.low_latency = low_latency if not backtest else True
    ctx.replay = replay
    ctx.backtest = backtest
    ctx.category = 'strategy'
    if ctx.backtest:
        mode = pyyjj.mode.BACKTEST
    elif ctx.replay:
        mode = pyyjj.mode.REPLAY
    else:
        mode = pyyjj.mode.LIVE
    ctx.mode = pyyjj.get_mode_name(mode)
    ctx.location = pyyjj.location(mode, pyyjj.category.STRATEGY, group, name, ctx.locator)
    ctx.logger = create_logger(name, ctx.log_level, ctx.location)

    ctx.strategies = {}
    ctx.configs = {}    # define "configs" for strategy assigning config
    config_files = {}
    for c in config:
        conf_path = pathlib.Path(c)
        if conf_path.is_file() or cancel:
            config_files[str(conf_path.absolute())] = 1
        elif conf_path.is_dir():
            for file in [p for p in conf_path.iterdir()]:
                if (cancel or file.is_file()) and file.suffix == '.json':
                    config_files[str(file.absolute())] = 1
    if cancel:
        acct_db = AccountsDB(pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', ctx.locator), 'accounts')
        accounts = acct_db.get_accounts()
        apis = []
        for a in accounts:
            url = "http://sapi.xt.com"
            if "config" in a and "source_name" in a:
                if "run_env" in a["config"] and a["source_name"] == "xtc":
                    if a["config"]["run_env"] == 'prod':
                        url = "http://sapi-inc.xt.com"
                    if a["config"]["run_env"] == 'test':
                        url = "http://sapi.xt-qa.com"
            xtapi = XTApi(host=url, access_key=a["config"]["access_key"], secret_key=a["config"]["secret_key"])
            apis.append(xtapi)
        for conf in config_files:
            strategy_id = pyyjj.hash_str_32(path+str(conf))
            order_db = LedgerDB(ctx.location, ctx.name)
            xtapi = None
            if symbol != "":
                if len(config_files) > 1:
                    ctx.logger.error("cancel by symbol support only 1 config file")
                    break
                for xtapi in apis:
                    try:
                        ret = xtapi.cancel_open_orders(symbol)
                        ctx.logger.info(ret)
                        break
                    except Exception as e:
                        ctx.logger.error(str(e))
            else:
                orders = order_db.get_strategy_orders(strategy_id)
                for o in orders:
                    if o['ex_order_id'] != "" and o["status"] not in wc_constants.AllFinalOrderStatus:
                        for xtapi in apis:
                            try:
                                ret = xtapi.cancel_order(o['ex_order_id'])
                                ctx.logger.info(ret)
                                break
                            except Exception as e:
                                if (str(e).find('订单不存在') >= 0):
                                    continue
                                else:
                                    ctx.logger.error(str(e))
            order_db.cancel_strategy_orders(strategy_id)
        return

    str_conf = {}
    runner = Runner(ctx, mode)
    for conf in config_files.keys():
        try:
            with open(str(conf)) as f:
                str_conf = json.load(f)
        except Exception as e:
            print(f"{pathlib.Path.cwd()}")
            print(f"invalid config file {str(e)}: {c}")
            continue
        if path.endswith('.py'):
            ctx.path = path
            ctx.config = str_conf
            ctx.config_path = str(conf)
            s = Strategy(ctx)
        else:
            spec = util.spec_from_file_location(str(pathlib.Path(path).name).split('.')[0], path)  # noqa: E501
            cpp = util.module_from_spec(spec)
            spec.loader.exec_module(cpp)
            s = cpp.Strategy(ctx.location)
        runner.add_strategy(s, path+str(conf))
        ctx.strategies[s.get_uid()] = s  # keep strategy alive for pybind11

    if backtest:
        if begin_time:
            runner.set_begin_time(int(begin_time))
        else:
            runner.set_begin_time(0)
        if end_time:
            runner.set_end_time(end_time)
        else:
            runner.set_end_time(int(time.time()) * 1000000000)
    elif replay:
        ctx.session_id = session_id
        replay_setup.setup(ctx, session_id, strategy, runner)

    runner.run()

