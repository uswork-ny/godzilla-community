'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''

# Environment variables that affects:
#   KF_HOME - base folder for kungfu files
#   KF_LOG_LEVEL - logging level
#   KF_NO_EXT - disable extensions if set


import kungfu.command as kfc
from kungfu.command import __all__

def main():
    kfc.execute()

if __name__ == "__main__":
    main()
