from __future__ import absolute_import

import logging
import numpy as np
import nest

logger = logging.getLogger(__name__)


class WeightCommunicator(object):
    """
        Utility component for gathering and broadcasting weights via ZMQ.
        Weights of registered connections are pulled and broadcasted each time `communicate` is called.
    """
    def __init__(self, send, serialize=lambda data: repr(data), prefix="weight"):
        self._serialize = serialize
        self._prefix = prefix
        self._conn_spec = {}
        self._send = send

    @staticmethod
    def _create_base_message(pre, post, conns):
        conns_col = np.rollaxis(np.array(conns), 1)
        return {
            # TODO @discuss src vs. pre; trg vs. post
            'src_gid': conns_col[0], 'trg_gid': conns_col[1],
            'src_lid': conns_col[0] - min(pre),
            'trg_lid': conns_col[1] - min(post),
        }

    def add_conn_spec(self, key, pre, post, conns):
        assert key not in self._conn_spec, "association between prefix and connection specification should be unique"
        self._conn_spec[key] = (conns, WeightCommunicator._create_base_message(pre, post, conns))

    def communicate(self, sim_time):
        for key, (conns, msg) in self._conn_spec.items():
            msg['sim_time'] = sim_time
            msg['weight'] = nest.GetStatus(conns, 'weight')
            self._send(["{prefix}/{key}".format(prefix=self._prefix, key=key), self._serialize(msg)])
