import bisect


def is_number(instance):
    return isinstance(instance, float) or isinstance(instance, int)


def is_point(instance):
    return isinstance(instance, tuple) and len(instance) == 2 and is_number(instance[0]) and is_number(instance[1])


def is_list_of_numbers(numbers):
    return isinstance(numbers, list) and all(is_number(number) for number in numbers)


def is_list_of_events(events):
    return is_list_of_numbers(events)


def is_sorted_list_of_events(events):
    return is_list_of_events(events) and all(events[i - 1] <= events[i] for i in range(1, len(events)))


def is_list_of_points(points):
    return isinstance(points, list) and all(is_point(point) for point in points)


def is_sorted_list_of_points_along_x_axis(points):
    return is_list_of_points(points) and all(points[i - 1][0] <= points[i][0] for i in range(1, len(points)))


def is_histogram(histogram):
    return isinstance(histogram, dict) and all(is_number(key) and is_number(value) for key, value in histogram.items())


def make_difference_events(events):
    assert is_list_of_events(events)

    result = [] if len(events) < 2 else [events[i] - events[i - 1] for i in range(1, len(events))]

    assert is_list_of_events(result)
    return result


def make_weighted_events(events, max_merge_distance, unit_weight=1, weight_function=lambda _: 1):
    assert is_list_of_events(events)
    assert is_number(max_merge_distance) and max_merge_distance > 0
    assert is_number(unit_weight)

    result = []
    if len(events) != 0:
        result = [(events[0], unit_weight)]
        for event in events[1:]:
            if abs(event - result[-1][0]) <= max_merge_distance:
                weight_delta = unit_weight * weight_function(event - result[-1][0] / max_merge_distance)
                result[-1] = (result[-1][0], result[-1][1] + weight_delta)
            else:
                result.append((event, unit_weight))

    assert is_list_of_points(result)
    return result


def reduce_gaps_between_points_along_x_axis(points, max_gap_size, y_coord=0):
    assert is_list_of_points(points)
    assert is_number(max_gap_size) and max_gap_size > 0
    assert is_number(y_coord)

    result = []
    if len(points) != 0:
        result.append(points[0])
        for point in points[1:]:
            dx = max_gap_size if point[0] >= result[-1][0] else -max_gap_size
            x = result[-1][0] + dx
            temp = []
            while abs(point[0] - x) >= max_gap_size:
                temp.append(x)
                x += dx
            if len(temp) != 0:
                result.append((temp[0], y_coord))
                if len(temp) > 1:
                    result.append((temp[-1], y_coord))
            result.append(point)

    assert is_list_of_points(result)
    return result


def compute_bin_size(events, desired_bins_count):
    assert is_list_of_events(events)
    assert isinstance(desired_bins_count, int) and desired_bins_count > 0

    if len(events) == 0:
        result = 0
    else:
        result = float(max(events) - min(events)) / float(desired_bins_count)

    assert is_number(result)
    assert result


def make_histogram(events, bin_size=None):
    assert is_list_of_events(events)
    assert is_number(bin_size) and bin_size > 0

    result = {}
    min_event = min(events)
    for event in events:
        bin_idx = int(float(event - min_event) / float(bin_size) + float(bin_size) / 2.0)
        key = min_event + bin_idx * bin_size
        result[key] = 1 if key not in result else result[key] + 1

    assert is_histogram(result)
    return result


def make_histogram_from_points(points, num_bins=100):
    assert is_list_of_points(points)
    assert len(points) > 1
    assert isinstance(num_bins, int) and num_bins > 0

    points = sorted(points, key=lambda p: p[0])
    x = [p[0] for p in points]
    lo = min(x)
    hi = max(x)
    assert hi > lo + 0.0001
    y = [p[1] for p in points]
    result = {}
    for i in range(num_bins):
        t = i / float(num_bins - 1)
        t = lo + t * (hi - lo)
        assert t not in result
        j = bisect.bisect_left(x, t)
        assert j < len(x)
        if j == 0 or x[j] - x[j - 1] < 0.0001:
            result[t] = y[j]
        else:
            assert x[j - 1] < t
            result[t] = y[j - 1] + (t - x[j - 1]) * ((y[j] - y[j - 1]) / (x[j] - x[j - 1]))
            if abs(result[t]) < 0.00001:
                result[t] = 0.0
            assert result[t] >= 0.0

    assert is_histogram(result)
    return result


def merge_sorted_lists_of_events(list_of_lists_of_events):
    assert isinstance(list_of_lists_of_events, list)
    assert all(is_sorted_list_of_events(events) for events in list_of_lists_of_events)

    def _get_index_of_list_with_minimal_pivot_element(list_of_lists_of_events, indices):
        result = None
        for idx in range(len(list_of_lists_of_events)):
            if indices[idx] < len(list_of_lists_of_events[idx]):
                value = list_of_lists_of_events[idx][indices[idx]]
                current_min_value = list_of_lists_of_events[result][indices[result]] if result is not None else None
                result = idx if result is None or value < current_min_value else result
        return result

    result = []
    indices = [0 for _ in range(len(list_of_lists_of_events))]
    while True:
        idx = _get_index_of_list_with_minimal_pivot_element(list_of_lists_of_events, indices)
        if idx is None:
            break
        result.append(list_of_lists_of_events[idx][indices[idx]])
        indices[idx] += 1

    assert is_sorted_list_of_events(result)
    assert len(result) == sum(len(events) for events in list_of_lists_of_events)
    return result


def compose_sorted_lists_of_points(list_of_lists_of_points, multipliers=None, epsilon=0.00001):
    assert isinstance(list_of_lists_of_points, list)
    assert all(is_sorted_list_of_points_along_x_axis(points) for points in list_of_lists_of_points)
    assert multipliers is None or (is_list_of_numbers(multipliers) and len(list_of_lists_of_points) == len(multipliers))

    def _get_indices_of_lists_with_minimal_pivot_element(list_of_lists_of_events, indices, epsilon):
        result = []
        for idx in range(len(list_of_lists_of_events)):
            if indices[idx] < len(list_of_lists_of_events[idx]):
                if len(result) == 0:
                    result.append(idx)
                else:
                    current_min_value = list_of_lists_of_events[result[0]][indices[result[0]]][0]
                    value = list_of_lists_of_events[idx][indices[idx]][0]
                    if value < current_min_value - epsilon:
                        result.clear()
                        result.append(idx)
                    elif value <= current_min_value + epsilon:
                        result.append(idx)
        return result

    result = []
    indices = [0 for _ in range(len(list_of_lists_of_points))]
    while True:
        idx = _get_indices_of_lists_with_minimal_pivot_element(list_of_lists_of_points, indices, epsilon)
        if len(idx) == 0:
            break
        x, y = list_of_lists_of_points[idx[0]][indices[idx[0]]]
        for i in idx[1:]:
            y += list_of_lists_of_points[i][indices[i]][1] * (1 if multipliers is None else multipliers[i])
        assert len(result) == 0 or result[-1][0] <= x
        result.append((x, y))
        for i in idx:
            indices[i] += 1

    assert is_sorted_list_of_points_along_x_axis(result)
    return result


def make_points_of_hermit_cubic_spline(p0, m0, p1, m1, num_points=100):
    assert is_point(p0)
    assert is_point(m0)
    assert is_point(p1)
    assert is_point(m1)
    assert isinstance(num_points, int) and num_points > 1

    def h00(t):
        return 2.0 * t**3 - 3.0 * t**2 + 1

    def h10(t):
        return t**3 - 2.0 * t**2 + t

    def h01(t):
        return -2.0 * t**3 + 3.0 * t**2

    def h11(t):
        return t**3 - t**2

    def point(t):
        return (h00(t)*p0[0] + h10(t)*m0[0] + h01(t)*p1[0] + h11(t)*m1[0],
                h00(t)*p0[1] + h10(t)*m0[1] + h01(t)*p1[1] + h11(t)*m1[1])

    result = [point(i / float(num_points - 1)) for i in range(num_points)]

    assert is_list_of_points(result)
    return result


def move_scale_curve_points(
        points,
        scale_x=1.0,
        scale_y=1.0,
        pow_y=1.0,
        shift_x=0.0
        ):
    assert is_list_of_points(points)
    assert is_number(scale_x) and scale_x > 0.001
    assert is_number(scale_y) and scale_y > 0.001

    result = [(scale_x * p[0] + shift_x, (scale_y * p[1])**pow_y) for p in points]

    assert is_list_of_points(result)
    return result


########################################################################################################################
########################################################################################################################
########################################################################################################################


# def mkhist_by_linear_interpolation(points, dx):
#     assert dx > 0.00001
#     hist = dict(points)
#     for i in range(1, len(points)):
#         a = points[i - 1]
#         b = points[i]
#         x = a[0] + dx
#         while x < b[0]:
#             assert x not in hist
#             hist[x] = a[1] + ((x - a[0]) / (b[0] - a[0])) * (b[1] - a[1])
#             x += dx
#     return hist


# def mkhist_from_curve_points(points, num_bars=100):
#     assert len(points) > 1
#     assert num_bars > 0
#     points = sorted(points, key=lambda p: p[0])
#     x = [p[0] for p in points]
#     lo = min(x)
#     hi = max(x)
#     assert hi > lo + 0.0001
#     y = [p[1] for p in points]
#     hist = {}
#     for i in range(num_bars):
#         t = i / float(num_bars - 1)
#         t = lo + t * (hi - lo)
#         assert t not in hist
#         j = bisect.bisect_left(x, t)
#         assert j < len(x)
#         if j == 0 or x[j] - x[j - 1] < 0.0001:
#             hist[t] = y[j]
#         else:
#             assert x[j - 1] < t
#             hist[t] = y[j - 1] + (t - x[j - 1]) * ((y[j] - y[j - 1]) / (x[j] - x[j - 1]))
#             if abs(hist[t]) < 0.00001:
#                 hist[t] = 0.0
#             assert hist[t] >= 0.0
#     return hist


# def make_fx_points(pairs, function, dx=1.0):
#     assert dx > 0.000001
#     assert callable(function)
#     if len(pairs) == 0:
#         return []
#     result = [pairs[0]]
#     for x, fx in pairs[1:]:
#         if abs(x - result[-1][0]) < 0.5 * dx:
#             result[-1] = (result[-1][0], function(result[-1][1], fx))
#         else:
#             result.append((x, fx))
#     return result
#
#
# def make_sum_po1ints(pairs, dx=1.0):
#     return make_fx_points(pairs, float.__add__, dx)
#
#
# def make_min_points(pairs, dx=1.0):
#     return make_fx_points(pairs, min, dx)
#
#
# def make_max_points(pairs, dx=1.0):
#     return make_fx_points(pairs, max, dx)


# def mkhist(events, nbins=100, lo=None, hi=None, use_bins_domain=False):
#     assert type(nbins) == int and nbins > 0
#     if len(events) == 0:
#         return {}, 1.0
#     if lo is None:
#         lo = min(events)
#     if hi is None:
#         hi = max(events)
#     dx = float(hi - lo) / float(nbins)
#     if dx < 0.00001:
#         return {events[0]: len(events)}, 1.0
#     hist = {}
#     for x in events:
#         b = int((x - lo) / dx + dx / 2.0)
#         if b in hist:
#             hist[b] += 1
#         else:
#             hist[b] = 1
#     if use_bins_domain:
#         return hist, dx
#     return {lo + i * dx: c for i, c in hist.items()}, dx


# def get_standard_spike_noise():
#     s = numpy.random.exponential(1, 10000)
#     hist, _ = mkhist(s, 500, use_bins_domain=True)
#     xhist = {}
#     keys = sorted(hist.keys())
#     for idx in range(0, min(300, len(keys))):
#         if idx < 5:
#             value = 0.0
#         elif idx < 10:
#             x = (idx - 5.0) / 10.0
#             assert x >= 0.0 and x <= 1.0
#             value = hist[keys[10]] * (3.0 * x**2 - 2.0 * x**3)
#         else:
#             value = hist[keys[idx]]
#         xhist[0.001 * (idx + 2)] = value
#         idx += 1
#     return Distribution(xhist)


# def make_isi_histogram(time_events, dt=0.001):
#     assert all(isinstance(t, float) for t in time_events)
#     assert isinstance(dt, float) and dt > 0.00001
#     if len(time_events) == 0:
#         return {}
#     nbins = int(float(time_events[-1] + dt) / float(dt))
#     lo = 0.0
#     hi = nbins * dt
#     return mkhist([time_events[i] - time_events[i - 1] for i in range(1, len(time_events))], nbins, lo, hi)[0]


# def make_times_histogram(time_events, dt=0.001, start_time=None):
#     assert all(isinstance(t, float) for t in time_events)
#     assert start_time is None or isinstance(start_time, float)
#     assert isinstance(dt, float) and dt > 0.00001
#     nbins = int(float(max(time_events) + dt) / dt)
#     lo = dt * (int(float(min(time_events)) / dt) if start_time is None else int(start_time / dt))
#     hi = dt * nbins
#     return mkhist(time_events, nbins, lo, hi)[0]


# def make_counts_histogram(time_events, start_bin=0, bin_size=1):
#     assert bin_size > 0.000001
#     xhist = {}
#     for event in time_events:
#         idx = int(float(event - start_bin) / float(bin_size) + float(bin_size) / 2.0)
#         if idx in xhist:
#             xhist[idx] += 1
#         else:
#             xhist[idx] = 1
#     return {start_bin + i * bin_size: c for i, c in xhist.items()}


# def make_counts_curve(time_events, dx=1.0):
#     assert dx > 0.000001
#     if len(time_events) == 0:
#         return []
#     result = [(time_events[0], 1)]
#     for x in time_events[1:]:
#         if abs(x - result[-1][0]) < 0.5 * dx:
#             result[-1] = (result[-1][0], result[-1][1] + 1)
#         else:
#             if abs(x - result[-1][0]) >= 1.5 * dx:
#                 result.append((result[-1][0]+dx, 0))
#                 if x-dx > result[-1][0]+dx:
#                     result.append((x-dx, 0))
#             result.append((x, 1))
#     return result


