'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import os, shutil
import extensions
import click
from kungfu.command.ext import ext as kfext, pass_ctx_from_parent


@kfext.command(help='uninstall extension')
@click.option("-n", "--name", required=True, help="KungFu extension name")
@click.pass_context
def uninstall(ctx, name):
    pass_ctx_from_parent(ctx)
    install_path = os.path.join(extensions.extension_path, name)
    if os.path.exists(install_path):
        shutil.rmtree(install_path)
        ctx.parent.logger.info('Uninstalled extension %s', name)
        click.echo('Uninstalled extension ' + name)
    else:
        click.echo('Extension ' + name + 'not found')
