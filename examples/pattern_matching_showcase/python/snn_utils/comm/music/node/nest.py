from __future__ import absolute_import

import logging

import nest
from snn_utils.comm.music.node import Node

logger = logging.getLogger(__name__)


class PyNestNode(Node):
    def __init__(self, total_time=None, pre_run_barrier=False):
        Node.__init__(self, total_time, pre_run_barrier)

    def _create_event_in_proxy(self, size, port_name, maxBuffered=None, accLatency=None):
        proxy_population = nest.Create('music_event_in_proxy', size)
        nest.SetStatus(proxy_population, {'port_name': port_name})
        if maxBuffered is not None:
            if 'SetMaxBuffered' in dir(nest):
                nest.SetMaxBuffered(port_name, maxBuffered)
            else:
                logger.warning(
                    "Omitting max buffer size for proxy {},"
                    "as this NEST version does not support SetMaxBuffered".format(port_name))
        if accLatency is not None:
            nest.SetAcceptableLatency(port_name, float(accLatency))
        for i, n in enumerate(proxy_population):
            nest.SetStatus([n], 'music_channel', i)
        return proxy_population

    def _create_event_out_proxy(self, source_population, port_name):
        proxy_population = nest.Create('music_event_out_proxy')
        nest.SetStatus(proxy_population, {'port_name': port_name})
        for i, n in enumerate(source_population):
            nest.Connect([n], proxy_population, 'one_to_one', {'music_channel': i})
        return proxy_population

    def _setup(self):
        pass

    def _run(self):
        pass

    def main(self):
        self._setup()
        self._execute_pre_run_barrier()
        logger.info("Starting NEST simulation...")
        self._run()
