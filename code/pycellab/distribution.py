import numpy


class disi:
    def __init__(self, histogram):
        assert isinstance(histogram, dict)
        assert len(histogram) > 0
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
        self._bars_line[idx] = sum_bars
        if sum_bars > 0.001:
            for i in range(0, len(self._histogram)):
                self._bars_line[i] /= sum_bars
        self._bars_line[-1] /= sum_bars
        assert len(self._events_line) + 1 == len(self._bars_line)

    def __str__(self):
        return (
            "disi {\n" +
            "  histogram=" + str(self._histogram) + "\n"
            "  bars_line=" + str(self._bars_line) + "\n"
            "  events_line=" + str(self._events_line) + "\n"
            "}\n"
            )

    def get_histogram(self):
        return self._histogram

    def next_event(self):
        sample = numpy.random.uniform(0.0, 1.0)
        idx = self._bars_line.searchsorted(sample)
        if idx != 0:
            idx -= 1
        return self._events_line[idx]


def mk_isi_histogram(sorted_events, dt):
    assert dt > 0.0
    hdict = {}
    t = 0.0
    for event in sorted_events:
        if event > t + dt / 2.0:
            delta = event - t
            if delta in hdict:
                hdict[delta] += 1
            else:
                found = False
                for key in hdict.keys():
                    if abs(key - delta) < dt / 2.0:
                        hdict[key] += 1
                        found = True
                        break
                if not found:
                    hdict[delta] = 1
            while t < event:
                t += dt
    return [(t, n) for t, n in hdict.items()]


def test():
    def doit(hist, n):
        xhist = hist.copy()
        for k in xhist.keys():
            xhist[k] = 0
        isi = disi(hist)
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
        print("*******")
        for k in sorted(hist.keys()):
            print(str(k) + ": " + str(hist[k]) + " ; " + str(xhist[k]))

    for hist in [
            {1: 0},
            {123: 10},
            {1: 1, 2: 1, 3: 1, 4: 1, 5: 1},
            {"A": 2, "B": 4, "C": 2},
            {10: 60, 20: 100, 30: 65, 40: 35, 50: 20, 60: 10, 70: 5, 80: 3, 90: 2, 100: 1}
            ]:
        show(hist, doit(hist, 10000))
