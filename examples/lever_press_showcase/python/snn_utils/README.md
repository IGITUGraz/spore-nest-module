# *snn_utils*: Utilities for Spiking Neural Networks

```
snn_utils
├── buffer.py
├── comm
│   ├── __init__.py
│   ├── music
│   │   ├── __init__.py
│   │   └── node.py
│   ├── nest.py
│   ├── serializer.py
│   └── zmq
│       └── __init__.py
├── __init__.py
├── plotter
│   ├── backends
│   │   ├── __init__.py
│   │   ├── mpl.py
│   │   └── tk.py
│   ├── data_provider.py
│   ├── __init__.py
│   ├── interface
│   │   └── __init__.py
│   └── plots
│       └── __init__.py
└── README.md
```

## MUSIC

This package contains utilities for quick music node development.
Wrappers like `publish_cont_output` allow for port creation & mapping in one line,
while also returning a stub buffer in case of the port being unconnected.

### Examples

TODO: Test examples

#### Simple continuous signal generator.

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
from snn_utils.comm.music.node import PyMusicNode


class SignalNode(PyMusicNode):
    def __init__(self):
        PyMusicNode.__init__(self, time_step=0.02, total_time=100.0)

    def _setup(self, music_setup):
        self._signal_out = self.publish_cont_output('signal_out', fallback_width=1)

    def _run_single_cycle(self, curr_time):
        self._signal_out[0] = np.mod(curr_time, 1.0) # 1s-periodic ramp

    def _post_cycle(self, curr_time, measured_cycle_time):
        print("[{}] sending {}".format(curr_time, self._signal_out[0]))

if __name__ == '__main__':
    SignalNode().main()
```

#### Simple continuous relay node

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

from snn_utils.comm.music.node import PyMusicNode


class RelayNode(PyMusicNode):
    def __init__(self):
        PyMusicNode.__init__(self, time_step=0.02, total_time=100.0)

    def _setup(self, music_setup):
        self._signal_in = self.publish_cont_input('signal_in', falback_width=1)
        self._signal_out = self.publish_cont_output('signal_out', fallback_width=1)

    def _run_single_cycle(self, curr_time):
        self._signal_out[0] = self._signal_in[0]

if __name__ == '__main__':
    RelayNode().main()
```

#### Buffering receiver

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

from snn_utils.comm.music.node import PyMusicNode
from snn_utils.comm.music import WindowedBuffer


class BufferingNode(PyMusicNode):
    def __init__(self):
        PyMusicNode.__init__(self, time_step=0.02, total_time=100.0)
        self._set_buffer(WindowedBuffer(2.0))  # save time windows of 2.0s

    def _setup(self, music_setup):
        self._reward_buffer = self.publish_buffering_cont_input('reward_in', fallback_width=1)
        self._activity_in_buffer = self.publish_buffering_event_input('activity_in', fallback_width=10, base=0)

    def _run_single_cycle(self, curr_time):
        # len(self._reward_buffer[0]) ~ 2.0 / 0.02 (history length / timestep)
        print("last rewards: {}".format(self._reward_buffer[0]))
        # each neuron (== channel) has its own buffer
        for i, spike_buffer in enumerate(self._activity_in_buffer):
            print("spikes for neuron {}: {}".format(i, spike_buffer))

if __name__ == '__main__':
    BufferingNode().main()
```
