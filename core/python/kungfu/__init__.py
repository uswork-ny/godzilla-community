'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''

import os
from .env import setup_environment_variables
from .msg import monkey_patch
from .version import get_version

setup_environment_variables()
monkey_patch()

version_file_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "version.info"))
if os.path.exists(version_file_path):
    with open(version_file_path, 'r') as version_file:
        __version__ = version_file.readline()
else:
    __version__ = get_version()
