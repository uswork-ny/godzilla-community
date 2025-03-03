'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import click
from kungfu.command.algo import algo
from kungfu.command import pass_ctx_from_parent
import pyyjj

@algo.command()
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('-r', '--replay', is_flag=True, help='run in replay mode')
@click.option('-i', '--session_id', type=int, help='replay session id, MUST be specified if replay is set')
@click.pass_context
def service(ctx, low_latency, replay, session_id):
    from kungfu.wingchun.service.algo import Algo
    pass_ctx_from_parent(ctx)
    ctx.low_latency = low_latency if not replay else True
    ctx.replay = replay
    ctx.mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.LIVE
    ctx.session_id = session_id
    instance = Algo(ctx)
    if replay:
        #TODO
        pass
    instance.run()
