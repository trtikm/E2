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


def __write_preffix(ofile, ftype, title, xaxis_name, faxis_name):
    if title:
        title = "set title \"" + title + "\"\n"
    else:
        title = ""
    if xaxis_name:
        xaxis_name = "set xlabel \"" + xaxis_name + "\"\n"
    else:
        xaxis_name = ""
    if faxis_name:
        faxis_name = "set ylabel \"" + faxis_name + "\"\n"
    else:
        faxis_name = ""

    ofile.write(
        title +
        "set terminal " + ftype + " font 'Liberation serif,16' size 1500,768\n"
        "set grid\n" +
        xaxis_name +
        faxis_name
        )


def __write_points(ofile, points):
    for x, fx in points:
        ofile.write("    " + str(x) + "    " + str(fx) + "\n")
    ofile.write("    e\n")


def __write_plot(plotline, points, pathname, title=None, xaxis_name=None, faxis_name=None):
    os.makedirs(os.path.dirname(pathname), exist_ok=True)
    tmp_pathname = pathname + ".plt"
    with open(tmp_pathname, "w") as script_file:
        __write_preffix(script_file, os.path.splitext(pathname)[1][1:].lower(), title, xaxis_name, faxis_name)
        script_file.write(plotline)
        __write_points(script_file, points)
    mksvg(tmp_pathname, pathname)
    os.remove(tmp_pathname)


def mksvg(src_pathname, dst_pathname):
    assert os.path.exists(src_pathname)
    os.makedirs(os.path.dirname(dst_pathname), exist_ok=True)
    os.system("gnuplot \"" + src_pathname + "\" > \"" + dst_pathname + "\"")


def curve(points, pathname, title=None, xaxis_name=None, faxis_name=None):
    __write_plot(
        "plot '-' using 1:2 with lines linetype 1 notitle\n",
        points, pathname, title, xaxis_name, faxis_name)


def scatter(points, pathname, title=None, xaxis_name=None, faxis_name=None):
    __write_plot(
        "plot '-' using 1:2 with points pointsize 0.25 pointtype 7 notitle\n",
        points, pathname, title, xaxis_name, faxis_name)


def histogram(distrib, pathname, bar_width=None, normalised=True, title=None, xaxis_name=None, faxis_name=None):
    if isinstance(distrib, dict):   # 'distrib' is a plain histogram.
        distrib = distribution.distribution(distrib)
    assert isinstance(distrib, distribution.distribution)
    if not bar_width:
        width = None
        for i in range(1, len(distrib.get_events())):
            if (type(distrib.get_events()[i]) in [int, float] and
                    type(distrib.get_events()[i-1]) in [int, float]):
                delta = abs(distrib.get_events()[i] - distrib.get_events()[i-1])
            else:
                delta = 1.0
            if width:
                width = min(width, delta)
            else:
                width = delta
        bar_width = width
    if not title:
        title = (
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
    __write_plot(
        "set style fill pattern 5 border\n"
        "set boxwidth " + str(bar_width) + " absolute\n"
        "plot '-' using 1:2 with boxes notitle\n",
        points, pathname, title, xaxis_name, faxis_name)


def __write_xplot(draw_fn, points, pathname, title, xaxis_name, faxis_name):
    os.makedirs(os.path.dirname(pathname), exist_ok=True)
    fig = plt.figure(figsize=(15.5, 7.5))
    ax = fig.gca()
    if title: ax.set_title(title)
    if xaxis_name: ax.set_xlable(xaxis_name)
    if faxis_name: ax.set_ylable(faxis_name)
    ax.grid(True)
    x, y = __split_points(points)
    draw_fn(ax, x, y)
    fig.savefig(pathname)
    plt.close()


def xcurve(points, pathname, title=None, xaxis_name=None, faxis_name=None):
    def f(ax, x, y): ax.plot(x, y)
    __write_xplot(f, points, pathname, title, xaxis_name, faxis_name)


def xscatter(points, pathname, title=None, xaxis_name=None, faxis_name=None):
    def f(ax, x, y): ax.scatter(x, y, s=2)
    __write_xplot(f, points, pathname, title, xaxis_name, faxis_name)


def xhistogram(distrib, pathname, bar_width=None, normalised=True, title=None, xaxis_name=None, faxis_name=None):
    if isinstance(distrib, dict):   # 'distrib' is a plain histogram.
        distrib = distribution.distribution(distrib)
    assert isinstance(distrib, distribution.distribution)
    if not bar_width:
        width = None
        for i in range(1, len(distrib.get_events())):
            if (type(distrib.get_events()[i]) in [int, float] and
                    type(distrib.get_events()[i-1]) in [int, float]):
                delta = abs(distrib.get_events()[i] - distrib.get_events()[i-1])
            else:
                delta = 1.0
            if width:
                width = min(width, delta)
            else:
                width = delta
        bar_width = width
    if not title:
        title = (
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
    def f(ax, x, y): ax.bar(x, y, align='center', width=bar_width, edgecolor="black")
    __write_xplot(f, points, pathname, title, xaxis_name, faxis_name)
