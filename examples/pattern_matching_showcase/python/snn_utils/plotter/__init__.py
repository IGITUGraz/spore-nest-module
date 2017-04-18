import matplotlib.gridspec as gridspec

from . import plots as plot_widgets


class BaseRow(object):
    def __init__(self, height_ratio, label, x_spines, y_spines):
        self._height_ratio = height_ratio
        self.label = label
        self.x_spines = x_spines
        self.y_spines = y_spines

    def _create_row_ax(self, fig, coord):
        ax = fig.add_subplot(coord)
        if self.label:
            ax.set_ylabel(self.label)
        ax.set_yticks([])
        ax.set_xticks([])
        ax.spines['top'].set_visible(self.x_spines)
        ax.spines['bottom'].set_visible(self.x_spines)
        ax.spines['left'].set_visible(self.y_spines)
        ax.spines['right'].set_visible(self.y_spines)
        return ax

    def fill_figure(self, fig, gs_coord, **build_params):
        raise NotImplementedError()

    def n_cols(self):
        raise NotImplementedError(self)

    def height_ratio(self):
        return self._height_ratio


class Row(BaseRow):
    def __init__(self, height_ratio, label, x_spines=False, y_spines=False):
        BaseRow.__init__(self, height_ratio=height_ratio, label=label, x_spines=x_spines, y_spines=y_spines)
        self.plots = []

    def add_plot(self, plot):
        self.plots.append(plot)

    def fill_figure(self, fig, gs_coord, **build_params):
        self._create_row_ax(fig, gs_coord)
        sub_gs = gridspec.GridSpecFromSubplotSpec(1, self.n_cols(), subplot_spec=gs_coord,
                                                  width_ratios=[1 for _ in range(self.n_cols())])

        plots = []
        for col_index, plot in enumerate(self.plots):
            coord = sub_gs[col_index]
            ax = fig.add_subplot(coord)
            plots.append(plot.build(ax, **build_params))
        return plots

    def n_cols(self):
        return len(self.plots)


class EmptyRow(BaseRow):
    def __init__(self, height_ratio, label, border_gap, x_spines=False, y_spines=False, hline=False):
        BaseRow.__init__(self, height_ratio=height_ratio, label=label, x_spines=x_spines, y_spines=y_spines)
        self.border_gap = border_gap
        self.hline = hline

    def fill_figure(self, fig, coord, **build_params):
        ax = fig.add_subplot(coord)
        plot_widgets.EmptyPlot(
            spine_left=not self.border_gap, spine_right=not self.border_gap,
            label=self.label,
            hline=self.hline,
        ).build(ax, **build_params)
        return []  # nothing to update

    def n_cols(self):
        return None


class RowContainer(object):
    def __init__(self, v_padding=None, border_gap=True, width_ratios=None, hlines=False):
        self._rows = []
        self._v_padding = v_padding
        self._width_ratios = width_ratios
        self._border_gap = border_gap
        self._hlines = hlines

    def _wrap_plot(self, plot, height_ratio, label):
        row = Row(height_ratio=height_ratio, label=label)
        if isinstance(plot, list):
            for p in plot:
                row.add_plot(p)
        else:
            row.add_plot(plot)
        return row

    def _add_auto_vertical_padding(self):
        self.add_vertical_padding(height_ratio=self._v_padding, border_gap=self._border_gap)

    def add_vertical_padding(self, height_ratio, border_gap=True, label=None):
        self._rows.append(EmptyRow(height_ratio=height_ratio, border_gap=border_gap, label=label, hline=self._hlines))

    def add_row(self, row):
        if self._v_padding and len(self._rows) > 0 and not isinstance(self._rows[-1], EmptyRow):
            self._add_auto_vertical_padding()
        self._rows.append(row)

    def add_plot(self, plot, height_ratio=1, label=None):
        self.add_row(self._wrap_plot(plot, height_ratio, label=label))

    def n_rows(self):
        return len(self._rows)


class AggregateRow(RowContainer, BaseRow):
    def __init__(self, border_gap=True, auto_vertical_padding=None, label=None, width_ratios=None,
                 x_spines=False, y_spines=False, show_x=False, hlines=False):
        RowContainer.__init__(self, v_padding=auto_vertical_padding, border_gap=border_gap, hlines=hlines)
        BaseRow.__init__(self, height_ratio=1, label=label, x_spines=x_spines, y_spines=y_spines)
        self.border_gap = border_gap
        self.label = label
        self.width_ratios = width_ratios
        self.show_x = show_x

    def height_ratio(self):
        return sum([row.height_ratio() for row in self._rows])

    def fill_figure(self, fig, gs_coord, **build_params):
        self._create_row_ax(fig, gs_coord)

        n_rows = self.n_rows()
        n_cols = self.n_cols()
        # assert n_cols == 1, "More than one column is not implemented yet."
        height_ratios = [row.height_ratio() for row in self._rows]

        # assert self.width_ratios is None or len(self.width_ratios) == n_cols

        sub_gs = gridspec.GridSpecFromSubplotSpec(n_rows, n_cols, height_ratios=height_ratios,
                                                  subplot_spec=gs_coord, hspace=0)

        dynamic_plots = []
        for row_index, row in enumerate(self._rows):
            sub_gs_coords = sub_gs[row_index]
            build_params = {}
            if self.show_x and row_index == 0:
                build_params['show_x'] = True
            dynamic_plots.extend(row.fill_figure(fig, sub_gs_coords, **build_params))
        return dynamic_plots

    def n_cols(self):
        return max([row.n_cols() for row in self._rows])

    def add_row_group(self, plots, vertical_padding_ratio=None, label=None, height_ratio=1, hlines=False):
        if isinstance(height_ratio, int) or isinstance(height_ratio, float):
            height_ratios = [height_ratio] * len(plots)
        else:
            assert isinstance(height_ratio, list) and len(height_ratio) == len(plots), \
                "{} vs {}".format(height_ratio, plots)
            height_ratios = height_ratio

        row_agg = AggregateRow(border_gap=False, auto_vertical_padding=vertical_padding_ratio, label=label,
                               hlines=hlines)

        for plot, height_ratio in zip(plots, height_ratios):
            row_agg.add_plot(plot, height_ratio=height_ratio)

        self.add_row(row_agg)

    def add_plot(self, plot, height_ratio=1, label=None):
        RowContainer.add_plot(self, plot, height_ratio=height_ratio, label=label)


class PlotWindowBuilder(object):
    def __init__(self, width_ratios=None, auto_vertical_padding=None):
        self._width_ratios = width_ratios
        self._parent_row = []
        self._width_ratios = []
        self._auto_vertical_padding = auto_vertical_padding

    def add_column(self, width_ratio=1, x_spines=False):
        agg_row = AggregateRow(auto_vertical_padding=self._auto_vertical_padding, x_spines=x_spines, show_x=x_spines)
        self._parent_row.append(agg_row)
        self._width_ratios.append(width_ratio)
        return agg_row

    def build(self, fig):
        coords = gridspec.GridSpec(1, len(self._parent_row), hspace=0, width_ratios=self._width_ratios)

        plots = []
        for parent_row, coord in zip(self._parent_row, coords):
            plots.extend(parent_row.fill_figure(fig, coord))
        return plots


class PlotWindow(object):
    def __init__(self, plot_builder, data_source, max_time_window=None, quick_draw=True):
        self._fig = self._create_figure()
        self._plots = plot_builder.build(self._fig)
        self._data_source = data_source

        self._max_time_window = max_time_window
        self._max_time_to_show = None

        self._quick_draw = quick_draw
        self._quick_draw_init = quick_draw and True
        self._quick_draw_update = quick_draw and 'update' in dir(self._fig.canvas)

    def _create_figure(self):
        raise NotImplementedError()

    def _draw(self):
        raise NotImplementedError()

    def get_figure(self):
        return self._fig

    def draw(self):
        if self._data_source.get_max_time() is None or self._data_source.get_min_time() is None:
            return

        upper_time = self._max_time_to_show
        if upper_time is None:
            upper_time = self._data_source.get_max_time()
        lower_time = self._data_source.get_min_time()
        if self._max_time_window is not None:
            lower_time = max(lower_time, upper_time - self._max_time_window)

        for plot in self._plots:
            if hasattr(plot, 'update_axis'):
                plot.update_axis(lower_time, upper_time)

        if self._quick_draw_init:
            # initial draw for caching of artists
            self._draw()
            self._quick_draw_init = False
        for plot in self._plots:
            plot.update()
            if self._quick_draw:
                plot.draw()
        if self._quick_draw_update:
            self._fig.canvas.update()
        elif not self._quick_draw:
            self._draw()
