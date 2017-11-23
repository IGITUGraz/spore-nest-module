#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np

import matplotlib.cm

import snn_utils.plotter as plotter
import snn_utils.plotter.backends.mpl as mpl_plotter
import snn_utils.plotter.interface as interface
import snn_utils.plotter.plots as plots
from config import *
from snn_utils.comm.serializer import SERIALIZERS
from snn_utils.plotter import data_provider


def split_list(l, n_chunks):
    n_elements_per_chunk = len(l) // n_chunks
    for i in range(0, len(l), n_elements_per_chunk):
        yield l[i:i + n_elements_per_chunk]


def configure_detail_plot(data_source):
    p = plotter.PlotWindowBuilder(auto_vertical_padding=0.2)

    c1 = p.add_column(x_spines=True)

    pattern_colors = list(matplotlib.cm.plasma(np.linspace(0.2, 0.8, n_patterns)))

    c1.add_plot(plots.PhasePlot(data_source, 'pattern_id', n_patterns, label="cue", y_pos=0,
                                common_line_style={'linewidth': 5},
                                colors=pattern_colors, zero_is_value=False), height_ratio=0.5
                )

    c1.add_plot(plots.SpikeTrainPlot(data_source,
                                     [('pattern_in', i) for i in range(n_input_neurons)],
                                     label='input activity (#neurons = {})'.format(n_input_neurons),
                                     colors='black'),
                height_ratio=1.5)

    n_out = n_up_neurons + n_down_neurons
    c1.add_row_group([plots.SpikeTrainPlot(data_source, subset, colors=pattern_colors[i])
                      for i, subset in enumerate(split_list([('activity_in', i) for i in range(n_out)],
                                                            n_patterns))],
                     vertical_padding_ratio=0.1, height_ratio=[0.5] * n_patterns, hlines=True,
                     label="Up/Down neuron activity".format(n_out))

    c1.add_plot(plots.AnalogSignalPlot(data_source,
                                       ['activity_rate_{}'.format(i) for i in range(n_patterns)],
                                       label='lever position', colors=pattern_colors,
                                       y_ticks=[], y_lim=(-100.0, 100.0 )))

    c1.add_plot(plots.AnalogSignalPlot(data_source,
                                       ['curr', 'mean'], 'reward', ['current', 'average'],
                                       y_lim=(0.0, 1.0), y_ticks=[]))

    return p


if __name__ == '__main__':
    master = interface.Master()

    # data sources
    data_source = data_provider.ProxyDataSource(sender=False)

    # subscriptions
    comm = master.communicator()

    comm.add_subscriber(music_zmq_proxy_config['communication']['host'],
                        music_zmq_proxy_config['communication']['port'],
                        lambda delta: data_source.read_delta(delta),
                        deserialize=SERIALIZERS[music_zmq_proxy_config['communication']['format']].deserialize)

    mpl_plotter.configure_matplotlib()
    window = mpl_plotter.MatplotlibWindow(configure_detail_plot(data_source),
                                          data_source=data_source,
                                          max_time_window=plotter_node_time_window)

    master.scheduler().add_handle(lambda: window.draw(), 0.3)

    master.scheduler().add_handle(
        lambda: data_source.truncate(lower=data_source.get_max_time() - plotter_node_time_window)
        if data_source.get_max_time() is not None else None,
        10000)

    master.mainloop()
