#!/bin/bash
#######################################################
# The script to copy libs for release build
#######################################################
cd ../core/build/Release || exit
cp *.* ../
cd ../build_extensions || exit
cp -rf binance/ ../../../core/python/extensions/