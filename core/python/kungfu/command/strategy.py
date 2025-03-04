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
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import Runner, replay_setup
from kungfu.wingchun.strategy import Strategy
from kungfu.yijinjing.log import create_logger


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
        print('not implemented')
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

