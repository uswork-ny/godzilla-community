#!/bin/bash

WORK_HOME=`dirname $0`


start() {
    python3 $WORK_HOME/../../core/python/dev_run.py -l trace account -s binance show | grep gz_user1
    if [ $? != 0 ]; then
        echo "please add test user with name/id [gz_user1]"
        python3 $WORK_HOME/../../core/python/dev_run.py -l trace account -s binance add
    fi
    # clear journal
    echo "clearing journal..."
    find ~/.config/kungfu/app/ -name "*.journal" | xargs rm -f
    
    # start master
    pm2 start master.json
    echo "starting master..."
    sleep 10
    
    # start ledger
    pm2 start ledger.json
    echo "starting ledger..."
    sleep 10
    
    # start binance md
    pm2 start md_binance.json
    echo "starting md binance..."
    sleep 10

    # start binance td
    pm2 start td_binance.json
    echo "starting td binance..."
    sleep 10

    echo "pm2 ls to show the services"
}

stop() {
    master_pid=`ps -ef | grep python |  grep master | awk '{ print $2 }'`
    if [ "$master_pid" != "" ]; then
        kill -2 $master_pid
    fi
}


if [ $# -lt 1 ]; then
    echo "please indicate action [start/stop]"
    exit 1
fi
if [ "$1" = "start" ]; then
    start
elif [ "$1" = "stop" ]; then
    stop
else
    echo "invalid action: $1"
fi
