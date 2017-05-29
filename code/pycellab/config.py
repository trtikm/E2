import os
import soma
import synapse
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
                 num_sub_iterations,
                 cell_soma,
                 excitatory_noise_distributions,
                 inhibitory_noise_distributions,
                 excitatory_synapses,
                 inhibitory_synapses,
                 output_dir,
                 plot_files_extension,
                 plot_time_step,
                 description
                 ):
        assert dt > 0.0
        assert nsteps >= 0
        assert isinstance(num_sub_iterations, list)
        assert False not in list(map(lambda x: type(x) is int and x > 0, num_sub_iterations))
        assert isinstance(cell_soma, list)
        assert isinstance(excitatory_synapses, list)
        assert isinstance(inhibitory_synapses, list)
        assert len(cell_soma) == len(excitatory_synapses) and len(cell_soma) == len(inhibitory_synapses)
        assert len(cell_soma) == len(num_sub_iterations)
        assert False not in list(map(lambda x: len(x) == len(excitatory_noise_distributions), excitatory_synapses))
        assert False not in list(map(lambda x: len(x) == len(inhibitory_noise_distributions), inhibitory_synapses))
        assert plot_files_extension.lower() == ".svg" or plot_files_extension.lower() == ".png"
        assert plot_time_step > 0.0

        self.start_time = start_time
        self.dt = dt
        self.nsteps = nsteps
        self.num_sub_iterations = num_sub_iterations
        self.cell_soma = cell_soma
        self.excitatory_noise_distributions = excitatory_noise_distributions
        self.inhibitory_noise_distributions = inhibitory_noise_distributions
        self.excitatory_synapses = excitatory_synapses
        self.inhibitory_synapses = inhibitory_synapses
        self.output_dir = output_dir
        self.plot_files_extension = plot_files_extension.lower()
        self.name = os.path.basename(self.output_dir)
        self.plot_time_step = plot_time_step
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
        excitatory_noise_distributions=[distribution.distribution({1.0*dt: 1.0})],
        inhibitory_noise_distributions=[],
        excitatory_synapses=[[synapse.synapse.constant()]],
        inhibitory_synapses=[[]],
        output_dir=os.path.join(__output_root_dir(), "leaky_integrate_and_fire_const_input"),
        plot_files_extension=".png",
        plot_time_step=1.0,
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
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.constant() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.constant() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "leaky_integrate_and_fire_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
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
        num_sub_iterations=[1],
        cell_soma=[soma.izhikevich.regular_spiking(spike_magnitude=0.15)],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.constant() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.constant() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "izhikevich_regular_spiking_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
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
        num_sub_iterations=[1],
        cell_soma=[soma.izhikevich.chattering(spike_magnitude=0.15)],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.constant() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.constant() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "izhikevich_chattering_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
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
        num_sub_iterations=[1],
        cell_soma=[soma.izhikevich.fast_spiking(spike_magnitude=0.15)],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.constant() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.constant() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "izhikevich_fast_spiking_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
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
        num_sub_iterations=[100],
        cell_soma=[soma.hodgkin_huxley(
            spike_magnitude=0.15,
            integrator_function=integrator.midpoint
            )],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.constant() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.constant() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "hodgkin_huxley_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
        description="A simulation of a 'hodgkin-huxley' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1. Spike magnitude is 0.15."
        )


def wilson_reguar_spiking_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
        num_sub_iterations=[100],
        cell_soma=[soma.wilson.regular_spiking()],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.constant() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.constant() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "wilson_reguar_spiking_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
        description="A simulation of a 'wilson's regular spiking' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1. Spike magnitude is 1.0."
        )


def wilson_fast_spiking_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
        num_sub_iterations=[100],
        cell_soma=[soma.wilson.fast_spiking()],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.constant() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.constant() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "wilson_fast_spiking_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
        description="A simulation of a 'wilson's fast spiking' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1. Spike magnitude is 1.0."
        )


def wilson_bursting_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
        num_sub_iterations=[100],
        cell_soma=[soma.wilson.bursting()],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.constant() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.constant() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "wilson_bursting_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
        description="A simulation of a 'wilson's bursting' neuron with a 800 excitatory and 200 "
                    "inhibitory input spike trains, all with the standard noise distribution. Weights of "
                    "all synapses is 1. Spike magnitude is 1.0."
        )


def izhikevich_hodgkin_huxley_wison_input_800ex_200in_std_noise():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
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
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[
            [synapse.synapse.constant() for _ in range(num_excitatory)],
            [synapse.synapse.constant() for _ in range(num_excitatory)],
            [synapse.synapse.constant() for _ in range(num_excitatory)],
            [synapse.synapse.constant() for _ in range(num_excitatory)],
            [synapse.synapse.constant() for _ in range(num_excitatory)]
            ],
        inhibitory_synapses=[
            [synapse.synapse.constant() for _ in range(num_inhibitory)],
            [synapse.synapse.constant() for _ in range(num_inhibitory)],
            [synapse.synapse.constant() for _ in range(num_inhibitory)],
            [synapse.synapse.constant() for _ in range(num_inhibitory)],
            [synapse.synapse.constant() for _ in range(num_inhibitory)]
            ],
        output_dir=os.path.join(__output_root_dir(), "izhikevich_hodgkin_huxley_wison_input_800ex_200in_std_noise"),
        plot_files_extension=".png",
        plot_time_step=1.0,
        description="A simulation of three neurons 'izhikevich regular spiking', 'izhikevich fast spiking', "
                    "'hodgkin-huxley', 'wilson regular spiking', and 'wilson fast spiking', with common spike "
                    "input trains 800 excitatory and 200 inhibitory, all with the standard noise distribution. "
                    "Initial weights of all synapses of all neurons are 1. Spike magnitude for all neurons is 0.15, "
                    "except for both wilson neurons for which it is 1.0."
        )


def development():
    num_excitatory = 800
    num_inhibitory = 200
    excitatory_noise = distribution.get_standard_spike_noise()
    inhibitory_noise = excitatory_noise
    return configuration(
        start_time=0.0,
        dt=0.001,
        nsteps=1000,
        num_sub_iterations=[1],
        cell_soma=[soma.izhikevich.regular_spiking(spike_magnitude=0.15)],
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_synapses=[[synapse.synapse.plastic_peek_np() for _ in range(num_excitatory)]],
        inhibitory_synapses=[[synapse.synapse.plastic_peek_pn() for _ in range(num_inhibitory)]],
        output_dir=os.path.join(__output_root_dir(), "development"),
        plot_files_extension=".png",
        plot_time_step=1.0,
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
        wilson_reguar_spiking_input_800ex_200in_std_noise(),
        wilson_fast_spiking_input_800ex_200in_std_noise(),
        wilson_bursting_input_800ex_200in_std_noise(),
        izhikevich_hodgkin_huxley_wison_input_800ex_200in_std_noise(),
        development()
        ], key=lambda cfg: cfg.output_dir)


###########################################################################################################
###########################################################################################################
###########################################################################################################


# import plot
# import numpy
# import matplotlib.pyplot as plt
#
#
# def __Xget_standard_spike_noise():
#     s = numpy.random.poisson(15, 100000)
#     count, bins, ignored = plt.hist(s, 500, normed=True)
#     plt.close()
#     n = min([len(count), len(bins)])
#     assert n >= 300
#     hist = {}
#     for i in range(n):
#         assert bins[i] not in hist
#         hist[float(bins[i])] = float(count[i]) # float(abs(count[i] - 1.0 * (1.0 - numpy.sqrt(1 + count[i]))))
#     xhist = {}
#     keys = sorted(hist.keys())
#     for idx in range(0, min(300, len(keys))):
#         if idx < 5:
#             value = 0.0
#         elif idx < 10:
#             x = (idx - 5.0) / 10.0
#             assert x >= 0.0 and x <= 1.0
#             value = hist[keys[10]] * (3.0 * x**2 - 2.0 * x**3)
#         else:
#             value = hist[keys[idx]]
#         xhist[0.001 * (idx + 2)] = value
#         idx += 1
#     return distribution.distribution(xhist)
#
#
# def __dbg():
#     hist = distribution.mkhist(numpy.random.power(5.0, 10000))
#
#     plt.stem([x for x in sorted(hist.keys())],
#              [hist[x] for x in sorted(hist.keys())],
#              linefmt="b-", markerfmt="b-",
#              basefmt="None")
#
#     # distrib = __Xget_standard_spike_noise()
#     # plt.stem([x for x, _ in distrib.get_points()],
#     #          [fx for _, fx in distrib.get_points()],
#     #          linefmt="b-", markerfmt="b-", basefmt="None")
#
#     # s = numpy.random.binomial(10, 0.5, 100000)
#     # count, bins, ignored = plt.hist(s, 150, normed=True)
#
#     # s = numpy.random.exponential(1, 100000)
#     # count, bins, ignored = plt.hist(s, 500, normed=False)
#     plt.show()
#
#     # s = numpy.random.poisson(1, 100000)
#     # count, bins, ignored = plt.hist(s, 500, normed=True)
#     #
#     #
#     # distrib = distribution.get_standard_spike_noise()
#
#     # for k in sorted(distrib.get_histogram().keys()):
#     #     print(str(k) + ": " + format(distrib.get_histogram()[k], ".4f") + ",")
#
#     # plot.histogram(
#     #     distrib,
#     #     os.path.join(__output_root_dir(), "__dbg", "poisson_noise_distribution.svg"),
#     #     normalised=True
#     #     )
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
#
