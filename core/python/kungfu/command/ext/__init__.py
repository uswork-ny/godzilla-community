'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import pyyjj
import os
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.yijinjing.log import create_logger


@kfc.group()
@click.pass_context
def ext(ctx):
    pass_ctx_from_root(ctx)
    ctx.journal_util_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'util', 'extension', ctx.locator)
    ctx.logger = create_logger('ext', ctx.log_level, ctx.journal_util_location)
    if not os.getenv('KF_NO_EXT'):
        pass
    else:
        print('Extension disabled by KF_NO_EXT')
        ctx.logger.warning('Trying to manage extension while disallowed by KF_NO_EXT')


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = ctx.parent.logger
