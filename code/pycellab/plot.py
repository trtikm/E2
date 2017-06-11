import os
import matplotlib.pyplot as plt
import distribution


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


def curve(points, pathname, colours=None, title=None, xaxis_name=None, faxis_name=None, marker="-"):
    if not title:
        title = (
            "#points=" + str(len(points)) +
            ", mean=" + format(sum([n for _, n in points]) / (len(points) + 0.000001), ".2f")
            )
    __write_xplot(lambda ax, x, y: ax.plot(x, y, marker, c=colours), points, pathname, title, xaxis_name, faxis_name)


def curve_per_partes(points, pathname, start, end, step, on_plot_part_callback_fn=None,
                     colours=None, title=None, xaxis_name=None, faxis_name=None, marker="-"):
    assert start < end and step > 0.0001
    assert len(points) > 0
    assert on_plot_part_callback_fn is None or callable(on_plot_part_callback_fn)
    base_pathname, extension = os.path.splitext(pathname)
    x = start
    idx = 0
    while x < end:
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
        if title is None:
            part_title += "#points=" + str(len(part_points)) +\
                          ", mean=" + format(sum([n for _, n in part_points]) / (len(part_points) + 0.000001), ".2f")
        else:
            part_title += title
        if on_plot_part_callback_fn is not None:
            on_plot_part_callback_fn(part_pathname)
        curve(part_points, part_pathname, part_colours, part_title, xaxis_name, faxis_name, marker)
        x += step
        idx += 1


def scatter(points, pathname, colours=None, title=None, xaxis_name=None, faxis_name=None):
    if not title:
        title = "#points=" + str(len(points))
    __write_xplot(lambda ax, x, y: ax.scatter(x, y, s=2, c=colours), points, pathname, title, xaxis_name, faxis_name)


def histogram(distrib, pathname, normalised=True, colours=None, title=None, xaxis_name=None, faxis_name=None):
    if isinstance(distrib, dict):   # 'distrib' is a plain histogram.
        distrib = distribution.distribution(distrib)
    assert isinstance(distrib, distribution.distribution)
    if not title:
        if normalised:
            title = "[normalised] "
        else:
            title = ""
        title += (
            "sum bars=" + format(sum([value for _, value in distrib.get_histogram().items()]), ".3f") + ", "
            "median=" + format(distrib.get_median(), ".3f") + ", "
            "mean=" + format(distrib.get_mean(), ".3f") + ", "
            "mean frequency=" + format(1.0 / (distrib.get_mean() + 0.0001), ".3f") + ", "
            "variance=" + format(distrib.get_variance(), ".3f") + ", "
            "standard deviation=" + format(distrib.get_standard_deviation(), ".3f") + ", "
            "coefficient of variation=" + format(distrib.get_coefficient_of_variation(), ".3f")
            )
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
