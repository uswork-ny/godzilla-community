## startup scripts for tri strategy

# 0.account setting
xtc: for inner setup an account called xt_user1
xtc: for project setup an account called xt_user2

# 1.cancel open orders

$ python startup.py cancel-all-orders -a xt_user1 -i "/root/dev/godzilla/xtlianghua_sanjiao_inner.csv"
$ python startup.py cancel-all-orders -a xt_user2 -i "/root/dev/godzilla/xtlianghua_sanjiao_project.csv"

# 2.generate pm2 start scripts using startup.py like this:

$ python startup.py strategy -p tri_inner -a xt_user1 -n 32 -i "/root/dev/godzilla/xtlianghua_sanjiao_inner.csv" -o "/opt/configs" -l info
$ python startup.py strategy -p tri_project -a xt_user2 -n 4 -i "/root/dev/godzilla/xtlianghua_sanjiao_project.csv" -o "/opt/configs" -l info

the script will generate a shell script called tri_inner.sh and tri_project.sh

# 3.startup system services

$ cd /opt/app/godzilla/scripts/triangular_arbitrage
$ bash run.sh start

# 4.startup strategy services

$ cd /opt/pm2
$ bash tri_inner.sh
$ bash tri_project.sh

# 5. startup lhplat http server
$ cd /opt/app/pm2-http-server
$ pm2 start http.js

# 6.startup docker
$ cd /opt/app/godzilla/intergration
$ docker build ./ -t intergration
if `docker ps` shows intergetion not running:
$ docker run -d -u 0:0 -p 5000:5000 -e HOST_IP="[current server ip]" -v /root/.pm2:/root/.pm2 -v /root/.pm2/rpc.sock:/root/.pm2/rpc.sock -v /root/.pm2/pub.sock:/root/.pm2/pub.sock -v /opt/configs:/opt/configs -v /opt/pm2:/opt/pm2 -v ~/.ssh:/root/.ssh -v /opt/app/godzilla:/opt/app/godzilla intergration flask run --host=0.0.0.0

# note: the input csv demo is commited in inut.csv.sample
