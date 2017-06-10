import numpy
import math
import bisect


class distribution:
    def __init__(self, histogram):
        assert isinstance(histogram, dict)
        if histogram is None or len(histogram) == 0:
            self._histogram = {1e23: 0.0}
        else:
            self._histogram = histogram.copy()
        self._bars_line = numpy.arange(len(self._histogram) + 1, dtype=float)
        self._events_line = []
        idx = 0
        sum_bars = 0.0
        for event in sorted(self._histogram.keys()):
            self._bars_line[idx] = sum_bars
            self._events_line.append(event)
            bar_size = self._histogram[event]
            assert bar_size >= 0.0
            sum_bars += bar_size
            idx += 1
        for i in range(0, len(self._histogram)):
            self._bars_line[i] /= (sum_bars + 0.00001)
        self._bars_line[-1] = 1.0
        assert len(self._events_line) + 1 == len(self._bars_line)
        x = sorted(self._histogram.keys())
        for k in self._histogram.keys():
            if not (isinstance(k, int) or isinstance(k, float)):
                x = [i for i in range(len(x))]
                break
        self._probabilities = numpy.array([float(self._histogram[k]) for k in sorted(self._histogram.keys())])
        self._probabilities *= 1.0 / (sum(self._probabilities) + 0.00001)
        assert len(x) == len(self._probabilities)
        self._mean = sum([x[i]*self._probabilities[i] for i in range(len(x))])
        self._median = self.event_with_probability(0.5)
        self._variance = sum([((x[i] - self._mean)**2) * self._probabilities[i] for i in range(len(x))])
        self._standard_deviation = numpy.sqrt(self._variance)
        self._coefficient_of_variation = self._standard_deviation / (self._mean + 0.00001)

    def __str__(self):
        return (
            "distribution {\n" +
            "  histogram=" + str(self._histogram) + "\n"
            "  bars_line=" + str(self._bars_line) + "\n"
            "  events_line=" + str(self._events_line) + "\n"
            "  median=" + str(self.get_median()) + "\n"
            "  mean=" + str(self.get_mean()) + "\n"
            "  variance=" + str(self.get_variance()) + "\n"
            "  standard deviation=" + str(self.get_standard_deviation()) + "\n"
            "  coefficient of variation=" + str(self.get_coefficient_of_variation()) + "\n"
            "}"
            )

    def get_histogram(self):
        return self._histogram

    def get_events(self):
        return self._events_line

    def get_counts_of_events(self):
        return [self._histogram[k] for k in self._events_line]

    def get_probabilities_of_events(self):
        return [self._probabilities[i] for i in range(len(self._events_line))]

    def get_points(self):
        return [(k, self._histogram[k]) for k in self._events_line]

    def get_probability_points(self):
        return [(self._events_line[i], self._probabilities[i]) for i in range(len(self._events_line))]

    def next_event(self):
        return self.event_with_probability(numpy.random.uniform(0.0, 1.0))

    def generate(self, n):
        assert type(n) == int and n >= 0
        return [self.next_event() for _ in range(n)]

    def generate_incremental(self, n):
        assert type(n) == int and n >= 0
        if n == 0:
            return []
        result = [self.next_event()]
        for _ in range(n - 1):
            result.append(result[-1] + self.next_event())
        return result

    def event_with_probability(self, probability):
        assert probability >= 0.0 and probability <= 1.0
        idx = self._bars_line.searchsorted(probability)
        if idx != 0:
            idx -= 1
        return self._events_line[idx]

    def get_mean(self):
        return self._mean

    def get_median(self):
        return self._median

    def get_variance(self):
        return self._variance

    def get_standard_deviation(self):
        return self._standard_deviation

    def get_coefficient_of_variation(self):
        return self._coefficient_of_variation


def mkhist(events, nbins=100, lo=None, hi=None):
    assert type(nbins) == int and nbins > 0
    if len(events) == 0:
        return {}, 1.0
    if lo is None:
        lo = min(events)
    if hi is None:
        hi = max(events)
    dx = (hi - lo) / nbins
    if dx < 0.00001:
        return {events[0]: len(events)}, 1.0
    hist = {}
    for x in events:
        b = int((x - lo) / dx)
        if b in hist:
            hist[b] += 1
        else:
            hist[b] = 1
    return hist, dx


def get_standard_spike_noise():
    s = numpy.random.exponential(1, 10000)
    hist, _ = mkhist(s, 500)
    xhist = {}
    keys = sorted(hist.keys())
    for idx in range(0, min(300, len(keys))):
        if idx < 5:
            value = 0.0
        elif idx < 10:
            x = (idx - 5.0) / 10.0
            assert x >= 0.0 and x <= 1.0
            value = hist[keys[10]] * (3.0 * x**2 - 2.0 * x**3)
        else:
            value = hist[keys[idx]]
        xhist[0.001 * (idx + 2)] = value
        idx += 1
    return distribution(xhist)


def make_isi_histogram(time_events, nbins, start_time, end_time):
    isi = [time_events[i] - time_events[i - 1] for i in range(1, len(time_events))]
    raw_hist, dt = mkhist(isi, nbins, start_time, end_time)
    return dict([(bin_index * dt, count) for bin_index, count in raw_hist.items()])


def make_counts_histogram(time_events, start_bin=0, bin_size=1):
    assert bin_size > 0.000001
    xhist = {}
    for event in time_events:
        idx = int((event - start_bin) / bin_size)
        if idx in xhist:
            xhist[idx] += 1
        else:
            xhist[idx] = 1
    return {i * bin_size: c for i, c in xhist.items()}


def make_counts_curve(time_events, dx=1.0):
    assert dx > 0.000001
    if len(time_events) == 0:
        return []
    result = [(time_events[0], 1)]
    for x in time_events[1:]:
        if abs(x - result[-1][0]) < 0.5 * dx:
            result[-1] = (result[-1][0], result[-1][1] + 1)
        else:
            if abs(x - result[-1][0]) >= 1.5 * dx:
                result.append((result[-1][0]+dx, 0))
                if x-dx > result[-1][0]+dx:
                    result.append((x-dx, 0))
            result.append((x, 1))
    return result


def make_fx_points(pairs, function, dx=1.0):
    assert dx > 0.000001
    assert callable(function)
    if len(pairs) == 0:
        return []
    result = [pairs[0]]
    for x, fx in pairs[1:]:
        if abs(x - result[-1][0]) < 0.5 * dx:
            result[-1] = (result[-1][0], function(result[-1][1], fx))
        else:
            result.append((x, fx))
    return result


def make_sum_points(pairs, dx=1.0):
    return make_fx_points(pairs, float.__add__, dx)


def make_min_points(pairs, dx=1.0):
    return make_fx_points(pairs, min, dx)


def make_max_points(pairs, dx=1.0):
    return make_fx_points(pairs, max, dx)


def make_points_of_normal_distribution(
        num_points=200,
        nu=0.0,
        sigma=1.0,
        epsilon=None,
        lo=None,
        hi=None,
        normalise=True
        ):
    assert num_points > 0
    a = 1.0 / (sigma * math.sqrt(2.0 * math.pi))
    if epsilon is None:
        epsilon = a / (2.0 * num_points)
    zero_x = math.sqrt(abs(math.log(epsilon * sigma * math.sqrt(2.0 * math.pi)) * 2 * sigma**2)) + nu
    assert zero_x >= nu
    # f_zero_x = a * math.e**((-(zero_x - nu)**2) / (2.0 * sigma**2))
    # print("zero_x = " + str(zero_x))
    # print("f_zero_x = " + str(f_zero_x))
    # print("f_max = " + str(a))
    if lo is None:
        lo = nu - (zero_x - nu)
    if hi is None:
        hi = nu + (zero_x - nu)
    assert hi - lo > 0.001
    dx = (hi - lo) / num_points
    points = [(x, a * math.e**((-(x - nu)**2) / (2.0 * sigma**2))) for x in numpy.arange(lo, hi, dx, float)]
    if normalise:
        points = [((p[0] - lo) / (hi - lo), p[1] / a) for p in points]
    points = sorted(points, key=lambda p: p[0])
    points[0] = (points[0][0], 0.0)
    points[-1] = (points[-1][0], 0.0)
    return points


def points_of_hermit_cubic_spline(p0, m0, p1, m1, num_points=100):
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

    assert num_points > 1
    return [point(i / float(num_points - 1)) for i in range(num_points)]


def make_points_of_hermit_cubic_approximation_of_normal_distribution(
        peek_x=0.5,
        mult_m01=1.0,
        mult_mx=1.0,
        num_points=200
        ):
    assert peek_x > 0.001 and peek_x < 0.999
    assert mult_m01 > 0.001
    assert mult_mx > 0.001
    assert num_points > 0
    k_m0 = (1.1 - 0.1) / (0.5 - 0.1)
    k_m1 = -k_m0
    k_mx = -(0.4 - 0.2) / (0.5 - 0.1)
    d_05 = (0.5, 0.4, 1.1, 1.1)
    return points_of_hermit_cubic_spline(
                (0.0, 0.0),
                (mult_m01 * (d_05[2] + k_m0 * (peek_x - d_05[0])), 0.0),
                (peek_x, 1.0),
                (mult_mx * (d_05[1] + k_mx * abs(peek_x - d_05[0])), 0.0),
                int(num_points / 2.0)
                )[:-1] +\
           points_of_hermit_cubic_spline(
                (peek_x, 1.0),
                (mult_mx * (d_05[1] + k_mx * abs(peek_x - d_05[0])), 0.0),
                (1.0, 0.0),
                (mult_m01 * (d_05[3] + k_m1 * (peek_x - d_05[0])), 0.0),
                num_points - (int(num_points / 2.0) - 1)
                )


def move_scale_curve_points(
        points,
        scale_x=1.0,
        scale_y=1.0,
        pow_y=1.0,
        shift_x=0.0
        ):
    assert scale_x > 0.001
    assert scale_y > 0.001
    return [(scale_x * p[0] + shift_x, (scale_y * p[1])**pow_y) for p in points]


def mkhist_from_curve_points(points, num_bars=100):
    assert len(points) > 1
    assert num_bars > 0
    points = sorted(points, key=lambda p: p[0])
    x = [p[0] for p in points]
    lo = min(x)
    hi = max(x)
    assert hi > lo + 0.0001
    y = [p[1] for p in points]
    hist = {}
    for i in range(num_bars):
        t = i / float(num_bars - 1)
        t = lo + t * (hi - lo)
        assert t not in hist
        j = bisect.bisect_left(x, t)
        assert j < len(x)
        if j == 0 or x[j] - x[j - 1] < 0.0001:
            hist[t] = y[j]
        else:
            assert x[j - 1] < t
            hist[t] = y[j - 1] + (t - x[j - 1]) * ((y[j] - y[j - 1]) / (x[j] - x[j - 1]))
            assert hist[t] >= 0.0
    return hist


def hermit_distribution(
        peek_x=0.5,
        scale_x=0.298,
        scale_y=10,
        pow_y=1.5,
        shift_x=0.002,
        num_bars=300,
        mult_m01=1.0,
        mult_mx=1.0
        ):
    return __package__.distribution(
                mkhist_from_curve_points(
                    move_scale_curve_points(
                        make_points_of_hermit_cubic_approximation_of_normal_distribution(
                            peek_x=peek_x,
                            mult_m01=mult_m01,
                            mult_mx=mult_mx,
                            num_points=max(2, int(num_bars / 3))
                            ),
                        scale_x=scale_x,
                        scale_y=scale_y,
                        pow_y=pow_y,
                        shift_x=shift_x
                        ),
                    num_bars=num_bars
                    )
                )
