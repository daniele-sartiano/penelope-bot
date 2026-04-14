"""
Benchmark: sequential download (C++ downloader approach) vs
parallel download (pycurl-downloader approach).

Connects to an HTTP server running in a separate process/container.
The parallel strategy keeps at most MAX_CONCURRENT handles in flight
at the same time (realistic for a crawler, prevents backlog overflow).
"""

import hashlib
import os
import shutil
import sys
import tempfile
import time

import pycurl


SERVER_URL = os.getenv("SERVER_URL", "http://server:8888")
MAX_CONCURRENT = int(os.getenv("MAX_CONCURRENT", "20"))


# ---------------------------------------------------------------------------
# Sequential downloader (mimics C++ Downloader logic)
# ---------------------------------------------------------------------------

def download_sequential(urls, out_dir):
    """One pycurl.Curl handle per URL, one after another."""
    for url in urls:
        fname = os.path.join(out_dir, "seq.{}.out".format(
            hashlib.md5(url.encode()).hexdigest()))
        fp = open(fname, "wb")
        c = pycurl.Curl()
        c.setopt(c.URL, url)
        c.setopt(c.WRITEDATA, fp)
        c.setopt(c.FOLLOWLOCATION, True)
        c.setopt(c.USERAGENT, "penelope-bot")
        c.setopt(c.CONNECTTIMEOUT, 30)
        c.setopt(c.TIMEOUT, 300)
        c.perform()
        fp.close()
        c.close()


# ---------------------------------------------------------------------------
# Parallel downloader (mimics pycurl-downloader logic, with concurrency cap)
# ---------------------------------------------------------------------------

def download_parallel(urls, out_dir, max_concurrent=MAX_CONCURRENT):
    """Uses pycurl.CurlMulti with at most max_concurrent handles in flight."""
    m = pycurl.CurlMulti()
    handles = []
    for _ in range(max_concurrent):
        c = pycurl.Curl()
        c.setopt(c.FOLLOWLOCATION, True)
        c.setopt(c.USERAGENT, "penelope-bot")
        c.setopt(c.CONNECTTIMEOUT, 30)
        c.setopt(c.TIMEOUT, 300)
        c.setopt(c.NOSIGNAL, 1)
        c.fp = None
        handles.append(c)

    queue = list(urls)
    freelist = handles[:]
    num_processed = 0
    total = len(urls)
    failed = 0

    while num_processed < total:
        while queue and freelist:
            url = queue.pop()
            fname = os.path.join(out_dir, "par.{}.out".format(
                hashlib.md5(url.encode()).hexdigest()))
            c = freelist.pop()
            c.fp = open(fname, "wb")
            c.setopt(c.URL, url)
            c.setopt(c.WRITEDATA, c.fp)
            c.fname = fname
            m.add_handle(c)

        while True:
            ret, num_handles = m.perform()
            if ret != pycurl.E_CALL_MULTI_PERFORM:
                break

        if num_handles:
            m.select(0.1)

        while True:
            num_q, ok_list, err_list = m.info_read()
            for c in ok_list:
                c.fp.close()
                c.fp = None
                m.remove_handle(c)
                freelist.append(c)
            for c, errno, errmsg in err_list:
                c.fp.close()
                c.fp = None
                m.remove_handle(c)
                freelist.append(c)
                failed += 1
            num_processed += len(ok_list) + len(err_list)
            if num_q == 0:
                break

    for c in handles:
        c.close()
    m.close()
    if failed:
        print(f"  ({failed} failed downloads)", file=sys.stderr)


# ---------------------------------------------------------------------------
# Benchmark runner
# ---------------------------------------------------------------------------

def wait_for_server(timeout=30):
    """Block until the HTTP server answers."""
    c = pycurl.Curl()
    c.setopt(c.URL, SERVER_URL + "/ready")
    c.setopt(c.CONNECTTIMEOUT, 2)
    c.setopt(c.TIMEOUT, 2)
    c.setopt(c.NOBODY, 1)
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            c.perform()
            c.close()
            return
        except pycurl.error:
            time.sleep(0.2)
    c.close()
    raise RuntimeError(f"server at {SERVER_URL} not reachable")


def run_benchmark(num_urls):
    urls = [f"{SERVER_URL}/page/{i}" for i in range(num_urls)]
    seq_dir = tempfile.mkdtemp()
    par_dir = tempfile.mkdtemp()

    try:
        # warm up
        download_sequential(urls[:1], seq_dir)

        t0 = time.perf_counter()
        download_sequential(urls, seq_dir)
        seq_time = time.perf_counter() - t0

        t0 = time.perf_counter()
        download_parallel(urls, par_dir)
        par_time = time.perf_counter() - t0

        return seq_time, par_time
    finally:
        shutil.rmtree(seq_dir, ignore_errors=True)
        shutil.rmtree(par_dir, ignore_errors=True)


def main():
    wait_for_server()

    scenarios = [10, 50, 100, 200]
    latency_label = os.getenv("SERVER_LATENCY", "0.05")

    print("=" * 75)
    print("BENCHMARK: sequential (C++ approach) vs parallel (pycurl-downloader)")
    print(f"server={SERVER_URL}  latency={latency_label}s  max_concurrent={MAX_CONCURRENT}")
    print("=" * 75)
    print(f"{'URLs':>6}  {'Sequential':>12}  {'Parallel':>12}  {'Speedup':>8}")
    print("-" * 75)

    for num_urls in scenarios:
        seq_time, par_time = run_benchmark(num_urls)
        speedup = seq_time / par_time if par_time > 0 else float("inf")
        print(f"{num_urls:>6}  {seq_time:>10.3f} s  {par_time:>10.3f} s  {speedup:>7.1f}x")

    print("-" * 75)
    print("Speedup > 1 means parallel is faster.")


if __name__ == "__main__":
    main()
