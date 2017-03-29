import itertools
import logging
import time

import music
from mpi4py import MPI

from snn_utils.comm.music import PortUtility

logger = logging.getLogger(__name__)


class Node(object):
    def __init__(self, total_time, pre_run_barrier):
        self._total_time = total_time
        self._pre_run_barrier = pre_run_barrier

    def _execute_pre_run_barrier(self):
        if self._pre_run_barrier:
            logger.info("Synchronize world communicator before entering runtime...")
            MPI.COMM_WORLD.Barrier()


class PyMusicNode(Node, PortUtility):
    def __init__(self, time_step, total_time=None, pre_run_barrier=False, measure_cycle_time=True):
        Node.__init__(self, total_time, pre_run_barrier)
        PortUtility.__init__(self)
        self._time_step = time_step
        self._measure_cycle_time = measure_cycle_time

    @staticmethod
    def __runtime(music_setup, timestep, total_time):
        if total_time is None:
            raise NotImplementedError("Unlimited simulation not implemented yet.")
        assert total_time > 0
        max_time = total_time + timestep
        times = music_setup.runtime(timestep)
        # ignore ticks before first timestep is reached
        times = itertools.dropwhile(lambda t: t < timestep, times)
        # stop iteration when maxtime is reached
        return itertools.takewhile(lambda t: t < max_time, times)

    def _setup(self, music_setup):
        pass

    def _run(self, times):
        for curr_time in times:
            if self._get_buffer() is not None:
                self._get_buffer().pre_cycle(curr_time)
            self._pre_cycle(curr_time)
            before_time = time.time() if self._measure_cycle_time else None
            self._run_single_cycle(curr_time)
            if before_time is not None:
                self._post_cycle(curr_time, time.time() - before_time)
            if self._get_buffer() is not None:
                self._get_buffer().post_cycle(curr_time)

    def _pre_run(self, music_setup):
        pass

    def _post_run(self):
        pass

    def _run_single_cycle(self, curr_time):
        pass

    def _pre_cycle(self, curr_time):
        pass

    def _post_cycle(self, curr_time, measured_cycle_time):
        pass

    def main(self):
        self._set_music_setup(music.Setup())
        self._setup(self._music_setup)
        self._pre_run(self._music_setup)
        self._execute_pre_run_barrier()
        logger.info("Dropping to runtime with timestep {}...".format(self._time_step))
        times = PyMusicNode.__runtime(self._music_setup, self._time_step, self._total_time)
        self._set_music_setup(None)
        self._run(times)
        self._post_run()

