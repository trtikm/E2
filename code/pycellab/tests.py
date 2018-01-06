import os
import shutil
import time
import numpy
import json
import plot
import distribution
import spike_train
import synapse
import datalgo
import utility


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
        isi = distribution.Distribution(hist)
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
        points = datalgo.move_scale_curve_points(
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
        hist = datalgo.make_histogram_from_points(points)
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
            synapse.Synapse.plastic_peek_np(),
            synapse.Synapse.plastic_peek_pn(),
            synapse.Synapse.plastic_peek_pp(),
            synapse.Synapse.plastic_peek_nn(),
            ]:
        print("  Starting simulation of '" + the_synapse.get_name() + "'.")

        start_time = 0.0
        dt = 0.001
        nsteps = 20000

        pre_spikes_train = spike_train.create(distribution.default_excitatory_isi_distribution(), 0.0)
        post_spikes_train = spike_train.create(distribution.default_excitatory_isi_distribution(), 0.0)
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
            utility.print_progress_string(step, nsteps)

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
            datalgo.merge_close_points_by_add(weights_delta, dt),
            pathname,
            xaxis_name="post_t - pre_t",
            faxis_name="weight delta"
            )

    return 0


def _test_default_excitatory_isi_distribution(info):
    """
    The test creates the default excitatory ISI distribution, and
    uses it to generate 10000 events (ISIs). Then plot of the ISI
    histogram is saved.
    """
    assert isinstance(info, TestInfo)
    plot.histogram(
        datalgo.make_histogram(
            distribution.default_excitatory_isi_distribution().generate(100000),
            0.001,
            0.0,
            ),
        os.path.join(info.output_dir, "default_excitatory_isi_distribution.png"),
        normalised=False,
        title="default_excitatory_isi_distribution " + plot.get_title_placeholder()
        )
    return 0


def _test_default_inhibitory_isi_distribution(info):
    """
    The test creates the default inhibitory ISI distribution, and
    uses it to generate 10000 events (ISIs). Then plot of the ISI
    histogram is saved.
    """
    assert isinstance(info, TestInfo)
    plot.histogram(
        datalgo.make_histogram(
            distribution.default_inhibitory_isi_distribution().generate(100000),
            0.00025,
            0.0,
            ),
        os.path.join(info.output_dir, "default_inhibitory_isi_distribution.png"),
        normalised=False,
        title="default_inhibitory_isi_distribution " + plot.get_title_placeholder()
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
    nsteps = 5 * 60 * 1000
    num_spikers_per_kind = 11

    trains = [spike_train.create(distribution.default_excitatory_isi_distribution(), 10.0 * i)
              for i in range(num_spikers_per_kind)] +\
             [spike_train.create(distribution.default_inhibitory_isi_distribution(), 10.0 * i)
              for i in range(num_spikers_per_kind)]

    t = start_time
    for step in range(nsteps):
        utility.print_progress_string(step, nsteps)
        for train in trains:
            train.on_time_step(t, dt)
        t += dt

    print("  Saving results.")

    for i, train in enumerate(trains):
        if i < num_spikers_per_kind:
            train_id = "excitatory[" + str(i) + "]"
            colour = plot.get_colour_pre_excitatory(0.75)
        else:
            train_id = "inhibitory[" + str(i - num_spikers_per_kind) + "]"
            colour = plot.get_colour_pre_inhibitory(0.75)

        file_name = train_id + "_info.json"
        pathname = os.path.join(info.output_dir, file_name)
        print("    Saving info " + pathname)
        with open(pathname, "w") as ofile:
            ofile.write(json.dumps({"configuration": train.get_configuration(), "statistics": train.get_statistics()},
                                   sort_keys=True,
                                   indent=4))

        file_name = train_id + "_isi_histogram.png"
        pathname = os.path.join(info.output_dir, file_name)
        print("    Saving plot " + pathname)
        plot.histogram(
            datalgo.make_histogram(
                datalgo.make_difference_events(
                    train.get_spikes_history()
                    ),
                dt,
                start_time
                ),
            pathname,
            False,
            colour,
            plot.get_title_placeholder()
            )

        # file_name = train_id + "_histogram_reguatory_lengths.png"
        # pathname = os.path.join(info.output_dir, file_name)
        # print("    Saving plot " + pathname)
        # plot.histogram(
        #     train.get_regularity_length_distribution(),
        #     pathname,
        #     False,
        #     colour,
        #     plot.get_title_placeholder()
        #     )
        #
        # file_name = train_id + "_histogram_noise_lengths.png"
        # pathname = os.path.join(info.output_dir, file_name)
        # print("    Saving plot " + pathname)
        # plot.histogram(
        #     train.get_noise_length_distribution(),
        #     pathname,
        #     False,
        #     colour,
        #     plot.get_title_placeholder()
        #     )

        isi_delta =\
            datalgo.make_function_from_events(
                datalgo.make_difference_events(
                    datalgo.make_difference_events(
                        train.get_spikes_history()
                        )
                    )
                )
        plot.curve_per_partes(
            isi_delta,
            os.path.join(info.output_dir, train_id + "_isi_delta_curve.png"),
            0,
            len(isi_delta),
            1000,
            None,
            lambda p: print("    Saving plot " + p),
            colour,
            plot.get_title_placeholder()
            )

    return 0


def _test_surface_under_function(info):
    """
    Test of the function 'utility.compute_surface_under_function'
    on constant, linear, and quadratic functions.
    """
    assert isinstance(info, TestInfo)
    data = [
        # Each element is a tuple (function, x_lo, x_hi, delta_x, correct_result, func_text)
        (lambda x: 2.0, -1.0, 2.0, 0.01, 6.0, "lambda x: 2.0"),
        (lambda x: 3 - x, 0.0, 3.0, 0.0005, 4.5, "lambda x: 3 - x"),
        (lambda x: x**2 - 1, -2.0, 2.0, 0.001, 2.0*(-2.0/3.0 + 4.0/3.0), "lambda x: x**2 - 1")
        ]
    num_failures = 0
    for func, x_lo, x_hi, delta_x, correct_result, func_text in data:
        result = utility.compute_surface_under_function(func, x_lo, x_hi, None, delta_x)
        if abs(result - correct_result) > 0.001:
            print("FAILURE: func=" + func_text +
                  ", x_lo=" + str(x_lo) +
                  ", x_hi=" + str(x_hi) +
                  ", delta_x=" + str(delta_x) +
                  ", correct_result=" + str(correct_result) +
                  ", computed_result=" + str(result)
                  )
            num_failures += 1
    return num_failures


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
    out_dir = os.path.join(os.path.join(test_info["output_dir"], "tests", test_info["name"]))
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir)
    os.makedirs(out_dir)
    start_time = time.time()
    retval = test_info["function_ptr"](TestInfo(test_info["name"], out_dir))
    end_time = time.time()
    print("The test has finished " + ("successfully" if retval == 0 else "with an error") +
          " in " + utility.duration_string(start_time, end_time) + " seconds.")
    return retval
