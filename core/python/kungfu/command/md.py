'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import pyyjj
import click
from kungfu.command import kfc, pass_ctx_from_parent
from extensions import EXTENSION_REGISTRY_MD
from kungfu.data.sqlite.data_proxy import AccountsDB


@kfc.command(help_priority=3)
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_MD.names()), help='data source')
@click.option('-a', '--account', type=str, help='account')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def md(ctx, source, account, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.db = AccountsDB(pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', ctx.locator), 'accounts')
    if account:
        account_config = ctx.db.get_td_account_config(source, source + '_' + account)
    else:
        account_config = ctx.db.get_md_account_config(source)
    ext = EXTENSION_REGISTRY_MD.get_extension(source)(low_latency, ctx.locator, account_config)
    ext.run()

