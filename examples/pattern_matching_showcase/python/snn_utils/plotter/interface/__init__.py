import logging
import sys
import time
from snn_utils.comm.zmq import MultiSubscriber


logger = logging.getLogger(__name__)


class SimpleTaskScheduler(object):
    def __init__(self):
        self._handles = []

    def add_handle(self, callback, interval):
        self._handles.append([callback, interval, 0])

    def tick(self, sim_time):
        for handle in self._handles:
            callback, interval, last_tick = handle
            if (sim_time - last_tick) > interval:
                callback()
                handle[2] = sim_time


class Master(object):
    """
    This class combines a communication sink with a simple periodic task scheduler.
    """
    def __init__(self, poll_timeout=None):
        if poll_timeout is None:
            self._comm = MultiSubscriber()
        else:
            self._comm = MultiSubscriber(poll_timeout=poll_timeout)
        self._scheduler = SimpleTaskScheduler()

    def communicator(self):
        return self._comm

    def scheduler(self):
        return self._scheduler

    def mainloop(self, time_source=lambda: time.time() * 1000.0):
        try:
            logger.info("Entering mainloop. Abort with SIGINT [CTRL+C].")
            while True:
                # networking
                self._comm.tick()
                # ui and other secondary stuff
                self._scheduler.tick(time_source())
        except KeyboardInterrupt:
            logger.info(" -- SIGINT received. Shutting down.")
            self._comm.close()
            sys.exit(0)
