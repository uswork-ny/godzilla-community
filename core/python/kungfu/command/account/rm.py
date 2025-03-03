'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import click
from kungfu.command.account import *


@account.command()
@click.option('-i', '--id', type=str, required=True, help='id')
@click.pass_context
def rm(ctx, id):
    pass_ctx_from_parent(ctx)
    account_id = ctx.source + '_' + id
    ctx.db.delete_account(account_id)
