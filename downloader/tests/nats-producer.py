#!/usr/bin/env python

# pip install asyncio-nats-client

import json
import asyncio
from datetime import datetime
from nats.aio.client import Client as NATS
from nats.aio.errors import ErrConnectionClosed, ErrTimeout, ErrNoServers


async def main():

    nc = NATS()

    async def disconnected_cb():
        print("Got disconnected...")

    async def reconnected_cb():
        print("Got reconnected...")

    await nc.connect("nats://ruser:T0pS3cr3t@127.0.0.1:4222",
                     reconnected_cb=reconnected_cb,
                     disconnected_cb=disconnected_cb,
                     max_reconnect_attempts=-1)

    #for url in ["www.corriere.it", "www.repubblica.it", "www.sartiano.info", "www.unipi.it"]:
    for urls in [["https://www.unipi.it", "https://www.sartiano.info", "https://www.corriere.it", "https://www.repubblica.it", "https://www.github.com", "https://news.google.com"]]:
        models = []
        for url in urls:
            models.append({"link": url, "timestamp": 0})
        msg = {"models": models}
        print(msg)
        await nc.publish("downloader", json.dumps(msg).encode('UTF-8'))

    await nc.close()

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
    loop.close()
