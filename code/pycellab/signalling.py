import bisect
import numpy
import distribution


class DataSignal:
    def __init__(self, phase_distribution, spiking_distributions, selector_distribution, start_time):
        assert isinstance(phase_distribution, distribution.distribution)
        assert isinstance(spiking_distributions, list)
        assert len(spiking_distributions) > 0
        assert all(isinstance(dist, distribution.distribution) for dist in spiking_distributions)
        assert isinstance(selector_distribution, distribution.distribution)
        assert selector_distribution.get_events() == [i for i in range(len(spiking_distributions))]
        assert isinstance(start_time, float)
        self._phase_distribution = phase_distribution
        self._spiking_distributions = spiking_distributions
        self._selector_distribution = selector_distribution
        self._phase_end_time = start_time + self._phase_distribution.next_event()
        self._spiking_index = self._selector_distribution.next_event()
        self._last_spike_time = start_time + self._spiking_distributions[self._spiking_index].next_event()

    def on_time_step(self, t, dt):
        if self._phase_end_time <= t + dt:
            self._phase_end_time = t + dt + self._phase_distribution.next_event()
            self._spiking_index = self._selector_distribution.next_event()
            self._last_spike_time = t + dt + self._spiking_distributions[self._spiking_index].next_event()
            return False
        elif self._last_spike_time <= t + dt:
            self._last_spike_time = t + dt + self._spiking_distributions[self._spiking_index].next_event()
            return True
        else:
            return False

    @staticmethod
    def constant_isi(inter_spikes_interval, start_time=0.0):
        assert isinstance(inter_spikes_interval, float) and inter_spikes_interval > 0.00001
        return DataSignal(
            phase_distribution=distribution.distribution(None),
            spiking_distributions=[distribution.distribution({inter_spikes_interval: 1})],
            selector_distribution=distribution.distribution({0: 1}),
            start_time=start_time
        )

    @staticmethod
    def default_excitatory(start_time=0.0):
        return DataSignal(
            phase_distribution=distribution.distribution(distribution.mkhist_by_linear_interpolation([
                # (ISI, count)
                (0.010, 600),
                (0.020, 800),
                (0.030, 600),
                (0.040, 400),
                (0.050, 300),
                (0.060, 200),
                (0.070, 100),
                (0.080, 50),
                (0.090, 25),
                (0.100, 10),
                ], 0.001)),
            spiking_distributions=[
                distribution.distribution(distribution.mkhist_by_linear_interpolation([
                    # (ISI, count)
                    (0.005, 25),
                    (0.020, 100),
                    (0.050, 10),
                    ], 0.001)),
                # distribution.distribution(distribution.mkhist_by_linear_interpolation([
                #     # (ISI, count)
                #     (0.010, 100),
                #     (0.050, 10),
                #     (0.100, 0),
                #     ], 0.001)),
                ],
            selector_distribution=distribution.distribution(dict([(i, 1.0) for i in range(1)])),
            start_time=start_time
        )


    @staticmethod
    def default_inhibitory(start_time=0.0):
        return DataSignal(
            phase_distribution=distribution.distribution(distribution.mkhist_by_linear_interpolation([
                # (ISI, count)
                (0.010, 600),
                (0.020, 800),
                (0.030, 600),
                (0.040, 400),
                (0.050, 300),
                (0.060, 200),
                (0.070, 100),
                (0.080, 50),
                (0.090, 25),
                (0.100, 10),
                ], 0.001)),
            spiking_distributions=[
                distribution.distribution(distribution.mkhist_by_linear_interpolation([
                    # (ISI, count)
                    (0.005, 10),
                    (0.010, 100),
                    ], 0.001)),
                distribution.distribution(distribution.mkhist_by_linear_interpolation([
                    # (ISI, count)
                    (0.010, 100),
                    (0.050, 10),
                    (0.100, 0),
                    ], 0.001)),
                ],
            selector_distribution=distribution.distribution(dict([(i, 1.0) for i in range(2)])),
            start_time=start_time
        )


class ISIDistributionMatrix:
    def __init__(self, events_distribution, distances_distribution):
        assert isinstance(events_distribution, distribution.distribution)
        assert isinstance(distances_distribution, distribution.distribution)
        self._events_distribution = events_distribution
        self._distances_distribution = distances_distribution
        self._distributions_matrix = {}
        for event0 in events_distribution.get_events():
            hist = {}
            for event1 in events_distribution.get_events():
                probability = events_distribution.get_probability_of_event(event1)
                distance = distances_distribution.get_probability_of_event(event1 - event0)
                # hist[event1] = max(0.0, probability * distance)
                # hist[event1] = max(0.0, probability + distance)
                hist[event1] = max(0.0, probability * (1.0 + 10000*distance))
            self._distributions_matrix[event0] = distribution.distribution(hist)

    def get_events_distribution(self):
        return self._events_distribution

    def get_distances_distribution(self):
        return self._distances_distribution

    def get_distributions_matrix(self):
        return self._distributions_matrix

    def get_distribution_of_event(self, event):
        assert event in self.get_distributions_matrix()
        return self.get_distributions_matrix()[event]

    @staticmethod
    def create(frequency_coef, distance_correlation_coef):
        assert isinstance(frequency_coef, float) and frequency_coef >= 0.0 and frequency_coef <= 1.0
        assert isinstance(distance_correlation_coef, float) and distance_correlation_coef >= 0.0 and distance_correlation_coef <= 1.0
        events_distribution = distribution.hermit_distribution(0.137, pow_y=2)
        distances_distribution = distribution.hermit_distribution(0.5, pow_y=1.5, scale_x=0.2, shift_x=-0.2 / 2.0)
        return ISIDistributionMatrix(events_distribution, distances_distribution)


class DataSignalEx:
    def __init__(self, distribution_matrix, start_time=0.0):
        assert isinstance(distribution_matrix, ISIDistributionMatrix)
        self._distribution_matrix = distribution_matrix
        self._event_construction_time = start_time
        self._last_event = self._distribution_matrix.get_events_distribution().next_event()

    def on_time_step(self, t, dt):
        if self._event_construction_time + self._last_event > t + dt:
            return False
        self._event_construction_time = t
        self._last_event = self._distribution_matrix.get_distribution_of_event(self._last_event).next_event()
        return True

    def get_distribution_matrix(self):
        return self._distribution_matrix


class DataSignalFX:
    def __init__(self, spiking_distribution, chunk_size_distribution, array_size):
        assert isinstance(spiking_distribution, distribution.distribution)
        assert isinstance(chunk_size_distribution, distribution.distribution)
        assert all(isinstance(event, int) and event > 0 for event in chunk_size_distribution.get_events())
        assert isinstance(array_size, int) and array_size >= chunk_size_distribution.get_events()[-1]
        self._spiking_distribution = spiking_distribution
        self._chunk_size_distribution = chunk_size_distribution
        self._array_size = array_size
        self._array = []
        self._spikes_buffer = []

    @staticmethod
    def create_plain_distribution(spiking_distribution):
        return DataSignalFX(spiking_distribution, distribution.distribution({1: 1.0}), 1)

    @staticmethod
    def create_excitatory(array_size=150, min_chunk_size=5, max_chunk_size=25,
                          chunk_frequency_function=lambda size: 1.0):
        # print("DataSignalFX.create_excitatory(\n"
        #       "    array_size=" + str(array_size) + "\n"
        #       "    min_chunk_size=" + str(min_chunk_size) + "\n"
        #       "    max_chunk_size=" + str(max_chunk_size) + "\n"
        #       ")\n")
        assert min_chunk_size > 0 and min_chunk_size <= max_chunk_size and max_chunk_size <= array_size
        return DataSignalFX(distribution.default_excitatory_isi_distribution(),
                            distribution.distribution({size: chunk_frequency_function(size)
                                                       for size in range(min_chunk_size, max_chunk_size + 1)}),
                            array_size)

    @staticmethod
    def create_excitatory_by_param(param):
        assert param >= 0.0 and param <= 1.0
        return DataSignalFX.create_excitatory(
                        array_size=1 + int(149.0 * param**2.0 + 0.5),
                        min_chunk_size=1 + int(4.0 * param**3.0 + 0.5),
                        max_chunk_size=1 + int(24.0 * param**2.0 + 0.5),
                        chunk_frequency_function=lambda size: 1.0
                        )

    @staticmethod
    def create_inhibitory(array_size=150, min_chunk_size=5, max_chunk_size=25,
                          chunk_frequency_function=lambda size: 1.0):
        # print("DataSignalFX.create_inhibitory(\n"
        #       "    array_size=" + str(array_size) + "\n"
        #       "    min_chunk_size=" + str(min_chunk_size) + "\n"
        #       "    max_chunk_size=" + str(max_chunk_size) + "\n"
        #       ")\n")
        assert min_chunk_size > 0 and min_chunk_size <= max_chunk_size and max_chunk_size <= array_size
        return DataSignalFX(distribution.default_inhibitory_isi_distribution(),
                            distribution.distribution({size: chunk_frequency_function(size)
                                                       for size in range(min_chunk_size, max_chunk_size + 1)}),
                            array_size)

    @staticmethod
    def create_inhibitory_by_param(param):
        assert param >= 0.0 and param <= 1.0
        return DataSignalFX.create_inhibitory(
                        array_size=1 + int(149.0 * param**2.0 + 0.5),
                        min_chunk_size=1 + int(4.0 * param**3.0 + 0.5),
                        max_chunk_size=1 + int(24.0 * param**2.0 + 0.5),
                        chunk_frequency_function=lambda size: 1.0
                        )

    def get_spiking_distribution(self):
        return self._spiking_distribution

    def get_chunk_size_distribution(self):
        return self._chunk_size_distribution

    def get_array_size(self):
        return self._array_size

    def _recharge_array(self):
        while len(self._array) < self.get_array_size():
            event = self.get_spiking_distribution().next_event()
            self._array.insert(bisect.bisect_left(self._array, event), event)

    def _recharge_spikes_buffer(self, t):
        assert len(self._array) == self.get_array_size()
        assert len(self._spikes_buffer) == 0
        start, size = self._get_next_chunk()
        self._copy_chunk_to_buffer(start, size)
        self._random_shuffle_buffer()
        self._concretise_spike_times_in_buffer(t)
        self._remove_chunk_from_array(start, size)

    def _get_next_chunk(self):
        start = int(self.get_array_size() * numpy.random.uniform(0.0, 1.0))
        assert start in range(self.get_array_size())
        size = self.get_chunk_size_distribution().next_event()
        assert size in range(1, self.get_array_size() + 1)
        return start, size

    def _copy_chunk_to_buffer(self, start, size):
        self._spikes_buffer = [self._array[(start + i) % self.get_array_size()] for i in range(size)]
        assert len(self._spikes_buffer) > 0

    def _random_shuffle_buffer(self):
        for _ in range(len(self._spikes_buffer)):
            i = int(len(self._spikes_buffer) * numpy.random.uniform(0.0, 1.0))
            j = int(len(self._spikes_buffer) * numpy.random.uniform(0.0, 1.0))
            tmp = self._spikes_buffer[i]
            self._spikes_buffer[i] = self._spikes_buffer[j]
            self._spikes_buffer[j] = tmp

    def _concretise_spike_times_in_buffer(self, start_time):
        assert len(self._spikes_buffer) > 0
        self._spikes_buffer[0] += start_time
        for i in range(1, len(self._spikes_buffer)):
            self._spikes_buffer[i] += self._spikes_buffer[i - 1]
        self._spikes_buffer.reverse()

    def _remove_chunk_from_array(self, start, size):
        end = (start + size) % self.get_array_size()
        if end >= start:
            for _ in range(size):
                self._array.pop(start)
        else:
            while len(self._array) >= start:
                self._array.pop()
            for _ in range(end):
                self._array.pop(0)

    def on_time_step(self, t, dt):
        if len(self._spikes_buffer) == 0:
            self._recharge_array()
            self._recharge_spikes_buffer(t)
            assert len(self._spikes_buffer) > 0
        if self._spikes_buffer[-1] > t + dt:
            return False
        self._spikes_buffer.pop()
        return True
