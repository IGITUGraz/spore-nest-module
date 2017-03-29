import collections


class TimeBuffer(object):
    def __init__(self):
        self._times = self._create_container()

    def _create_container(self):
        return []

    def clear(self):
        self._times[:] = []

    def update(self, curr_sim_time):
        pass

    def get_times(self):
        return self._times

    def __len__(self):
        return len(self._times)

    def __repr__(self):
        return str(self._times)

    def __getitem__(self, sliced):
        return self._times[sliced]

    def append(self, time):
        self._times.append(time)


class WindowedTimeBuffer(TimeBuffer):
    def __init__(self, time_window):
        TimeBuffer.__init__(self)
        self._time_window = time_window

    def _create_container(self):
        return collections.deque()

    def clear(self):
        self._times.clear()

    def update(self, current_time):
        threshold = current_time - self._time_window
        if self._times:
            time = self._times.popleft()
            while self._times and time < threshold:
                time = self._times.popleft()
            if time >= threshold:
                self._times.appendleft(time)


class SpikeBuffer(TimeBuffer):
    def append_spike(self, time):
        self.append(time)


class WindowedSpikeBuffer(WindowedTimeBuffer, SpikeBuffer):
    def __init__(self, time_window):
        SpikeBuffer.__init__(self)
        WindowedTimeBuffer.__init__(self, time_window)

    def rate(self):
        return float(len(self)) / self._time_window


class ValueBuffer(TimeBuffer):
    def __init__(self):
        TimeBuffer.__init__(self)
        self._values = []

    def append_value(self, time, value):
        self.append(time)
        self._values.append(value)

    def clear(self):
        TimeBuffer.clear(self)
        self._values[:] = []

    def update(self, current_time):
        TimeBuffer.update(self, current_time)
        self._values[:] = self._values[-len(self._times):]

    def get_values(self):
        return self._values

    def get_timed_values(self):
        return zip(self._times, self._values)

    def __repr__(self):
        return str(self.get_timed_values())

    def __getitem__(self, sliced):
        return self.get_timed_values()[sliced]


class WindowedValueBuffer(WindowedTimeBuffer, ValueBuffer):
    def __init__(self, time_window):
        ValueBuffer.__init__(self)
        WindowedTimeBuffer.__init__(self, time_window)

    def update(self, current_time):
        WindowedTimeBuffer.update(self, current_time)
        ValueBuffer.update(self, current_time)
