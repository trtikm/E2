import spike_train
import soma


class neuron:
    def __init__(self,
                 cell_soma,
                 excitatory_weights,
                 inhibitory_weights,
                 num_sub_iterations=1,
                 start_time=0.0
                 ):
        assert type(num_sub_iterations) is int and num_sub_iterations > 0
        self._weights_excitatory = excitatory_weights.copy()
        self._weights_inhibitory = inhibitory_weights.copy()
        self._spikes = []
        self._soma = cell_soma
        self._soma_recording = {}
        for key, value in self._soma.variables.items():
            self._soma_recording[key] = [(start_time, value)]
        self._num_sub_iterations = num_sub_iterations

    def get_excitatory_weights(self):
        return self._weights_excitatory

    def get_inhibitory_weights(self):
        return self._weights_inhibitory

    def get_spikes(self):
        return self._spikes

    def get_soma_recording(self):
        return self._soma_recording

    def get_soma(self):
        return self._soma

    def get_short_description(self):
        return self._soma.get_short_description() + ", #subiters=" + str(self._num_sub_iterations)

    def on_excitatory_spike(self, excitatory_spike_train_index):
        self._soma.on_excitatory_spike(self._weights_excitatory[excitatory_spike_train_index])

    def on_inhibitory_spike(self, inhibitory_spike_train_index):
        self._soma.on_inhibitory_spike(self._weights_inhibitory[inhibitory_spike_train_index])

    def integrate(self, t, dt):
        was_spike_generated = False
        sub_dt = dt / self._num_sub_iterations
        for _ in range(self._num_sub_iterations):
            was_spiking = self._soma.is_spiking()
            self._soma.integrate(sub_dt)
            is_spiking = self._soma.is_spiking()
            if not was_spiking and is_spiking:
                was_spike_generated = True
        if was_spike_generated:
            self._spikes.append(t + dt)
        for key, value in self._soma.variables.items():
            self._soma_recording[key].append((t + dt, value))
