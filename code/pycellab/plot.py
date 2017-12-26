import os
import matplotlib.pyplot as plt
import mpl_toolkits.mplot3d.axes3d as axes3d
import argparse
import numpy
import json
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
        "mean freq.=" + format(1.0 / (distrib.get_mean() if abs(distrib.get_mean()) > 0.00001 else 0.00001), ".3f") + ", "
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


def _autodetect_plot_kind(plot_data):
    if isinstance(plot_data, dict):
        if "points" in plot_data and isinstance(plot_data["points"], list):
            if "num_dimensions" in plot_data:
                if plot_data["num_dimensions"] == 2:
                    if len(plot_data["points"]) > 0:
                        return "points_2d"
                elif plot_data["num_dimensions"] == 3:
                    if len(plot_data["points"]) > 0 and "error" in plot_data["points"][0]:
                        return "points_3d_with_error"
    return None


def _show_points_2d_impl(plot_data, line_style, point_style):
    fig = plt.figure(dpi=100)
    ax = fig.gca()
    ax.set_title(plot_data["plot_title"] if "plot_title" in plot_data else "points_2d")
    ax.set_xlabel(plot_data["plot_x_axis_label"] if "plot_x_axis_label" in plot_data else "x")
    ax.set_ylabel(plot_data["plot_y_axis_label"] if "plot_y_axis_label" in plot_data else "y")
    ax.grid(plot_data["plot_show_grid"] if "plot_show_grid" in plot_data else True, linestyle='dotted')
    fx = [p["x"] for p in plot_data["points"]]
    fy = [p["y"] for p in plot_data["points"]]
    ax.plot(fx, fy, linestyle=line_style, marker=point_style)
    plt.show()


def _show_points_2d(plot_data):
    _show_points_2d_impl(plot_data, "None", ".")


def _show_lines_2d(plot_data):
    _show_points_2d_impl(plot_data, "-", "None")


def _show_points_and_lines_2d(plot_data):
    _show_points_2d_impl(plot_data, "-", ".")


def _show_points_3d_with_error(plot_data):
    fig = plt.figure(dpi=100)
    ax = fig.add_subplot(111, projection='3d')

    points = plot_data["points"]

    fx = [p["x"] for p in points]
    fy = [p["y"] for p in points]
    fz = [p["z"] for p in points]

    elo = [p["error"]["lo"] for p in points]
    ehi = [p["error"]["hi"] for p in points]

    for i in range(len(points)):
        ax.plot([fx[i], fx[i]], [fy[i], fy[i]], [elo[i], ehi[i]], marker="_", color="red")
    ax.plot(fx, fy, fz, linestyle="None", marker=".")

    # region = datalgo.VoltageEffectRegion()
    # rfz = [region.get_low_voltage(p["x"], p["y"]) for p in points]
    # ax.plot(fx, fy, rfz, linestyle="None", marker=".", color="green")
    #
    # max_err = 0.0
    # for i in range(len(points)):
    #     max_err = max(max_err, abs(fz[i] - rfz[i]))
    # print("max_err=" + str(max_err))

    plt.show()


def _compute_points_of_function_2d(user_function, dom_x):
    result = []
    for x in numpy.arange(dom_x[0], dom_x[1] + 0.0001, float(dom_x[1] - dom_x[0]) / (dom_x[2] - 1)):
        try:
            y = user_function(x)
            result.append({"x": x, "y": y})
        except:
            pass
    return {"num_dimensions": 2, "points":  result}


def _compute_points_of_function_3d(user_function, dom_x, dom_y):
    result = []
    for x in numpy.arange(dom_x[0], dom_x[1] + 0.0001, float(dom_x[1] - dom_x[0]) / (dom_x[2] - 1)):
        for y in numpy.arange(dom_y[0], dom_y[1] + 0.0001, float(dom_y[1] - dom_y[0]) / (dom_y[2] - 1)):
            try:
                z = user_function(x, y)
                result.append({"x": x, "y": y, "z": z})
            except:
                pass
    return {"num_dimensions": 3, "points":  result}


def _get_plot_kinds_bindings():
    return {
        "points_2d": _show_points_2d,
        "lines_2d": _show_lines_2d,
        "points_and_lines_2d": _show_points_and_lines_2d,
        "points_3d_with_error": _show_points_3d_with_error
    }


def _main(cmdline):
    if cmdline.function is not None:
        plot_data = _compute_points_of_function_2d(cmdline.function, cmdline.dom_x)\
                        if cmdline.dom_y is None\
                        else _compute_points_of_function_3d(cmdline.function, cmdline.dom_x, cmdline.dom_y)
        with open(cmdline.input, "w") as ofile:
            ofile.write(json.dumps(plot_data, sort_keys=True, indent=4))
    else:
        with open(cmdline.input, "r") as ifile:
            plot_data = json.load(ifile)
    if cmdline.kind is None:
        cmdline.kind = _autodetect_plot_kind(plot_data)
        if cmdline.kind is None:
            print("ERROR: The automatic detection of plot kind for the data in the passed JSON file has FAILED. "
                  "Use the option '--kind' to supply the desired plot kind.")
            return 8
    if cmdline.kind not in _get_plot_kinds_bindings():
        print("ERROR: Unknown plot kind '" + cmdline.kind + "'. See option '--kind' for valid values.")
        return 9
    _get_plot_kinds_bindings()[cmdline.kind](plot_data)
    return 0


def _parse_command_line_options():
    parser = argparse.ArgumentParser(
        description="The module provides view of plot data stored in JSON format.",
        )
    parser.add_argument(
        "input", type=str,
        help="A pathname of a JSON file containing plot data to be viewed."
        )
    parser.add_argument(
        "--kind", type=str,
        help="Specifies a kind of plot to be used for data in a passed JSON file. When omitted, "
             "then the most relevant plot kind is automatically chosen. Here are available kinds: " +
             ", ".join(_get_plot_kinds_bindings().keys())
        )
    parser.add_argument(
        "--function", type=str,
        help="Defines a function, namely some Python lambda expression, to be uses for generating content of JSON "
             "file (whose path-name is passed to the script) which will then by plotted. For example, to plot "
             "a quadratic function y=x^2 pass the string \"lambda x: x*x\" to this option. Note that option --kind "
             "must be compatible with the kind and dimensionality of data produced by the passed function."
        )
    parser.add_argument(
        "--dom-x", type=str,
        help="Defines a domain of the passed function along the X axis. The domain is defined in form of a sting "
             "containing a tuple (lo, hi, n), where [lo, hi) is the interval along the axis and n is a number of "
             "samples in that interval to be considered. We require that lo < hi and n >= 2."
        )
    parser.add_argument(
        "--dom-y", type=str,
        help="Defines a domain of the passed function along the Y axis. The domain is defined in form of a sting "
             "containing a tuple (lo, hi, n), where [lo, hi) is the interval along the axis and n is a number of "
             "samples in that interval to be considered. We require that lo < hi and n >= 2."
        )
    cmdline = parser.parse_args()

    if cmdline.function is not None:
        if not cmdline.function.startswith("lambda "):
            print("ERROR: The string '" + cmdline.function + "' passed to --function option is not a lambda expression.")
            exit(3)
        try:
            cmdline.function = eval(cmdline.function)
            assert callable(cmdline.function)
        except Exception as e:
            print("ERROR: The conversion of the string '" + cmdline.function + "' to callable has FILED! Details: " + str(e))
            exit(4)
        if cmdline.dom_x is None:
            print("ERROR: The option --dom-x was not specified.")
            exit(5)

    if cmdline.dom_x is not None:
        try:
            cmdline.dom_x = eval(cmdline.dom_x)
            assert isinstance(cmdline.dom_x, tuple) and len(cmdline.dom_x) == 3
            assert type(cmdline.dom_x[0]) in [int, float] and type(cmdline.dom_x[1]) in [int, float]
            assert type(cmdline.dom_x[1]) in [int]
            assert cmdline.dom_x[0] < cmdline.dom_x[1] and cmdline.dom_x[2] >= 2
        except Exception as e:
            print("ERROR: The conversion of the string '" + cmdline.dom_x + "' to domain has FILED! Details: " + str(e))
            exit(6)

    if cmdline.dom_y is not None:
        try:
            cmdline.dom_y = eval(cmdline.dom_y)
            assert isinstance(cmdline.dom_y, tuple) and len(cmdline.dom_y) == 3
            assert type(cmdline.dom_y[0]) in [int, float] and type(cmdline.dom_y[1]) in [int, float]
            assert type(cmdline.dom_y[1]) in [int]
            assert cmdline.dom_y[0] < cmdline.dom_y[1] and cmdline.dom_y[2] >= 2
        except Exception as e:
            print("ERROR: The conversion of the string '" + cmdline.dom_y + "' to domain has FILED! Details: " + str(e))
            exit(7)

    if not os.path.isfile(cmdline.input) and cmdline.function is None:
        print("ERROR: The passed path '" + cmdline.input + "' does not reference a file.")
        exit(1)
    if os.path.splitext(cmdline.input)[1].lower() != ".json":
        print("ERROR: The passed path '" + cmdline.input + "' does not have '.json' extension.")
        exit(2)
    cmdline.input = os.path.abspath(cmdline.input)

    return cmdline


if __name__ == "__main__":
    exit(_main(_parse_command_line_options()))
