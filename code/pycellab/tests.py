import os
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
