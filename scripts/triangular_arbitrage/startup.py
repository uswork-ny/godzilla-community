import click
import os
import shutil
import json
import pyyjj
import kungfu.yijinjing.journal as kfj
import pandas as pd

from kungfu.data.sqlite.data_proxy import AccountsDB
from xt4.spot import XTApi
from kungfu.wingchun.constants import *



@click.group()
def run():
    ...


@click.command()
@click.option('-p', '--project', required=True, type=str, help='project name eg."tri_inner" or "tri_project"')
@click.option('-a', '--account', required=True, type=str, help='account eg."xt_user1"')
@click.option('-n', '--num_of_groups', required=True, type=int, help='num of groups to generate eg."32"')
@click.option('-i', '--input_file', required=True, type=str, help='input csv file eg."/root/dev/godzilla/xtlianghua_sanjiao_inner.csv"')
@click.option('-o', '--out_path', required=True, type=str, help='config out path eg."/opt/configs"')
@click.option('-l', '--level', default="warning", type=str, help='log level [info,warning...]')
def strategy(project, account, num_of_groups, input_file, out_path, level):
    '''global vars for tri strategy production startup scripts'''
    config_sample = "/opt/app/godzilla/strategies/triangular_arbitrage/str_para.json.sample"
    exchange = 'xtc'
    url = "http://sapi-inc.xt.com"
    pm2_script_path = "/opt/pm2/"
    pm2_proj_name = project
    config_out_path = os.path.join(out_path, pm2_proj_name)

    '''query accounts db'''
    home = os.path.join(os.path.expanduser("~"), '.config', 'kungfu', 'app')
    locator = kfj.Locator(home)
    location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', locator)
    account_db = AccountsDB(location, 'accounts')
    accounts = account_db.get_accounts()
    print(accounts)
    ak = None
    sk = None
    env = "prod"
    for a in accounts:
        u_id = a["config"]["user_id"]
        source_name = a['source_name']
        if u_id == account and source_name == exchange:
            ak = a["config"]["access_key"]
            sk = a["config"]["secret_key"]
            env = "test" if a["config"]["run_env"] == 'test' else env
        if env == "test":
            url = "http://sapi.xt-qa.com"

    '''not found in db then exit'''
    if not ak:
        print(f'access_key not found in db for user {account} in exhange {exchange}')
        return

    '''rest api'''
    api = XTApi(host=url, access_key=ak, secret_key=sk)

    '''read config sample'''
    f = open(config_sample, "r")
    data = json.loads(f.read())
    data['account'] = account
    print(data)

    res = api.balances()
    print(res)

    df = pd.read_csv(input_file)
    print(df)

    '''delete old dir'''
    if os.path.exists(config_out_path):
        for dir_name in os.listdir(config_out_path):
            full_path = os.path.join(config_out_path, dir_name)
            if os.path.isdir(full_path):
                shutil.rmtree(full_path)
    else:
        os.makedirs(config_out_path)

    '''create new dir'''
    counter = 1
    while counter <= num_of_groups:
        dir_name = 'conf' + str(counter)
        full_path = os.path.join(config_out_path, dir_name)
        if not os.path.exists(full_path):
            os.makedirs(full_path)
        counter += 1

    '''create pm2 json'''
    pm2_proj_path = os.path.join(pm2_script_path, pm2_proj_name)
    if os.path.exists(pm2_proj_path):
        shutil.rmtree(pm2_proj_path)
        os.makedirs(pm2_proj_path)
    else:
        os.makedirs(pm2_proj_path)

    counter = 1
    while counter <= num_of_groups:
        app_list = []
        json_script_path = os.path.join(pm2_proj_path, pm2_proj_name + "__conf" + str(counter) + ".json")
        app = {}
        dir_name = 'conf' + str(counter)
        full_path = os.path.join(config_out_path, dir_name)
        app["name"] = pm2_proj_name+"__conf"+str(counter)
        app["cwd"] = "./"
        app["script"] = "/opt/app/godzilla/core/python/dev_run.py"
        app["exec_interpreter"] = "python3"
        app["args"] = f"-l {level} strategy -n {pm2_proj_name}__conf" + str(counter) + f" -p /opt/app/godzilla/strategies/triangular_arbitrage/triangular_arbitrage.py -c "+full_path+"/"
        app["watch"] = "false"
        app_list.append(app)
        counter += 1
        with open(json_script_path, 'w') as file:
            json.dump({"apps": app_list}, file)

    '''create pm2 start-up shell script'''
    pm2_shell_script_path = pm2_script_path + pm2_proj_name + ".sh"
    f = open(pm2_shell_script_path, 'w')
    counter = 1
    while counter <= num_of_groups:
        file_name = f'{pm2_proj_name}__conf' + str(counter) + '.json'
        full_path = os.path.join(pm2_proj_name, file_name)
        f.write('pm2 start ' + full_path + '\n')
        f.write('sleep 2\n')
        counter += 1
    f.close()
    os.chmod(pm2_shell_script_path, 0o755)

    '''create cancel orders shell script'''
    cancel_shell_script_path = pm2_script_path + "cancel_" + pm2_proj_name + ".sh"
    f = open(cancel_shell_script_path, 'w')
    counter = 1
    while counter <= num_of_groups:
        total_path = os.path.join(config_out_path, 'conf' + str(counter))
        f.write("python3 /opt/app/godzilla/core/python/dev_run.py -l info strategy -n "+pm2_proj_name+"__conf"+str(counter)+" -p /opt/app/godzilla/strategies/triangular_arbitrage/triangular_arbitrage.py -c "+total_path+"/ -d\n")
        f.write("sleep 2\n")
        counter += 1
    f.close()
    os.chmod(cancel_shell_script_path, 0o755)

    '''asset available in curr account'''
    assets = {}
    for i in res['assets']:
        ccy = i['currency']
        assets[ccy] = i

    '''loop over pari list'''
    symbols = []
    counter = 1
    for index, row in df.iterrows():
        # print(row['symbol'], row['base'], row['quote'])
        if row['base'] in assets:
            data['assets']['base']['asset'] = row['base']
            data['assets']['base']['cash_limit'] = float(assets[row['base']]['availableAmount'])
            data['assets']['quote']['asset'] = 'usdt'
            data['assets']['quote']['cash_limit'] = float(assets['usdt']['availableAmount'])
            data['assets']['currency']['asset'] = row['quote']
            data['assets']['currency']['cash_limit'] = float(assets[row['quote']]['availableAmount'])
            print(data)
            symbols.append(row['symbol'])
            json_object = json.dumps(data)
            dir_name = 'conf' + str(counter)
            full_path = os.path.join(config_out_path, dir_name)
            with open(os.path.join(full_path, row['symbol'] + '.str_para.json'), 'w') as outfile:
                outfile.write(json_object)
            counter += 1
            if counter == num_of_groups + 1:
                counter = 1
    json_object = json.dumps({'symbols': symbols})
    with open(os.path.join(config_out_path, 'symbols.json'), 'w') as outfile:
        outfile.write(json_object)


@click.command()
@click.option('-a', '--account', required=True, type=str, help='account eg."xt_user1"')
@click.option('-i', '--input_file', required=True, type=str, help='input csv file eg."/root/dev/godzilla/xtlianghua_sanjiao_inner.csv"')
def cancel_all_orders(account, input_file):
    '''global vars for tri strategy production startup scripts'''
    exchange = 'xtc'
    url = "http://sapi-inc.xt.com"

    '''query accounts db'''
    home = os.path.join(os.path.expanduser("~"), '.config', 'kungfu', 'app')
    locator = kfj.Locator(home)
    location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', locator)
    account_db = AccountsDB(location, 'accounts')
    accounts = account_db.get_accounts()
    print(accounts)
    ak = None
    sk = None
    env = "prod"
    for a in accounts:
        u_id = a["config"]["user_id"]
        source_name = a['source_name']
        if u_id == account and source_name == exchange:
            ak = a["config"]["access_key"]
            sk = a["config"]["secret_key"]
            env = "test" if a["config"]["run_env"] == 'test' else env
        if env == "test":
            url = "http://sapi.xt-qa.com"

    '''not found in db then exit'''
    if not ak:
        print(f'access_key not found in db for user {account} in exhange {exchange}')
        return

    '''rest api'''
    api = XTApi(host=url, access_key=ak, secret_key=sk)

    '''read csv'''
    df = pd.read_csv(input_file)
    print(df)

    for index, row in df.iterrows():
        symbol = row['symbol']
        print(f'cancel orders for {symbol}')
        api.cancel_open_orders(symbol=symbol)


run.add_command(strategy)
run.add_command(cancel_all_orders)

if __name__ == '__main__':
    run()
