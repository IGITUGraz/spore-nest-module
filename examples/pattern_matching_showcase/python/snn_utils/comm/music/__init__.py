import logging
import numpy as np

import music

from snn_utils import buffer

logger = logging.getLogger(__name__)


class PortUtility(object):
    """
    Helper class for publishing ports and collecting data on input ports.
    """

    def __init__(self, music_setup=None, fail_on_unconnected=False, buffer=None):
        self._music_setup = music_setup
        self._fail_on_unconnected = fail_on_unconnected
        self._published_port_names = []
        self._buffer = buffer

    def _set_music_setup(self, music_setup):
        self._music_setup = music_setup

    def _set_buffer(self, buffer_factory):
        self._buffer = buffer_factory

    def _get_buffer(self):
        return self._buffer

    @staticmethod
    def _init_buffer(proxy, initial_value=None, fallback_width=0):
        width = proxy.width() if proxy.isConnected() else fallback_width
        assert width is not None
        if initial_value is not None:
            assert len(initial_value) == width
            return np.array(initial_value, dtype=np.double), width
        else:
            return np.zeros(width, dtype=np.double), width

    def _port_name_check(self, port_name):
        assert port_name not in self._published_port_names, \
            "A port name may only be published once: '{}'".format(port_name)
        self._published_port_names.append(port_name)

    def _handle_unconnected_port(self, msg):
        if self._fail_on_unconnected:
            raise music.MUSICError(msg)
        logger.warning(msg)

    def _check_parameters(self, port_name, required_params, kwargs):
        for param_name in required_params:
            if param_name not in kwargs:
                raise music.MUSICError("Missing parameter {} for port {}".format(param_name, port_name))

    def publish_cont_output(self, port_name, initial_value=None, fallback_width=0, **kwargs):
        assert self._music_setup is not None
        self._port_name_check(port_name)
        proxy = self._music_setup.publishContOutput(port_name)
        buf, width = PortUtility._init_buffer(proxy, initial_value, fallback_width)
        if proxy.isConnected():
            proxy.map(buf, **kwargs)
        else:
            self._handle_unconnected_port("Output port {} is not connected".format(port_name))
        return buf

    def publish_cont_input(self, port_name, initial_value=None, fallback_width=0, **kwargs):
        assert self._music_setup is not None
        self._port_name_check(port_name)
        proxy = self._music_setup.publishContInput(port_name)
        buf, width = PortUtility._init_buffer(proxy, initial_value, fallback_width)
        if proxy.isConnected():
            proxy.map(buf, **kwargs)
        else:
            self._handle_unconnected_port("Input port {} is not connected".format(port_name))
        return buf

    def publish_buffering_cont_input(self, *args, **kwargs):
        buf = self.publish_cont_input(*args, **kwargs)
        return self._buffer.buffer_cont_input(buf)

    def publish_event_input(self, port_name, spike_callback, **kwargs):
        assert self._music_setup is not None
        self._port_name_check(port_name)
        proxy = self._music_setup.publishEventInput(port_name)
        if proxy.isConnected():
            proxy.map(spike_callback, music.Index.GLOBAL, size=proxy.width(), **kwargs)
        else:
            self._handle_unconnected_port("Input port {} is not connected".format(port_name))

    def publish_buffering_event_input(self, port_name, fallback_width=1,
                                      width_to_n_buffers=lambda size: size, idx_to_buffer=lambda idx: idx,
                                      **kwargs):
        self._port_name_check(port_name)
        proxy = self._music_setup.publishEventInput(port_name)
        width = proxy.width() if proxy.isConnected() else fallback_width
        assert self._buffer is not None
        spike_buffers = self._buffer.buffer_event_input(width_to_n_buffers(width))
        if proxy.isConnected():
            self._check_parameters(port_name, ['base'], kwargs)
            assert kwargs['base'] == 0, "base != 0 not implemented yet"  # TODO
            proxy.map(lambda time, _, index: spike_buffers[idx_to_buffer(index)].append_spike(time),
                      music.Index.GLOBAL, size=width, **kwargs)
        else:
            self._handle_unconnected_port("Input port {} is not connected".format(port_name))
        return spike_buffers

    def publish_event_output(self, port_name, **kwargs):
        self._port_name_check(port_name)
        proxy = self._music_setup.publishEventOutput(port_name)
        if proxy.isConnected():
            self._check_parameters(port_name, ['base'], kwargs)
            proxy.map(music.Index.GLOBAL, size=proxy.width(), **kwargs)
        else:
            self._handle_unconnected_port("Output port {} is not connected".format(port_name))
            proxy = DummyEventOutput()
        return proxy


class DummyEventOutput(object):
    def insertEvent(self, time, index, mapping):
        pass


class BaseBuffer(object):
    def __init__(self):
        self._cont_array_buffers = []
        self._all_buffers = []

    def _create_value_buffer(self):
        return buffer.ValueBuffer()

    def _create_event_buffer(self):
        return buffer.SpikeBuffer()

    def buffer_cont_input(self, array_buffer):
        buffers = [self._create_value_buffer() for _ in range(len(array_buffer))]
        self._cont_array_buffers.append((buffers, array_buffer))
        self._all_buffers.extend(buffers)
        return buffers

    def buffer_event_input(self, n_buffers):
        buffers = [self._create_event_buffer() for _ in range(n_buffers)]
        self._all_buffers.extend(buffers)
        return buffers

    def pre_cycle(self, curr_sim_time):
        for buffers, array_buffer in self._cont_array_buffers:
            for i, buffer in enumerate(buffers):
                buffer.append_value(curr_sim_time, array_buffer[i])

    def post_cycle(self, curr_sim_time):
        pass


class WindowedBuffer(BaseBuffer):
    def __init__(self, time_window):
        BaseBuffer.__init__(self)
        self._time_window = time_window

    def _create_value_buffer(self):
        return buffer.WindowedValueBuffer(self._time_window)

    def _create_event_buffer(self):
        return buffer.WindowedSpikeBuffer(self._time_window)

    def post_cycle(self, curr_sim_time):
        for buffer in self._all_buffers:
            buffer.update(curr_sim_time)


class SingleStepBuffer(BaseBuffer):
    def post_cycle(self, curr_sim_time):
        for buffer in self._all_buffers:
            buffer.clear()
