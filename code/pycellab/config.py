import os
import soma
import neuron
import synapse
import integrator
import distribution
import spike_train


def __get_my_dir():
    return os.path.dirname(__file__)


def output_root_dir():
    if str(__get_my_dir()).replace("\\", "/").endswith("E2/code/pycellab"):
        return os.path.normpath(os.path.join(__get_my_dir(), "..", "..", "dist", "evaluation", "pycellab"))
    else:
        return __get_my_dir()


class CommonProps:
    def __init__(self,
                 name,
                 start_time,
                 dt,
                 nsteps,
                 plot_files_extension,
                 plot_time_step
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
        self.name = name
        self.start_time = start_time
        self.dt = dt
        self.nsteps = nsteps
        self.output_dir = os.path.abspath(os.path.join(output_root_dir(), name))
        self.plot_files_extension = plot_files_extension.lower()
        self.plot_time_step = plot_time_step


class NeuronWithInputSynapses(CommonProps):
    def __init__(self,
                 name,
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
        super(NeuronWithInputSynapses, self).__init__(name, start_time, dt, nsteps, plot_files_extension, plot_time_step)
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
    def leaky_integrate_and_fire_const_input(my_precomputed_full_name):
        """
        A simulation of a 'leaky integrate and fire' neuron with a constant
        input current. strong enough to trigger constant firing of the neuron.
        """
        dt = 0.001
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
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
    def leaky_integrate_and_fire_input_800ex_200in_std_noise(my_precomputed_full_name):
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
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def izhikevich_regular_spiking_input_800ex_200in_std_noise(my_precomputed_full_name):
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
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[1],
            cell_soma=[soma.izhikevich.regular_spiking(spike_magnitude=0.15)],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def izhikevich_chattering_input_800ex_200in_std_noise(my_precomputed_full_name):
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
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[1],
            cell_soma=[soma.izhikevich.chattering(spike_magnitude=0.15)],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def izhikevich_fast_spiking_input_800ex_200in_std_noise(my_precomputed_full_name):
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
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[1],
            cell_soma=[soma.izhikevich.fast_spiking(spike_magnitude=0.15)],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def hodgkin_huxley_input_800ex_200in_std_noise(my_precomputed_full_name):
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
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def wilson_reguar_spiking_input_800ex_200in_std_noise(my_precomputed_full_name):
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
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[100],
            cell_soma=[soma.wilson.regular_spiking()],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def wilson_fast_spiking_input_800ex_200in_std_noise(my_precomputed_full_name):
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
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[100],
            cell_soma=[soma.wilson.fast_spiking()],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def wilson_bursting_input_800ex_200in_std_noise(my_precomputed_full_name):
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
            start_time=0.0,
            dt=0.001,
            nsteps=1000,
            num_sub_iterations=[100],
            cell_soma=[soma.wilson.bursting()],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                )
            )

    @staticmethod
    def izhikevich_hodgkin_huxley_wison_input_800ex_200in_std_noise(my_precomputed_full_name):
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
                [synapse.Synapse.constant() for _ in range(num_excitatory)],
                [synapse.Synapse.constant() for _ in range(num_excitatory)],
                [synapse.Synapse.constant() for _ in range(num_excitatory)],
                [synapse.Synapse.constant() for _ in range(num_excitatory)],
                [synapse.Synapse.constant() for _ in range(num_excitatory)]
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
    def development(my_precomputed_full_name):
        """
        This is not a genuine configuration. It serves only for development,
        testing, and bug-fixing of this evaluation system.
        """
        count_base = 2000
        num_excitatory = 4*count_base
        num_inhibitory = 1*count_base
        excitatory_noise = distribution.default_excitatory_isi_distribution()
        inhibitory_noise = distribution.default_inhibitory_isi_distribution()
        return NeuronWithInputSynapses(
            name=my_precomputed_full_name,
            start_time=0.0,
            dt=0.001,
            nsteps=1*1000,
            num_sub_iterations=[1],
            cell_soma=[soma.izhikevich.regular_spiking(
                spike_magnitude=1.0,
                input_cooling_coef=-0.05
                )],
            excitatory_spike_trains=[spike_train.create(excitatory_noise, 0.0) for _ in range(num_excitatory)],
            inhibitory_spike_trains=[spike_train.create(inhibitory_noise, 0.0) for _ in range(num_inhibitory)],
            excitatory_synapses=[[synapse.Synapse.constant() for _ in range(num_excitatory)]],
            inhibitory_synapses=[[synapse.Synapse.constant() for _ in range(num_inhibitory)]],
            plot_files_extension=".png",
            plot_time_step=1.0,
            recording_config=neuron.RecordingConfig(
                do_recording_of_post_synaptic_spikes=True,
                do_recording_of_soma=False,
                do_recording_of_excitatory_synapses=False,
                do_recording_of_inhibitory_synapses=False,
                do_recording_of_key_variables_only=False,
                do_recording_controller=lambda last_recording_time, current_time, was_post_spike_generated: True
                )
            )


class SynapseAndSpikeNoise(CommonProps):
    def __init__(self,
                 name,
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
        super(SynapseAndSpikeNoise, self).__init__(name, start_time, dt, nsteps, plot_files_extension, plot_time_step)
        self.the_synapse = the_synapse
        self.pre_spikes_distributions = pre_spikes_distributions
        self.post_spikes_distributions = post_spikes_distributions

    @staticmethod
    def development(my_precomputed_full_name):
        """
        This is not a genuine configuration. It serves only for development,
        testing, and bug-fixing of this evaluation system.
        """
        return SynapseAndSpikeNoise(
            name=my_precomputed_full_name,
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
            name, start_time, dt, nsteps, plot_files_extension, plot_time_step
            )
        self.save_per_partes_plots = save_per_partes_plots
        self.pre_spikes_distributions = pre_spikes_distributions
        self.post_spikes_distributions = post_spikes_distributions
        self.synaptic_input_cooler = synaptic_input_cooler

    @staticmethod
    def pre_1_post_1_clipped(my_precomputed_full_name):
        """
        The experiment generates pre- and post- spike trains for the same
        distribution and computes statistical plots and histograms of time
        differences between individual pre- and post- spikes. Also, ISI
        histograms are reconstructed from spike trains.
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
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
    def pre_1_post_2_clipped(my_precomputed_full_name):
        """
        The experiment generates pre- and post- spike trains for Hermit
        distributions with peeks at 0.1 and 0.2 respectively. The synaptic
        input variables are clipped to the range [0, spike_magnitude].
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
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
    def pre_1_post_3_clipped(my_precomputed_full_name):
        """
        The experiment generates pre- and post- spike trains for Hermit
        distributions with peeks at 0.1 and 0.3 respectively. The synaptic
        input variables are clipped to the range [0, spike_magnitude].
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
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
    def pre_2_post_1_clipped(my_precomputed_full_name):
        """
        The experiment generates pre- and post- spike trains for Hermit
        distributions with peeks at 0.2 and 0.1 respectively. The synaptic
        input variables are clipped to the range [0, spike_magnitude].
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
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
    def pre_3_post_1_clipped(my_precomputed_full_name):
        """
        The experiment generates pre- and post- spike trains for Hermit
        distributions with peeks at 0.3 and 0.1 respectively. The synaptic
        input variables are clipped to the range [0, spike_magnitude].
        """
        return PrePostSpikeNoisesDifferences(
            name=my_precomputed_full_name,
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
    return experiment_info["function_ptr"](experiment_info["name"])
