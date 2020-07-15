import json
import time
import asyncio
import hashlib

from nats.aio.client import Client as NATS
from nats.aio.errors import ErrConnectionClosed, ErrTimeout, ErrNoServers

from pyppeteer import launch

class Downloader:
    def __init__(self, browser, nc):
        self.browser = browser
        self.nc = nc
    
    async def __aenter__(self):
        return self

    async def download(self, url):
        obj = {}
        
        print('Downloading', url)
        page = await self.browser.newPage()
        obj['timestamp'] = int(time.time())
        await page.goto(url)
        
        elements = await page.querySelectorAll('a')

        for i, el in enumerate(elements):
            l = await page.evaluate('(element) => element.href', el)
            print(i, url, l)
        t = await page.plainText()
        # add a classifier able for detecting how much wait
        await page.waitFor(3000)
        msg = await page.content()

        hashed = hashlib.sha512(url.encode()).hexdigest()
        fname = 'jspage.{}.out'.format(hashed)
        with open(fname) as fout:
            print(msg, out=fout)

        obj['filename'] = fname
        obj['ip'] = ''
        
        await self.nc.publish("parser", json.dumps(msg).encode('UTF-8'))


async def run(loop):

    nc = NATS()
    browser = await launch()
    downloader = Downloader(browser, nc)
    
    async def disconnected_cb():
        print("Got disconnected...")

    async def reconnected_cb():
        print("Got reconnected...")

    await nc.connect("nats://ruser:T0pS3cr3t@127.0.0.1:4222",
                     reconnected_cb=reconnected_cb,
                     disconnected_cb=disconnected_cb,
                     max_reconnect_attempts=-1,
                     loop=loop)

    async def message_handler(msg):
        try:
            subject = msg.subject
            reply = msg.reply
            data = json.loads(msg.data.decode())
        
            print("Received a message on '{subject} {reply}': {data}".format(
            subject=subject, reply=reply, data=data))
            # await nc.publish("downloader", json.dumps(msg).encode('UTF-8'))
            #{"models": [{"link": "www.unipi.it", "timestamp": 0}, {"link": "www.sartiano.info", "timestamp": 0}, {"link": "www.corriere.it", "timestamp": 0}]}
            for el in data['models']:
                await downloader.download(el['link'])

        except Exception as e:
            print(e)
                
    await nc.subscribe("downloader", cb=message_handler)


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(loop))
    loop.run_forever()
    loop.close()
