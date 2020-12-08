import asyncio
import socket
import pycares
import json
from functools import partial
from nats.aio.client import Client as NATS


class DNSResolver:
    EVENT_READ = 0
    EVENT_WRITE = 1

    def __init__(self, loop=None):
        self._channel = pycares.Channel(sock_state_cb=self._sock_state_cb)
        self._timer = None
        self._fds = set()
        self.loop = loop or asyncio.get_event_loop()

    def _sock_state_cb(self, fd, readable, writable):
        if readable or writable:
            if readable:
                self.loop.add_reader(fd, self._process_events, fd, self.EVENT_READ)
            if writable:
                self.loop.add_writer(fd, self._process_events, fd, self.EVENT_WRITE)
            self._fds.add(fd)
            if self._timer is None:
                self._timer = self.loop.call_later(1.0, self._timer_cb)
        else:
            # socket is now closed
            self._fds.discard(fd)
            if not self._fds:
                self._timer.cancel()
                self._timer = None

    def _timer_cb(self):
        self._channel.process_fd(pycares.ARES_SOCKET_BAD, pycares.ARES_SOCKET_BAD)
        self._timer = self.loop.call_later(1.0, self._timer_cb)

    def _process_events(self, fd, event):
        if event == self.EVENT_READ:
            read_fd = fd
            write_fd = pycares.ARES_SOCKET_BAD
        elif event == self.EVENT_WRITE:
            read_fd = pycares.ARES_SOCKET_BAD
            write_fd = fd
        else:
            read_fd = write_fd = pycares.ARES_SOCKET_BAD
        self._channel.process_fd(read_fd, write_fd)

    def query(self, query_type, name, cb):
        self._channel.query(query_type, name, cb)

    def gethostbyname(self, name, cb):
        self._channel.gethostbyname(name, socket.AF_INET, cb)


async def run(loop):
    resolver = DNSResolver(loop)

    async def disconnected_cb():
        print("Got disconnected...")

    async def reconnected_cb():
        print("Got reconnected...")

    nc = NATS()
    await nc.connect("nats://ruser:T0pS3cr3t@127.0.0.1:4222",
                     reconnected_cb=reconnected_cb,
                     disconnected_cb=disconnected_cb,
                     max_reconnect_attempts=-1,
                     loop=loop)

    # def cb(model, result, error):
    def mcb(a, b, c):
        print(a)
        print(b)
        print(c)
        # print(model, result.name, result.addresses, result.aliases)

    async def message_handler(msg):
        try:
            subject = msg.subject
            reply = msg.reply
            data = json.loads(msg.data.decode())

            print("Received a message on '{subject} {reply}': {data}".format(
                subject=subject, reply=reply, data=data))
            for model in data['models']:
                f = partial(mcb, model)
                resolver.gethostbyname(model['link'], f)

        except Exception as e:
            print(e)

    await nc.subscribe("resolver", cb=message_handler)


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(loop))
    loop.run_forever()
    loop.close()


def main():
    def cb_gethostbyname(result, error):
        print(result.name, result.addresses, result.aliases)

    def cb(result, error):
        print("Result: {}, Error: {}".format(result, error))

    loop = asyncio.get_event_loop()
    resolver = DNSResolver(loop)
    resolver.query('google.com', pycares.QUERY_TYPE_A, cb)
    resolver.query('sip2sip.info', pycares.QUERY_TYPE_SOA, cb)
    resolver.gethostbyname('apple.com', cb)
    loop.run_forever()

# if __name__ == '__main__':
#     main()
