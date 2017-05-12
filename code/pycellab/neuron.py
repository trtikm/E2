import spike_train
import soma


class neuron:
    def __init__(self,
                 cell_soma,
                 excitatory_synapses,
                 inhibitory_synapses,
                 num_sub_iterations=1,
                 start_time=0.0
                 ):
        assert type(num_sub_iterations) is int and num_sub_iterations > 0
        self._excitatory_synapses = excitatory_synapses
        self._inhibitory_synapses = inhibitory_synapses
        self._spikes = []
        self._soma = cell_soma
        self._soma_recording = {}
        for key, value in self._soma.variables.items():
            self._soma_recording[key] = [(start_time, value)]
        self._num_sub_iterations = num_sub_iterations

    def get_excitatory_synapses(self):
        return self._excitatory_synapses

    def get_inhibitory_synapses(self):
        return self._inhibitory_synapses

    def get_spikes(self):
        return self._spikes

    def get_soma_recording(self):
        return self._soma_recording

    def get_soma(self):
        return self._soma

    def get_short_description(self):
        return self._soma.get_short_description() + ", #subiters=" + str(self._num_sub_iterations)

    def on_excitatory_spike(self, excitatory_spike_train_index):
        synapse = self._excitatory_synapses[excitatory_spike_train_index]
        synapse.on_pre_synaptic_spike()
        self._soma.on_excitatory_spike(synapse.get_weight())

    def on_inhibitory_spike(self, inhibitory_spike_train_index):
        synapse = self._inhibitory_synapses[inhibitory_spike_train_index]
        synapse.on_pre_synaptic_spike()
        self._soma.on_inhibitory_spike(synapse.get_weight())

    def integrate(self, t, dt):
        was_spike_generated = False
        sub_dt = dt / self._num_sub_iterations
        for _ in range(self._num_sub_iterations):
            was_spiking = self._soma.is_spiking()
            self._soma.integrate(sub_dt)
            is_spiking = self._soma.is_spiking()
            if not was_spiking and is_spiking:
                was_spike_generated = True
        for synapse in self._excitatory_synapses:
            synapse.integrate(dt)
        for synapse in self._inhibitory_synapses:
            synapse.integrate(dt)
        if was_spike_generated:
            for synapse in self._excitatory_synapses:
                synapse.on_post_synaptic_spike()
            for synapse in self._inhibitory_synapses:
                synapse.on_post_synaptic_spike()
            self._spikes.append(t + dt)
        for key, value in self._soma.variables.items():
            self._soma_recording[key].append((t + dt, value))
