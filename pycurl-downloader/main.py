import sys
import time
import asyncio
import os
from nats.aio.client import Client as NATS

from downloader.downloader import Downloader

async def run(loop):
    nc = NATS()

    async def disconnected_cb():
        print("Got disconnected...", file=sys.stderr)

    async def reconnected_cb():
        print("Got reconnected...", file=sys.stderr)

    await nc.connect(os.getenv('NATS_URI'),
                     reconnected_cb=reconnected_cb,
                     disconnected_cb=disconnected_cb,
                     max_reconnect_attempts=-1,
                     loop=loop)

    async def message_handler(msg):
        subject = msg.subject
        reply = msg.reply
        start_time = time.time()
        d = Downloader(os.getenv('DOWNLOAD_DIRECTORY'))
        for models in d.download(msg.data.decode()):
            print('sending msg to parser', file=sys.stderr)
            await nc.publish(os.getenv('PARSER_SUBJECT'), models)
        print("---- #{} seconds".format(time.time() - start_time), file=sys.stderr)

    await nc.subscribe(os.getenv('DOWNLOADER_SUBJECT'), os.getenv('DOWNLOADER_QUEUE'), cb=message_handler)


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(loop))
    loop.run_forever()
    loop.close()
