'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import pyyjj
import pywingchun
import click
from kungfu.command.journal import journal, pass_ctx_from_parent
import kungfu.yijinjing.msg as yjj_msg
import kungfu.yijinjing.journal as kfj
import kungfu.yijinjing.time as kft
import time
import sys
import csv
import traceback
import pprint
import importlib
import os
import kungfu.msg

@journal.command()
@click.option("--max-messages", type=int, default=sys.maxsize, help="The maximum number of messages to reader before exiting")
@click.option('--msg', type=click.Choice(['all'] + kungfu.msg.Registry.type_names()), default='all',help="msg type to read")
@click.option("--continuous", is_flag=True, help="reader not to stop when no data avail util the session end")
@click.option('--category', type=str, help='category of location')
@click.option('--group', type=str, help='group of location')
@click.option('--name', type=str, help='name of location')
@click.option('--mode', type=str, default='live', help='name of location')
@click.option('--dest', type=str, default='00000000', help='dest of journal file')
@click.option('--start_time', type=int, default=0, help='start time to read message')
@click.option('--end_time', type=int, default=sys.maxsize, help='end time to read message')
@click.pass_context
def inspect(ctx, max_messages, msg, continuous, category, group, name, mode, start_time, end_time, dest):
    pass_ctx_from_parent(ctx)
    uname = '{}/{}/{}/{}'.format(category, group, name, mode)
    uid = pyyjj.hash_str_32(uname)
    ctx.category = category
    ctx.group = group
    ctx.name = name
    ctx.mode = mode
    locations = kfj.collect_journal_locations(ctx)
    location = locations[uid]
    home = kfj.make_location_from_dict(ctx, location)
    io_device = pyyjj.io_device(home)
    reader = io_device.open_reader_to_subscribe()

    for dest in location['readers']:
        dest_id = int(dest, 16)
        reader.join(home, dest_id, start_time)
    msg_count = 0
    msg_type_to_read = None if msg == "all" else kungfu.msg.Registry.meta_from_name(msg)["id"]
    pp = pprint.PrettyPrinter(indent=4)
    frame_handler = pp.pprint
    def handle(frame):
        data_as_dict = frame["data"]
        if data_as_dict is None:
            return
        print(data_as_dict)
    frame_handler = handle


    while True:
        if reader.data_available() and msg_count < max_messages:
            frame = reader.current_frame()
            if frame.gen_time > end_time:
                ctx.logger.info("reach end_time {}".format(end_time))
                break
            if frame.gen_time >= start_time and (msg == "all" or msg_type_to_read ==  frame.msg_type):
                try:
                    frame_handler(frame.as_dict())
                except Exception as e:
                    exc_type, exc_obj, exc_tb = sys.exc_info()
                    ctx.logger.error('error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
                msg_count +=1
            reader.next()
        elif msg_count >= max_messages:
            ctx.logger.info("reach max messages {}".format(max_messages))
            break
        elif not reader.data_available():
            if not continuous:
                ctx.logger.info("no data is available")
                break
            else:
                time.sleep(0.1)