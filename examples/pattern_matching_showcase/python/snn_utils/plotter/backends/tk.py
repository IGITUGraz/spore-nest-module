from __future__ import absolute_import

from Tkinter import *
from ttk import *

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

import snn_utils.plotter as plotter


class FormFrame(Frame):
    def __init__(self, parent, *args, **kwargs):
        Frame.__init__(self, parent, *args, **kwargs)
        self._row_count = 0

    def add_row(self, label_text, widget_funcs):
        Label(self, text=label_text, anchor=W).grid(row=self._row_count, column=0)

        row = Frame(self)
        row.grid(row=self._row_count, column=1, padx=5, pady=5)

        widgets = []
        for widget_func in widget_funcs:
            widget = widget_func(row)
            widget.pack(side=LEFT, expand=YES, fill=X)
            widgets.append(widget)

        self._row_count += 1
        return widgets


class TkPlotCanvas(FigureCanvasTkAgg):
    def __init__(self, fig, *args, **kwargs):
        FigureCanvasTkAgg.__init__(self, fig, *args, **kwargs)
        self._fig = fig

    def resize(self, event):
        FigureCanvasTkAgg.resize(self, event)
        self._fig.tight_layout()


class PlotFrame(plotter.PlotWindow, Frame):
    def __init__(self, tk_parent, plot_builder, data_source, max_time_window=None):
        plotter.PlotWindow.__init__(self, plot_builder, data_source, max_time_window, quick_draw=False)
        Frame.__init__(self, tk_parent)

        self._tk_canvas = TkPlotCanvas(self.get_figure(), master=tk_parent)
        self._tk_canvas.get_tk_widget().pack(fill=BOTH, expand=1)

        self._enabled_var = BooleanVar()
        self._enabled_var.set(True)

        self._plot_invalidated = True

    def _create_figure(self):
        return Figure()

    def _draw(self):
        self._tk_canvas.draw()

    def draw(self):
        if self._enabled_var.get() and self._plot_invalidated:
            plotter.PlotWindow.draw(self)
            # self._plot_invalidated = False

    def invalidate(self):
        self._plot_invalidated = True

    def get_enabled_variable(self):
        return self._enabled_var


class PlotFrameControl(Frame):
    def __init__(self, parent, plot_frame):
        Frame.__init__(self, parent)

        self._plot_frame = plot_frame

        self._sim_time_scale_track_max_threshold = 1.0
        self._sim_time_scale = Scale(self, orient=HORIZONTAL, from_=0, to=0,
                                     command=self._set_sim_time)
        self._sim_time_scale.pack(fill=X)

        control_frame = FormFrame(self)

        self._sim_time_label, = control_frame.add_row("Sim. Time:", [lambda p: Label(p)])
        self._update_frequency_entry, = control_frame.add_row("Frequency:", [lambda p: Entry(p)])

        self._enabled_var = BooleanVar()
        self._enabled_var.set(True)
        self._auto_update_btn = Checkbutton(control_frame, text="Auto Update",
                                            onvalue=True, offvalue=False, variable=self._enabled_var)
        self._auto_update_btn.grid(row=0, rowspan=2, column=2, sticky=W + E + N + S)

        self._plot_invalidated = True

        control_frame.pack(fill=NONE, side=BOTTOM)

    def _set_sim_time(self, sim_time):
        sim_time = float(sim_time)
        if self._data_source.get_max_time() - sim_time < self._sim_time_scale_track_max_threshold:
            self._max_time_to_show = None
        else:
            self._max_time_to_show = sim_time
        self._plot_invalidated = True

    def draw(self):
        if self._plot_invalidated or self._max_time_to_show is None:
            plotter.PlotWindow.draw(self)
            self._plot_invalidated = False
        self._sim_time_label.config(text="{:.2f}s".format(self._data_source.get_max_time()))
        self._sim_time_scale.config(from_=self._data_source.get_min_time(), to=self._data_source.get_max_time())
        if self._max_time_to_show is None:
            self._sim_time_scale.set(self._data_source.get_max_time())
