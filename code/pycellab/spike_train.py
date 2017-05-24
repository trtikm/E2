import numpy
import distribution


class spike_train:
    """
    It models a sequence of presynaptic spikes (PSPs) reaching to an axon terminal.
    The intervals between individual spikes are distributed in the sequence according
    to a distribution passed to the constructor.
    """
    def __init__(self, noise_distribution, data_signal, start_time=0.0):
        assert isinstance(noise_distribution, distribution.distribution)
        assert isinstance(data_signal, list)
        self._noise_isi = noise_distribution
        self._data_signal = [start_time + t for t in data_signal]
        self._last_spike_time = self.next_spike_time(start_time)
        self._spikes = []

    def get_isi_noise_distribution(self):
        return self._noise_isi

    def next_spike_time(self, t):
        while len(self._data_signal) != 0 and self._data_signal[0] < t:
            del self._data_signal[0]
        t_noise = t + self._noise_isi.next_event()
        assert t_noise >= t
        if len(self._data_signal) == 0 or t_noise < self._data_signal[0]:
            return t_noise
        return self._data_signal.pop(0)

    def get_spikes(self):
        return self._spikes

    def on_time_step(self, t, dt):
        if self._last_spike_time > t + dt:
            return False
        self._spikes.append(self._last_spike_time)
        self._last_spike_time = self.next_spike_time(t + dt)
        return True
