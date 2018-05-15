import numpy
import math
import bisect
import datalgo
import random


class Distribution:
    def __init__(self, histogram, seed=None):
        assert histogram is None or isinstance(histogram, dict)
        if histogram is None or len(histogram) == 0:
            self._histogram = {1e23: 0.0}
        else:
            self._histogram = histogram.copy()
        self._bars_line = numpy.arange(len(self._histogram), dtype=float)
        self._events_line = []
        sum_bars = 0.0
        for idx, event in enumerate(sorted(self._histogram.keys())):
            bar_size = self._histogram[event]
            assert bar_size >= 0.0
            sum_bars += bar_size
            self._bars_line[idx] = sum_bars
            self._events_line.append(event)
        for i in range(0, len(self._histogram)):
            self._bars_line[i] /= (sum_bars + 0.00001)
        self._bars_line[-1] = 1.0
        assert len(self._events_line) == len(self._bars_line)
        self._probabilities = numpy.array([float(self._histogram[k]) for k in self._events_line])
        self._probabilities *= 1.0 / (sum(self._probabilities) + 0.00001)
        self._has_numeric_events = all(isinstance(x, int) or isinstance(x, float) for x in self._events_line)
        numeric_event_line = self._events_line if self._has_numeric_events else range(len(self._events_line))
        self._mean = sum([numeric_event_line[i]*self._probabilities[i] for i in range(len(self._events_line))])
        self._median = self.event_with_probability(0.5)
        self._variance = sum([((numeric_event_line[i] - self._mean)**2) * self._probabilities[i] for i in range(len(self._events_line))])
        self._standard_deviation = numpy.sqrt(self._variance)
        self._coefficient_of_variation = self._standard_deviation / (self._mean if abs(self._mean) > 0.00001 else 0.00001)
        self._rnd_generator = random.Random(seed)

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
    
    def to_json(self):
        return {
            "histogram": self._histogram.copy(),
            "events": self.get_events(),
            "counts": self.get_counts_of_events(),
            "probabilities": self.get_probabilities_of_events(),
            "median": self.get_median(),
            "mean": self.get_mean(),
            "variance": self.get_variance(),
            "standard_deviation": self.get_standard_deviation(),
            "coefficient_of_variation": self.get_coefficient_of_variation()
        }

    @staticmethod
    def from_json(data_in_json):
        """
        Constructs a distribution instance for the data in JSON format as produced from the function 'to_json' above.
        """
        assert isinstance(data_in_json, dict)
        assert "histogram" in data_in_json or ("events" in data_in_json and "probabilities" in data_in_json)
        if "histogram" in data_in_json:
            return Distribution(data_in_json["histogram"])
        else:
            assert len(data_in_json["events"]) == len(data_in_json["probabilities"])
            return Distribution(dict(zip(data_in_json["events"], data_in_json["probabilities"])))

    def copy(self):
        return Distribution(self.get_histogram())

    def has_numeric_events(self):
        return self._has_numeric_events

    def get_histogram(self):
        return self._histogram

    def get_events(self):
        return self._events_line

    def get_event_index(self, event):
        return min(bisect.bisect_left(self.get_events(), event), len(self.get_events()) - 1) if self.has_numeric_events()\
               else self.get_events().index(event)

    def get_probability_of_event(self, event):
        return self._probabilities[self.get_event_index(event)]

    def get_counts_of_events(self):
        return [self._histogram[k] for k in self._events_line]

    def get_probabilities_of_events(self):
        return [self._probabilities[i] for i in range(len(self._events_line))]

    def get_points(self):
        return [(k, self._histogram[k]) for k in self._events_line]

    def get_probability_points(self):
        return [(self._events_line[i], self._probabilities[i]) for i in range(len(self._events_line))]

    def next_event(self):
        return self.event_with_probability(self._rnd_generator.random())

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
        idx = min(bisect.bisect_left(self._bars_line, probability), len(self._bars_line) - 1)
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


def compute_mean_range_lower_bound(disribub, max_percentage=1.0):
    assert isinstance(disribub, Distribution)
    assert isinstance(max_percentage, float) and max_percentage >= 0.0
    max_probability = disribub.get_probability_of_event(disribub.get_mean()) * max_percentage / 100.0
    for i in range(disribub.get_event_index(disribub.get_mean())):
        if disribub.get_probability_of_event(disribub.get_events()[i]) >= max_probability:
            return disribub.get_events()[i]
    return disribub.get_event_index(disribub.get_mean())


def compute_mean_range_upper_bound(disribub, max_percentage=1.0):
    assert isinstance(disribub, Distribution)
    assert isinstance(max_percentage, float) and max_percentage >= 0.0
    max_probability = disribub.get_probability_of_event(disribub.get_mean()) * max_percentage / 100.0
    for i in range(len(disribub.get_events()) - 1, disribub.get_event_index(disribub.get_mean()), -1):
        if disribub.get_probability_of_event(disribub.get_events()[i]) >= max_probability:
            return disribub.get_events()[i]
    return disribub.get_event_index(disribub.get_mean())


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
    raw_curve_points =\
        datalgo.make_points_of_hermit_cubic_spline(
                (0.0, 0.0),
                (mult_m01 * (d_05[2] + k_m0 * (peek_x - d_05[0])), 0.0),
                (peek_x, 1.0),
                (mult_mx * (d_05[1] + k_mx * abs(peek_x - d_05[0])), 0.0),
                int(num_points / 2.0)
                )[:-1] +\
        datalgo.make_points_of_hermit_cubic_spline(
                (peek_x, 1.0),
                (mult_mx * (d_05[1] + k_mx * abs(peek_x - d_05[0])), 0.0),
                (1.0, 0.0),
                (mult_m01 * (d_05[3] + k_m1 * (peek_x - d_05[0])), 0.0),
                num_points - (int(num_points / 2.0) - 1)
                )
    assert len(raw_curve_points) >= 2
    result = [raw_curve_points[0]]
    for i in range(1, len(raw_curve_points) - 1):
        if raw_curve_points[i][0] > result[-1][0] and raw_curve_points[i][0] < raw_curve_points[-1][0]:
            result.append(raw_curve_points[i])
    result.append(raw_curve_points[-1])
    assert all(result[i][0] >= 0.0 and result[i][0] <= 1.0 for i in range(len(result)))
    assert all(result[i-1][0] < result[i][0] for i in range(1, len(result)))
    return result


def hermit_distribution_histogram(
        peek_x=0.5,
        scale_x=0.298,
        scale_y=10,
        pow_y=1.5,
        shift_x=0.002,
        bin_size=0.001,
        mult_m01=1.0,
        mult_mx=1.0
        ):
    return datalgo.make_histogram_from_points(
                [p for p in datalgo.move_scale_curve_points(
                                make_points_of_hermit_cubic_approximation_of_normal_distribution(
                                    peek_x=peek_x,
                                    mult_m01=mult_m01,
                                    mult_mx=mult_mx,
                                    num_points=333
                                    ),
                                scale_x=scale_x,
                                scale_y=scale_y,
                                pow_y=pow_y,
                                shift_x=shift_x
                                ) if abs(p[1]) > 0.00001],
                bin_size,
                0.0
                )


def hermit_distribution(
        peek_x=0.5,
        scale_x=0.298,
        scale_y=10,
        pow_y=1.5,
        shift_x=0.002,
        bin_size=0.001,
        mult_m01=1.0,
        mult_mx=1.0,
        seed=None
        ):
    return Distribution(
                hermit_distribution_histogram(peek_x, scale_x, scale_y, pow_y, shift_x, bin_size, mult_m01, mult_mx),
                seed
                )


_cache_of_hermit_distribution_with_desired_mean = dict()


def hermit_distribution_with_desired_mean(
        mean,
        lo,
        hi,
        max_mean_error,
        scale_y=10,
        pow_y=1.5,
        bin_size=0.001,
        mult_m01=1.0,
        mult_mx=1.0,
        use_cache=True,
        seed=None
        ):
    assert type(mean) in [int, float]
    assert type(lo) in [int, float]
    assert type(hi) in [int, float]
    assert lo <= mean and mean <= hi

    if use_cache:
        cache_key = (
            mean,
            lo,
            hi,
            max_mean_error,
            scale_y,
            pow_y,
            bin_size,
            mult_m01,
            mult_mx
            )
        if cache_key in _cache_of_hermit_distribution_with_desired_mean:
            return Distribution(_cache_of_hermit_distribution_with_desired_mean[cache_key], seed=seed)

    if hi - lo < 0.00001:
        return Distribution({(lo + hi) / 2.0: 1.0})
    peek_mid = (mean - lo) / (hi - lo)
    peek_lo = 0.001
    peek_hi = 0.999
    while peek_hi - peek_lo > 0.00001:
        H = hermit_distribution_histogram(
                peek_x=peek_mid,
                scale_x=hi - lo,
                shift_x=lo,
                scale_y=scale_y,
                pow_y=pow_y,
                bin_size=bin_size,
                mult_m01=mult_m01,
                mult_mx=mult_mx
                )
        D = Distribution(H, seed=seed)
        if D.get_mean() < mean - max_mean_error:
            peek_lo = peek_mid
        elif mean + max_mean_error < D.get_mean():
            peek_hi = peek_mid
        else:
            if use_cache:
                _cache_of_hermit_distribution_with_desired_mean[cache_key] = H
            return D
        peek_mid = (peek_hi + peek_lo) / 2.0
    raise Exception("hermit_distribution_with_desired_mean(mean=" + str(mean) + "): The distribution does not exist.")


def default_excitatory_isi_distribution():
    return hermit_distribution(0.1372, pow_y=2)


def default_inhibitory_isi_distribution():
    return hermit_distribution(0.092, 0.08, bin_size=0.0002, pow_y=2)
