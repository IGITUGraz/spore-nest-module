from __future__ import absolute_import

import logging
import zmq

logger = logging.getLogger(__name__)


class ContextHelper(object):
    def __init__(self, context=None):
        self._context = zmq.Context() if context is None else context
        self.__scoped = context is None

    def close(self):
        if self.__scoped:
            logger.info("Terminating ZMQ context.")
            self._context.term()


class Publisher(ContextHelper):
    """
        Minimal wrapper around a ZMQ publisher.
    """
    def __init__(self, port, context=None, host="*", transport='tcp'):
        ContextHelper.__init__(self, context)
        address = "{}://{}:{}".format(transport, host, port)
        logger.info("Publishing on {}".format(address))
        self._sock = self._context.socket(zmq.PUB)
        self._sock.bind(address)

    def send(self, data):
        self._sock.send(data)

    def send_multipart(self, *data):
        self._sock.send_multipart(*data)

    def close(self):
        logger.info("Closing socket")
        self._sock.close()
        ContextHelper.close(self)


class MultiSubscriber(ContextHelper):
    def __init__(self, context=None, poll_timeout=0.000000001):
        ContextHelper.__init__(self, context)
        self._poller = zmq.Poller()
        self._handler = {}
        self._poll_timeout = poll_timeout

    def add_subscriber(self, host, port, callback, transport="tcp", prefix="", deserialize=None, multipart=False):
        address = "{}://{}:{}".format(transport, host, port)
        logger.info("Subscribing to {}".format(address))
        sock = self._context.socket(zmq.SUB)
        sock.connect(address)
        sock.setsockopt(zmq.SUBSCRIBE, prefix)

        if multipart:
            receive = lambda: sock.recv_multipart(zmq.NOBLOCK)
        else:
            receive = lambda: sock.recv(zmq.NOBLOCK)

        if deserialize is None:
            handle = lambda: callback(receive())
        else:
            if multipart:
                handle = lambda: callback([elem if i == 0 else deserialize(elem) for i, elem in enumerate(receive())])
            else:
                handle = lambda: callback(deserialize(receive()))

        self._handler[sock] = handle
        self._poller.register(sock, zmq.POLLIN)

    def tick(self):
        # exhaustive poll
        polled = []
        exhausted = False
        while not exhausted:
            for sock, kind in polled:
                assert kind is zmq.POLLIN
                self._handler[sock]()
            polled = self._poller.poll(self._poll_timeout)
            exhausted = not polled

    def close(self):
        logger.info("Closing {} sockets.".format(len(self._handler.keys())))
        for sock in self._handler.keys():
            sock.close()
        ContextHelper.close(self)
