# -*- coding: utf-8 -*-

import bisect
import collections
import itertools
import logging
import operator
import time

logger = logging.getLogger(__name__)


class DataSource(object):
    """
        This kind of object is given to plots to retrieve the necessary data for plotting.
        Plots usually are configured with a set of keys, they can use to obtain the desired data
        from this object.
    """

    def get_event_data(self, keys, time_window=None):
        raise NotImplementedError()

    def get_cont_data(self, keys, time_window=None):
        raise NotImplementedError()

    def get_weight_data(self, key, time_window=None):
        raise NotImplementedError()

    def get_min_time(self):
        raise NotImplementedError()

    def get_max_time(self):
        raise NotImplementedError()


class SimpleDataSource(DataSource):
    def __init__(self):
        self._map = collections.defaultdict(list)
        self._weight_map = {}

    def extend_cont_data(self, key, data):
        self._map[key].extend(data)

    def extend_event_data(self, key, data):
        self._map[key].extend(data)

    def extend_weight_data(self, key, df):
        if key not in self._weight_map:
            self._weight_map[key] = df.copy()
        else:
            self._weight_map[key] = df.copy()
            #self._weight_map[key] = self._weight_map[key].append(df)

    def get_cont_data(self, keys, time_window=None):
        return map(lambda key: self._map[key], keys)

    def get_event_data(self, keys, time_window=None):
        return map(lambda key: self._map[key], keys)

    def get_weight_data(self, key, time_window=None):
        df = self._weight_map[key]
        if time_window is None:
            return df
        else:
            return df[(df.sim_time >= time_window[0]) & (df.sim_time < time_window[1])]

    def reset(self):
        self._map.clear()


class ProxyDataSource(DataSource):
    def __init__(self, sender=True, auto_reset=True):
        self._sender = sender
        if sender:
            # sender
            self._map = {
                'cont': {},
                'event': collections.defaultdict(dict)
            }
        else:
            # receiver
            self._map = collections.defaultdict(list)
            self._min_time = None
            self._max_time = None
            self._auto_reset = auto_reset

    def map_cont_buffers(self, keys, buffers):
        assert self._sender
        for key, buffer in zip(keys, buffers):
            self._map['cont'][key] = buffer

    def map_event_buffers(self, key, buffers):
        assert self._sender
        self._map['event'][key] = list(enumerate(buffers))

    def dump_delta(self, curr_time):
        assert self._sender

        event_updates = list(
            # send only those buffers (with index) which actually contain data
            # e.g. [ ('activity_in', [ (3, [1, 3]), (6, [0, 2]), ... ]), ....]
            filter(lambda (key, updates): updates,
                   [(key, filter(lambda (i, buffer): buffer, buffers)) for key, buffers in self._map['event'].items()]
                   )
        )

        result = [list(self._map['cont'].items())]
        if event_updates:
            result.append(event_updates)
        return curr_time, result

    def read_delta(self, delta):
        curr_time, updates = delta

        if self._min_time is None:
            self._min_time = curr_time
        if self._max_time > curr_time:
            # automatic reset
            if self._auto_reset:
                logger.info(
                    "Simulation reset detected ({} -> {}). Resetting buffers.".format(self._max_time, curr_time))
                self._reset()
                self._min_time = curr_time
        self._max_time = curr_time
        assert self._min_time <= self._max_time, "{} should be <= {}".format(self._min_time, self._max_time)

        for key, buffer in updates[0]:
            self._map[key].extend(buffer)
        if len(updates) > 1:
            for key, buffer_map in updates[1]:
                for i, buffer in buffer_map:
                    self._map[(key, int(i))].extend(buffer)

    def get_cont_data(self, keys, time_window=None):
        assert not self._sender
        if time_window is None:
            return map(lambda key: self._map[key], keys)
        else:
            lower, upper = time_window
            assert lower >= self.get_min_time()
            assert upper <= self.get_max_time()
            return map(lambda key: list(
                itertools.takewhile(
                    lambda (t, v): t < upper,
                    itertools.dropwhile(
                        lambda (t, v): t < lower,
                        self._map[key]))),
                       keys)

    def get_event_data(self, keys, time_window=None):
        assert not self._sender
        if time_window is None:
            return map(lambda key: self._map[key], keys)
        else:
            lower, upper = time_window
            assert lower >= self.get_min_time()
            assert upper <= self.get_max_time()
            return map(lambda key: list(
                itertools.takewhile(
                    lambda t: t < upper,
                    itertools.dropwhile(
                        lambda t: t < lower,
                        self._map[key]))),
                       keys)

    def _reset(self):
        assert not self._sender
        self._map.clear()

    def get_max_time(self):
        return self._max_time

    def get_min_time(self):
        return self._min_time

    def truncate(self, lower=None, upper=None):
        assert not self._sender
        assert lower is not None or upper is not None
        assert lower is None or upper is None or lower <= upper
        cleanup = 0
        ts_before = time.time()
        for key, buffer in self._map.items():
            times = buffer
            if times and (isinstance(times[0], tuple) or isinstance(times[0], list)):
                # continuous
                times = map(operator.itemgetter(0), times)
            lower_idx = bisect.bisect_left(times, lower) if lower is not None else None
            upper_idx = bisect.bisect_right(times, upper) if upper is not None else None
            buffer[:] = buffer[lower_idx:upper_idx]
            cleanup += (lower_idx if lower_idx else 0) + (len(buffer) - upper_idx if upper_idx else 0)
        logger.info("Clearing buffers: discarding {} elements which are {} [sim-time]; took {:f}s [real-time]."
                    .format(cleanup, " and ".join([s.format(v) for s, v in
                                                   zip(["older than {:.2f}s", "younger than {:.2f}s"], [lower, upper])
                                                   if v]),
                            time.time() - ts_before))
