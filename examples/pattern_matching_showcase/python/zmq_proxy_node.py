#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time

import snn_utils.comm.music
from config import *
from snn_utils.comm.music.node import PyMusicNode
from snn_utils.comm.serializer import SERIALIZERS
from snn_utils.comm.zmq import Publisher
from snn_utils.plotter.data_provider import ProxyDataSource

logger = logging.getLogger(__name__)


class ZMQProxyNode(PyMusicNode):
    """
        This proxy node is able to serve spiking and analogous data communicated in MUSIC to the outside world.
    """

    def __init__(self):
        PyMusicNode.__init__(self, music_node_timestep, total_time=total_simulation_time, pre_run_barrier=True)
        self._set_buffer(snn_utils.comm.music.SingleStepBuffer())

    def _setup_cont_ports(self, port_configs, defaults):
        for port_name, config in port_configs.items():
            params = defaults.copy()
            if 'port' in config:
                params.update(config['port'])
            keys = config['keys']
            assert 'fallback_width' not in params or params['fallback_width'] == len(keys)
            params['fallback_width'] = len(keys)
            buffers = self.publish_buffering_cont_input(port_name=port_name, **params)
            self._data_source.map_cont_buffers(keys, buffers)

    def _setup_event_ports(self, port_configs, defaults):
        for port_name, config in port_configs.items():
            params = defaults.copy()
            if 'port' in config:
                params.update(config['port'])
            n_keys = config['keys']
            assert isinstance(n_keys, int)
            assert 'fallback_width' not in params or params['fallback_width'] == n_keys
            params['fallback_width'] = n_keys
            buffers = self.publish_buffering_event_input(port_name=port_name, **params)
            self._data_source.map_event_buffers(port_name, buffers)

    def _setup(self, music_setup):
        logger.info("Opening buffered ports...")

        self._data_source = ProxyDataSource()

        self._setup_cont_ports(music_zmq_proxy_config['cont'], music_zmq_proxy_config['cont_port_defaults'])
        self._setup_event_ports(music_zmq_proxy_config['event'], music_zmq_proxy_config['event_port_defaults'])

        self._serializer = SERIALIZERS[music_zmq_proxy_config['communication']['format']]

        self._server = Publisher(music_zmq_proxy_config['communication']['port'])

    def _run_single_cycle(self, curr_music_time):
        msg = self._data_source.dump_delta(curr_music_time)
        self._server.send(self._serializer.serialize(msg))


if __name__ == '__main__':
    ZMQProxyNode().main()
