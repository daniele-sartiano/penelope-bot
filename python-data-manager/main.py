import asyncio
import os
import sys

from nats.aio.client import Client as NATS
from datamanager.datamanager import *


async def main():
    dm = DataManager([os.getenv('SCYLLADB')])

    nc = NATS()

    async def disconnected_cb():
        print("Got disconnected...")

    async def reconnected_cb():
        print("Got reconnected...")

    await nc.connect(os.getenv('NATS_URI'), #"nats://ruser:T0pS3cr3t@127.0.0.1:4222",
                     reconnected_cb=reconnected_cb,
                     disconnected_cb=disconnected_cb,
                     max_reconnect_attempts=-1)

    async def message_handler(msg):
        subject = msg.subject
        reply = msg.reply
        models = dm.deserialize(msg.data.decode())['models']
        
        for m in models:
            # ['timestamp', 'link', 'text', 'filename', 'ip', 'links']
            print('{} - {} links'.format(m['link'], len(m['links'])))
            dm.insert(m)

            for d in dm.get_models_to_download(m, threshold=50):
                print('sending', file=sys.stderr)
                await nc.publish(os.getenv('DOWNLOADER_SUBJECT'), d)

    await nc.subscribe(os.getenv('DATA_MANAGER_SUBJECT'), os.getenv('DATA_MANAGER_QUEUE'), cb=message_handler)

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
    loop.run_forever()
    loop.close()
