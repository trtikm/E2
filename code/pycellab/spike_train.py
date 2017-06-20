import distribution
import signalling


class spike_train:
    """
    It models a sequence of presynaptic spikes (PSPs) reaching to an axon terminal.
    The intervals between individual spikes are distributed in the sequence according
    to a distribution passed to the constructor.
    """
    def __init__(self, noise_distribution, data_signal, start_time=0.0):
        assert isinstance(noise_distribution, distribution.distribution)
        assert data_signal is None or isinstance(data_signal, signalling.DataSignal)
        self._noise_isi = noise_distribution
        self._data_signal = data_signal
        self._last_noise_spike_time = start_time + self._noise_isi.next_event()
        self._spikes = []
        self._is_data_spike = []
        self.on_time_step(start_time, 0.0)

    def get_isi_noise_distribution(self):
        return self._noise_isi

    def get_data_signal(self):
        return self._data_signal

    def get_spikes(self):
        return self._spikes

    def get_is_data_signal_flags(self):
        return self._is_data_spike

    def on_time_step(self, t, dt):
        is_data_spiking = self._data_signal.on_time_step(t, dt) if self._data_signal is not None else False
        is_noise_spiking = self._last_noise_spike_time <= t + dt
        if not (is_data_spiking or is_noise_spiking):
            return False
        if is_noise_spiking:
            self._last_noise_spike_time = t + dt + self._noise_isi.next_event()
        self._spikes.append(t + dt)
        self._is_data_spike.append(is_data_spiking)
        return True
