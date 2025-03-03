'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import os
import sys

if __name__ == '__main__':
    py_dir = os.path.dirname(os.path.abspath(__file__))
    base_dir = os.path.dirname(py_dir)
    kfc_dir = os.path.join(base_dir, 'build', 'kfc')
    sys.path.append(py_dir)
    sys.kf_sdk_home = kfc_dir
    from test import __main__
