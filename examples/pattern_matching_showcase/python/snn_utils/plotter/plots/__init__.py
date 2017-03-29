import numpy as np

import matplotlib.ticker as ticker
from matplotlib.colors import LinearSegmentedColormap


class EmptyPlot(object):
    def __init__(self, spine_left=False, spine_right=False, spine_top=None, spine_bottom=None, label=None, hline=False):
        self._spine_top = spine_top
        self._spine_bottom = spine_bottom
        self._spine_left = spine_left
        self._spine_right = spine_right
        self._label = label
        self._hline = hline

    def build(self, ax, col_header=False, col_footer=False, **kwargs):
        ax.spines['top'].set_visible(self._spine_top if self._spine_top is not None else col_header)
        ax.spines['bottom'].set_visible(self._spine_top if self._spine_top is not None else col_footer)
        ax.spines['left'].set_visible(self._spine_left)
        ax.spines['right'].set_visible(self._spine_right)
        ax.xaxis.set_visible(False)
        ax.yaxis.set_ticks([])
        if self._label is not None:
            ax.set_ylabel(self._label)
        if self._hline:
            ax.set_ylim(-1, 1)
            ax.set_xlim(-1, 1)
            ax.hlines(0, -1, 1, lw=2, alpha=0.25, color='grey')
        return self

    def update_axis(self, *x_lim):
        pass

    def update(self, data_provider):
        pass

    def draw(self):
        pass


class Plot(object):
    def __init__(self, data_source, keys, label, legend, legend_loc, colors):
        self._data_source = data_source
        self._keys = keys
        self._label = label
        self._legend = legend
        self._legend_loc = legend_loc
        self._ax = None

        self._colors = colors

        if isinstance(colors, LinearSegmentedColormap):
            self._colors = self._gen_colors_with_colormap(colors)
        elif isinstance(colors, list):
            self._colors = colors
        else:
            # also handles None
            self._colors = self._gen_colors_with_atom(colors)

    def _gen_colors_with_colormap(self, colormap):
        return colormap(np.linspace(0, 1, len(self._keys)))

    def _gen_colors_with_atom(self, atom):
        return [atom] * len(self._keys)

    def _configure_axis(self):
        if self._label is not None:
            self._ax.set_ylabel(self._label)
        if self._legend:
            loc = self._legend_loc
            if loc is None:
                loc = 'upper right'
            self._ax.legend(self._legend, prop={'size': 12}, loc=loc, ncol=len(self._legend))

    def _create_primitive(self, color):
        return None

    def _create_primitives(self):
        self._ps = dict(zip(self._keys, map(lambda c: self._create_primitive(c), self._colors)))

    def _get_data_source(self):
        return self._data_source

    def update(self):
        pass

    def get_artists(self):
        return self._ps.values()

    def draw(self):
        for artist in self.get_artists():
            assert not isinstance(artist, list)
            self._ax.draw_artist(artist)


class TimeSeriesPlot(Plot):
    def build(self, ax, show_x=False, col_header=False, col_footer=False):
        self._ax = ax
        self._create_primitives()
        self._configure_axis()
        if show_x:
            self._ax.set_xlabel("Simulation time [s]")
            self._ax.xaxis.tick_top()
            self._ax.xaxis.set_label_position('top')
            self._ax.xaxis.set_major_formatter(ticker.FormatStrFormatter('%d'))  # fixes an evil formatting bug
        else:
            self._ax.xaxis.set_visible(False)
        self._ax.spines['top'].set_visible(col_header or show_x)
        self._ax.spines['bottom'].set_visible(col_footer)
        return self

    def update_axis(self, *x_lim):
        self._ax.set_xlim(*x_lim)


class TimePlotDual(object):
    def __init__(self):
        self._plots = []
        self._dynamic_plots = []

    def build(self, ax, show_x=False, **kwargs):
        assert self._plots
        self._dynamic_plots.append(self._plots[0].build(ax, show_x, **kwargs))
        if len(self._plots) > 1:
            twinx = ax.twinx()
            self._dynamic_plots.append(self._plots[1].build(twinx, show_x=False))
            twinx.spines['top'].set_visible(False)
            twinx.spines['bottom'].set_visible(False)
            twinx.spines['left'].set_visible(False)
            twinx.spines['right'].set_visible(False)
        return self

    def update(self):
        for plot in self._dynamic_plots:
            plot.update()

    def add_plot(self, plot):
        assert len(self._plots) < 2
        self._plots.append(plot)


class PhasePlot(TimeSeriesPlot):
    def __init__(self, data_source, key, n_values, zero_is_value=False, y_pos=0, common_line_style=None,
                 individual_line_styles=None, label=None, legend=None, legend_loc=None, colors=None):
        self._n_values = n_values
        TimeSeriesPlot.__init__(self, data_source, [key], label, legend, legend_loc, colors)
        assert legend is None or len(legend) == n_values
        assert len(self._colors) == n_values
        assert y_pos == 0 or np.abs(y_pos) >= 1
        self._drop_zero = not zero_is_value
        self._y_pos = y_pos
        self._line_styles = self._gen_line_styles(common_line_style if common_line_style else {},
                                                  individual_line_styles if individual_line_styles else [{}] * n_values)

    def _gen_line_styles(self, shared_ls, specific_lss):
        for specific_ls, color in zip(specific_lss, self._colors):
            ls = shared_ls.copy()
            if 'color' not in ls:
                ls['color'] = color
            ls.update(specific_ls)
            yield ls

    def _gen_colors_with_colormap(self, colormap):
        return colormap(np.linspace(0, 1, self._n_values))

    def _gen_colors_with_atom(self, atom):
        return [atom] * self._n_values

    def _configure_axis(self):
        TimeSeriesPlot._configure_axis(self)
        self._ax.set_ylim(min(-1, self._y_pos), max(1, self._y_pos))
        self._ax.yaxis.set_ticks([])

    def _create_primitives(self):
        self._ps = map(lambda ls: self._ax.plot([], [], **ls)[0], self._line_styles)

    def get_artists(self):
        return self._ps

    def update(self):
        TimeSeriesPlot.update(self)
        signal = np.array(self._get_data_source().get_cont_data(self._keys, self._ax.get_xlim())[0])
        times, values = np.rollaxis(signal, 1)
        for value_id, ps in enumerate(self._ps, start=self._drop_zero):
            transformed_values = values.copy()
            transformed_values[np.where(values != value_id)] = np.NaN
            transformed_values[np.where(values == value_id)] = 0
            ps.set_data(np.array([times, transformed_values]))


class AnalogSignalPlot(TimeSeriesPlot):
    def __init__(self, data_source, keys, label=None, legend=None, legend_loc=None, colors=None, y_lim=None,
                 y_ticks=None,
                 y_ticks_right=True):
        TimeSeriesPlot.__init__(self, data_source, keys, label, legend, legend_loc, colors)
        self._y_lim = y_lim
        self._y_ticks = y_ticks
        self._y_ticks_right = y_ticks_right

    def _configure_axis(self):
        TimeSeriesPlot._configure_axis(self)
        if self._y_lim:
            assert len(self._y_lim) == 2
            self._ax.set_ylim(*self._y_lim)
        if self._y_ticks is not None:
            if isinstance(self._y_ticks, dict):
                self._ax.yaxis.set_ticks(self._y_ticks.keys())
                # TODO test this. Presumable map onto values is needed to retain order
                self._ax.set_yticklabels(self._y_ticks.values())
            else:
                assert isinstance(self._y_ticks, list)
                self._ax.yaxis.set_ticks(self._y_ticks)
        if self._y_ticks_right:
            self._ax.yaxis.tick_right()

    def _create_primitive(self, color):
        return self._ax.plot([], [], color=color)[0]

    def update(self):
        TimeSeriesPlot.update(self)
        for key, signal in zip(self._keys, self._get_data_source().get_cont_data(self._keys, self._ax.get_xlim())):
            self._ps[key].set_data(np.rollaxis(np.array(signal), 1))
        if not self._y_lim:
            self._ax.relim()
            self._ax.autoscale_view(True, True, True)


class SpikeTrainPlot(TimeSeriesPlot):
    def __init__(self, data_source, keys, label=None, legend=None, legend_loc=None, colors=None,
                 y_tick_filter=lambda ids: [],
                 y_tick_labels=lambda ids: map(str, ids), y_ticks_right=True):
        Plot.__init__(self, data_source, keys, label, legend, legend_loc, colors)
        self._y_tick_filter = y_tick_filter
        self._y_tick_labels = y_tick_labels
        self._y_ticks_right = y_ticks_right

    def _configure_axis(self):
        TimeSeriesPlot._configure_axis(self)
        self._ax.set_ylim(-1, len(self._keys))

        y_ticks = self._y_tick_filter(range(len(self._keys)))
        if y_ticks:
            self._ax.yaxis.set_ticks(y_ticks)
            self._ax.set_yticklabels(self._y_tick_labels(y_ticks))
            if self._y_ticks_right:
                self._ax.yaxis.tick_right()
        else:
            self._ax.yaxis.set_ticks([])

    def _create_primitive(self, color):
        return self._ax.scatter([], [], color=color, marker='|', linewidth='1.5', s=20)

    def update(self):
        TimeSeriesPlot.update(self)
        for key, times in zip(self._keys, self._get_data_source().get_event_data(self._keys, self._ax.get_xlim())):
            # FIXME inefficient
            offsets = np.rollaxis(np.array([times, np.ones_like(times) * self._keys.index(key)]), 1)
            self._ps[key].set_offsets(offsets)


class SpikeMapPlot(Plot):
    def __init__(self, data_source, keys, key_to_xy, events_to_value=lambda events: len(events), value_bounds=None,
                 time_window=None, show_ticks=False, label=None, legend=None, legend_loc=None, colors=None):
        Plot.__init__(self, data_source, keys, label, legend, legend_loc, colors)
        self._positions = map(key_to_xy, keys)
        self._n_cols = max(map(lambda p: p[0], self._positions)) + 1
        self._n_rows = max(map(lambda p: p[1], self._positions)) + 1
        self._data = np.zeros(shape=(self._n_cols, self._n_rows))
        self._xlim = (0, 0)
        self._time_window = time_window if time_window is not None else np.inf
        self._events_to_value = events_to_value
        self._value_bounds = value_bounds
        self._show_ticks = show_ticks

    def _configure_axis(self):
        Plot._configure_axis(self)

    def _create_primitives(self):
        if self._value_bounds is not None:
            assert len(self._value_bounds) == 2
            self._ps = self._ax.imshow(self._data, vmin=self._value_bounds[0], vmax=self._value_bounds[1])
        else:
            self._ps = self._ax.imshow(self._data)
        self._ax.xaxis.tick_top()
        self._ax.xaxis.set_visible(self._show_ticks)
        self._ax.yaxis.set_visible(self._show_ticks)

    def update(self):
        Plot.update(self)

        max_time = self._get_data_source().get_max_time()
        min_time = max(max_time - self._time_window, self._get_data_source().get_min_time())

        for key, (x, y), times in zip(self._keys, self._positions,
                                      self._get_data_source().get_event_data(self._keys, (min_time, max_time))):
            self._data[y][x] = self._events_to_value(times)

        self._ps.set_array(self._data)
        # TODO autoscale if self._value_bounds == None

    def get_artists(self):
        return [self._ps]

    def build(self, ax, *args, **kwargs):
        self._ax = ax
        self._create_primitives()
        self._configure_axis()
        return self


class WeightPlot(Plot):
    def __init__(self, data_source, key, show_ticks=False, label=None, legend=None, legend_loc=None, colors=None):
        Plot.__init__(self, data_source, [key], label, legend, legend_loc, colors)
        self._last_plot_ts = None
        self._show_ticks = show_ticks

    def update(self):
        Plot.update(self)

        time_window = None
        if self._last_plot_ts is not None:
            time_window = (self._last_plot_ts + 0.00001, np.inf)
        df = self._get_data_source().get_weight_data(self._keys[0], time_window)
        if not df.sim_time.empty:
            if self._last_plot_ts is None or max(df.sim_time) > self._last_plot_ts:
                self._last_plot_ts = max(df.sim_time)
                self._update_plot(df[df.sim_time == self._last_plot_ts])

    def _update_plot(self, df):
        pass

    def build(self, ax, *args, **kwargs):
        self._ax = ax
        self._create_primitives()
        return self


class QuiverWeightPlot(WeightPlot):
    def __init__(self, data_source, key, weight_to_spatial, label=None, legend=None, legend_loc=None, colors=None):
        WeightPlot.__init__(self, data_source, key, label, legend, legend_loc, colors)
        self._weight_to_spatial = weight_to_spatial
        self._updated = True

    def _create_primitives(self):
        self._ps = None

    def _configure_axis(self):
        WeightPlot._configure_axis(self)
        self._ax.xaxis.tick_top()
        self._ax.set_aspect(1.)
        self._ax.xaxis.set_visible(self._show_ticks)
        self._ax.yaxis.set_visible(self._show_ticks)

    def _update_plot(self, weight_df):
        self._ax.clear()
        self._configure_axis()

        df = self._weight_to_spatial(weight_df)

        self._ps = self._ax.quiver(df.X, df.Y, df.dX, df.dY,
                        pivot='mid', units='xy',  # keep the arrow inside its pixel box
                        scale=np.max(np.hypot(df.dX, df.dY)) + 0.05,
                        # color=colors
                        )

        self._ax.set_xlim(min(df.X) - 0.5, max(df.X) + 0.5)
        self._ax.set_ylim(min(df.Y) - 0.5, max(df.Y) + 0.5)

        # camera pixels are usually top-down
        self._ax.set_ylim(self._ax.get_ylim()[::-1])

        # xs = np.arange(0, dvs_pixel_width - 1, 1) + 0.5
        # ys = np.arange(0, dvs_pixel_height - 1, 1) + 0.5
        # style = {'alpha': 0.15}
        # ax.vlines(xs, *ax.get_ylim(), **style)
        # ax.hlines(ys, *ax.get_xlim(), **style)
        self._updated = True

    def get_artists(self):
        if self._ps is not None:
            return [self._ps]
        else:
            return []

    def draw(self):
        if self._updated:
            WeightPlot.draw(self)
            self._updated = False
