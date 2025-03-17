#!/bin/bash
#######################################################
# The script to install libs for a new build machine
#######################################################

# update and upgrade
apt update
apt upgrade

# install rz sz and unzip
apt install lrzsz
apt install unzip

# adjust timezone and restart crontab
ln -snf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && echo Asia/Shanghai > /etc/timezone
service cron restart

# install build/cmake/libboost/libssl/librdkafka/glib
apt install -y build-essential cmake libboost-all-dev libssl-dev librdkafka-dev libglib2.0-dev

# install python-is-python3 and pip
apt install python-is-python3
apt install python3-pip

# pip install dep
pip install psutil sqlalchemy tabulate rx PyInquirer recordclass dateparser
pip install sortedcontainers dotted_dict loguru websocket_client click pandas requests web3 ccxt pydantic pydantic_settings

# pm2 install
apt install npm
npm install pm2@latest -g
pm2 install pm2-logrotate

# patch
python patch.py