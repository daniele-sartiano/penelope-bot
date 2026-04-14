"""
Tests that assert correct behavior in the C++ Downloader source code.

Each test reads the actual C++ source and checks that known bugs
have been fixed. Tests FAIL when the bug is present, and PASS
once the code is corrected.

Bugs covered:
1. directory.append("/") mutates the parameter by reference
2. curl_global_cleanup() called inside the download loop
3. File opened before discard() check (file descriptor leak)
4. curl_easy_perform() return value ignored
5. discard() calls curl_easy_getinfo before curl_easy_perform
6. No timeout configured on curl
7. Raw new/delete instead of smart pointers or stack allocation
8. Global counter without synchronization (data race)
"""

import os
import re
import unittest

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROJECT_DIR = os.path.dirname(BASE_DIR)

DOWNLOADER_CPP = os.path.join(BASE_DIR, "src", "Downloader.cpp")
DOWNLOADER_H = os.path.join(BASE_DIR, "src", "Downloader.h")
DOWNLOADER_MAIN = os.path.join(BASE_DIR, "main.cpp")
PARSER_MAIN = os.path.join(PROJECT_DIR, "parser", "main.cpp")


def read_source(path):
    with open(path) as f:
        return f.read()


class TestDirectoryNotMutated(unittest.TestCase):
    """
    BUG: Downloader.cpp - download() method
        std::string prefix = !directory.empty() ? directory.append("/") : "";

    directory is passed by reference (&directory) and .append("/")
    modifies it in-place. On repeated calls the path gets corrupted:
    "/data" -> "/data/" -> "/data//" -> "/data///"

    FIX: use concatenation (directory + "/") or take directory by value.
    """

    def test_directory_not_mutated_by_append(self):
        """download() must not call .append() on the directory parameter."""
        source = read_source(DOWNLOADER_CPP)
        self.assertNotIn(
            "directory.append",
            source,
            "directory.append() mutates the parameter by reference. "
            "Use (directory + \"/\") or take directory by value instead."
        )


class TestNoCurlGlobalCleanupInLoop(unittest.TestCase):
    """
    BUG: Downloader.cpp - download() method
        for (auto m : this->models) {
            ...
            curl_global_cleanup();  // called for EVERY URL
        }

    curl_global_cleanup() de-initializes the entire curl library.
    Calling it inside the loop means subsequent curl_easy_init()
    calls operate on a de-initialized library (undefined behavior).

    FIX: remove curl_global_cleanup() from download(). Call it once
    at program exit (or not at all - it's optional).
    """

    def test_no_curl_global_cleanup_in_download(self):
        """curl_global_cleanup must not appear in Downloader.cpp."""
        source = read_source(DOWNLOADER_CPP)
        self.assertNotIn(
            "curl_global_cleanup",
            source,
            "curl_global_cleanup() must not be called inside the download loop. "
            "It de-initializes the entire curl library."
        )


class TestDiscardRemovedOrCorrectOrder(unittest.TestCase):
    """
    BUG: Downloader.cpp - download() method
        pagefile = fopen(file_name.c_str(), "wb");  // file opened
        ...
        if (this->discard(m)) {
            continue;  // file never closed -> fd leak
        }

    The file was opened BEFORE the discard() check, leaking the fd
    whenever a page was discarded.

    FIX: either remove discard() entirely (the original implementation
    was broken anyway - see TestDiscardGetinfoAfterPerform) or call it
    BEFORE fopen().
    """

    def test_no_fd_leak_path_in_download(self):
        """
        If discard() is present, it must run before fopen().
        If discard() has been removed, there is nothing to leak.
        """
        source = read_source(DOWNLOADER_CPP)

        discard_pos = source.find("this->discard(")
        if discard_pos == -1:
            # discard() was removed - no leak possible
            return

        fopen_pos = source.find("fopen(")
        self.assertNotEqual(fopen_pos, -1, "fopen() call not found in source")
        self.assertLess(
            discard_pos, fopen_pos,
            "discard() must be called BEFORE fopen() to avoid "
            "file descriptor leak on discarded pages."
        )


class TestCurlPerformErrorChecked(unittest.TestCase):
    """
    BUG: Downloader.cpp - download() method
        curl_easy_perform(curl); /* ignores error */

    The return value of curl_easy_perform is not checked.
    Failed downloads (HTTP errors, DNS failures, timeouts)
    produce corrupted/empty files that are forwarded to the parser.

    FIX: check the CURLcode return value and skip failed downloads.
    """

    def test_curl_perform_result_is_checked(self):
        """curl_easy_perform return value must be stored and checked."""
        source = read_source(DOWNLOADER_CPP)

        # Find all curl_easy_perform calls in download() (not in discard())
        # The bug is the bare call: "curl_easy_perform(curl);"
        # The fix assigns the result: "res = curl_easy_perform(curl);"
        lines = source.split("\n")
        unchecked_performs = []
        for i, line in enumerate(lines, 1):
            stripped = line.strip()
            if "curl_easy_perform" in stripped and stripped.startswith("curl_easy_perform"):
                unchecked_performs.append((i, stripped))

        self.assertEqual(
            len(unchecked_performs), 0,
            f"curl_easy_perform() return value must be checked. "
            f"Unchecked calls at lines: {unchecked_performs}"
        )


class TestDiscardGetinfoAfterPerform(unittest.TestCase):
    """
    BUG: Downloader.cpp - discard() method
        curl = curl_easy_init();
        curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);  // BEFORE perform!
        ...
        res = curl_easy_perform(curl);

    curl_easy_getinfo(CURLINFO_FILETIME) is called before the request
    is executed, so filetime stays at -1. The condition `filetime > 0`
    is always false, making discard() never discard anything.

    FIX: call curl_easy_getinfo AFTER curl_easy_perform.
    """

    def test_getinfo_called_after_perform_in_discard(self):
        """If discard() exists, curl_easy_getinfo must come after curl_easy_perform."""
        source = read_source(DOWNLOADER_CPP)

        # discard() may have been removed entirely; in that case the bug
        # cannot exist.
        discard_start = source.find("Downloader::discard")
        if discard_start == -1:
            return

        discard_body = source[discard_start:]
        # Find the closing brace of the method (approximate)
        brace_count = 0
        discard_end = 0
        for i, ch in enumerate(discard_body):
            if ch == '{':
                brace_count += 1
            elif ch == '}':
                brace_count -= 1
                if brace_count == 0:
                    discard_end = i
                    break
        discard_body = discard_body[:discard_end]

        perform_pos = discard_body.find("curl_easy_perform")
        getinfo_filetime_pos = discard_body.find("CURLINFO_FILETIME")

        self.assertNotEqual(perform_pos, -1, "curl_easy_perform not found in discard()")
        self.assertNotEqual(getinfo_filetime_pos, -1, "CURLINFO_FILETIME not found in discard()")

        # The FIRST getinfo(CURLINFO_FILETIME) must come AFTER perform
        first_getinfo = discard_body.find("CURLINFO_FILETIME")
        self.assertGreater(
            first_getinfo, perform_pos,
            "curl_easy_getinfo(CURLINFO_FILETIME) must be called AFTER "
            "curl_easy_perform() in discard(), otherwise filetime is always -1."
        )


class TestCurlTimeoutsConfigured(unittest.TestCase):
    """
    BUG: No CURLOPT_TIMEOUT or CURLOPT_CONNECTTIMEOUT set anywhere.

    Without timeouts, if a server doesn't respond the thread blocks
    indefinitely. With 100 threads in main, 100 unresponsive URLs
    stall the entire crawler.

    FIX: set CURLOPT_CONNECTTIMEOUT and CURLOPT_TIMEOUT on every
    curl easy handle.
    """

    def test_connect_timeout_is_set(self):
        """CURLOPT_CONNECTTIMEOUT must be configured."""
        source = read_source(DOWNLOADER_CPP)
        self.assertIn(
            "CURLOPT_CONNECTTIMEOUT",
            source,
            "CURLOPT_CONNECTTIMEOUT must be set to prevent infinite hangs "
            "on unresponsive servers."
        )

    def test_transfer_timeout_is_set(self):
        """CURLOPT_TIMEOUT must be configured."""
        source = read_source(DOWNLOADER_CPP)
        self.assertIn(
            "CURLOPT_TIMEOUT",
            source,
            "CURLOPT_TIMEOUT must be set to prevent infinite hangs "
            "on slow transfers."
        )


class TestNoRawNewInCallbacks(unittest.TestCase):
    """
    BUG: downloader/main.cpp
        auto *producer = new NatsProducer(server);
        producer->send(parser_subject, r);
        delete producer;

    A new NatsProducer (with a new NATS connection) is created
    for every received message. This is wasteful and leak-prone.

    BUG: parser/main.cpp
        auto *p = new Parser(natsMsg_GetData(msg));
        auto v = p->parse();
        // no delete p -> leak on every message

    FIX: create the producer once outside the callback, pass it
    via the closure pointer (like data-manager already does).
    Use stack allocation for Parser.
    """

    def test_no_raw_new_natsproducer_in_downloader_callback(self):
        """NatsProducer must not be created inside the onMsg callback."""
        source = read_source(DOWNLOADER_MAIN)
        # Find the onMsg function body
        on_msg_start = source.find("static void onMsg")
        self.assertNotEqual(on_msg_start, -1, "onMsg not found")
        on_msg_body = source[on_msg_start:]
        # Find end of onMsg (next function definition)
        next_func = on_msg_body.find("\nvoid*")
        if next_func == -1:
            next_func = on_msg_body.find("\nint main")
        on_msg_body = on_msg_body[:next_func] if next_func != -1 else on_msg_body

        self.assertNotIn(
            "new NatsProducer",
            on_msg_body,
            "NatsProducer must not be heap-allocated inside onMsg. "
            "Create it once and pass via closure."
        )
        self.assertNotIn(
            "NatsProducer producer(",
            on_msg_body,
            "NatsProducer must not be stack-allocated inside onMsg "
            "(creates a new connection per message). Pass via closure."
        )

    def test_no_raw_new_parser_in_parser_main(self):
        """Parser should not be allocated with raw new."""
        source = read_source(PARSER_MAIN)
        self.assertNotIn(
            "new Parser",
            source,
            "Use stack allocation (Parser p(...)) instead of raw new."
        )


class TestCounterThreadSafety(unittest.TestCase):
    """
    BUG: parser/main.cpp:6
        int counter = 0;  // global, non-atomic, no mutex

    In onMsg callback (called from multiple NATS threads):
        counter++;  // data race -> undefined behavior

    FIX: use std::atomic<int> or protect with std::mutex.
    """

    def test_counter_is_atomic(self):
        """The global counter must use std::atomic or a mutex."""
        source = read_source(PARSER_MAIN)

        has_atomic = "std::atomic" in source or "atomic<" in source
        has_mutex = "std::mutex" in source or "std::lock_guard" in source

        self.assertTrue(
            has_atomic or has_mutex,
            "The global counter is modified from multiple threads without "
            "synchronization (data race / undefined behavior). "
            "Use std::atomic<int> or protect with std::mutex."
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
