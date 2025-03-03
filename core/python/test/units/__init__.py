'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import unittest
import click
from test import test
from test import pass_ctx_from_parent as pass_ctx_from_root

@test.command()
@click.pass_context
def units(ctx):
    pass_ctx_from_root(ctx)
    loader = unittest.TestLoader()
    suite = loader.discover('.')
    runner = unittest.TextTestRunner()
    runner.run(suite)
