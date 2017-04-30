import distribution


class spike_train:
    """
    It models a sequence of presynaptic spikes (PSPs) reaching to an axon terminal.
    The intervals between individual spikes are distributed in the sequence according
    to a distribution passed to the constructor.
    """
    def __init__(self, noise_distribution, is_excitatory):
        assert type(is_excitatory) is bool
        self._noise_isi = distribution.disi(noise_distribution)
        self._excitatory = is_excitatory

    def copy(self):
        return spike_train(self._noise_isi.get_histogram(), self.is_excitatory())

    def next_spike_time(self):
        return self._noise_isi.next_event()

    def is_excitatory(self):
        return self._excitatory
