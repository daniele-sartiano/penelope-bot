import asyncio
import os
import sys
import time

from nats.aio.client import Client as NATS
from playwright.async_api import async_playwright

from downloader.downloader import Downloader


async def run():
    nc = NATS()

    async def disconnected_cb():
        print("Got disconnected...", file=sys.stderr)

    async def reconnected_cb():
        print("Got reconnected...", file=sys.stderr)

    await nc.connect(
        os.getenv("NATS_URI"),
        reconnected_cb=reconnected_cb,
        disconnected_cb=disconnected_cb,
        max_reconnect_attempts=-1,
    )

    max_concurrent = int(os.getenv("MAX_CONCURRENT", "5"))
    directory = os.getenv("DOWNLOAD_DIRECTORY", "/data")
    parser_subject = os.getenv("PARSER_SUBJECT", "parser")
    downloader_subject = os.getenv("DOWNLOADER_SUBJECT", "downloader")
    downloader_queue = os.getenv("DOWNLOADER_QUEUE", "qdownloader")

    async with async_playwright() as p:
        browser = await p.chromium.launch(
            args=["--no-sandbox", "--disable-dev-shm-usage"],
        )
        d = Downloader(browser, directory, max_concurrent=max_concurrent)

        async def message_handler(msg):
            start_time = time.time()
            async for models in d.download(msg.data.decode()):
                print("sending msg to parser", file=sys.stderr)
                await nc.publish(parser_subject, models)
            print(
                "---- {:.2f} seconds".format(time.time() - start_time),
                file=sys.stderr,
            )

        await nc.subscribe(downloader_subject, downloader_queue, cb=message_handler)

        # Keep the process alive
        while True:
            await asyncio.sleep(3600)


if __name__ == "__main__":
    asyncio.run(run())
