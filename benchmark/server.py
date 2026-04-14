"""Standalone HTTP server for the benchmark.

Runs as a separate process so the benchmark client's GIL does not
interfere with server throughput.
"""

import http.server
import os
import time


class BenchmarkHandler(http.server.BaseHTTPRequestHandler):
    LATENCY = float(os.getenv("SERVER_LATENCY", "0.05"))

    def do_GET(self):
        time.sleep(self.LATENCY)
        body = (
            "<html><body>"
            f"<h1>Page {self.path}</h1>"
            "<p>" + ("lorem ipsum " * 200) + "</p>"
            "<a href='/other'>link</a>"
            "</body></html>"
        ).encode()
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, fmt, *args):
        pass


class Server(http.server.ThreadingHTTPServer):
    # large accept backlog so burst of parallel connections is not reset
    request_queue_size = 512
    daemon_threads = True


def main():
    port = int(os.getenv("SERVER_PORT", "8888"))
    server = Server(("0.0.0.0", port), BenchmarkHandler)
    print(f"server listening on :{port} (latency={BenchmarkHandler.LATENCY}s)", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
