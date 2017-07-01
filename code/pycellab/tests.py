import os
import shutil
import numpy
from config import output_root_dir
import plot
import distribution
import spike_train
import signalling
import synapse


def _test_distribution(my_precomputed_full_name):
    """ The test _test_distribution """
    def doit(hist, n):
        xhist = hist.copy()
        for k in xhist.keys():
            xhist[k] = 0
        isi = distribution.distribution(hist)
        print(isi)
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

    def show(hist, xhist):
        for k in sorted(hist.keys()):
            print(str(k) + ": " + str(hist[k]) + " ; " + str(xhist[k]))

    print("Starting test '" + my_precomputed_full_name + "':")
    for hist in [
            {1: 1},
            {123: 10},
            {1: 1, 2: 1, 3: 1, 4: 1, 5: 1},
            {"A": 2, "B": 4, "C": 10, "D": 2, "E": 3},
            {10: 60, 20: 100, 30: 65, 40: 35, 50: 20, 60: 10, 70: 5, 80: 3, 90: 2, 100: 1}
            ]:
        print("*******************************************************")
        show(hist, doit(hist, 10000))
    print("Done.")


def _test_hermit_distribution(my_precomputed_full_name):
    """ The test _test_hermit_distribution """
    print("Starting test '" + my_precomputed_full_name + "':")
    out_dir = os.path.join(output_root_dir(), my_precomputed_full_name)
    print("  Generating graph " + os.path.join(out_dir, "ns_curve.png"))
    plot.curve(
        distribution.make_points_of_normal_distribution(
            num_points=100,
            nu=0.0,
            sigma=1.0,
            normalise=True
            ),
        os.path.join(out_dir, "ns_curve.png")
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
        print("  Saving " + os.path.join(out_dir, "ns_curve_hermit_adapted_" + format(peek_x, ".2f") + ".png"))
        plot.curve(
            points,
            os.path.join(out_dir, "ns_curve_hermit_adapted_" + format(peek_x, ".2f") + ".png")
            )
        print("  Computing histogram from the hermit cubic.")
        hist = distribution.mkhist_from_curve_points(points, num_bars=300)
        print("  Saving " + os.path.join(out_dir, "ns_hist_adapted_" + format(peek_x, ".2f") + ".png"))
        plot.histogram(
            hist,
            os.path.join(out_dir, "ns_hist_adapted_" + format(peek_x, ".2f") + ".png"),
            normalised=False
            )
        print("  Saving " + os.path.join(out_dir, "ns_hist_normalised_adapted_" + format(peek_x, ".2f") + ".png"))
        plot.histogram(
            hist,
            os.path.join(out_dir, "ns_hist_normalised_adapted_" + format(peek_x, ".2f") + ".png"),
            normalised=True
            )
        print("  Saving " + os.path.join(out_dir, "hd_" + format(peek_x, ".2f") + ".png"))
        plot.histogram(
            distribution.hermit_distribution(peek_x),
            os.path.join(out_dir, "hd_" + format(peek_x, ".2f") + ".png"),
            )

    print("Done.")


def _test_synapse(my_precomputed_full_name):
    """This is test _test_synapse"""
    print("Starting test '" + my_precomputed_full_name + "':")

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

        pre_spikes_train = spike_train.spike_train(distribution.get_standard_spike_noise(), None, start_time)
        post_spikes_train = spike_train.spike_train(distribution.get_standard_spike_noise(), None, start_time)
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

        output_dir = os.path.join(output_root_dir(), my_precomputed_full_name, the_synapse.get_name())
        os.makedirs(output_dir, exist_ok=True)

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

    print("Done.")


def _test_data_signal_constant_isi(my_precomputed_full_name):
    """This is test 'data_signal_constant_isi'."""
    print("Starting test '" + my_precomputed_full_name + "':")

    start_time = 0.0
    dt = 0.001
    nsteps = 1000
    train = spike_train.spike_train(distribution.distribution(None),
                                    signalling.DataSignal.constant_isi(0.025, start_time),
                                    start_time)
    t = start_time
    for step in range(nsteps):
        print("    " + format(100.0 * step / float(nsteps), '.1f') + "%", end='\r')
        train.on_time_step(t, dt)
        t += dt

    print("  Saving results.")

    output_dir = os.path.join(output_root_dir(), my_precomputed_full_name)
    os.makedirs(output_dir, exist_ok=True)

    pathname = os.path.join(output_dir, "spikes_board.png")
    print("    Saving plot " + pathname)
    plot.scatter(
        [(event, 0.0) for event in train.get_spikes()],
        pathname,
        ['C4' if is_data_spike else 'C0' for is_data_spike in train.get_is_data_signal_flags()],
        )

    print("Done.")


def _test_data_signal_default_excitatory(my_precomputed_full_name):
    """This is test 'data_signal_default_excitatory'."""
    print("Starting test '" + my_precomputed_full_name + "':")

    # output_dir = os.path.join(output_root_dir(), my_precomputed_full_name)
    # os.makedirs(output_dir, exist_ok=True)
    #
    # pathname = os.path.join(output_dir, "xe_isi_hist.png")
    # print("    Saving plot " + pathname)
    # plot.histogram(
    #     distribution.hermit_distribution(0.137, pow_y=2),
    #     pathname,
    #     normalised=True
    #     )
    #
    # pathname = os.path.join(output_dir, "xi_isi_hist.png")
    # print("    Saving plot " + pathname)
    # plot.histogram(
    #     distribution.hermit_distribution(0.0918, 0.08, pow_y=2),
    #     pathname,
    #     normalised=True
    #     )
    #
    # pathname = os.path.join(output_dir, "xi_counts.png")
    # print("    Saving plot " + pathname)
    # plot.histogram(
    #     dict((x, (0.08 - 0.002) / x) for x in numpy.arange(0.002, 0.08, 0.001)),
    #     pathname,
    #     normalised=False
    #     )
    #
    # print("Done.")
    # return

    start_time = 0.0
    dt = 0.001
    nsteps = 60000
    train = spike_train.spike_train(distribution.hermit_distribution(0.25),
                                    signalling.DataSignal.default_excitatory(start_time),
                                    start_time)
    t = start_time
    for step in range(nsteps):
        print("    " + format(100.0 * step / float(nsteps), '.1f') + "%", end='\r')
        train.on_time_step(t, dt)
        t += dt

    print("  Saving results.")

    output_dir = os.path.join(output_root_dir(), my_precomputed_full_name)
    os.makedirs(output_dir, exist_ok=True)

    pathname = os.path.join(output_dir, "isi_hist.png")
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(train.get_spikes(), dt),
        pathname,
        normalised=False
        )
    if False:
        pathname = os.path.join(output_dir, "spikes_board", "spikes.png")
        print("    Saving plot " + pathname)
        plot.scatter_per_partes(
            [(event, 0.0) for event in train.get_spikes()],
            pathname,
            start_time,
            start_time + nsteps * dt,
            1.0,
            max(1, nsteps // 10),
            lambda p: print("    Saving plot " + p),
            ['C4' if is_data_spike else 'C0' for is_data_spike in train.get_is_data_signal_flags()],
            )

    print("Done.")


def _test_data_signal_ex(my_precomputed_full_name):
    """This is test 'data_signal_ex'."""
    print("Starting test '" + my_precomputed_full_name + "':")

    start_time = 0.0
    dt = 0.001
    nsteps = 60000

    plot_dt = 1.0
    plot_parts = 10
    plot_stride = max(1, int((nsteps * dt / plot_dt) / plot_parts + 0.5))

    data_signal = signalling.DataSignalEx(signalling.ISIDistributionMatrix.create(0.0, 1.0))
    train = spike_train.spike_train(data_signal.get_distribution_matrix().get_events_distribution(), None, start_time)
    spikes_ex = []

    t = start_time
    for step in range(nsteps):
        print("    " + format(100.0 * step / float(nsteps), '.1f') + "%", end='\r')
        train.on_time_step(t, dt)
        if data_signal.on_time_step(t, dt):
            spikes_ex.append(t + dt)
        t += dt

    print("  Saving results.")

    output_dir = os.path.join(output_root_dir(), my_precomputed_full_name)
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir, exist_ok=True)

    pathname = os.path.join(output_dir, "hist_matrix_event_dist.png")
    print("    Saving plot " + pathname)
    plot.histogram(
        data_signal.get_distribution_matrix().get_events_distribution(),
        pathname,
        normalised=True
        )

    pathname = os.path.join(output_dir, "hist_matrix_distance_dist.png")
    print("    Saving plot " + pathname)
    plot.histogram(
        data_signal.get_distribution_matrix().get_distances_distribution(),
        pathname,
        normalised=True
        )

    idx = 0
    for event in sorted(data_signal.get_distribution_matrix().get_distributions_matrix().keys()):
        distrib = data_signal.get_distribution_matrix().get_distributions_matrix()[event]
        if idx % max(1, len(data_signal.get_distribution_matrix().get_distributions_matrix().keys()) / 10) == 0:
            pathname = os.path.join(output_dir, "matrix_distributions",
                                    "hist_matrix_hist_event_" + str(idx) + "_" + format(event, ".3f") + ".png")
            print("    Saving plot " + pathname)
            plot.histogram(
                distrib,
                pathname,
                normalised=True
                )
        idx += 1

    pathname = os.path.join(output_dir, "isi_hist.png")
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(train.get_spikes(), dt),
        pathname,
        normalised=False
        )

    pathname = os.path.join(output_dir, "isi_hist_ex.png")
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(spikes_ex, dt),
        pathname,
        normalised=False
        )

    pathname = os.path.join(output_dir, "spikes_board", "spikes.png")
    print("    Saving plot " + pathname)
    plot.scatter_per_partes(
        [(event, 0.0) for event in train.get_spikes()] + [(event, 1.0) for event in spikes_ex],
        pathname,
        start_time,
        start_time + nsteps * dt,
        plot_dt,
        plot_stride,
        lambda p: print("    Saving plot " + p)
        )

    print("Done.")


def _test_data_signal_fx(my_precomputed_full_name):
    """This is test 'data_signal_fx'."""
    print("Starting test '" + my_precomputed_full_name + "':")

    start_time = 0.0
    dt = 0.001
    nsteps = 60000

    num_spikers_per_kind = 10

    plot_dt = 2.0
    plot_parts = 10
    plot_stride = max(1, int((nsteps * dt / plot_dt) / plot_parts + 0.5))

    data_signals = [signalling.DataSignalFX.create_excitatory_by_param(
                        min(max(0.0, float(i) / float(num_spikers_per_kind - 1)), 1.0)
                        )
                    for i in range(num_spikers_per_kind)]

    # def interpol(i, coef=3.0):
    #     print("i=" + str(i))
    #     x = min(max(0.0, float(i) / float(num_spikers_per_kind - 1)), 1.0) if num_spikers_per_kind > 1 else 1.0
    #     return x**coef
    #
    # data_signals = [signalling.DataSignalFX.create_inhibitory(
    #                     array_size=1 + int(149.0 * interpol(i, 2.0) + 0.5),
    #                     min_chunk_size=1 + int(4.0 * interpol(i, 3.0) + 0.5),
    #                     max_chunk_size=1 + int(24.0 * interpol(i, 2.0) + 0.5),
    #                     chunk_frequency_function=lambda size: 1.0) for i in range(num_spikers_per_kind)]
    # exit(0)
    # data_signals = [signalling.DataSignalFX.create_excitatory() for _ in range(num_spikers_per_kind)]
    # data_signals = [signalling.DataSignalFX.create_plain_distribution(distribution.default_excitatory_isi_distribution())
    #                 for _ in range(num_spikers_per_kind)]
    trains = [spike_train.spike_train(data_signals[i].get_spiking_distribution(), None, start_time)
              for i in range(num_spikers_per_kind)]
    spikes_of_data_signals = [[] for _ in range(num_spikers_per_kind)]

    t = start_time
    for step in range(nsteps):
        print("    " + format(100.0 * step / float(nsteps), '.1f') + "%", end='\r')
        for i in range(num_spikers_per_kind):
            trains[i].on_time_step(t, dt)
            if data_signals[i].on_time_step(t, dt):
                spikes_of_data_signals[i].append(t + dt)
        t += dt

    print("  Saving results.")

    output_dir = os.path.join(output_root_dir(), my_precomputed_full_name)
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir, exist_ok=True)

    for idx in range(0, num_spikers_per_kind, num_spikers_per_kind // min(plot_parts, num_spikers_per_kind)):
        isi_delta_signal_points = [(i - 2, (spikes_of_data_signals[idx][i] - spikes_of_data_signals[idx][i - 1]) -
                                           (spikes_of_data_signals[idx][i - 1] - spikes_of_data_signals[idx][i - 2]))
                                   for i in range(2, len(spikes_of_data_signals[idx]))]

        pathname = os.path.join(output_dir, "isi_delta_signal_" + str(idx) + "_curve",
                                            "isi_delta_signal_" + str(idx) + ".png")
        print("    Saving plot " + pathname)
        plot.curve_per_partes(
            isi_delta_signal_points,
            pathname,
            0,
            len(isi_delta_signal_points),
            1000,
            lambda p: print("    Saving plot " + p),
            )

        pathname = os.path.join(output_dir, "isi_delta_signal_" + str(idx) + "_hist.png")
        print("    Saving plot " + pathname)
        plot.histogram(distribution.make_times_histogram([p[1] for p in isi_delta_signal_points], dt),
                       pathname, normalised=False)

        pathname = os.path.join(output_dir, "isi_hist_signal_" + str(idx) + ".png")
        print("    Saving plot " + pathname)
        plot.histogram(distribution.make_isi_histogram(spikes_of_data_signals[idx], dt), pathname, normalised=False)

    isi_delta_plain_0_points = [(i - 2, (trains[0].get_spikes()[i] - trains[0].get_spikes()[i - 1]) -
                                        (trains[0].get_spikes()[i - 1] - trains[0].get_spikes()[i - 2]))
                                for i in range(2, len(trains[0].get_spikes()))]

    pathname = os.path.join(output_dir, "isi_delta_plain_0_curve", "isi_delta_plain_0.png")
    print("    Saving plot " + pathname)
    plot.curve_per_partes(
        isi_delta_plain_0_points,
        pathname,
        0,
        len(isi_delta_plain_0_points),
        1000,
        lambda p: print("    Saving plot " + p),
        )

    pathname = os.path.join(output_dir, "isi_delta_plain_0_hist.png")
    print("    Saving plot " + pathname)
    plot.histogram(distribution.make_times_histogram([p[1] for p in isi_delta_plain_0_points], dt),
                   pathname, normalised=False)

    pathname = os.path.join(output_dir, "isi_hist_plain_0.png")
    print("    Saving plot " + pathname)
    plot.histogram(distribution.make_isi_histogram(trains[0].get_spikes(), dt), pathname, normalised=False)

    pathname = os.path.join(output_dir, "hist_event_dist_0.png")
    print("    Saving plot " + pathname)
    plot.histogram(
        data_signals[0].get_spiking_distribution(),
        pathname,
        normalised=True
        )

    pathname = os.path.join(output_dir, "spikes_board", "spikes.png")
    print("    Saving plot " + pathname)
    plot.scatter_per_partes(
        [(event, i) for i in range(num_spikers_per_kind) for event in trains[i].get_spikes()] +
        [(event, num_spikers_per_kind + i) for i in range(num_spikers_per_kind) for event in spikes_of_data_signals[i]],
        pathname,
        start_time,
        start_time + nsteps * dt,
        plot_dt,
        plot_stride,
        lambda p: print("    Saving plot " + p),
        ["C0" for i in range(num_spikers_per_kind) for _ in trains[i].get_spikes()] +
        ["C2" for i in range(num_spikers_per_kind) for _ in spikes_of_data_signals[i]]
        )

    print("Done.")


####################################################################################################
####################################################################################################
####################################################################################################


_automatically_registered_tests_ = sorted([{"name": "test/" + elem.__name__[len("_test_"):],
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
    return test_info["function_ptr"](test_info["name"])
