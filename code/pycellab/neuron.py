import numpy
import spike_train
import soma


class neuron:
    def __init__(self,
                 cell_soma,
                 excitatory_noise_distributions,
                 inhibitory_noise_distributions,
                 excitatory_weights,
                 inhibitory_weights
                 ):
        assert len(excitatory_noise_distributions) == len(excitatory_weights)
        assert len(inhibitory_noise_distributions) == len(inhibitory_weights)
        self._num_excitatory_spike_trains = len(excitatory_noise_distributions)
        self._trains =\
            [spike_train.spike_train(noise, True) for noise in excitatory_noise_distributions] +\
            [spike_train.spike_train(noise, False) for noise in inhibitory_noise_distributions]
        self._weights = excitatory_weights.copy() + inhibitory_weights.copy()
        self._spike_times = numpy.array([self._trains[i].next_spike_time() for i in range(len(self._trains))])
        self._time = 0.0
        self._pre_spikes = []
        self._post_spikes = []
        self._soma = cell_soma
        self._soma_recording = {}
        for key, value in self._soma.variables.items():
            self._soma_recording[key] = [(self._time, value)]
        self._soma_recording_epsilons = {}
        for key, (low, high) in self._soma.ranges_of_variables(len(self._trains)).items():
            self._soma_recording_epsilons[key] = (high - low) / 2000.0

    def get_interval_of_excitatory_spike_trains(self):
        return 0, self._num_excitatory_spike_trains

    def get_interval_of_inhibitory_spike_trains(self):
        return self._num_excitatory_spike_trains, len(self._trains)

    def get_time(self):
        return self._time

    def get_weights(self):
        return self._weights

    def get_pre_spikes(self):
        return self._pre_spikes

    def get_pre_spikes_excitatory(self):
        return [(x, n) for x, n in self.get_pre_spikes() if n < self._num_excitatory_spike_trains]

    def get_pre_spikes_inhibitory(self):
        return [(x, n) for x, n in self.get_pre_spikes() if n >= self._num_excitatory_spike_trains]

    def get_post_spikes(self):
        return self._post_spikes

    def get_soma_recording(self):
        return self._soma_recording

    def get_soma(self):
        return self._soma

    def integrate(self, dt):
        was_spiking = self._soma.is_spiking()
        self._soma.integrate(dt)
        self._time += dt
        for i in range(len(self._spike_times)):
            if self._spike_times[i] <= self._time:
                self._pre_spikes.append((self._time, i))
                if self._trains[i].is_excitatory():
                    self._soma.on_excitatory_spike(self._weights[i])
                else:
                    self._soma.on_inhibitory_spike(self._weights[i])
                self._spike_times[i] = self._time + self._trains[i].next_spike_time()
        is_spiking = self._soma.is_spiking()
        if not was_spiking and is_spiking:
            self._post_spikes.append(self._time)
        for key, value in self._soma.variables.items():
            self._soma_recording[key].append((self._time, value))
