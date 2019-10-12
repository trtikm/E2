import os
import matplotlib.pyplot as plt
import mpl_toolkits.mplot3d.axes3d as axes3d
from matplotlib import cm
import argparse
import numpy
import math
import json
import distribution


def get_title_placeholder():
    return "@$@"


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

    def histogram(self, distrib, colours=None, normalised=None, legend=None):
        if isinstance(distrib, dict):   # 'distrib' is a plain histogram.
            distrib = distribution.Distribution(distrib)
        assert isinstance(distrib, distribution.Distribution)
        if normalised:
            points = distrib.get_probability_points()
        else:
            points = distrib.get_points()

        if len(distrib.get_events()) < 2:
            bar_width = 0.8
        else:
            bar_width = min(map(lambda x: abs(x[1] - x[0]), zip(distrib.get_events()[:-1], distrib.get_events()[1:])))
        self.make_plot(
            lambda ax, x, y: ax.bar(x, y, bar_width, color=self._choose_colours(colours), align="center", label=legend),
            points,
            legend is not None
            )


def __write_xplot(draw_fn, points, pathname, title, xaxis_name, faxis_name, size_xy=None, dpi=None):
    with Plot(pathname, title, xaxis_name, faxis_name, size_xy, dpi) as the_plot:
        the_plot.make_plot(draw_fn, points)


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
    assert start < end and step > 0.000001
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


def scatter(points, pathname, colours=None, title=None, xaxis_name=None, faxis_name=None, size_xy=None, dpi=None):
    if title is not None:
        title = title.replace(get_title_placeholder(), "#points=" + str(len(points)))
    __write_xplot(lambda ax, x, y: ax.scatter(x, y, s=2, c=colours),
                  points, pathname, title, xaxis_name, faxis_name, size_xy, dpi)


def scatter_per_partes(points, pathname, start, end, step, max_num_parts=None, on_plot_part_callback_fn=None,
                       colours=None, title=None, xaxis_name=None, faxis_name=None):
    assert start < end and step > 0.000001
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
    assert start < end and step > 0.000001
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
        "sum x*bar=" + format(sum([x*value for x, value in distrib.get_histogram().items()]), ".3f") + ", "
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


def _plot_points_2d_impl(points, line_style, point_style):
    fig = plt.figure(figsize=(7, 7), dpi=100)
    ax = fig.gca()
    fx = [p[0] for p in points]
    fy = [p[1] for p in points]
    ax.plot(fx, fy, linestyle=line_style, marker=point_style)
    plt.show()


def plot_points_2d(points):
    _plot_points_2d_impl(points, "None", ".")


def plot_lines_2d(points):
    _plot_points_2d_impl(points, "-", "None")


def plot_points_and_lines_2d(plot_data):
    _plot_points_2d_impl(plot_data, "-", ".")


#########################################################################################################
#########################################################################################################
#########################################################################################################


def ik_rotate_line_to_look_at_point():
    """It is an "experiment" function"""
    R = 1.0
    q = 2.0
    beta = math.pi / 4.0

    if R <= 0.0:
        print("ERROR: Bad input R (must be > 0).")
        return

    if q <= 0.0:
        print("ERROR: Bad input q (must be > 0).")
        return

    S = [0, 0]
    P = [q, 0]

    print("INPUT:")
    print("    R=" + str(R))
    print("    q=" + str(q))
    print("    beta=" + str(beta))
    print("    ---------------")
    print("    S" + str(S))
    print("    P" + str(P))

    sin_beta = math.sin(beta)
    cos_beta = math.cos(beta)

    D = q**2 - R**2 * sin_beta**2
    if D < 0.0:
        print("ERROR D < 0.0")
        return

    cos_alphas = [
        (R * sin_beta**2 + cos_beta * math.sqrt(D)) / q,
        (R * sin_beta**2 - cos_beta * math.sqrt(D)) / q
        ]

    gfx_points = [S, P, [0, q]]
    gfx_lines = [S, P, [0, q], [0, q]]

    print("RESULTS:")
    for i in range(len(cos_alphas)):
        if cos_alphas[i] < -1.0 or cos_alphas[i] > 1.0:
            print("WARNING: cos_alpha[" + str(i) + "] is out of range  <-1, 1>: " + str(cos_alphas[i]))
            print("         So, skipping the computation for the angle.")
            continue
        alpha = math.acos(cos_alphas[i])

        cos_alpha = cos_alphas[i]
        sin_alpha = math.sin(alpha)

        X = [R*cos_alpha, R*sin_alpha]
        T = [q*cos_alpha**2, q*sin_alpha*cos_alpha]

        print("    for alpha[" + str(i) + "]=" + str(alpha) + ":")
        print("        X" + str(X))
        print("        T" + str(T))

        if i == 2:
            gfx_points.append(X)
            gfx_points.append(T)

            gfx_lines.append(S)
            gfx_lines.append(X)

            gfx_lines.append(X)
            gfx_lines.append(T)

            gfx_lines.append(X)
            gfx_lines.append(P)

            gfx_lines.append(T)
            gfx_lines.append(P)

        # verifying angle between 'XT' and 'XP', which must be equal to 'beta'
        XT = [T[0]-X[0], T[1]-X[1]]
        XP = [P[0]-X[0], P[1]-X[1]]
        dot_XTXT = XT[0]*XT[0] + XT[1]*XT[1]
        dot_XPXP = XP[0]*XP[0] + XP[1]*XP[1]
        dot_XTXP = XT[0]*XP[0] + XT[1]*XP[1]
        if dot_XTXT * dot_XPXP <= 0.00000001:
            print("ERROR: verifivation: dot_XTXT * dot_XPXP < 0.00000001.")
            continue
        cos_beta_verify = dot_XTXP / math.sqrt(dot_XTXT * dot_XPXP)
        if cos_beta_verify < -1.0 or cos_beta_verify > 1.0:
            print("ERROR: cos_beta_verify is out of range  <-1, 1>: " + str(cos_beta_verify))
            continue
        beta_verify = math.acos(cos_beta_verify)

        print("        beta=" + str(beta_verify))
        print("        beta[error]=" + str(beta - beta_verify))

    # plot_points_2d(gfx_points)
    # plot_lines_2d(gfx_lines)


#########################################################################################################


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
                    else:
                        return "points_3d"
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


def _show_lines_list_2d(plot_data):
    assert plot_data["num_dimensions"] == 2
    new_plot_data = {
        "num_dimensions": 2,
        "points": [
            {
                "x": [],     # x-coords of all first points of lines
                "y": []      # y-coords of all first points of lines
            },
            {
                "x": [],     # x-coords of all second points of lines
                "y": []      # y-coords of all second points of lines
            }
        ]
    }
    for i, p in enumerate(plot_data["points"]):
        j = i % 2
        new_plot_data["points"][j]["x"].append(p["x"])
        new_plot_data["points"][j]["y"].append(p["y"])
    _show_points_2d_impl(new_plot_data, "-", "None")


def _show_points_and_lines_2d(plot_data):
    _show_points_2d_impl(plot_data, "-", ".")


def _show_points_3d_impl(plot_data, line_style, point_style):
    fig = plt.figure(dpi=100)
    ax = fig.gca(projection='3d')
    ax.set_title(plot_data["plot_title"] if "plot_title" in plot_data else "points_3d")
    ax.set_xlabel(plot_data["plot_x_axis_label"] if "plot_x_axis_label" in plot_data else "x")
    ax.set_ylabel(plot_data["plot_y_axis_label"] if "plot_y_axis_label" in plot_data else "y")
    ax.set_zlabel(plot_data["plot_z_axis_label"] if "plot_z_axis_label" in plot_data else "z")

    def _make_grid_points_3d(points):
        def _split_by_same_coord(points, coord_name):
            result = []
            for p in points:
                if len(result) == 0 or result[-1][-1][coord_name] != p[coord_name]:
                    result.append([p])
                else:
                    result[-1].append(p)
            return result

        x = []
        y = []
        z = []
        for grid_line in [sorted(same_x_points, key=lambda p: p["y"])
                          for same_x_points in _split_by_same_coord(sorted(points, key=lambda p: p["x"]), "x")]:
            x.append([p["x"] for p in grid_line])
            y.append([p["y"] for p in grid_line])
            z.append([p["z"] for p in grid_line])
        return x, y, z

    if line_style is "wireframe":
        fx, fy, fz = _make_grid_points_3d(plot_data["points"])
        ax.plot_wireframe(fx, fy, fz)
    elif line_style is "surface":
        fx, fy, fz = _make_grid_points_3d(plot_data["points"])
        ax.plot_surface(fx, fy, fz, cmap=cm.coolwarm, antialiased=False)
    else:
        fx = [p["x"] for p in plot_data["points"]]
        fy = [p["y"] for p in plot_data["points"]]
        fz = [p["z"] for p in plot_data["points"]]
        if "rgb" in plot_data:
            colours = [(rgb["r"], rgb["g"], rgb["b"]) for rgb in plot_data["rgb"]]
            assert len(colours) == len(fz)
            ax.scatter(fx, fy, fz, c=colours, s=1)
        else:
            ax.plot(fx, fy, fz, linestyle="None", marker=point_style)
    plt.show()


def _show_points_3d(plot_data):
    _show_points_3d_impl(plot_data, "None", ".")


def _show_lines_3d(plot_data):
    _show_points_3d_impl(plot_data, "wireframe", "None")


def _show_surface_3d(plot_data):
    _show_points_3d_impl(plot_data, "surface", ".")


def _show_points_3d_with_error(plot_data):
    fig = plt.figure(dpi=100)
    ax = fig.gca(projection='3d')

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
        "lines_list_2d": _show_lines_list_2d,
        "points_and_lines_2d": _show_points_and_lines_2d,
        "points_3d": _show_points_3d,
        "lines_3d": _show_lines_3d,
        "surface_3d": _show_surface_3d,
        "points_3d_with_error": _show_points_3d_with_error
    }


def _main(cmdline):
    if cmdline.experiment is not None:
        print("*** EXPERIMENT[" + str(cmdline.experiment) + "] ***")
        eval(cmdline.experiment)()
        print("*** DONE ***")
        return 0
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
    parser.add_argument(
        "--experiment", type=str,
        help="Runs an experiment function hard-coded in this script."
        )
    cmdline = parser.parse_args()

    if cmdline.experiment is not None:
        return cmdline

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
            assert type(cmdline.dom_x[2]) in [int]
            assert cmdline.dom_x[0] < cmdline.dom_x[1] and cmdline.dom_x[2] >= 2
        except Exception as e:
            print("ERROR: The conversion of the string '" + str(cmdline.dom_x) + "' to domain has FILED! Details: " + str(e))
            exit(6)

    if cmdline.dom_y is not None:
        try:
            cmdline.dom_y = eval(cmdline.dom_y)
            assert isinstance(cmdline.dom_y, tuple) and len(cmdline.dom_y) == 3
            assert type(cmdline.dom_y[0]) in [int, float] and type(cmdline.dom_y[1]) in [int, float]
            assert type(cmdline.dom_y[2]) in [int]
            assert cmdline.dom_y[0] < cmdline.dom_y[1] and cmdline.dom_y[2] >= 2
        except Exception as e:
            print("ERROR: The conversion of the string '" + str(cmdline.dom_y) + "' to domain has FILED! Details: " + str(e))
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
