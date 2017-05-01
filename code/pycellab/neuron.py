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

    def get_time(self):
        return self._time

    def get_weights(self):
        return self._weights

    def get_pre_spikes(self):
        return self._pre_spikes

    def get_post_spikes(self):
        return self._post_spikes

    def get_soma_recording(self):
        return self._soma_recording

    def integrate(self, dt, is_this_last_step):
        was_spiking = self._soma.is_spiking()
        self._soma.integrate(dt)
        self._time += dt
        was_pre_spike = False
        for i in range(len(self._spike_times)):
            if self._spike_times[i] <= self._time:
                self._pre_spikes.append((self._time, i))
                if self._trains[i].is_excitatory():
                    self._soma.on_excitatory_spike(self._weights[i])
                else:
                    self._soma.on_inhibitory_spike(self._weights[i])
                self._spike_times[i] = self._time + self._trains[i].next_spike_time()
                was_pre_spike = True
        is_spiking = self._soma.is_spiking()
        if was_spiking is False and is_spiking is True:
            self._post_spikes.append(self._time)
        if was_pre_spike:
            for key in self._soma.get_on_spike_variable_names():
                last_value = self._soma_recording[key][-1][1]
                self._soma_recording[key].append((self._time, last_value))
        for key, value in self._soma.variables.items():
            last_value = self._soma_recording[key][-1][1]
            if abs(value - last_value) > self._soma_recording_epsilons[key] or is_this_last_step:
                self._soma_recording[key].append((self._time, value))
