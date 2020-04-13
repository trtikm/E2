import os
import matplotlib.pyplot as plt
import numpy


def _split_points(points):
    xs = []
    ys = []
    for x, y in points:
        xs.append(x)
        ys.append(y)
    return xs, ys


def get_predefined_colour_names():
    return [
        "blue",
        "green",
        "red",
        "cyan",
        "magenta",
        "orange",
        "black",
        "brown",
        "navy",
        "khaki",
        "olive",
        "pink",
        "violet",
        "purple",
        "yellow",
        "salmon",
        ]


def get_random_rgb_colour(min_component=0.0, max_component=0.9, rnd_generator=None):
    if rnd_generator is None:
        rnd_generator = lambda lo, hi: numpy.random.uniform(lo, hi)
    return (rnd_generator(min_component, max_component),
            rnd_generator(min_component, max_component),
            rnd_generator(min_component, max_component) )


class Plot:
    def __init__(self, pathname, title=None, xaxis_name=None, faxis_name=None, size_xy=None, dpi=None):
        assert isinstance(pathname, str) and len(pathname) > 0
        assert title is None or isinstance(title, str)
        assert xaxis_name is None or isinstance(xaxis_name, str)
        assert faxis_name is None or isinstance(faxis_name, str)
        assert size_xy is None or (isinstance(size_xy, tuple) and len(size_xy) == 2)
        assert dpi is None or isinstance(dpi, int)
        if size_xy is None:
            size_xy = (19, 9)
        if dpi is None:
            dpi = 100
        self._pathname = pathname
        os.makedirs(os.path.dirname(self._pathname), exist_ok=True)
        self._fig = plt.figure(figsize=size_xy, dpi=dpi)
        self._ax = self._fig.gca()
        if title:
            self._ax.set_title(title)
        if xaxis_name:
            self._ax.set_xlabel(xaxis_name)
        if faxis_name:
            self._ax.set_ylabel(faxis_name)
        self._ax.grid(True, linestyle='dotted')
        self._auto_colour_index = 0
        self._show_legend = False

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self._show_legend is True:
            self._ax.legend()
        self._fig.savefig(self._pathname, bbox_inches='tight')
        plt.close()

    def make_plot(self, draw_fn, points, has_legend=False):
        assert callable(draw_fn)
        assert isinstance(points, list) and all(isinstance(p, tuple) and len(p) == 2 for p in points)
        x, y = _split_points(points)
        draw_fn(self._ax, x, y)
        self._show_legend = self._show_legend or has_legend is True

    def _choose_colours(self, colours):
        if colours is not None:
            return colours
        if self._auto_colour_index < len(get_predefined_colour_names()):
            self._auto_colour_index += 1
            return get_predefined_colour_names()[self._auto_colour_index - 1]
        return (numpy.random.uniform(0.0, 0.75), numpy.random.uniform(0.0, 0.75), numpy.random.uniform(0.0, 0.75))

    def curve(self, points, colours=None, marker=None, legend=None):
        self.make_plot(
            lambda ax, x, y: ax.plot(x, y, "-" if marker is None else marker, c=self._choose_colours(colours), label=legend),
            points,
            legend is not None
            )

    def scatter(self, points, colours=None, legend=None):
        self.make_plot(
            lambda ax, x, y: ax.scatter(x, y, s=2, c=self._choose_colours(colours), label=legend),
            points,
            legend is not None
            )

    def histogram(self, hist, colours=None, normalised=None, legend=None):
        events = [p[0] for p in hist]
        if len(hist) < 2:
            bar_width = 0.8
        else:
            bar_width = min(map(lambda x: abs(x[1] - x[0]), zip(events[:-1], events[1:])))
        self.make_plot(
            lambda ax, x, y: ax.bar(x, y, bar_width, color=self._choose_colours(colours), align="center", label=legend),
            hist,
            legend is not None
            )
