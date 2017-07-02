import os
import shutil
import numpy
from config import output_root_dir
import plot
import distribution
import spike_train
import synapse


class TestInfo:
    def __init__(self, test_name, output_dir):
        self.test_name = test_name
        self.output_dir = output_dir


def _test_distribution(info):
    """
    The test builds 5 distributions. Four of them are with numeric events
    and one is with string events. For each distribution there is generated
    a sequence of events and then histogram of counts of the events is
    build, which should match the histogram the distribution was built from.
    """
    assert isinstance(info, TestInfo)

    def doit(hist, n, ofile):
        xhist = hist.copy()
        for k in xhist.keys():
            xhist[k] = 0
        isi = distribution.distribution(hist)
        print(isi)
        ofile.write(str(isi) + "\n")
        for _ in range(n):
            e = isi.next_event()
            assert e in hist.keys()
            xhist[e] += 1
        osum = 0.0
        xsum = 0.0
        for k in hist.keys():
            osum += hist[k]
            xsum += xhist[k]
        if xsum > 0:
            for k in xhist.keys():
                xhist[k] *= osum / xsum
        return xhist

    def show(hist, xhist, ofile):
        for k in sorted(hist.keys()):
            msg = str(k) + ": " + str(hist[k]) + " ; " + str(xhist[k])
            print(msg)
            ofile.write(msg + "\n")

    with open(os.path.join(info.output_dir, "results.txt"), "w") as ofile:
        for hist in [
                {1: 1},
                {123: 10},
                {1: 1, 2: 1, 3: 1, 4: 1, 5: 1},
                {"A": 2, "B": 4, "C": 10, "D": 2, "E": 3},
                {10: 60, 20: 100, 30: 65, 40: 35, 50: 20, 60: 10, 70: 5, 80: 3, 90: 2, 100: 1}
                ]:
            print("*******************************************************")
            ofile.write("*******************************************************\n")
            show(hist, doit(hist, 10000, ofile), ofile)
    return 0


def _test_hermit_distribution(info):
    """
    The test build 10 hermit distributions, each for a different
    peek. Curves and histograms and normalised histograms are build
    and corresponding plots are saved.
    """
    assert isinstance(info, TestInfo)
    print("  Generating graph " + os.path.join(info.output_dir, "ns_curve.png"))
    plot.curve(
        distribution.make_points_of_normal_distribution(
            num_points=100,
            nu=0.0,
            sigma=1.0,
            normalise=True
            ),
        os.path.join(info.output_dir, "ns_curve.png")
        )
    for peek_x in numpy.arange(0.1, 0.95, 0.1, float):
        print("  Computing points of hermit cubic at peek " + str(peek_x) + ".")
        points = distribution.move_scale_curve_points(
                    distribution.make_points_of_hermit_cubic_approximation_of_normal_distribution(
                        peek_x=peek_x,
                        mult_m01=1.0,
                        mult_mx=1.0,
                        num_points=100
                        ),
                    scale_x=0.298,
                    scale_y=10,
                    pow_y=1.5,
                    shift_x=0.002
                    )
        print("  Saving " + os.path.join(info.output_dir, "ns_curve_hermit_adapted_" + format(peek_x, ".2f") + ".png"))
        plot.curve(
            points,
            os.path.join(info.output_dir, "ns_curve_hermit_adapted_" + format(peek_x, ".2f") + ".png")
            )
        print("  Computing histogram from the hermit cubic.")
        hist = distribution.mkhist_from_curve_points(points, num_bars=300)
        print("  Saving " + os.path.join(info.output_dir, "ns_hist_adapted_" + format(peek_x, ".2f") + ".png"))
        plot.histogram(
            hist,
            os.path.join(info.output_dir, "ns_hist_adapted_" + format(peek_x, ".2f") + ".png"),
            normalised=False
            )
        print("  Saving " + os.path.join(info.output_dir, "ns_hist_normalised_adapted_" +
                                         format(peek_x, ".2f") + ".png"))
        plot.histogram(
            hist,
            os.path.join(info.output_dir, "ns_hist_normalised_adapted_" + format(peek_x, ".2f") + ".png"),
            normalised=True
            )
        print("  Saving " + os.path.join(info.output_dir, "hd_" + format(peek_x, ".2f") + ".png"))
        plot.histogram(
            distribution.hermit_distribution(peek_x),
            os.path.join(info.output_dir, "hd_" + format(peek_x, ".2f") + ".png"),
            )

    return 0


def _test_synapse(info):
    """
    The test checks functionality of all four supported plastic
    synapses (namely, 'np','pn','pp', and 'nn'). For each type
    there are generated plots of progress of input variables,
    synaptic weights, and changes of weights w.r.t pre- and
    post- spikes.
    """
    assert isinstance(info, TestInfo)
    for the_synapse in [
            synapse.synapse.plastic_peek_np(),
            synapse.synapse.plastic_peek_pn(),
            synapse.synapse.plastic_peek_pp(),
            synapse.synapse.plastic_peek_nn(),
            ]:
        print("  Starting simulation of '" + the_synapse.get_name() + "'.")

        start_time = 0.0
        dt = 0.001
        nsteps = 2000

        pre_spikes_train = spike_train.SpikeTrain.create(distribution.get_standard_spike_noise(), 0.0)
        post_spikes_train = spike_train.SpikeTrain.create(distribution.get_standard_spike_noise(), 0.0)
        # pre_spikes_train = spike_train.spike_train(
        #     distribution.distribution({}),
        #     [0.001],
        #     start_time
        #     )
        # post_spikes_train = spike_train.spike_train(
        #     distribution.distribution({}),
        #     [0.002],
        #     start_time
        #     )

        synapse_recording = {"last_pre_times": [], "last_post_times": []}
        for var, value in the_synapse.get_variables().items():
            synapse_recording[var] = []

        last_pre_spike_time = start_time
        last_post_spike_time = start_time
        t = start_time
        for step in range(nsteps):
            print("    " + format(100.0 * step / float(nsteps), '.1f') + "%", end='\r')

            the_synapse.integrate(dt)

            was_pre_spike_generated = pre_spikes_train.on_time_step(t, dt)
            if was_pre_spike_generated:
                the_synapse.on_pre_synaptic_spike()
                last_pre_spike_time = t + dt
            was_post_spike_generated = post_spikes_train.on_time_step(t, dt)
            if was_post_spike_generated:
                the_synapse.on_post_synaptic_spike()
                last_post_spike_time = t + dt

            for var, value in the_synapse.get_variables().items():
                synapse_recording[var].append((t + dt, value))
            synapse_recording["last_pre_times"].append(last_pre_spike_time)
            synapse_recording["last_post_times"].append(last_post_spike_time)

            t += dt

        print("  Saving results.")

        output_dir = os.path.join(info.output_dir, the_synapse.get_name())

        for var in the_synapse.get_variables().keys():
            pathname = os.path.join(output_dir, "synapse_var_" + var + ".png")
            if var == the_synapse.get_weight_variable_name():
                title = the_synapse.get_short_description()
            else:
                title = None
            print("    Saving plot " + pathname)
            plot.curve(
                synapse_recording[var],
                pathname,
                title=title,
                colours="C1"
                )

        weights_delta = []
        for i in range(1, len(synapse_recording[the_synapse.get_weight_variable_name()])):
            t, w = synapse_recording[the_synapse.get_weight_variable_name()][i]
            pre_t = synapse_recording["last_pre_times"][i]
            post_t = synapse_recording["last_post_times"][i]
            w0 = synapse_recording[the_synapse.get_weight_variable_name()][i - 1][1]
            weights_delta.append((post_t - pre_t, w - w0))
        weights_delta.sort(key=lambda pair: pair[0])

        pathname = os.path.join(output_dir, "plasticity.png")
        print("    Saving plot " + pathname)
        plot.scatter(
            distribution.make_sum_points(weights_delta, dt),
            pathname,
            xaxis_name="post_t - pre_t",
            faxis_name="weight delta"
            )

    return 0


def _test_spike_trains(info):
    """
    The test generates several excitatory and inhibitory spike strains.
    Each excitatory/inhibitory spike train differs from another one by
    a different level of noise in time intervals between individual
    spikes. Nevertheless, spiking distribution is preserved for each
    spike train for any chosen level of noise.
    """
    assert isinstance(info, TestInfo)

    start_time = 0.0
    dt = 0.001
    nsteps = 60000

    num_spikers_per_kind = 10

    plot_dt = 2.0
    plot_parts = 10
    plot_stride = max(1, int((nsteps * dt / plot_dt) / plot_parts + 0.5))

    def index_to_param(i):
        return min(max(0.0, float(i) / float(num_spikers_per_kind - 1)), 1.0) if num_spikers_per_kind > 1 else 1.0

    trains = [spike_train.SpikeTrain.create(distribution.default_excitatory_isi_distribution(), index_to_param(i))
              for i in range(num_spikers_per_kind)] +\
             [spike_train.SpikeTrain.create(distribution.default_inhibitory_isi_distribution(), index_to_param(i))
              for i in range(num_spikers_per_kind)]

    t = start_time
    for step in range(nsteps):
        print("    " + format(100.0 * step / float(nsteps), '.1f') + "%", end='\r')
        for i in range(2 * num_spikers_per_kind):
            trains[i].on_time_step(t, dt)
        t += dt

    print("  Saving results.")

    for idx_base, typename in [(0, "excitatory"), (num_spikers_per_kind, "inhibitory")]:
        for idx_shift in range(0, num_spikers_per_kind, num_spikers_per_kind // min(plot_parts, num_spikers_per_kind)):
            idx = idx_base + idx_shift
            signal = trains[idx].get_spikes_history()

            isi_delta_signal_points = [(i - 2, (signal[i] - signal[i - 1]) - (signal[i - 1] - signal[i - 2]))
                                       for i in range(2, len(signal))]

            pathname = os.path.join(info.output_dir, "isi_delta_" + typename + "_" + str(idx_shift) + "_curve",
                                                     "isi_delta_" + typename + "_" + str(idx_shift) + ".png")
            print("    Saving plot " + pathname)
            plot.curve_per_partes(
                isi_delta_signal_points,
                pathname,
                0,
                len(isi_delta_signal_points),
                1000,
                lambda p: print("    Saving plot " + p),
                )

            pathname = os.path.join(info.output_dir, "isi_delta_" + typename + "_" + str(idx_shift) + "_hist.png")
            print("    Saving plot " + pathname)
            plot.histogram(distribution.make_times_histogram([p[1] for p in isi_delta_signal_points], dt),
                           pathname, normalised=False)

            pathname = os.path.join(info.output_dir, "isi_hist_" + typename + "_" + str(idx_shift) + ".png")
            print("    Saving plot " + pathname)
            plot.histogram(distribution.make_isi_histogram(signal, dt), pathname, normalised=False)

        pathname = os.path.join(info.output_dir, "isi_distribution_" + typename + "_0.png")
        print("    Saving plot " + pathname)
        plot.histogram(
            trains[idx_base].get_spiking_distribution(),
            pathname,
            normalised=True
            )

    pathname = os.path.join(info.output_dir, "spikes_board", "spikes.png")
    print("    Saving plot " + pathname)
    plot.scatter_per_partes(
        [(event, i) for i in range(2 * num_spikers_per_kind) for event in trains[i].get_spikes_history()],
        pathname,
        start_time,
        start_time + nsteps * dt,
        plot_dt,
        plot_stride,
        lambda p: print("    Saving plot " + p),
        ["C0" if i < num_spikers_per_kind else "C2"
         for i in range(2 * num_spikers_per_kind) for _ in trains[i].get_spikes_history()]
        )

    return 0


####################################################################################################
####################################################################################################
####################################################################################################


_automatically_registered_tests_ = sorted([{"name": elem.__name__[len("_test_"):],
                                            "function_ptr": elem,
                                            "description": str(elem.__doc__)}
                                           for elem in list(map(eval, dir()))
                                           if callable(elem) and elem.__name__.startswith("_test_")],
                                          key=lambda x: x["name"])


def get_registered_tests():
    return _automatically_registered_tests_


def run_test(test_info):
    assert isinstance(test_info, dict)
    assert "function_ptr" in test_info and callable(test_info["function_ptr"])
    assert "name" in test_info
    print("Starting test '" + test_info["name"] + "':")
    out_dir = os.path.join(os.path.join(output_root_dir(), "tests", test_info["name"]))
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir)
    os.makedirs(out_dir)
    retval = test_info["function_ptr"](TestInfo(test_info["name"], out_dir))
    print("The test has finished " + ("successfully" if retval == 0 else "with an error") + ".")
    return retval
