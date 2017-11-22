import os
import matplotlib.pyplot as plt
import distribution


def get_title_placeholder():
    return "@$@"


def __split_points(points):
    xs = []
    ys = []
    for x, y in points:
        xs.append(x)
        ys.append(y)
    return xs, ys


def __write_xplot(draw_fn, points, pathname, title, xaxis_name, faxis_name):
    os.makedirs(os.path.dirname(pathname), exist_ok=True)
    fig = plt.figure(figsize=(19, 9), dpi=100)
    ax = fig.gca()
    if title: ax.set_title(title)
    if xaxis_name: ax.set_xlabel(xaxis_name)
    if faxis_name: ax.set_ylabel(faxis_name)
    ax.grid(True, linestyle='dotted')
    x, y = __split_points(points)
    draw_fn(ax, x, y)
    fig.savefig(pathname, bbox_inches='tight')
    plt.close()


def _scale_rgb_colour(rgb, weight):
    assert isinstance(rgb, tuple) and len(rgb) == 3 and all(isinstance(c, float) for c in rgb)
    assert isinstance(weight, float)
    scale = min(1.0, max(0.0, weight))
    return rgb[0] * scale, rgb[1] * scale, rgb[2] * scale


def get_colour_pre_excitatory(weight=1.0):
    return _scale_rgb_colour((0.0, 0.5, 1.0), weight)


def get_colour_pre_inhibitory(weight=1.0):
    return _scale_rgb_colour((0.0, 1.0, 0.5), weight)


def get_colour_post():
    return 1.0, 0.0, 0.0


def curve(points, pathname, colours=None, title=None, xaxis_name=None, faxis_name=None, marker="-"):
    if title is not None:
        title = title.replace(
            get_title_placeholder(),
            (
                "#points=" + str(len(points)) +
                ", mean=" + format(sum([n for _, n in points]) / (len(points) + 0.000001), ".2f")
            )
        )
    __write_xplot(lambda ax, x, y: ax.plot(x, y, marker, c=colours), points, pathname, title, xaxis_name, faxis_name)


def curve_per_partes(points, pathname, start, end, step, max_num_parts=None, on_plot_part_callback_fn=None,
                     colour=None, title=None, xaxis_name=None, faxis_name=None, marker="-"):
    assert start < end and step > 0.0001
    assert len(points) > 0
    assert max_num_parts is None or isinstance(max_num_parts, int) and max_num_parts > 0
    assert on_plot_part_callback_fn is None or callable(on_plot_part_callback_fn)
    if not os.path.exists(os.path.dirname(pathname)):
        os.mkdir(os.path.dirname(pathname))
    base_pathname, extension = os.path.splitext(pathname)
    stride = 1 if max_num_parts is None else max(1, int(((end - start) / step) / max_num_parts))
    x = start
    idx = 0
    while x < end:
        if idx % stride == 0:
            part_end = min(x + step, end)
            part_points = []
            for i, p in enumerate(points):
                if x <= p[0] and p[0] <= part_end:
                    part_points.append(p)
            part_pathname = base_pathname + "_" + str(idx).zfill(4) + "_" + format(x, ".4f") + extension
            part_title = "[part #" + str(idx) + "] "
            if title is not None:
                part_title += title
            if on_plot_part_callback_fn is not None:
                on_plot_part_callback_fn(part_pathname)
            curve(part_points, part_pathname, colour, part_title, xaxis_name, faxis_name, marker)
        x += step
        idx += 1


def scatter(points, pathname, colours=None, title=None, xaxis_name=None, faxis_name=None):
    if title is not None:
        title = title.replace(get_title_placeholder(), "#points=" + str(len(points)))
    __write_xplot(lambda ax, x, y: ax.scatter(x, y, s=2, c=colours), points, pathname, title, xaxis_name, faxis_name)


def scatter_per_partes(points, pathname, start, end, step, max_num_parts=None, on_plot_part_callback_fn=None,
                       colours=None, title=None, xaxis_name=None, faxis_name=None):
    assert start < end and step > 0.0001
    assert max_num_parts is None or isinstance(max_num_parts, int) and max_num_parts > 0
    assert len(points) > 0
    assert on_plot_part_callback_fn is None or callable(on_plot_part_callback_fn)
    if not os.path.exists(os.path.dirname(pathname)):
        os.mkdir(os.path.dirname(pathname))
    base_pathname, extension = os.path.splitext(pathname)
    stride = 1 if max_num_parts is None else max(1, int(((end - start) / step) / max_num_parts))
    x = start
    idx = 0
    while x < end:
        if idx % stride == 0:
            part_end = min(x + step, end)
            part_points = []
            if colours is None:
                part_colours = None
            else:
                part_colours = []
            for i, p in enumerate(points):
                if x <= p[0] and p[0] <= part_end:
                    part_points.append(p)
                    if colours is not None:
                        if isinstance(colours, list):
                            part_colours.append(colours[i])
                        else:
                            part_colours.append(colours)
            part_pathname = base_pathname + "_" + str(idx).zfill(4) + "_" + format(x, ".4f") + extension
            part_title = "[part #" + str(idx) + "] "
            if title is not None:
                part_title += title
            if on_plot_part_callback_fn is not None:
                on_plot_part_callback_fn(part_pathname)
            scatter(part_points, part_pathname, part_colours, part_title, xaxis_name, faxis_name)
        x += step
        idx += 1


def event_board(events, pathname, colours=None, title=None, xaxis_name=None, faxis_name=None):
    assert isinstance(events, list) and all(all(isinstance(e, float) or isinstance(e, int) for e in l) for l in events)
    assert colours is None or isinstance(colours, list)
    assert colours is None or all(all(isinstance(rgb, tuple) and
                                  len(rgb) == 3 and
                                  all(isinstance(e, float) or isinstance(e, int) for e in rgb)
                                      for rgb in l)
                                  for l in colours)
    assert colours is None or len(events) == len(colours)
    assert colours is None or all(len(events[i]) == len(colours[i]) for i in range(len(events)))
    scatter(
        [(events[i][j], i) for i in range(len(events)) for j in range(len(events[i]))],
        pathname,
        [colours[i][j] for i in range(len(colours)) for j in range(len(colours[i]))]
            if colours is not None
            else colours,
        title.replace(
            get_title_placeholder(),
            "["
            "rows=" + str(len(events)) + ", "
            "events=" + str(sum([len(l) for l in events])) +
            "]"
            ),
        xaxis_name,
        faxis_name
        )


def event_board_per_partes(events, pathname, start, end, step, max_num_parts=None, on_plot_part_callback_fn=None,
                           colours=None, title=None, xaxis_name=None, faxis_name=None):
    assert isinstance(events, list) and all(all(isinstance(e, float) or isinstance(e, int) for e in l) for l in events)
    assert colours is None or isinstance(colours, list)
    assert colours is None or all(all(isinstance(rgb, tuple) and
                                  len(rgb) == 3 and
                                  all(isinstance(e, float) or isinstance(e, int) for e in rgb)
                                      for rgb in l)
                                  for l in colours)
    assert colours is None or len(events) == len(colours)
    assert colours is None or all(len(events[i]) == len(colours[i]) for i in range(len(events)))
    assert isinstance(start, float) or isinstance(start, int)
    assert isinstance(end, float) or isinstance(end, int)
    assert start < end and step > 0.0001
    assert max_num_parts is None or isinstance(max_num_parts, int) and max_num_parts > 0
    assert on_plot_part_callback_fn is None or callable(on_plot_part_callback_fn)
    if not os.path.exists(os.path.dirname(pathname)):
        os.mkdir(os.path.dirname(pathname))
    base_pathname, extension = os.path.splitext(pathname)
    stride = 1 if max_num_parts is None else max(1, int(((end - start) / step) / max_num_parts))
    indices = [0 for _ in range(len(events))]
    x = start
    idx = 0
    while x < end:
        if idx % stride == 0:
            part_end = min(x + step, end)
            part_events = []
            if colours is None:
                part_colours = None
            else:
                part_colours = []
            is_part_empty = True
            for row_idx, row in enumerate(events):
                while indices[row_idx] < len(row) and row[indices[row_idx]] < x:
                    indices[row_idx] += 1
                part_events.append([])
                if colours is not None:
                    part_colours.append([])
                while indices[row_idx] < len(row) and row[indices[row_idx]] < part_end:
                    part_events[-1].append(row[indices[row_idx]])
                    if colours is not None:
                        part_colours[-1].append(colours[row_idx][indices[row_idx]])
                    is_part_empty = False
                    indices[row_idx] += 1
            if not is_part_empty:
                part_pathname = base_pathname + "_" + str(idx).zfill(4) + "_" + format(x, ".4f") + extension
                part_title = "[part #" + str(idx) + "] "
                if title is not None:
                    part_title += title
                if on_plot_part_callback_fn is not None:
                    on_plot_part_callback_fn(part_pathname)
                event_board(part_events, part_pathname, part_colours, part_title, xaxis_name, faxis_name)
        x += step
        idx += 1


def histogram(distrib, pathname, normalised=True, colours=None, title=None, xaxis_name=None, faxis_name=None):
    if isinstance(distrib, dict):   # 'distrib' is a plain histogram.
        distrib = distribution.Distribution(distrib)
    assert isinstance(distrib, distribution.Distribution)
    if title is None:
        title = get_title_placeholder()
    if normalised:
        title_addon = "[normalised] "
    else:
        title_addon = ""
    title_addon += (
        "#bars=" + str(len(distrib.get_histogram())) + ", "
        "sum bars=" + format(sum([value for _, value in distrib.get_histogram().items()]), ".3f") + ", "
        "median=" + format(distrib.get_median(), ".3f") + ", "
        "mean=" + format(distrib.get_mean(), ".3f") + ", "
        "mean freq.=" + format(1.0 / (distrib.get_mean() + 0.0001), ".3f") + ", "
        "variance=" + format(distrib.get_variance(), ".3f") + ", "
        "std. deviation=" + format(distrib.get_standard_deviation(), ".3f") + ", "
        "CV=" + format(distrib.get_coefficient_of_variation(), ".3f")
        )
    title = title.replace(get_title_placeholder(), title_addon)
    if normalised:
        points = distrib.get_probability_points()
    else:
        points = distrib.get_points()

    if len(distrib.get_events()) < 2:
        bar_width = 0.8
    else:
        bar_width = min(map(lambda x: abs(x[1] - x[0]), zip(distrib.get_events()[:-1], distrib.get_events()[1:])))
    __write_xplot(
        lambda ax, x, y: ax.bar(x, y, bar_width, color=colours, align="center"),
        points, pathname, title, xaxis_name, faxis_name)
