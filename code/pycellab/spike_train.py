import numpy
import distribution


class spike_train:
    """
    It models a sequence of presynaptic spikes (PSPs) reaching to an axon terminal.
    The intervals between individual spikes are distributed in the sequence according
    to a distribution passed to the constructor.
    """
    def __init__(self, noise_distribution, is_excitatory, start_time=0.0):
        assert isinstance(noise_distribution, distribution.distribution)
        assert type(is_excitatory) is bool
        self._noise_isi = noise_distribution
        self._excitatory = is_excitatory
        self._last_spike_time = start_time + self.next_spike_time()
        self._spikes = []

    def get_isi_noise_distribution(self):
        return self._noise_isi

    def copy(self):
        return spike_train(self._noise_isi.get_histogram(), self.is_excitatory())

    def next_spike_time(self):
        return self._noise_isi.next_event()

    def is_excitatory(self):
        return self._excitatory

    def get_spikes(self):
        return self._spikes

    def on_time_step(self, t, dt):
        if self._last_spike_time > t + dt:
            return False
        self._spikes.append(t + dt)
        self._last_spike_time = t + dt + self.next_spike_time()
        return True
