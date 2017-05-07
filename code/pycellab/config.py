import os
import soma
import integrator
import distribution


def __get_my_dir():
    return os.path.dirname(__file__)


def __output_root_dir():
    if str(__get_my_dir()).replace("\\", "/").endswith("E2/code/pycellab"):
        return os.path.normpath(os.path.join(__get_my_dir(), "..", "..", "dist", "evaluation", "pycellab"))
    else:
        return __get_my_dir()


class configuration:
    def __init__(self,
                 start_time,
                 dt,
                 nsteps,
                 cell_soma,
                 excitatory_noise_distributions,
                 inhibitory_noise_distributions,
                 excitatory_weights,
                 inhibitory_weights,
                 output_dir,
                 plot_files_extension,
                 description
                 ):
        assert dt > 0.0
        assert nsteps >= 0
        assert isinstance(cell_soma, list)
        assert isinstance(excitatory_weights, list)
        assert isinstance(inhibitory_weights, list)
        assert len(cell_soma) == len(excitatory_weights) and len(cell_soma) == len(inhibitory_weights)
        assert False not in list(map(lambda x: len(x) == len(excitatory_noise_distributions), excitatory_weights))
        assert False not in list(map(lambda x: len(x) == len(inhibitory_noise_distributions), inhibitory_weights))
        assert plot_files_extension.lower() == ".svg" or plot_files_extension.lower() == ".png"
        self.start_time = start_time
        self.dt = dt
        self.nsteps = nsteps
        self.cell_soma = cell_soma
        self.excitatory_noise_distributions = excitatory_noise_distributions
        self.inhibitory_noise_distributions = inhibitory_noise_distributions
        self.excitatory_weights = excitatory_weights
        self.inhibitory_weights = inhibitory_weights
        self.output_dir = output_dir
        self.plot_files_extension = plot_files_extension.lower()
        self.name = os.path.basename(self.output_dir)
        self.description = description
        self.are_equal_excitatory_noise_distributions = len(self.excitatory_noise_distributions) > 0
        for d in self.excitatory_noise_distributions:
            if d.get_histogram() != self.excitatory_noise_distributions[0].get_histogram():
                self.are_equal_excitatory_noise_distributions = False
                break
        self.are_equal_inhibitory_noise_distributions = len(self.inhibitory_noise_distributions) > 0
        for d in self.inhibitory_noise_distributions:
            if d.get_histogram() != self.inhibitory_noise_distributions[0].get_histogram():
                self.are_equal_inhibitory_noise_distributions = False
                break
        self.are_equal_noise_distributions = (
            self.are_equal_excitatory_noise_distributions and
            self.are_equal_inhibitory_noise_distributions and
            self.excitatory_noise_distributions[0].get_histogram() ==
                self.inhibitory_noise_distributions[0].get_histogram()
            )


def leaky_integrate_and_fire_const_input():
    dt = 0.001
    return configuration(
        start_time=0.0,
        dt=dt,
        nsteps=1000,
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
        excitatory_noise_distributions=[distribution.distribution({1.0*dt: 1.0})],
        inhibitory_noise_distributions=[],
        excitatory_weights=[[1.0]],
        inhibitory_weights=[[]],
        output_dir=os.path.join(__output_root_dir(), "leaky_integrate_and_fire_const_input"),
        plot_files_extension=".png",
        description="A simulation of a 'leaky integrate and fire' neuron with a constant input current. "
                    "strong enough to trigger constant firing of the neuron."
        )


def leaky_integrate_and_fire_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
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
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_weights=[[1.0 for _ in range(num_excitatory)]],
        inhibitory_weights=[[1.0 for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "leaky_integrate_and_fire_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        description="A simulation of a 'leaky integrate and fire' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1."
        )


def izhikevich_regular_spiking_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
        cell_soma=[soma.izhikevich.regular_spiking(spike_magnitude=0.15)],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_weights=[[1.0 for _ in range(num_excitatory)]],
        inhibitory_weights=[[1.0 for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "izhikevich_regular_spiking_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        description="A simulation of a 'izhikevich regular spiking' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1. Spike magnitude is 0.15. This neuron typically creates excitatory "
                    "synapses to other neurons. These neurons are the most common in the cortex."
        )


def izhikevich_chattering_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
        cell_soma=[soma.izhikevich.chattering(spike_magnitude=0.15)],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_weights=[[1.0 for _ in range(num_excitatory)]],
        inhibitory_weights=[[1.0 for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "izhikevich_chattering_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        description="A simulation of a 'izhikevich chattering' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1. Spike magnitude is 0.15. This neuron typically creates excitatory "
                    "synapses to other neurons."
        )


def izhikevich_fast_spiking_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
        cell_soma=[soma.izhikevich.fast_spiking(spike_magnitude=0.15)],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_weights=[[1.0 for _ in range(num_excitatory)]],
        inhibitory_weights=[[1.0 for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "izhikevich_fast_spiking_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        description="A simulation of a 'izhikevich fast spiking' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1. Spike magnitude is 0.15. This neuron typically creates inhibitory "
                    "synapses to other neurons."
        )


def hodgkin_huxley_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
        cell_soma=[soma.hodgkin_huxley(
            spike_magnitude=0.15,
            integrator_function=integrator.midpoint
            )],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_weights=[[1.0 for _ in range(num_excitatory)]],
        inhibitory_weights=[[1.0 for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "hodgkin_huxley_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        description="A simulation of a 'hodgkin-huxley' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1. Spike magnitude is 0.15."
        )


def development():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=100,
        cell_soma=[
            soma.izhikevich.default(spike_magnitude=0.15),
            soma.izhikevich.fast_spiking(spike_magnitude=0.15),
            soma.izhikevich.chattering(spike_magnitude=0.15)
            ],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_weights=[
            [1.0 for _ in range(num_excitatory)],
            [1.0 for _ in range(num_excitatory)],
            [1.0 for _ in range(num_excitatory)]
            ],
        inhibitory_weights=[
            [1.0 for _ in range(num_inhibitory)],
            [1.0 for _ in range(num_inhibitory)],
            [1.0 for _ in range(num_inhibitory)]
            ],
        output_dir=os.path.join(__output_root_dir(), "development"),
        plot_files_extension=".png",
        description="This is not a genuine configuration. It serves only for development, testing, and "
                    "bug-fixing of this evaluation system."
        )


def get_registered_configurations():
    return sorted([
        leaky_integrate_and_fire_const_input(),
        leaky_integrate_and_fire_input_800ex_200in_std_noise(),
        izhikevich_regular_spiking_input_800ex_200in_std_noise(),
        izhikevich_chattering_input_800ex_200in_std_noise(),
        izhikevich_fast_spiking_input_800ex_200in_std_noise(),
        hodgkin_huxley_input_800ex_200in_std_noise(),
        development()
        ], key=lambda cfg: cfg.output_dir)


###########################################################################################################
###########################################################################################################
###########################################################################################################


# import plot
# def __dbg():
#     distrib = distribution.get_standard_spike_noise()
#
#     # for k in sorted(distrib.get_histogram().keys()):
#     #     print(str(k) + ": " + format(distrib.get_histogram()[k], ".4f") + ",")
#
#     plot.histogram(
#         distrib,
#         os.path.join(__output_root_dir(), "__dbg", "poisson_noise_distribution.svg"),
#         normalised=True
#         )
#
#     # samples = distrib.generate(10000)
#     # print("median = " + str(numpy.median(samples)))
#     # print("mean = " + str(numpy.mean(samples)))
#     # print("std = " + str(numpy.std(samples)))
#     # print("cv = " + str(numpy.std(samples) / numpy.mean(samples)))
#
#     # print("***************")
#
#     # n = 10000
#     # samples = distrib.generate(n)
#     # assert len(samples) == n
#     # time = 0.0
#     # for t in samples:
#     #     time += t
#     # print("#spikes = " + str(n))
#     # print("time = " + str(time))
#     # print("average spiking rate = " + str(n / time))

