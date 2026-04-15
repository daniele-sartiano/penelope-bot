import asyncio
import hashlib
import os
import sys

import orjson


class Downloader:
    """JavaScript-aware page downloader.

    Uses a shared Playwright browser to render pages (including JS-rendered
    content) and writes the final HTML to disk. Concurrency is bounded so
    one NATS message does not spawn hundreds of Chromium contexts at once.
    """

    USER_AGENT = "penelope-bot"
    DEFAULT_MAX_CONCURRENT = 5
    NAVIGATION_TIMEOUT_MS = 30_000
    BATCH_SIZE = 10

    def __init__(self, browser, directory, max_concurrent=DEFAULT_MAX_CONCURRENT):
        self.browser = browser
        self.directory = directory
        self.prefix = "{}/".format(directory) if directory else ""
        self.semaphore = asyncio.Semaphore(max_concurrent)

    async def _download_one(self, model):
        async with self.semaphore:
            context = await self.browser.new_context(user_agent=self.USER_AGENT)
            try:
                page = await context.new_page()
                # networkidle waits until the page stops making requests,
                # more accurate than a fixed sleep.
                await page.goto(
                    model["link"],
                    wait_until="networkidle",
                    timeout=self.NAVIGATION_TIMEOUT_MS,
                )
                html = await page.content()
            finally:
                await context.close()

            digest = hashlib.sha256(model["link"].encode()).hexdigest()[:32]
            filename = "{}jspage.{}.out".format(self.prefix, digest)
            with open(filename, "w", encoding="utf-8") as f:
                f.write(html)

            model["filename"] = filename
            model["timestamp"] = -1
            model["ip"] = ""
            return model

    async def download(self, msg):
        """Process a NATS payload and yield batched results."""
        models = orjson.loads(msg)["models"]
        pending = [asyncio.create_task(self._download_one(m)) for m in models]

        batch = []
        for task in asyncio.as_completed(pending):
            try:
                result = await task
            except Exception as e:
                print("jsdownload failed:", e, file=sys.stderr)
                continue
            batch.append(result)
            if len(batch) >= self.BATCH_SIZE:
                yield orjson.dumps({"models": batch})
                batch = []

        if batch:
            yield orjson.dumps({"models": batch})
