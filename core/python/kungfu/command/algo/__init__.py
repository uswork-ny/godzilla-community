'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import pyyjj
import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import replay_setup

@kfc.group(help_priority=6)
@click.pass_context
def algo(ctx):
    pass_ctx_from_parent(ctx)


