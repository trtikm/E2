import numpy


class distribution:
    def __init__(self, histogram):
        assert isinstance(histogram, dict)
        if not histogram or len(histogram) == 0:
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
            if type(k) not in [int, float]:
                x = range(len(self._histogram.keys()))
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


def mkhist(events, nbins=100):
    assert nbins > 0.00001
    if len(events) == 0:
        return {}
    lo = min(events)
    hi = max(events)
    dx = (hi - lo) / nbins
    if dx < 0.00001:
        return {events[0]: len(events)}
    hist = {}
    for x in events:
        b = int((x - lo) / dx)
        if b in hist:
            hist[b] += 1
        else:
            hist[b] = 1
    return hist


def get_standard_spike_noise():
    s = numpy.random.exponential(1, 100000)
    hist = mkhist(s, 500)
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


def make_isi_histogram(time_events, minimal_time_delta, start_time=0.0):
    assert minimal_time_delta > 0.0
    hist = {}
    t = start_time
    dt = minimal_time_delta
    for event in sorted(time_events):
        if event > t + dt / 2.0:
            delta = event - t
            if delta in hist:
                hist[delta] += 1
            else:
                found = False
                for key in hist.keys():
                    if abs(key - delta) < dt / 2.0:
                        hist[key] += 1
                        found = True
                        break
                if not found:
                    hist[delta] = 1
            while t < event:
                t += dt
    return hist


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


def test():
    def doit(hist, n):
        xhist = hist.copy()
        for k in xhist.keys():
            xhist[k] = 0
        isi = distribution(hist)
        print(isi)
        for _ in range(n):
            e = isi.next_event()
            assert e in hist.keys()
            xhist[e] += 1
        osum = 0.0
        xsum = 0.0
        for k in hist.keys():
            osum += hist[k]
            xsum += xhist[k]
        if xsum > 0:
            for k in xhist.keys():
                xhist[k] *= osum / xsum
        return xhist

    def show(hist, xhist):
        for k in sorted(hist.keys()):
            print(str(k) + ": " + str(hist[k]) + " ; " + str(xhist[k]))

    for hist in [
            {1: 1},
            {123: 10},
            {1: 1, 2: 1, 3: 1, 4: 1, 5: 1},
            {"A": 2, "B": 4, "C": 10, "D": 2, "E": 3},
            {10: 60, 20: 100, 30: 65, 40: 35, 50: 20, 60: 10, 70: 5, 80: 3, 90: 2, 100: 1}
            ]:
        print("*******************************************************")
        show(hist, doit(hist, 10000))
