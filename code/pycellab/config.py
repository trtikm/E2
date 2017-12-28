import os
import soma
import neuron
import synapse
import integrator
import distribution
import spike_train
import utility


class CommonProps:
    def __init__(self,
                 name,
                 output_dir,
                 start_time,
                 dt,
                 nsteps,
                 plot_files_extension,
                 plot_time_step,
                 dependencies=None
                 ):
        assert isinstance(name, str)
        assert isinstance(start_time, float)
        assert isinstance(dt, float)
        assert isinstance(nsteps, int)
        assert isinstance(plot_files_extension, str)
        assert isinstance(plot_time_step, float)
        assert dt > 0.0
        assert nsteps >= 0
        assert plot_files_extension.lower() == ".svg" or plot_files_extension.lower() == ".png"
        assert plot_time_step > 0.0
        assert dependencies is None or (isinstance(dependencies, list) and all(isinstance(p, str) for p in dependencies))
        self.name = name
        self.start_time = start_time
        self.dt = dt
        self.nsteps = nsteps
        self.output_dir = os.path.normpath(os.path.abspath(os.path.join(output_dir, name)))
        self.default_evaluation_root_dir = os.path.normpath(os.path.abspath(output_dir))
        self.plot_files_extension = plot_files_extension.lower()
        self.plot_time_step = plot_time_step
        self.dependencies = []
        for config_name in (dependencies if dependencies is not None else []):
            path = os.path.normpath(os.path.join(self.default_evaluation_root_dir, config_name))
            if os.path.isdir(path):
                self.dependencies.append(path)
        self.dependencies.sort()

    def to_json(self):
        return {
            "name": self.name,
            "start_time": self.start_time,
            "dt": self.dt,
            "nsteps": self.nsteps,
            "output_dir": self.output_dir,
            "default_evaluation_root_dir": self.default_evaluation_root_dir,
            "plot_files_extension": self.plot_files_extension,
            "plot_time_step": self.plot_time_step,
            "dependencies": self.dependencies.copy()
        }


class NeuronWithInputSynapses(CommonProps):
    def __init__(self,
                 name,
                 output_dir,
                 start_time,
                 dt,
                 nsteps,
                 num_sub_iterations,
                 cell_soma,
                 excitatory_spike_trains,
                 inhibitory_spike_trains,
                 excitatory_synapses,
                 inhibitory_synapses,
                 plot_files_extension,
                 plot_time_step,
                 recording_config
                 ):
        assert isinstance(num_sub_iterations, list)
        assert False not in list(map(lambda x: type(x) is int and x > 0, num_sub_iterations))
        assert isinstance(cell_soma, list)
        assert len({soma.get_name() for soma in cell_soma}) == len(cell_soma)
        assert isinstance(excitatory_synapses, list)
        assert isinstance(inhibitory_synapses, list)
        assert len(cell_soma) == len(excitatory_synapses) and len(cell_soma) == len(inhibitory_synapses)
        assert len(cell_soma) == len(num_sub_iterations)
        assert False not in list(map(lambda x: len(x) == len(excitatory_spike_trains), excitatory_synapses))
        assert False not in list(map(lambda x: len(x) == len(inhibitory_spike_trains), inhibitory_synapses))
        assert recording_config is None or isinstance(recording_config, neuron.RecordingConfig)
        super(NeuronWithInputSynapses, self).__init__(name, output_dir, start_time, dt, nsteps, plot_files_extension, plot_time_step)
        self.num_sub_iterations = num_sub_iterations
        self.cell_soma = cell_soma
        self.excitatory_spike_trains = excitatory_spike_trains
        self.inhibitory_spike_trains = inhibitory_spike_trains
        self.excitatory_synapses = excitatory_synapses
        self.inhibitory_synapses = inhibitory_synapses
        self.excitatory_plot_indices = list(range(0, len(excitatory_spike_trains), max(1, len(excitatory_spike_trains) // 2)))
        self.inhibitory_plot_indices = list(range(0, len(inhibitory_spike_trains), max(1, len(inhibitory_spike_trains) // 2)))
        self.recording_config = recording_config

    @staticmethod
    def leaky_integrate_and_fire_const_input(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'leaky integrate and fire' neuron with a constant
        input current. strong enough to trigger constant firing of the neuron.
        """
        dt = 0.001
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=dt,
            nsteps=1000,
            num_sub_iterations=[1],
            cell_soma=[soma.leaky_integrate_and_fire(
                initial_potential=-70,
                initial_input=40.0,
                resting_potential=-65,
                firing_potential=-55,
                saturation_potential=-70,
                potential_cooling_coef=-58.0,
                input_cooling_coef=-250.0 * 4.0,
                psp_max_potential=-50.0,
                spike_magnitude=40.0,
                integrator_function=integrator.midpoint
                )],
            excitatory_spike_trains=[spike_train.create(distribution.Distribution({1.0*dt: 1.0}), 0.0)],
            inhibitory_spike_trains=[],
            excitatory_synapses=[[synapse.Synapse.constant()]],
            inhibitory_synapses=[[]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def leaky_integrate_and_fire_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'leaky integrate and fire' neuron with a 800
        excitatory and 200 inhibitory input spike trains, all with the
        standard noise distributions. Weights of all synapses is 1.
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[1],
            cell_soma=[soma.leaky_integrate_and_fire(
                initial_potential=-70,
                initial_input=40.0,
                resting_potential=-65,
                firing_potential=-55,
                saturation_potential=-70,
                potential_cooling_coef=-58.0,
                input_cooling_coef=-250.0,
                psp_max_potential=-50.0,
                spike_magnitude=1.0,
                integrator_function=integrator.midpoint
                )],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.84) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def izhikevich_regular_spiking_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'izhikevich regular spiking' neuron with a 800
        excitatory and 200 inhibitory input spike trains, all with the standard
        noise distributions. Weights of all synapses is 1. Spike magnitude is
        0.15. This neuron typically creates excitatory synapses to other
        neurons. These neurons are the most common in the cortex.
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[1],
            cell_soma=[soma.izhikevich.regular_spiking(spike_magnitude=0.15)],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.84) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def izhikevich_chattering_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'izhikevich chattering' neuron with a 800
        excitatory and 200 inhibitory input spike trains, all with the standard
        noise distributions. Weights of all synapses is 1. Spike magnitude is
        0.15. This neuron typically creates excitatory synapses to other
        neurons.
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[1],
            cell_soma=[soma.izhikevich.chattering(spike_magnitude=0.15)],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.84) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def izhikevich_fast_spiking_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'izhikevich fast spiking' neuron with a 800
        excitatory and 200 inhibitory input spike trains, all with the standard
        noise distributions. Weights of all synapses is 1. Spike magnitude is
        0.15. This neuron typically creates inhibitory synapses to other
        neurons.
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[1],
            cell_soma=[soma.izhikevich.fast_spiking(spike_magnitude=0.15)],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.84) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def hodgkin_huxley_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'hodgkin-huxley' neuron with a 800 excitatory and
        200 inhibitory input spike trains, all with the standard noise
        distributions. Weights of all synapses is 1. Spike magnitude is
        0.15.
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[100],
            cell_soma=[soma.hodgkin_huxley(
                spike_magnitude=0.15,
                integrator_function=integrator.midpoint
                )],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.35) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def wilson_reguar_spiking_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'wilson's regular spiking' neuron with a 800
        excitatory and 200 inhibitory input spike trains, all with the
        standard noise distributions. Weights of all synapses is 1. Spike
        magnitude is 1.0.
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[100],
            cell_soma=[soma.wilson.regular_spiking()],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.84) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def wilson_fast_spiking_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'wilson's fast spiking' neuron with a 800
        excitatory and 200 inhibitory input spike trains, all with the
        standard noise distributions. Weights of all synapses is 1.
        Spike magnitude is 1.0.
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[100],
            cell_soma=[soma.wilson.fast_spiking()],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.5) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def wilson_bursting_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of a 'wilson's bursting' neuron with a 800
        excitatory and 200 inhibitory input spike trains, all with
        the standard noise distributions. Weights of all synapses is 1.
        Spike magnitude is 1.0.
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[100],
            cell_soma=[soma.wilson.bursting()],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.84) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def izhikevich_hodgkin_huxley_wison_input_800ex_200in_std_noise(my_precomputed_full_name, output_dir):
        """
        A simulation of three neurons 'izhikevich regular spiking', 'izhikevich
        fast spiking', 'hodgkin-huxley', 'wilson regular spiking', and 'wilson
        fast spiking', with common spike input trains 800 excitatory and 200
        inhibitory, all with the standard noise distributions. Initial weights
        of all synapses of all neurons are 1. Spike magnitude for all neurons
        is 0.15, except for both wilson neurons for which it is 1.0."
        """
        num_excitatory = 800
        num_inhibitory = 200
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[1, 1, 100, 100, 100],
            cell_soma=[
                soma.izhikevich.regular_spiking(spike_magnitude=0.15),
                soma.izhikevich.fast_spiking(spike_magnitude=0.15),
                soma.hodgkin_huxley(spike_magnitude=0.15),
                soma.wilson.regular_spiking(),
                soma.wilson.fast_spiking(),
                ],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[
                [synapse.Synapse.constant(1.84) for _ in range(num_excitatory)],
                [synapse.Synapse.constant(1.84) for _ in range(num_excitatory)],
                [synapse.Synapse.constant(1.35) for _ in range(num_excitatory)],
                [synapse.Synapse.constant(1.84) for _ in range(num_excitatory)],
                [synapse.Synapse.constant(1.5) for _ in range(num_excitatory)]
                ],
            inhibitory_synapses=[
                [synapse.Synapse.constant() for _ in range(num_inhibitory)],
                [synapse.Synapse.constant() for _ in range(num_inhibitory)],
                [synapse.Synapse.constant() for _ in range(num_inhibitory)],
                [synapse.Synapse.constant() for _ in range(num_inhibitory)],
                [synapse.Synapse.constant() for _ in range(num_inhibitory)]
                ],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def development(my_precomputed_full_name, output_dir):
        """
        This is not a genuine configuration. It serves only for development,
        testing, and bug-fixing of this evaluation system.
        """
        count_base = 200
        num_excitatory = 4*count_base
        num_inhibitory = 1*count_base
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=10*1000,
            num_sub_iterations=[1],
            cell_soma=[soma.izhikevich.regular_spiking(
                spike_magnitude=0.15,
                # input_cooling_coef=-0.05
                )],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant(1.84) for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                do_recording_of_post_synaptic_spikes=True,
                do_recording_of_soma=True,
                do_recording_of_excitatory_synapses=False,
                do_recording_of_inhibitory_synapses=False,
                do_recording_of_key_variables_only=True,
                do_recording_controller=lambda last_recording_time, current_time, was_post_spike_generated: True
                )
            )


class SynapseAndSpikeNoise(CommonProps):
    def __init__(self,
                 name,
                 output_dir,
                 start_time,
                 dt,
                 nsteps,
                 the_synapse,
                 pre_spikes_distributions,
                 post_spikes_distributions,
                 plot_files_extension,
                 plot_time_step
                 ):
        assert isinstance(the_synapse, synapse.Synapse)
        assert isinstance(pre_spikes_distributions, distribution.Distribution)
        assert isinstance(post_spikes_distributions, distribution.Distribution)
        super(SynapseAndSpikeNoise, self).__init__(name, output_dir, start_time, dt, nsteps, plot_files_extension, plot_time_step)
        self.the_synapse = the_synapse
        self.pre_spikes_distributions = pre_spikes_distributions
        self.post_spikes_distributions = post_spikes_distributions

    @staticmethod
    def development(my_precomputed_full_name, output_dir):
        """
        This is not a genuine configuration. It serves only for development,
        testing, and bug-fixing of this evaluation system.
        """
        return SynapseAndSpikeNoise(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            the_synapse=synapse.Synapse.plastic_peek_np(),
            pre_spikes_distributions=distribution.default_excitatory_isi_distribution(),
            post_spikes_distributions=distribution.default_excitatory_isi_distribution(),
            plot_files_extension=".png",
            plot_time_step=1.0,
            )


class PrePostSpikeNoisesDifferences(CommonProps):
    def __init__(self,
                 name,
                 output_dir,
                 start_time,
                 dt,
                 nsteps,
                 plot_files_extension,
                 plot_time_step,
                 save_per_partes_plots,
                 pre_spikes_distributions,
                 post_spikes_distributions,
                 synaptic_input_cooler
                 ):
        assert isinstance(pre_spikes_distributions, distribution.Distribution)
        assert isinstance(post_spikes_distributions, distribution.Distribution)
        super(PrePostSpikeNoisesDifferences, self).__init__(
            name, output_dir, start_time, dt, nsteps, plot_files_extension, plot_time_step
            )
        self.save_per_partes_plots = save_per_partes_plots
        self.pre_spikes_distributions = pre_spikes_distributions
        self.post_spikes_distributions = post_spikes_distributions
        self.synaptic_input_cooler = synaptic_input_cooler

    @staticmethod
    def pre_1_post_1_clipped(my_precomputed_full_name, output_dir):
        """
        The experiment generates pre- and post- spike trains for the same
        distribution and computes statistical plots and histograms of time
        differences between individual pre- and post- spikes. Also, ISI
        histograms are reconstructed from spike trains.
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=60000,
            plot_files_extension=".png",
            plot_time_step=1.0,
            save_per_partes_plots=False,
            pre_spikes_distributions=distribution.hermit_distribution(0.1),
            post_spikes_distributions=distribution.hermit_distribution(0.1),
            synaptic_input_cooler=synapse.SynapticInputCooler.default(clip_var_to_their_ranges=True)
            )

    @staticmethod
    def pre_1_post_2_clipped(my_precomputed_full_name, output_dir):
        """
        The experiment generates pre- and post- spike trains for Hermit
        distributions with peeks at 0.1 and 0.2 respectively. The synaptic
        input variables are clipped to the range [0, spike_magnitude].
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=60000,
            plot_files_extension=".png",
            plot_time_step=1.0,
            save_per_partes_plots=False,
            pre_spikes_distributions=distribution.hermit_distribution(0.1),
            post_spikes_distributions=distribution.hermit_distribution(0.2),
            synaptic_input_cooler=synapse.SynapticInputCooler.default(clip_var_to_their_ranges=True)
            )

    @staticmethod
    def pre_1_post_3_clipped(my_precomputed_full_name, output_dir):
        """
        The experiment generates pre- and post- spike trains for Hermit
        distributions with peeks at 0.1 and 0.3 respectively. The synaptic
        input variables are clipped to the range [0, spike_magnitude].
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=60000,
            plot_files_extension=".png",
            plot_time_step=1.0,
            save_per_partes_plots=False,
            pre_spikes_distributions=distribution.hermit_distribution(0.1),
            post_spikes_distributions=distribution.hermit_distribution(0.3),
            synaptic_input_cooler=synapse.SynapticInputCooler.default(clip_var_to_their_ranges=True)
            )

    @staticmethod
    def pre_2_post_1_clipped(my_precomputed_full_name, output_dir):
        """
        The experiment generates pre- and post- spike trains for Hermit
        distributions with peeks at 0.2 and 0.1 respectively. The synaptic
        input variables are clipped to the range [0, spike_magnitude].
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=60000,
            plot_files_extension=".png",
            plot_time_step=1.0,
            save_per_partes_plots=False,
            pre_spikes_distributions=distribution.hermit_distribution(0.2),
            post_spikes_distributions=distribution.hermit_distribution(0.1),
            synaptic_input_cooler=synapse.SynapticInputCooler.default(clip_var_to_their_ranges=True)
            )

    @staticmethod
    def pre_3_post_1_clipped(my_precomputed_full_name, output_dir):
        """
        The experiment generates pre- and post- spike trains for Hermit
        distributions with peeks at 0.3 and 0.1 respectively. The synaptic
        input variables are clipped to the range [0, spike_magnitude].
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
            output_dir=output_dir,
            start_time=0.0,
            dt=0.001,
            nsteps=60000,
            plot_files_extension=".png",
            plot_time_step=1.0,
            save_per_partes_plots=False,
            pre_spikes_distributions=distribution.hermit_distribution(0.3),
            post_spikes_distributions=distribution.hermit_distribution(0.1),
            synaptic_input_cooler=synapse.SynapticInputCooler.default(clip_var_to_their_ranges=True)
            )


class EffectOfInputSpikeTrains:

    class Configuration(CommonProps):
        def __init__(self,
                     name,
                     output_dir,
                     start_time,
                     dt,
                     nsteps,
                     excitatory_spike_trains,
                     inhibitory_spike_trains,
                     excitatory_plot_indices,
                     inhibitory_plot_indices,
                     plot_time_step,
                     num_plots_parts,
                     plot_files_extension
                     ):
            assert isinstance(num_plots_parts, int) and num_plots_parts > 0
            super(EffectOfInputSpikeTrains.Configuration, self).__init__(
                name, output_dir, start_time, dt, nsteps, plot_files_extension, plot_time_step
                )
            self.excitatory_spike_trains = excitatory_spike_trains
            self.inhibitory_spike_trains = inhibitory_spike_trains
            self.excitatory_plot_indices = excitatory_plot_indices
            self.inhibitory_plot_indices = inhibitory_plot_indices
            self.num_plots_parts = num_plots_parts

        def to_json(self):
            return {
                "name": self.name,
                "start_time": self.start_time,
                "dt": self.dt,
                "nsteps": self.nsteps,
                "excitatory_spike_trains": len(self.excitatory_spike_trains),
                "inhibitory_spike_trains": len(self.inhibitory_spike_trains),
                "excitatory_plot_indices": self.excitatory_plot_indices,
                "inhibitory_plot_indices": self.inhibitory_plot_indices,
                "plot_time_step": self.plot_time_step,
                "num_plots_parts": self.num_plots_parts,
                "plot_files_extension": self.plot_files_extension,
            }

        @staticmethod
        def _build_spike_trains(
                num_trains,
                use_excitatory,
                histogram_of_percentages_of_regularity_phases
                ):
            base_spikes_distribution = (distribution.default_excitatory_isi_distribution()
                                        if use_excitatory
                                        else distribution.default_inhibitory_isi_distribution())
            rpd = distribution.Distribution(histogram_of_percentages_of_regularity_phases)
            return [spike_train.create(base_spikes_distribution, rpd.next_event()) for _ in range(num_trains)]

        @staticmethod
        def create(
                my_precomputed_full_name,
                output_dir,
                num_trains_excitatory,
                histogram_of_percentages_of_excitatory_regularity_phases,
                num_trains_inhibitory,
                histogram_of_percentages_of_inhibitory_regularity_phases,
                num_seconds_to_simulate=30,
                num_plots_of_excitatory_trains=10,
                num_plots_of_inhibitory_trains=10,
                plot_time_step=1.0,
                num_plots_parts=10
                ):
            assert type(num_seconds_to_simulate) in [int, float] and num_seconds_to_simulate > 0
            return EffectOfInputSpikeTrains.Configuration(
                name=my_precomputed_full_name,
                output_dir=output_dir,
                start_time=0.0,
                dt=0.001,
                nsteps=int(num_seconds_to_simulate * 1000 + 0.5),
                excitatory_spike_trains=EffectOfInputSpikeTrains.Configuration._build_spike_trains(
                    num_trains_excitatory,
                    True,
                    histogram_of_percentages_of_excitatory_regularity_phases
                    ),
                inhibitory_spike_trains=EffectOfInputSpikeTrains.Configuration._build_spike_trains(
                    num_trains_inhibitory,
                    False,
                    histogram_of_percentages_of_inhibitory_regularity_phases,
                    ),
                excitatory_plot_indices=(
                    list(range(0, num_trains_excitatory, max(1, num_trains_excitatory // num_plots_of_excitatory_trains)))
                    ),
                inhibitory_plot_indices=(
                    list(range(0, num_trains_inhibitory, max(1, num_trains_inhibitory // num_plots_of_inhibitory_trains)))
                    ),
                plot_time_step=plot_time_step,
                num_plots_parts=num_plots_parts,
                plot_files_extension=".png",
                )

    class ConstructionData:
        def __init__(
                self,
                name,
                output_dir,
                sub_dir,
                num_trains_excitatory,
                histogram_of_percentages_of_excitatory_regularity_phases,
                num_trains_inhibitory,
                histogram_of_percentages_of_inhibitory_regularity_phases,
                num_seconds_to_simulate=30,
                num_plots_of_excitatory_trains=10,
                num_plots_of_inhibitory_trains=10,
                plot_time_step=1.0,
                num_plots_parts=10
                ):
            self._name = name
            self._output_dir = output_dir
            self._sub_dir = sub_dir
            self._num_trains_excitatory = num_trains_excitatory
            self._histogram_of_percentages_of_excitatory_regularity_phases = histogram_of_percentages_of_excitatory_regularity_phases
            self._num_trains_inhibitory = num_trains_inhibitory
            self._histogram_of_percentages_of_inhibitory_regularity_phases = histogram_of_percentages_of_inhibitory_regularity_phases
            self._num_seconds_to_simulate = num_seconds_to_simulate
            self._num_plots_of_excitatory_trains = num_plots_of_excitatory_trains
            self._num_plots_of_inhibitory_trains = num_plots_of_inhibitory_trains
            self._plot_time_step = plot_time_step
            self._num_plots_parts = num_plots_parts

        def get_name(self):
            return os.path.join(self._name, self._sub_dir)

        def get_output_root_dir(self):
            return os.path.join(self._output_dir, self.get_name())

        def apply(self):
            return EffectOfInputSpikeTrains.Configuration.create(
                self.get_name(),
                self._output_dir,
                self._num_trains_excitatory,
                self._histogram_of_percentages_of_excitatory_regularity_phases,
                self._num_trains_inhibitory,
                self._histogram_of_percentages_of_inhibitory_regularity_phases,
                self._num_seconds_to_simulate,
                self._num_plots_of_excitatory_trains,
                self._num_plots_of_inhibitory_trains,
                self._plot_time_step,
                self._num_plots_parts
                )

        def to_json(self):
            return {
                "name": self._name,
                "output_dir": self._output_dir,
                "sub_dir": self._sub_dir,
                "num_trains_excitatory": self._num_trains_excitatory,
                "histogram_of_percentages_of_excitatory_regularity_phases":
                    distribution.Distribution(self._histogram_of_percentages_of_excitatory_regularity_phases).to_json(),
                "num_trains_inhibitory": self._num_trains_inhibitory,
                "histogram_of_percentages_of_inhibitory_regularity_phases":
                    distribution.Distribution(self._histogram_of_percentages_of_inhibitory_regularity_phases).to_json(),
                "num_seconds_to_simulate": self._num_seconds_to_simulate,
                "num_plots_of_excitatory_trains": self._num_plots_of_excitatory_trains,
                "num_plots_of_inhibitory_trains": self._num_plots_of_inhibitory_trains,
                "plot_time_step": self._plot_time_step,
                "num_plots_parts": self._num_plots_parts,
            }

    def __init__(self, list_of_construction_data):
        assert isinstance(list_of_construction_data, list)
        assert all(isinstance(data, EffectOfInputSpikeTrains.ConstructionData) for data in list_of_construction_data)
        self._list_of_construction_data = list_of_construction_data
        self._output_dirs = [os.path.abspath(cfg.get_output_root_dir()) for cfg in self.get_list_of_construction_data()]
        self._interconfig_output_dir = os.path.join(utility.get_common_prefix_of_disk_paths(self._output_dirs), "interconfig")

    def get_list_of_construction_data(self):
        return self._list_of_construction_data

    def get_output_dirs_of_configurations(self):
        return self._output_dirs

    def get_interconfig_output_dir(self):
        return self._interconfig_output_dir

    @staticmethod
    def all_in_one(my_precomputed_full_name, output_dir):
        """
        A simulation of spike trains which are considers as an input
        to a single neuron. Some of the trains are from an excitatory
        pre-synaptic neurons and remaining from inhibitory neurons. The
        main focus of the simulation is on the summary effect of the
        trains on the post-synaptic neuron. This is reflected in the
        count of plots with summary data saved on the disk.
        This configuration consists of several sub-configurations.
        They are completely independent and they are executed one by
        one. Their results are saved into separate sub-directories.
        They differ in total number of spike trains, in percentage
        of spike trains from excitatory/inhibitory pre-synaptic
        neurons. and in degree of spike regularities in individual
        spike trains. NOTE: The simulation typically take several
        hours or days to complete all the configurations (depending
        on computer). Also note, resulting plots take several gigabytes
        of disk space.
        """
        trains_counts = [
            5 * 100,
            5 * 200,
            5 * 400,
            5 * 800,
            5 * 1600,
            5 * 3200,
            5 * 6400
            ]
        excitatory_percentages = [
            89.0,
            86.0,
            83.0,
            80.0,
            77.0,
            74.0,
            71.0
        ]
        histograms = [
            distribution.hermit_distribution_histogram(0.5, 100.0, 100.0, 1.0, 0.0, 1.0),
        ]
        return EffectOfInputSpikeTrains(
            [EffectOfInputSpikeTrains.ConstructionData(
                my_precomputed_full_name,
                output_dir,
                os.path.join(
                    "num_trains_" + str(num_trains),
                    "percentage_" + format(excitatory_percentage, ".2f") + "e (" +
                    format(int(0.01 * excitatory_percentage * num_trains), ".2f") +
                    "e_" +
                    format(int(0.01 * (100.0 - excitatory_percentage) * num_trains), ".2f") +
                    "i)",
                    "regularity_mean_" +
                    format(distribution.Distribution(histogram).get_mean(), ".2f") +
                    "_max_" +
                    format(sorted((p, e) for e, p in histogram.items())[-1][1], ".2f")
                    ),
                int(0.01 * excitatory_percentage * num_trains),
                histogram,
                int(0.01 * (100.0 - excitatory_percentage) * num_trains),
                histogram,
                30
                )
             for num_trains in trains_counts
             for excitatory_percentage in excitatory_percentages
             for histogram in histograms
             ]
            )


class TimeDifferencesBetweenPrePostSpikes:

    class Configuration(CommonProps):
        def __init__(self,
                     name,
                     output_dir,
                     start_time,
                     dt,
                     nsteps,
                     pre_spike_train,
                     post_spike_train
                     ):
            super(TimeDifferencesBetweenPrePostSpikes.Configuration, self).__init__(
                name, output_dir, start_time, dt, nsteps, ".png", 1.0
                )
            self.pre_spike_train = pre_spike_train
            self.post_spike_train = post_spike_train

        def to_json(self):
            return {
                "name": self.name,
                "start_time": self.start_time,
                "dt": self.dt,
                "nsteps": self.nsteps,
                "pre_spike_train": self.pre_spike_train.to_json(),
                "post_spike_train": self.post_spike_train.to_json(),
            }

    class ConstructionData:
        def __init__(
                self,
                name,
                output_dir,
                pre_is_excitatory,
                pre_mean_frequency,
                pre_percentage_of_regularity_phases,
                post_is_excitatory,
                post_mean_frequency,
                post_percentage_of_regularity_phases,
                num_seconds_to_simulate=30
                ):
            assert type(pre_is_excitatory) == bool
            assert type(pre_mean_frequency) in [int, float] and pre_mean_frequency > 0.0
            assert type(pre_percentage_of_regularity_phases) in [int, float]
            assert pre_percentage_of_regularity_phases >= 0 and pre_percentage_of_regularity_phases <= 1
            assert type(post_is_excitatory) == bool
            assert type(post_mean_frequency) in [int, float] and post_mean_frequency > 0.0
            assert type(post_percentage_of_regularity_phases) in [int, float]
            assert post_percentage_of_regularity_phases >= 0 and post_percentage_of_regularity_phases <= 1
            self._name = name
            self._output_dir = output_dir
            self._pre_is_excitatory = pre_is_excitatory
            self._pre_mean_frequency = pre_mean_frequency
            self._pre_percentage_of_regularity_phases = pre_percentage_of_regularity_phases
            self._post_is_excitatory = post_is_excitatory
            self._post_mean_frequency = post_mean_frequency
            self._post_percentage_of_regularity_phases = post_percentage_of_regularity_phases
            self._num_seconds_to_simulate = num_seconds_to_simulate

        def to_json(self):
            return {
                "name": self._name,
                "output_dir": self._output_dir,
                "pre_is_excitatory": self._pre_is_excitatory,
                "pre_mean_frequency": self._pre_mean_frequency,
                "pre_percentage_of_regularity_phases": self._pre_percentage_of_regularity_phases,
                "post_is_excitatory": self._post_is_excitatory,
                "post_mean_frequency": self._post_mean_frequency,
                "post_percentage_of_regularity_phases": self._post_percentage_of_regularity_phases,
                "num_seconds_to_simulate": self._num_seconds_to_simulate
            }

        def get_name(self):
            return self._name

        def get_output_root_dir(self):
            return os.path.join(self._output_dir, self.get_name())

        def apply(self):
            return TimeDifferencesBetweenPrePostSpikes.Configuration(
                self._name,
                self._output_dir,
                0.0,
                0.001,
                int(self._num_seconds_to_simulate * 1000 + 0.5),
                spike_train.create(
                    distribution.hermit_distribution_with_desired_mean(1.0 / self._pre_mean_frequency, 0.003, 0.3, 0.0001, pow_y=2)
                        if self._pre_is_excitatory
                        else distribution.hermit_distribution_with_desired_mean(1.0 / self._pre_mean_frequency, 0.001, 0.08, 0.0001, pow_y=2),
                    self._pre_percentage_of_regularity_phases
                    ),
                spike_train.create(
                    distribution.hermit_distribution_with_desired_mean(1.0 / self._post_mean_frequency, 0.003, 0.3, 0.0001, pow_y=2)
                        if self._post_is_excitatory
                        else distribution.hermit_distribution_with_desired_mean(1.0 / self._post_mean_frequency, 0.001, 0.08, 0.0001, pow_y=2),
                    self._post_percentage_of_regularity_phases
                    )
                )

    def __init__(self, list_of_construction_data):
        assert isinstance(list_of_construction_data, list)
        assert all(isinstance(data, TimeDifferencesBetweenPrePostSpikes.ConstructionData) for data in list_of_construction_data)
        self._list_of_construction_data = list_of_construction_data
        self._output_dirs = [os.path.abspath(cfg.get_output_root_dir()) for cfg in self.get_list_of_construction_data()]
        self._interconfig_output_dir = os.path.join(utility.get_common_prefix_of_disk_paths(self._output_dirs), "interconfig")

    def get_list_of_construction_data(self):
        return self._list_of_construction_data

    def get_output_dirs_of_configurations(self):
        return self._output_dirs

    def get_interconfig_output_dir(self):
        return self._interconfig_output_dir

    @staticmethod
    def all_in_one(my_precomputed_full_name, output_dir):
        """
        Evaluates time differences between pre- and post- synaptic spikes.
        Namely, for a pre- spikes train and a post- spikes train time
        differences between post and pre spikes are collected and
        histogram of the time differences is plotted. There are
        considered all combinations of the following properties of
        both spikes trains: Excitatory mean spike frequencies [12.0,
        13.5, 15.0, 16.5, 18.0], inhibitory mean spike frequencies
        [48.0, 54.0, 60.0, 66.0, 72.0], and these percentages of
        regulatory phases of the trains [0.0, 0.25, 0.5, 0.75].
        """
        excitatory_mean_frequencies = [12.0, 13.5, 15.0, 16.5, 18.0]
        inhibitory_mean_frequencies = [48.0, 54.0, 60.0, 66.0, 72.0]
        return TimeDifferencesBetweenPrePostSpikes(
            [TimeDifferencesBetweenPrePostSpikes.ConstructionData(
                os.path.join(
                    my_precomputed_full_name,
                    "pre_1" + ("e" if pre_is_excitatory else "i") + "_" +
                        format(pre_percentage_of_regularity_phases, ".2f") + "p__" +
                        "post_1" + ("e" if post_is_excitatory else "i") + "_" +
                            format(post_percentage_of_regularity_phases, ".2f") + "p",
                    "pre_mfreq_" + format(pre_mean_frequency, ".2f") + "__"
                        "post_mfreq_" + format(post_mean_frequency, ".2f")
                    ),
                output_dir,
                pre_is_excitatory,
                pre_mean_frequency,
                pre_percentage_of_regularity_phases,
                post_is_excitatory,
                post_mean_frequency,
                post_percentage_of_regularity_phases,
                120
                )
             for pre_is_excitatory in [True, False]
             for pre_percentage_of_regularity_phases in [0.0, 0.25, 0.5, 0.75]
             for post_is_excitatory in [True, False]
             for post_percentage_of_regularity_phases in [0.0, 0.25, 0.5, 0.75]
             for pre_mean_frequency in (excitatory_mean_frequencies if pre_is_excitatory else inhibitory_mean_frequencies)
             for post_mean_frequency in (excitatory_mean_frequencies if post_is_excitatory else inhibitory_mean_frequencies)
             ]
            )
        pass


class SynapticPlasticity(CommonProps):

    def __init__(self, name, output_dir, start_time=0.0, dt=0.001, nsteps=1000):
        super(SynapticPlasticity, self).__init__(
            name, output_dir, start_time, dt, nsteps, ".png", 1.0, ["TimeDifferencesBetweenPrePostSpikes/all_in_one"]
            )

    @staticmethod
    def all_in_one(my_precomputed_full_name, output_dir):
        """
        TODO
        """
        return SynapticPlasticity(my_precomputed_full_name, output_dir)


class AutoBalancedPreSynapticInput:

    class Configuration(CommonProps):
        def __init__(self,
                     name,
                     output_dir,
                     start_time,
                     dt,
                     nsteps,
                     num_sub_iterations,
                     cell_soma,
                     excitatory_spike_trains,
                     inhibitory_spike_trains
                     ):
            super(AutoBalancedPreSynapticInput.Configuration, self).__init__(
                name, output_dir, start_time, dt, nsteps, ".png", 1
                )
            assert isinstance(num_sub_iterations, int) and num_sub_iterations > 0
            self.num_sub_iterations = num_sub_iterations
            self.cell_soma = cell_soma
            self.excitatory_spike_trains = excitatory_spike_trains
            self.inhibitory_spike_trains = inhibitory_spike_trains

        def to_json(self):
            return {
                "name": self.name,
                "start_time": self.start_time,
                "dt": self.dt,
                "nsteps": self.nsteps,
                "num_sub_iterations": self.num_sub_iterations,
                "cell_soma": {
                    "name": self.cell_soma.get_name(),
                    "data":  self.cell_soma.get_short_description()
                    },
                "excitatory_spike_trains": len(self.excitatory_spike_trains),
                "inhibitory_spike_trains": len(self.inhibitory_spike_trains),
            }

    class ConstructionData:
        def __init__(
                self,
                name,
                output_dir,
                sub_dir,
                num_trains_excitatory,
                histogram_of_percentages_of_excitatory_regularity_phases,
                num_trains_inhibitory,
                histogram_of_percentages_of_inhibitory_regularity_phases,
                num_seconds_to_simulate=30
                ):
            self._name = name
            self._output_dir = output_dir
            self._sub_dir = sub_dir
            self._num_trains_excitatory = num_trains_excitatory
            self._histogram_of_percentages_of_excitatory_regularity_phases = histogram_of_percentages_of_excitatory_regularity_phases
            self._num_trains_inhibitory = num_trains_inhibitory
            self._histogram_of_percentages_of_inhibitory_regularity_phases = histogram_of_percentages_of_inhibitory_regularity_phases
            self._num_seconds_to_simulate = num_seconds_to_simulate

        def get_name(self):
            return os.path.join(self._name, self._sub_dir)

        def get_output_root_dir(self):
            return os.path.join(self._output_dir, self.get_name())

        def apply(self):
            return AutoBalancedPreSynapticInput.Configuration(
                self.get_name(),
                self._output_dir,
                self._num_trains_excitatory,
                self._histogram_of_percentages_of_excitatory_regularity_phases,
                self._num_trains_inhibitory,
                self._histogram_of_percentages_of_inhibitory_regularity_phases,
                self._num_seconds_to_simulate,
                )

        def to_json(self):
            return {
                "name": self._name,
                "output_dir": self._output_dir,
                "sub_dir": self._sub_dir,
                "num_trains_excitatory": self._num_trains_excitatory,
                "histogram_of_percentages_of_excitatory_regularity_phases":
                    distribution.Distribution(self._histogram_of_percentages_of_excitatory_regularity_phases).to_json(),
                "num_trains_inhibitory": self._num_trains_inhibitory,
                "histogram_of_percentages_of_inhibitory_regularity_phases":
                    distribution.Distribution(self._histogram_of_percentages_of_inhibitory_regularity_phases).to_json(),
                "num_seconds_to_simulate": self._num_seconds_to_simulate
            }

    pass


####################################################################################################
####################################################################################################
####################################################################################################


def __list_local_classes(items_list):
    result = []
    for item in items_list:
        try:
            issubclass(item, object)
            result.append(item)
        except:
            pass
    return result


def __list_static_methods_of_class(the_class):
    result = []
    for key, value in the_class.__dict__.items():
        if isinstance(value, staticmethod):
            function = eval(the_class.__name__ + "." + key)
            result.append({
                "class_name": str(the_class.__name__),
                "function_name": str(key),
                "function_ptr": function,
                "description": str(function.__doc__),
                "name": str(the_class.__name__ + "/" + key)
            })
    return result


def __list_static_methods_of_local_classes(items_list):
    result = []
    for the_class in __list_local_classes(items_list):
        result += __list_static_methods_of_class(the_class)
    return sorted(result, key=lambda x: x["name"])


__automatically_registered_configurations = __list_static_methods_of_local_classes(list(map(eval, dir())))


def get_registered_configurations():
    return __automatically_registered_configurations


def construct_experiment(experiment_info):
    assert isinstance(experiment_info, dict)
    assert "function_ptr" in experiment_info and callable(experiment_info["function_ptr"])
    assert "name" in experiment_info
    assert "output_dir" in experiment_info
    return experiment_info["function_ptr"](experiment_info["name"], experiment_info["output_dir"])
