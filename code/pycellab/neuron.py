import numpy


class RecordingConfig:
    def __init__(self,
                 do_recording_of_post_synaptic_spikes=True,
                 do_recording_of_soma=True,
                 do_recording_of_excitatory_synapses=True,
                 do_recording_of_inhibitory_synapses=True,
                 do_recording_of_key_variables_only=True,
                 do_recording_controller=lambda last_recording_time, current_time, was_post_spike_generated: True
                 ):
        assert callable(do_recording_controller)
        self.post_synaptic_spikes = do_recording_of_post_synaptic_spikes
        self.soma = do_recording_of_soma
        self.excitatory_synapses = do_recording_of_excitatory_synapses
        self.inhibitory_synapses = do_recording_of_inhibitory_synapses
        self.key_variables_only = do_recording_of_key_variables_only
        self.controller = do_recording_controller


class Neuron:
    def __init__(self,
                 cell_soma,
                 excitatory_synapses,
                 inhibitory_synapses,
                 num_sub_iterations=1,
                 start_time=0.0,
                 recording_config=None
                 ):
        assert type(num_sub_iterations) is int and num_sub_iterations > 0
        assert recording_config is None or isinstance(recording_config, RecordingConfig)
        self._excitatory_synapses = excitatory_synapses
        self._inhibitory_synapses = inhibitory_synapses
        self._spikes = []
        self._soma = cell_soma
        self._num_sub_iterations = num_sub_iterations
        self._soma_recording = {}
        self._recording_config = recording_config if recording_config is not None else RecordingConfig()
        if self._recording_config.soma:
            for key, value in (self._soma.get_key_variables() if self._recording_config.key_variables_only else self._soma.get_variables()).items():
                self._soma_recording[key] = [(start_time, value)]
        self._excitatory_synapses_recording = [
            dict([(var, [(start_time, value)]) for var, value in (syn.get_key_variables() if self._recording_config.key_variables_only else syn.get_variables()).items()])
            for syn in self._excitatory_synapses
        ]
        self._inhibitory_synapses_recording = [
            dict([(var, [(start_time, value)]) for var, value in (syn.get_key_variables() if self._recording_config.key_variables_only else syn.get_variables()).items()])
            for syn in self._inhibitory_synapses
        ]
        self._last_recording_time = None

    def get_excitatory_synapses(self):
        return self._excitatory_synapses

    def get_inhibitory_synapses(self):
        return self._inhibitory_synapses

    def get_spikes(self):
        return self._spikes

    def get_soma_recording(self):
        return self._soma_recording

    def get_excitatory_synapses_recording(self):
        return self._excitatory_synapses_recording

    def get_inhibitory_synapses_recording(self):
        return self._inhibitory_synapses_recording

    def get_soma(self):
        return self._soma

    def get_short_description(self):
        return self._soma.get_short_description() + ", #subiters=" + str(self._num_sub_iterations)

    def get_recording_config(self):
        return self._recording_config

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
        for syn in self._excitatory_synapses:
            syn.integrate(dt)
        for syn in self._inhibitory_synapses:
            syn.integrate(dt)
        if was_spike_generated:
            for syn in self._excitatory_synapses:
                syn.on_post_synaptic_spike()
            for syn in self._inhibitory_synapses:
                syn.on_post_synaptic_spike()
        self._do_recording(t, dt, was_spike_generated)

    def _do_recording(self, t, dt, was_spike_generated):
        if self._last_recording_time is None:
            self._last_recording_time = t   # We assume the initial state is available to (i.e. recorded by) the user
        if self.get_recording_config().controller(self._last_recording_time, t + dt, was_spike_generated) is False:
            return
        self._last_recording_time = t + dt

        if self.get_recording_config().post_synaptic_spikes:
            if was_spike_generated:
                self._spikes.append(t + dt)

        if self.get_recording_config().soma:
            for key, value in (self._soma.get_key_variables()
                               if self.get_recording_config().key_variables_only
                               else self._soma.get_variables()).items():
                self._soma_recording[key].append((t + dt, value))

        if self.get_recording_config().excitatory_synapses:
            for i in range(len(self._excitatory_synapses)):
                for key, value in (self._excitatory_synapses[i].get_key_variables()
                                   if self.get_recording_config().key_variables_only
                                   else self._excitatory_synapses[i].get_variables()).items():
                    self._excitatory_synapses_recording[i][key].append((t + dt, value))

        if self.get_recording_config().inhibitory_synapses:
            for i in range(len(self._inhibitory_synapses)):
                for key, value in (self._inhibitory_synapses[i].get_key_variables()
                                   if self.get_recording_config().key_variables_only
                                   else self._inhibitory_synapses[i].get_variables()).items():
                    self._inhibitory_synapses_recording[i][key].append((t + dt, value))


def get_average_excitation_level(
        num_excitatory_input_trains,
        num_inhibitory_input_trains,
        spiking_frequency_of_excitatory_neouron=15.0,
        spiking_frequency_of_inhibitory_neouron=60.0
        ):
    assert type(num_excitatory_input_trains, int) and num_excitatory_input_trains >= 0
    assert type(num_inhibitory_input_trains, int) and num_inhibitory_input_trains >= 0
    assert spiking_frequency_of_excitatory_neouron >= 1
    assert spiking_frequency_of_inhibitory_neouron >= 1
    return (spiking_frequency_of_excitatory_neouron * num_excitatory_input_trains -
            spiking_frequency_of_inhibitory_neouron * num_inhibitory_input_trains) / 1000.0


def get_99percent_diversion_from_average_excitation_level(num_excitatory_input_trains):
    assert num_excitatory_input_trains >= 0
    return numpy.sqrt(num_excitatory_input_trains / 3.5)
