import time
import asyncio
from pyppeteer import launch


class Downloader:

    def __init__(self, browser):
        self.browser = browser
    
    async def __aenter__(self):
        return self

    async def download(self, url):
        print(url)
        page = await self.browser.newPage()
        await page.goto(url)
        elements = await page.querySelectorAll('a')

        for i, el in enumerate(elements):
            l = await page.evaluate('(element) => element.href', el)
            print(i, url, l)

        await page.waitFor(3000)
        t = await page.content()
        print(t)
        await page.setViewport({ 'width': 1366, 'height': 768})
        #await page.emulateMedia('screen')
        await page.pdf({'path': '{}.pdf'.format('a')})
        await page.screenshot({'path': '{}.png'.format('a'), 'fullPage': True})
        print(await page.metrics())

async def main():
    #loop = asyncio.get_event_loop()
    browser = await launch()
    d = Downloader(browser)
    #urls = ['https://www.repubblica.it', 'https://www.unipi.it', 'https://www.sartiano.info']
    urls = ['https://www.sartiano.info', 'http://pc-sartiano.iit.cnr.it:5000/']
    await asyncio.gather(*[d.download(url) for url in urls])
    #await asyncio.gather(*[sleep_print(url) for url in urls])

# async def factorial(name, number):
#     f = 1
#     for i in range(2, number+1):
#         print("Task %s: Compute factorial(%s)..." % (name, i))
#         await asyncio.sleep(1)
#         f *= i
#     print("Task %s: factorial(%s) = %s" % (name, number, f))

# loop = asyncio.get_event_loop()
# loop.run_until_complete(asyncio.gather(
#     factorial("A", 2),
#     factorial("B", 3),
#     factorial("C", 4),
# ))
# loop.close()
    
asyncio.get_event_loop().run_until_complete(main())
