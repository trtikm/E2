import bisect
import numpy
import math


def is_number(instance):
    return isinstance(instance, float) or isinstance(instance, int)


def is_point(instance):
    return isinstance(instance, tuple) and len(instance) == 2 and is_number(instance[0]) and is_number(instance[1])


def is_list_of_numbers(numbers):
    return isinstance(numbers, list) and all(is_number(number) for number in numbers)


def is_list_of_events(events):
    return is_list_of_numbers(events)


def is_sorted_list_of_numbers(numbers):
    return is_list_of_events(numbers) and all(numbers[i - 1] <= numbers[i] for i in range(1, len(numbers)))


def is_sorted_list_of_events(events):
    return is_sorted_list_of_numbers(events)


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


def make_weighted_events(events, max_merge_distance, unit_weight=1, weight_function=lambda _: 1, epsilon=0.00001):
    assert is_list_of_events(events)
    assert is_number(max_merge_distance) and max_merge_distance > 0
    assert is_number(unit_weight)

    result = []
    if len(events) != 0:
        result = [(events[0], unit_weight)]
        for event in events[1:]:
            if abs(event - result[-1][0]) <= max_merge_distance - epsilon:
                weight_delta = unit_weight * weight_function(event - result[-1][0] / max_merge_distance)
                result[-1] = (result[-1][0], result[-1][1] + weight_delta)
            else:
                result.append((event, unit_weight))

    assert is_list_of_points(result)
    return result


def make_function_from_events(events, x_coords=None):
    assert is_list_of_events(events)
    assert x_coords is None or (is_list_of_numbers(x_coords) and len(events) == len(x_coords))

    if x_coords is None:
        x_coords = range(len(events))
    result = list(zip(x_coords, events))

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


def make_histogram(events, bin_size, reference_event, count_unit=1):
    assert is_list_of_events(events)
    assert is_number(bin_size) and bin_size > 0
    assert is_number(reference_event)
    assert is_number(count_unit)

    if len(events) == 0:
        return {}
    result = {}
    for event in events:
        sign = 1 if event >= 0 else -1
        bin_idx = int(float(event - reference_event) / float(bin_size) + sign * 0.5)
        key = reference_event + bin_idx * bin_size
        result[key] = count_unit if key not in result else result[key] + count_unit

    assert is_histogram(result)
    return result


def merge_histograms(histograms, bin_size, reference_event, count_unit=1):
    assert isinstance(histograms, list) and all(is_histogram(h) for h in histograms)
    assert is_number(bin_size) and bin_size > 0
    assert is_number(reference_event)
    assert is_number(count_unit)

    result = {}
    for histogram in histograms:
        for event, count in histogram.items():
            sign = 1 if event >= 0 else -1
            bin_idx = int(float(event - reference_event) / float(bin_size) + sign * float(bin_size) / 2.0)
            key = reference_event + bin_idx * bin_size
            result[key] = count_unit if key not in result else result[key] + count_unit * count

    assert is_histogram(result)
    return result


def make_histogram_from_points(points, bin_size=None, reference_x_coord=0):
    assert is_list_of_points(points)
    assert len(points) > 1
    assert bin_size is None or (is_number(bin_size) and bin_size > 0)
    assert is_number(reference_x_coord)

    points = sorted(points, key=lambda p: p[0])
    x = [p[0] for p in points]
    lo = min(x)
    hi = max(x)
    assert hi > lo + 0.0001
    y = [p[1] for p in points]
    if bin_size is None:
        bin_size = (hi - lo) / 1000
        assert bin_size > 0.00001
    start_bin_idx = int(float(lo - reference_x_coord) / float(bin_size))
    end_bin_idx = int(float(hi - reference_x_coord) / float(bin_size))
    result = {}
    for bin_idx in range(start_bin_idx, end_bin_idx + 1):
        t = reference_x_coord + bin_idx * bin_size
        j = bisect.bisect_left(x, t)
        if j >= len(x):
            result[t] = y[-1]
        if j == 0 or x[j] - x[j - 1] < 0.00001:
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

    result = []
    for events in list_of_lists_of_events:
        for event in events:
            bisect.insort_right(result, event)

    assert is_sorted_list_of_events(result)
    assert len(result) == sum(len(events) for events in list_of_lists_of_events)
    return result


def merge_close_points(points, merge_function, dx=1.0):
    assert is_sorted_list_of_points_along_x_axis(points)
    assert dx > 0.000001
    assert callable(merge_function)

    result = []
    if len(points) > 0:
        result.append(points[0])
        for x, fx in points[1:]:
            if x < result[-1][0] + dx:
                result[-1] = (result[-1][0], merge_function(result[-1][1], fx))
            else:
                result.append((x, fx))

    assert is_sorted_list_of_points_along_x_axis(result)
    return result


def merge_close_points_by_add(points, dx=1.0):
    return merge_close_points(points, float.__add__, dx)


def merge_close_points_by_min(points, dx=1.0):
    return merge_close_points(points, min, dx)


def merge_close_points_by_max(points, dx=1.0):
    return merge_close_points(points, max, dx)


def compose_sorted_lists_of_points(list_of_lists_of_points, multipliers=None, epsilon=0.00001):
    assert isinstance(list_of_lists_of_points, list)
    assert all(is_sorted_list_of_points_along_x_axis(points) for points in list_of_lists_of_points)
    assert multipliers is None or (is_list_of_numbers(multipliers) and len(list_of_lists_of_points) == len(multipliers))

    def find_insertion_index_for_point(points, point, epsilon):
        lo = 0
        hi = len(points)
        while lo < hi:
            mid = (lo+hi) // 2
            if point[0] < points[mid][0] - epsilon:
                hi = mid
            elif point[0] > points[mid][0] + epsilon:
                lo = mid + 1
            else:
                return mid, True
        return lo, False

    result = []
    for list_idx, points in enumerate(list_of_lists_of_points):
        mult = 1 if multipliers is None else multipliers[list_idx]
        for point in points:
            idx, compose = find_insertion_index_for_point(result, point, epsilon)
            if compose is True:
                result[idx] = (result[idx][0], result[idx][1] + mult * point[1])
            else:
                result.insert(idx, (point[0], mult * point[1]))

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


def evaluate_discrete_function_using_liner_interpolation(x_values, discrete_function, value_outside_fn_domain=0):
    assert is_sorted_list_of_numbers(x_values)
    assert is_sorted_list_of_points_along_x_axis(discrete_function)
    assert is_number(value_outside_fn_domain)
    idx = 0
    result = []
    for x in x_values:
        lo = (x, value_outside_fn_domain)
        hi = (x, value_outside_fn_domain)
        while idx < len(discrete_function) and discrete_function[idx][0] <= x:
            lo = discrete_function[idx]
            idx += 1
        if idx < len(discrete_function):
            hi = discrete_function[idx]
        elif idx > 0:
            idx = len(discrete_function) - 1
        if hi[0] - lo[0] < 0.000001:
            y = lo[1]
        else:
            y = lo[1] + ((x - lo[0]) / hi[0] - lo[0]) * (hi[1] - lo[1])
        result.append((x, y))
    return result


def transform_discrete_function_to_inteval_0_1_using_liner_interpolation(points, y_zero, y_one):
    assert is_list_of_points(points)
    assert is_number(y_zero)
    assert is_number(y_one)
    assert y_one - y_zero > 0.00001
    result = []
    for x, y in points:
        result.append((x, min(1, max(0, (y - y_zero) / (y_one - y_zero)))))
    return result


class VoltageEffectRegion:
    def __init__(self):
        # self._origin = [12800.0, 3200.0, 3.654]
        # self._normal = numpy.cross(numpy.subtract([25600.-0, 6400.0, 7.251], self._origin),
        #                            numpy.subtract([11360.0, 4640.0, -105.457], self._origin))
        # magnitude = math.sqrt(numpy.dot(self._normal, self._normal))
        # self._normal = [c/magnitude for c in self._normal]
        self._origin = [0.0, 0.0, 0.0]
        self._normal = [-0.0153493403596, 0.0602754753955, 0.998063757891]

        self._low_origin = [12800.0, 3200.0, -63.0]
        self._low_normal = numpy.cross(numpy.subtract([25600.-0, 6400.0, -84.0], self._low_origin),
                                       numpy.subtract([11360.0, 4640.0, -171.0], self._low_origin))
        magnitude = math.sqrt(numpy.dot(self._low_normal, self._low_normal))
        self._low_normal = [c/magnitude for c in self._low_normal]

        self._high_origin = [12800.0, 3200.0, 68.0]
        self._high_normal = numpy.cross(numpy.subtract([25600.-0, 6400.0, 116.0], self._high_origin),
                                        numpy.subtract([11360.0, 4640.0, -37.0], self._high_origin))
        magnitude = math.sqrt(numpy.dot(self._high_normal, self._high_normal))
        self._high_normal = [c/magnitude for c in self._high_normal]

    def get_normal(self):
        return self._normal

    def get_origin(self):
        return self._origin

    def get_low_normal(self):
        return self._low_normal

    def get_low_origin(self):
        return self._low_origin

    def get_high_normal(self):
        return self._high_normal

    def get_high_origin(self):
        return self._high_origin

    @staticmethod
    def _get_voltage(origin, normal, num_excitatory_trains, num_inhibitory_trains):
        assert isinstance(num_excitatory_trains, int) and num_excitatory_trains >= 0
        assert isinstance(num_inhibitory_trains, int) and num_inhibitory_trains >= 0
        pos = [float(num_excitatory_trains), float(num_inhibitory_trains), 0.0]
        return (numpy.dot(normal, origin) - numpy.dot(normal, pos)) / normal[2]

    def get_median_voltage(self, num_excitatory_trains, num_inhibitory_trains):
        return self._get_voltage(self.get_origin(), self.get_normal(), num_excitatory_trains, num_inhibitory_trains)

    def get_low_voltage(self, num_excitatory_trains, num_inhibitory_trains):
        return self._get_voltage(self.get_low_origin(), self.get_low_normal(), num_excitatory_trains, num_inhibitory_trains)

    def get_high_voltage(self, num_excitatory_trains, num_inhibitory_trains):
        return self._get_voltage(self.get_high_origin(), self.get_high_normal(), num_excitatory_trains, num_inhibitory_trains)


def approximate_discrete_function(points, num_final_points=None):
    assert is_sorted_list_of_points_along_x_axis(points)
    assert num_final_points is None or (isinstance(num_final_points, int) and num_final_points > 1 and len(points) > num_final_points)
    if len(points) < 2:
        return points.copy()
    if num_final_points is None:
        num_final_points = max(2, len(points) // 10)
    x = [p[0] for p in points]
    min_x = min(x)
    max_x = max(x)
    dx = (max_x - min_x) / float(num_final_points - 1)
    di = (max_x - min_x) / float(len(points))
    assert dx - di > 1e-4
    min_cluster_size = 1 if di < 1e-4 else max(1, int(dx/di))

    def append_approximate_point(current_x, idx, output):
        j = bisect.bisect_left(x, current_x + dx, idx)
        cluster = [p[1] for p in points[idx:j]]
        output.append((current_x, 0.0 if len(cluster) == 0 else sum(cluster) / max(min_cluster_size, len(cluster))))
        return j

    result = []
    append_approximate_point(min_x, 0, result)
    i = 0
    curr_x = min_x + dx
    while i < len(points) and curr_x <= max_x - dx*0.75:
        end = append_approximate_point(curr_x, i, result)
        i = bisect.bisect_left(x, curr_x, i, end)
        curr_x += dx
    append_approximate_point(max_x, bisect.bisect_left(x, max_x - dx), result)
    return result


def interpolate_discrete_function(points, num_segment_inner_points=None):
    assert is_sorted_list_of_points_along_x_axis(points)
    assert num_segment_inner_points is None or (isinstance(num_segment_inner_points, int) and num_segment_inner_points >= 0)
    if len(points) < 2:
        return points.copy()
    if num_segment_inner_points is None:
        num_segment_inner_points = 5
    tangents = [(0.0, 0.0)]
    for i in range(1, len(points) - 1):
        tangent = 0.5 * numpy.subtract(points[i + 1], points[i - 1])
        tangents.append((tangent[0], tangent[1]))
    tangents.append((0.0, 0.0))
    assert len(points) == len(tangents)
    result = []
    for i in range(len(points) - 1):
        result += make_points_of_hermit_cubic_spline(points[i], tangents[i], points[i + 1], tangents[i + 1], 2 + num_segment_inner_points)
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


