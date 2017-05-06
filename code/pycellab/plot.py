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
    if xaxis_name: ax.set_xlable(xaxis_name)
    if faxis_name: ax.set_ylable(faxis_name)
    ax.grid(True, linestyle='dotted')
    x, y = __split_points(points)
    draw_fn(ax, x, y)
    fig.savefig(pathname, bbox_inches='tight')
    plt.close()


def curve(points, pathname, colours=None, title=None, xaxis_name=None, faxis_name=None):
    __write_xplot(lambda ax, x, y: ax.plot(x, y, c=colours), points, pathname, title, xaxis_name, faxis_name)


def scatter(points, pathname, colours=None, title=None, xaxis_name=None, faxis_name=None):
    if not title:
        title = "#points=" + str(len(points))
    __write_xplot(lambda ax, x, y: ax.scatter(x, y, s=2, c=colours), points, pathname, title, xaxis_name, faxis_name)


def histogram(distrib, pathname, normalised=True, markers="o", colours=None, title=None, xaxis_name=None, faxis_name=None):
    if isinstance(distrib, dict):   # 'distrib' is a plain histogram.
        distrib = distribution.distribution(distrib)
    assert isinstance(distrib, distribution.distribution)
    if not title:
        if normalised:
            title = "[normalised] "
        else:
            title = ""
        title += (
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
    __write_xplot(
        lambda ax, x, y: ax.stem(x, y, linefmt=colours+"-", markerfmt=colours+markers, basefmt="None"),
        points, pathname, title, xaxis_name, faxis_name)
