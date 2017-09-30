import bisect
import numpy
import distribution


class SpikeTrain:
    """
    The class generates a sequence of spikes (action potentials). Time intervals between individual spikes
    are distributed in the generated sequence according to a given distribution. The class provides several
    levels of noise of time intervals in the sequence. Nevertheless, the given distribution of the time
    intervals is always preserved for any level of noise you choose.
    """

    def __init__(self, spiking_distribution, chunk_size_distribution, array_size):
        assert isinstance(spiking_distribution, distribution.Distribution)
        assert isinstance(chunk_size_distribution, distribution.Distribution)
        assert all(isinstance(event, int) and event > 0 for event in chunk_size_distribution.get_events())
        assert isinstance(array_size, int) and array_size >= chunk_size_distribution.get_events()[-1]
        self._spiking_distribution = spiking_distribution
        self._chunk_size_distribution = chunk_size_distribution
        self._array_size = array_size
        self._array = []
        self._spikes_buffer = []
        self._spikes_history = []

    @staticmethod
    def create(base_spiking_distribution, noise_level):
        assert isinstance(base_spiking_distribution, distribution.Distribution)
        assert isinstance(noise_level, float) and noise_level >= 0.0 and noise_level <= 1.0
        array_size = 1 + int(149.0 * (1.0 - noise_level)**2.0 + 0.5)
        min_chunk_size = 1 + int(4.0 * (1.0 - noise_level)**3.0 + 0.5)
        max_chunk_size = 1 + int(24.0 * (1.0 - noise_level)**2.0 + 0.5)
        chunk_frequency_function = lambda size: 1.0
        return SpikeTrain(base_spiking_distribution,
                          distribution.Distribution({
                              size: chunk_frequency_function(size) for size in range(min_chunk_size, max_chunk_size + 1)
                              }),
                          array_size)

    def get_spikes_history(self):
        return self._spikes_history

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
        self._spikes_history.append(t + dt)
        return True
