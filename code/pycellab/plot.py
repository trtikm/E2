import os


def __write_preffix(ofile, title, xaxis_name, faxis_name):
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
        "set terminal svg font 'Liberation serif,16' size 1024,768\n"
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
    with open(pathname, "w") as script_file:
        __write_preffix(script_file, title, xaxis_name, faxis_name)
        script_file.write(plotline)
        __write_points(script_file, points)


def mksvg(src_pathname):
    assert os.path.exists(src_pathname)
    dst_pathname = os.path.join(
        os.path.dirname(src_pathname),
        os.path.splitext(os.path.basename(src_pathname))[0] + ".svg"
        )
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


def histogram(points, pathname, title=None, xaxis_name=None, faxis_name=None):
    __write_plot(
        "set style fill pattern 5 border\n"
        "set boxwidth 0.025 relative\n"
        "plot '-' using 1:2 with boxes notitle\n",
        points, pathname, title, xaxis_name, faxis_name)
