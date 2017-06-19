import os
import shutil
import argparse
import math
import config
import tests
import neuron
import spike_train
import distribution
import plot


def get_colour_pre_excitatory():
    return 'C0'     # blue


def get_colour_pre_inhibitory():
    return 'C2'     # green


def get_colour_pre_excitatory_and_inhibitory():
    return 'C4'     # purple


def get_colour_post():
    return 'k'      # black


def get_colour_soma():
    return 'k'      # black


def save_pre_isi_distributions(cfg):
    assert isinstance(cfg, config.NeuronWithInputSynapses)
    if cfg.are_equal_noise_distributions:
        pathname = os.path.join(cfg.output_dir, "pre_isi_all" + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        plot.histogram(
            cfg.excitatory_noise_distributions[0],
            pathname,
            colours=get_colour_pre_excitatory_and_inhibitory()
            )
    else:
        if cfg.are_equal_excitatory_noise_distributions:
            pathname = os.path.join(cfg.output_dir, "pre_isi_excitatory" + cfg.plot_files_extension)
            print("    Saving plot " + pathname)
            plot.histogram(
                cfg.excitatory_noise_distributions[0],
                pathname,
                colours=get_colour_pre_excitatory()
                )
        if cfg.are_equal_inhibitory_noise_distributions:
            pathname = os.path.join(cfg.output_dir, "pre_isi_inhibitory" + cfg.plot_files_extension)
            print("    Saving plot " + pathname)
            plot.histogram(
                cfg.inhibitory_noise_distributions[0],
                pathname,
                colours=get_colour_pre_inhibitory()
                )


def save_post_isi_distribution(cfg, post_spikes, subdir):
    assert isinstance(cfg, config.CommonProps)
    pathname = os.path.join(cfg.output_dir, subdir, "post_isi" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(post_spikes, cfg.nsteps, cfg.start_time, cfg.start_time + cfg.nsteps * cfg.dt),
        pathname,
        normalised=False,
        colours=get_colour_post()
        )


def save_spikes_board(
        cfg,
        pre_spikes_excitatory,
        pre_spikes_inhibitory,
        pre_weights_excitatory,
        pre_weights_inhibitory,
        post_spikes,
        soma_name,
        sub_dir,
        suffix,
        title=None
        ):
    assert isinstance(cfg, config.CommonProps)
    pathname = os.path.join(cfg.output_dir, sub_dir, "spikes_board" + suffix + cfg.plot_files_extension)
    print("    Saving plot " + pathname)

    if len(pre_spikes_excitatory) == 1 and len(pre_spikes_inhibitory) == 0 and len(post_spikes) == 1:
        stride = 0
        base_shift = -1
        if title is None:
            title = (
                "pre[0]=" + str(len(pre_spikes_excitatory[0])) +
                ", post[" + str(base_shift + stride) + "]=" + str(len(post_spikes[0]))
                )
        colours = ([get_colour_pre_excitatory() for _ in range(len(pre_spikes_excitatory[0]))] +
                   [get_colour_post() for _ in range(len(post_spikes[0]))])
    else:
        stride = -max(int((len(pre_spikes_excitatory) + len(pre_spikes_inhibitory))/100), 5)
        base_shift = -max(int((len(pre_spikes_excitatory) + len(pre_spikes_inhibitory))/10 + stride / 2), 5)
        if title is None:
            title = (
                "pre-total=" +
                        str(len(pre_spikes_excitatory) + len(pre_spikes_inhibitory)) +
                ", pre-excitatory[0," +
                        str(len(pre_spikes_excitatory)) + ")=" +
                        str(len(pre_spikes_excitatory)) +
                ", pre-inhibitory[" +
                        str(len(pre_spikes_excitatory)) + "," +
                        str(len(pre_spikes_excitatory) + len(pre_spikes_inhibitory)) + ")=" +
                        str(len(pre_spikes_inhibitory)) +
                ", post[" + str(base_shift + stride) + "]=" + soma_name
                )
        colours = ([(weight, 0.1, 1.0 - weight) for weights in pre_weights_excitatory for weight in weights] +
                   [(0.0, weight, 1.0 - weight) for weights in pre_weights_inhibitory for weight in weights] +
                   [(0.0, 0.0, 0.0) for spikes in post_spikes for _ in spikes])

    plot.scatter(
        points=[(pre_spikes_excitatory[i][j], i)
                for i in range(len(pre_spikes_excitatory))
                for j in range(len(pre_spikes_excitatory[i]))] +
               [(pre_spikes_inhibitory[i][j], i + len(pre_spikes_excitatory))
                for i in range(len(pre_spikes_inhibitory))
                for j in range(len(pre_spikes_inhibitory[i]))] +
               [(post_spikes[i][j], stride * i + base_shift)
                for i in range(len(post_spikes))
                for j in range(len(post_spikes[i]))],
        pathname=pathname,
        colours=colours,
        title=title
        )


def save_spikes_board_per_partes(
        cfg,
        pre_spikes_excitatory,
        pre_spikes_inhibitory,
        pre_weights_excitatory,
        pre_weights_inhibitory,
        post_spikes,
        soma_name,
        sub_dir,
        title=None
        ):
    assert isinstance(cfg, config.CommonProps)
    assert len(pre_spikes_excitatory) == len(pre_weights_excitatory)
    assert len(pre_spikes_inhibitory) == len(pre_weights_inhibitory)
    indices_excitatory = [0 for _ in pre_spikes_excitatory]
    indices_inhibitory = [0 for _ in pre_spikes_inhibitory]
    start_time = cfg.start_time
    end_time = cfg.start_time + cfg.nsteps * cfg.dt
    dt = cfg.plot_time_step
    indices_post = [0]
    idx = 0
    t = start_time
    while t < end_time:
        end = min(t + dt, end_time)

        def make_slice(spikes, weights, t_start, t_end, indices):
            assert weights is None or len(spikes) == len(weights)
            assert len(spikes) == len(indices)
            assert t_start <= t_end
            spikes_slice = []
            weights_slice = []
            for i, j in enumerate(indices):
                res_spikes = []
                res_weights = []
                while j < len(spikes[i]) and spikes[i][j] < t_end:
                    if spikes[i][j] >= t_start:
                        res_spikes.append(spikes[i][j])
                        if weights is not None:
                            res_weights.append(weights[i][j])
                    j += 1
                spikes_slice.append(res_spikes)
                if weights is not None:
                    weights_slice.append(res_weights)
                indices[i] = j
            if weights is not None:
                return spikes_slice, weights_slice
            else:
                return spikes_slice

        slice_spikes_excitatory, slice_weights_excitatory =\
            make_slice(pre_spikes_excitatory, pre_weights_excitatory, t, end, indices_excitatory)
        slice_spikes_inhibitory, slice_weights_inhibitory =\
            make_slice(pre_spikes_inhibitory, pre_weights_inhibitory, t, end, indices_inhibitory)

        save_spikes_board(
            cfg,
            slice_spikes_excitatory,
            slice_spikes_inhibitory,
            slice_weights_excitatory,
            slice_weights_inhibitory,
            make_slice([post_spikes], None, t, end, indices_post),
            soma_name,
            sub_dir,
            "_" + str(idx).zfill(4) + "_" + format(t, ".4f"),
            title
            )

        t += dt
        idx += 1


def save_pre_spike_counts_histograms(cfg, pre_spikes_excitatory, pre_spikes_inhibitory):
    assert isinstance(cfg, config.CommonProps)
    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_histogram" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_counts_histogram(
            [c for t, c in
                distribution.make_counts_histogram(
                    sorted([t for spikes in pre_spikes_excitatory + pre_spikes_inhibitory for t in spikes]),
                    bin_size=cfg.dt
                    ).items()]
            ),
        pathname,
        normalised=False,
        colours=get_colour_pre_excitatory_and_inhibitory()
        )

    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_histogram_excitatory" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_counts_histogram(
            [c for t, c in
                distribution.make_counts_histogram(
                    sorted([t for spikes in pre_spikes_excitatory for t in spikes]),
                    bin_size=cfg.dt
                    ).items()]),
        pathname,
        normalised=False,
        colours=get_colour_pre_excitatory()
        )

    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_histogram_inhibitory" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_counts_histogram(
            [c for t, c in
                distribution.make_counts_histogram(
                    sorted([t for spikes in pre_spikes_inhibitory for t in spikes]),
                    bin_size=cfg.dt
                    ).items()]),
        pathname,
        normalised=False,
        colours=get_colour_pre_inhibitory()
        )


def save_pre_spike_counts_curves(cfg, pre_spikes_excitatory, pre_spikes_inhibitory, suffix):
    assert isinstance(cfg, config.CommonProps)
    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_curve" + suffix + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.curve(
        distribution.make_counts_curve(
            sorted([t for spikes in pre_spikes_excitatory + pre_spikes_inhibitory for t in spikes]),
            dx=cfg.dt),
        pathname,
        colours=get_colour_pre_excitatory_and_inhibitory()
        )

    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_curve_excitatory" + suffix + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.curve(
        distribution.make_counts_curve(
            sorted([t for spikes in pre_spikes_excitatory for t in spikes]),
            dx=cfg.dt),
        pathname,
        colours=get_colour_pre_excitatory()
        )

    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_curve_inhibitory" + suffix + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.curve(
        distribution.make_counts_curve(
            sorted([t for spikes in pre_spikes_inhibitory for t in spikes]),
            dx=cfg.dt),
        pathname,
        colours=get_colour_pre_inhibitory()
        )


def save_pre_spike_counts_curves_per_partes(
        cfg,
        pre_spikes_excitatory,
        pre_spikes_inhibitory,
        start_time,
        end_time,
        dt
        ):
    assert isinstance(cfg, config.CommonProps)
    assert start_time <= end_time and dt > 0.0
    idx = 0
    t = start_time
    while t < end_time:
        end = min(t + dt, end_time)

        def time_filer(event):
            return t <= event and event < end

        save_pre_spike_counts_curves(
            cfg,
            [list(filter(time_filer, spikes)) for spikes in pre_spikes_excitatory],
            [list(filter(time_filer, spikes)) for spikes in pre_spikes_inhibitory],
            "_" + str(idx).zfill(4) + "_" + format(t, ".4f")
            )

        t += dt
        idx += 1


def save_soma_recording(cfg, data, title, subdir, suffix):
    assert isinstance(cfg, config.CommonProps)
    for key, points in data.items():
        pathname = os.path.join(cfg.output_dir, subdir, "soma_" + key + suffix + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        plot.curve(points, pathname, colours=get_colour_soma(), title=title)


def save_soma_recording_per_partes(
        cfg,
        data,
        title,
        subdir,
        start_time,
        end_time,
        dt
        ):
    assert isinstance(cfg, config.CommonProps)
    assert start_time <= end_time and dt > 0.0
    idx = 0
    t = start_time
    while t < end_time:
        end = min(t + dt, end_time)

        def time_filer(event):
            return t <= event and event < end

        save_soma_recording(
            cfg,
            dict([(key, list(filter(lambda p: time_filer(p[0]), points))) for key, points in data.items()]),
            title,
            subdir,
            "_" + str(idx).zfill(4) + "_" + format(t, ".4f")
            )

        t += dt
        idx += 1


def save_weights_recording_per_partes(
        cfg,
        weights_excitatory,
        weights_inhibitory,
        sub_dir
        ):
    assert isinstance(cfg, config.CommonProps)
    indices_excitatory = [0 for _ in weights_excitatory]
    indices_inhibitory = [0 for _ in weights_inhibitory]
    idx = 0
    end_time = cfg.start_time + cfg.nsteps * cfg.dt
    t = cfg.start_time
    while t < end_time:
        end = min(t + cfg.plot_time_step, end_time)

        def make_slice(weights, t_start, t_end, indices):
            assert len(weights) == len(indices)
            assert t_start <= t_end
            weights_slice = []
            for i, j in enumerate(indices):
                res_weights = []
                while j < len(weights[i]) and weights[i][j][0] < t_end:
                    if weights[i][j][0] >= t_start:
                        res_weights.append(weights[i][j])
                    j += 1
                weights_slice.append(res_weights)
                indices[i] = j
            return weights_slice

        def merge_lists_of_points(lists, merge_function, finalise_function=lambda x: x):
            assert all([isinstance(alist, list) for alist in lists])
            assert callable(merge_function)
            merge_result = []
            if len(lists) == 0:
                return []
            for i in range(max([len(x) for x in lists])):
                value = None
                for alist in lists:
                    if i < len(alist):
                        if value:
                            assert abs(value[0] - alist[i][0]) < 0.00001
                            value = (value[0], merge_function(value[1], alist[i][1]))
                        else:
                            value = alist[i]
                assert value is not None
                merge_result.append((value[0], finalise_function(value[1])))
            return merge_result

        plot_data = {}

        sliced_weights_excitatory = make_slice(weights_excitatory, t, end, indices_excitatory)
        plot_data["weights_excitatory_min"] = merge_lists_of_points(sliced_weights_excitatory, min)
        plot_data["weights_excitatory_max"] = merge_lists_of_points(sliced_weights_excitatory, max)
        plot_data["weights_excitatory_average"] = merge_lists_of_points(sliced_weights_excitatory, lambda x, y: x + y,
                                                                        lambda x: x / len(weights_excitatory))
        sliced_weights_inhibitory = make_slice(weights_inhibitory, t, end, indices_inhibitory)
        plot_data["weights_inhibitory_min"] = merge_lists_of_points(sliced_weights_inhibitory, min)
        plot_data["weights_inhibitory_max"] = merge_lists_of_points(sliced_weights_inhibitory, max)
        plot_data["weights_inhibitory_average"] = merge_lists_of_points(sliced_weights_inhibitory, lambda x, y: x + y,
                                                                        lambda x: x / len(weights_inhibitory))
        sliced_weights_both = sliced_weights_excitatory + sliced_weights_inhibitory
        plot_data["weights_both_min"] = merge_lists_of_points(sliced_weights_both, min)
        plot_data["weights_both_max"] = merge_lists_of_points(sliced_weights_both, max)
        plot_data["weights_both_average"] = merge_lists_of_points(sliced_weights_both, lambda x, y: x + y,
                                                                  lambda x: x / (len(weights_excitatory) +
                                                                                 len(weights_inhibitory)))
        suffix = "_" + str(idx).zfill(4) + "_" + format(t, ".4f")

        for name, points in plot_data.items():
            pathname = os.path.join(cfg.output_dir, sub_dir, name + suffix + cfg.plot_files_extension)
            print("    Saving plot " + pathname)
            plot.curve(points, pathname, colours=get_colour_pre_excitatory())

        t += cfg.plot_time_step
        idx += 1


def save_synapse_recording_per_partes(
        cfg,
        recording,
        title,
        sub_dir
        ):
    assert isinstance(cfg, config.CommonProps)
    if title is not None and len(title) == 0:
        xtitle = None
    else:
        xtitle = title
    idx = 0
    end_time = cfg.start_time + cfg.nsteps * cfg.dt
    t = cfg.start_time
    while t < end_time:
        end = min(t + cfg.plot_time_step, end_time)

        suffix = "_" + str(idx).zfill(4) + "_" + format(t, ".4f")
        for var, points in recording.items():
            pathname = os.path.join(cfg.output_dir, sub_dir, "synapse_" + title + "_" + var + "_" + suffix +
                                    cfg.plot_files_extension)
            print("    Saving plot " + pathname)
            if "excitatory" in title:
                colour = get_colour_pre_excitatory()
            elif "inhibitory" in title:
                colour = get_colour_pre_inhibitory()
            else:
                colour = get_colour_pre_excitatory_and_inhibitory()
            plot.curve(list(filter(lambda p: t <= p[0] and p[0] < end, points)), pathname, colours=colour, title=xtitle)

        t += cfg.plot_time_step
        idx += 1


def evaluate_neuron_with_input_synapses(cfg):
    assert isinstance(cfg, config.NeuronWithInputSynapses)

    print("Evaluating the configuration '" + cfg.name + "'.")

    print("  Constructing and initialising data structures,")
    excitatory_spike_trains = [spike_train.spike_train(noise, None, cfg.start_time)
                               for noise in cfg.excitatory_noise_distributions]
    inhibitory_spike_trains = [spike_train.spike_train(noise, None, cfg.start_time)
                               for noise in cfg.inhibitory_noise_distributions]
    cells = [neuron.neuron(
                cfg.cell_soma[i],
                cfg.excitatory_synapses[i],
                cfg.inhibitory_synapses[i],
                cfg.num_sub_iterations[i],
                cfg.start_time)
             for i in range(len(cfg.cell_soma))]

    print("  Starting simulation.")
    t = cfg.start_time
    for step in range(cfg.nsteps):
        print("    " + format(100.0 * step / float(cfg.nsteps), '.1f') + "%", end='\r')

        # time step of cells
        for cell in cells:
            cell.integrate(t, cfg.dt)

        # time step of excitatory spike trains
        for i in range(len(excitatory_spike_trains)):
            was_spike_generated = excitatory_spike_trains[i].on_time_step(t, cfg.dt)
            if was_spike_generated:
                for cell in cells:
                    cell.on_excitatory_spike(i)

        # time step of inhibitory spike trains
        for i in range(len(inhibitory_spike_trains)):
            was_spike_generated = inhibitory_spike_trains[i].on_time_step(t, cfg.dt)
            if was_spike_generated:
                for cell in cells:
                    cell.on_inhibitory_spike(i)

        t += cfg.dt

    print("  Saving results.")

    if os.path.exists(cfg.output_dir):
        shutil.rmtree(cfg.output_dir)
    os.makedirs(cfg.output_dir, exist_ok=True)

    save_pre_isi_distributions(cfg)
    save_pre_spike_counts_histograms(
        cfg,
        [train.get_spikes() for train in excitatory_spike_trains],
        [train.get_spikes() for train in inhibitory_spike_trains]
        )
    save_pre_spike_counts_curves_per_partes(
        cfg,
        [train.get_spikes() for train in excitatory_spike_trains],
        [train.get_spikes() for train in inhibitory_spike_trains],
        cfg.start_time,
        cfg.start_time + cfg.nsteps * cfg.dt,
        cfg.plot_time_step
        )
    for cell, sub_dir in list(zip(cells, map(lambda cell: cell.get_soma().get_name() * int(len(cells) > 1), cells))):
        os.makedirs(os.path.join(cfg.output_dir, sub_dir), exist_ok=True)
        save_post_isi_distribution(cfg, cell.get_spikes(), sub_dir)
        save_soma_recording_per_partes(
            cfg,
            cell.get_soma_recording(),
            cell.get_soma().get_short_description(),
            sub_dir,
            cfg.start_time,
            cfg.start_time + cfg.nsteps * cfg.dt,
            cfg.plot_time_step
            )
        save_weights_recording_per_partes(
            cfg,
            [recording[cell.get_excitatory_synapses()[i].get_weight_variable_name()]
             for i, recording in enumerate(cell.get_excitatory_synapses_recording())],
            [recording[cell.get_inhibitory_synapses()[i].get_weight_variable_name()]
             for i, recording in enumerate(cell.get_inhibitory_synapses_recording())],
            sub_dir
            )
        if len(cell.get_excitatory_synapses_recording()) > 0:
            save_synapse_recording_per_partes(
                cfg,
                cell.get_excitatory_synapses_recording()[0],
                "excitatory_0",
                sub_dir
                )
        if len(cell.get_excitatory_synapses_recording()) > 1:
            save_synapse_recording_per_partes(
                cfg,
                cell.get_excitatory_synapses_recording()[-1],
                "excitatory_" + str(len(cell.get_excitatory_synapses_recording()) - 1),
                sub_dir
                )
        if len(cell.get_inhibitory_synapses_recording()) > 0:
            save_synapse_recording_per_partes(
                cfg,
                cell.get_inhibitory_synapses_recording()[0],
                "inhibitory_0",
                sub_dir
                )
        if len(cell.get_inhibitory_synapses_recording()) > 1:
            save_synapse_recording_per_partes(
                cfg,
                cell.get_inhibitory_synapses_recording()[-1],
                "inhibitory_" + str(len(cell.get_inhibitory_synapses_recording()) - 1),
                sub_dir
                )

        def compute_normalised_weights(spikes, syn, points):
            weights = []
            idx = 0
            for j, event in enumerate(spikes):
                start_time = event
                if j + 1 < len(spikes):
                    end_time = spikes[j + 1]
                else:
                    end_time = cfg.start_time + cfg.nsteps * cfg.dt
                weight = 0.0
                count = 0
                while idx < len(points) and points[idx][0] < end_time:
                    if points[idx][0] >= start_time:
                        weight = points[idx][1]
                        count = 1
                        # weight += points[idx][1]
                        # count += 1
                    idx += 1
                if count == 0:
                    weight = syn.get_neutral_weight()
                else:
                    weight /= float(count)
                weight = (weight - syn.get_min_weight()) / (syn.get_max_weight() - syn.get_min_weight())
                weights.append(max(0.0, min(1.0, weight)))
            assert len(spikes) == len(weights)
            return weights

        save_spikes_board_per_partes(
            cfg,
            [train.get_spikes() for train in excitatory_spike_trains],
            [train.get_spikes() for train in inhibitory_spike_trains],
            [compute_normalised_weights(
                train.get_spikes(),
                cell.get_excitatory_synapses()[i],
                cell.get_excitatory_synapses_recording()[i][cell.get_excitatory_synapses()[i].get_weight_variable_name()]
                ) for i, train in enumerate(excitatory_spike_trains)],
            [compute_normalised_weights(
                train.get_spikes(),
                cell.get_inhibitory_synapses()[i],
                cell.get_inhibitory_synapses_recording()[i][cell.get_inhibitory_synapses()[i].get_weight_variable_name()]
                ) for i, train in enumerate(inhibitory_spike_trains)],
            cell.get_spikes(),
            cell.get_soma().get_name(),
            sub_dir
            )

    print("  Done.")


def evaluate_synapse_and_spike_noise(cfg):
    assert isinstance(cfg, config.SynapseAndSpikeNoise)
    print("Evaluating the configuration '" + cfg.name + "'.")

    print("  Constructing and initialising data structures,")
    pre_spike_train = spike_train.spike_train(cfg.pre_spikes_distributions, None, cfg.start_time)
    post_spike_train = spike_train.spike_train(cfg.post_spikes_distributions, None, cfg.start_time)
    synapse_recording = dict([(var, [(cfg.start_time, value)])
                              for var, value in cfg.the_synapse.get_variables().items()])

    print("  Starting simulation.")
    t = cfg.start_time
    for step in range(cfg.nsteps):
        print("    " + format(100.0 * step / float(cfg.nsteps), '.1f') + "%", end='\r')
        cfg.the_synapse.integrate(cfg.dt)
        if pre_spike_train.on_time_step(t, cfg.dt):
            cfg.the_synapse.on_pre_synaptic_spike()
        if post_spike_train.on_time_step(t, cfg.dt):
            cfg.the_synapse.on_post_synaptic_spike()
        for key, value in cfg.the_synapse.get_variables().items():
            synapse_recording[key].append((t + cfg.dt, value))
        t += cfg.dt

    print("  Saving results.")

    if os.path.exists(cfg.output_dir):
        shutil.rmtree(cfg.output_dir)
    os.makedirs(cfg.output_dir, exist_ok=True)

    pathname = os.path.join(cfg.output_dir, "isi_pre" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(
            pre_spike_train.get_spikes(),
            cfg.nsteps,
            cfg.start_time,
            cfg.start_time + cfg.nsteps * cfg.dt
            ),
        pathname,
        normalised=False,
        colours=get_colour_pre_excitatory_and_inhibitory()
        )

    pathname = os.path.join(cfg.output_dir, "isi_post" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(
            post_spike_train.get_spikes(),
            cfg.nsteps,
            cfg.start_time,
            cfg.start_time + cfg.nsteps * cfg.dt
            ),
        pathname,
        normalised=False,
        colours=get_colour_pre_excitatory_and_inhibitory()
        )

    save_synapse_recording_per_partes(cfg, synapse_recording, "", ".")

    print("  Done.")


def evaluate_pre_post_spike_noises_differences(cfg):
    assert isinstance(cfg, config.PrePostSpikeNoisesDifferences)
    print("Evaluating the configuration '" + cfg.name + "'.")

    print("  Constructing and initialising data structures,")
    pre_spike_train = spike_train.spike_train(cfg.pre_spikes_distributions, None, cfg.start_time)
    post_spike_train = spike_train.spike_train(cfg.post_spikes_distributions, None, cfg.start_time)

    print("  Starting simulation.")
    delta_post_pre = []
    if cfg.synaptic_input_cooler is not None:
        synaptic_input_vars = dict([(var, [(cfg.start_time, value)])
                                   for var, value in cfg.synaptic_input_cooler.get_variables().items()])
        assert synaptic_input_vars.keys() == {cfg.synaptic_input_cooler.get_var_pre_name(),
                                              cfg.synaptic_input_cooler.get_var_post_name()}
    else:
        synaptic_input_vars = None
    t = cfg.start_time
    for step in range(cfg.nsteps):
        print("    " + format(100.0 * step / float(cfg.nsteps), '.1f') + "%", end='\r')
        is_pre_spike = pre_spike_train.on_time_step(t, cfg.dt)
        is_post_spike = post_spike_train.on_time_step(t, cfg.dt)
        if cfg.synaptic_input_cooler is not None:
            cfg.synaptic_input_cooler.integrate(cfg.dt)
            if is_pre_spike:
                cfg.synaptic_input_cooler.on_pre_synaptic_spike()
            if is_post_spike:
                cfg.synaptic_input_cooler.on_post_synaptic_spike()
            for var, value in cfg.synaptic_input_cooler.get_variables().items():
                synaptic_input_vars[var].append((t + cfg.dt, value))
        if is_pre_spike and is_post_spike:
            delta_post_pre.append((t + cfg.dt, 0.0))
        elif is_pre_spike:
            if len(post_spike_train.get_spikes()) > 0:
                delta_post_pre.append((t + cfg.dt, post_spike_train.get_spikes()[-1] - (t + cfg.dt)))
        elif is_post_spike:
            if len(pre_spike_train.get_spikes()) > 0:
                delta_post_pre.append((t + cfg.dt, (t + cfg.dt) - pre_spike_train.get_spikes()[-1]))
        t += cfg.dt

    print("  Saving results.")

    if os.path.exists(cfg.output_dir):
        shutil.rmtree(cfg.output_dir)
    os.makedirs(cfg.output_dir, exist_ok=True)

    pathname = os.path.join(cfg.output_dir, "isi_pre_orig" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(cfg.pre_spikes_distributions, pathname)

    pathname = os.path.join(cfg.output_dir, "isi_pre" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(
            pre_spike_train.get_spikes(),
            cfg.nsteps,
            cfg.start_time,
            cfg.start_time + cfg.nsteps * cfg.dt
            ),
        pathname,
        normalised=False,
        colours=get_colour_pre_excitatory_and_inhibitory()
        )

    pathname = os.path.join(cfg.output_dir, "isi_post_orig" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(cfg.post_spikes_distributions, pathname)

    pathname = os.path.join(cfg.output_dir, "isi_post" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(
            post_spike_train.get_spikes(),
            cfg.nsteps,
            cfg.start_time,
            cfg.start_time + cfg.nsteps * cfg.dt
            ),
        pathname,
        normalised=False,
        colours=get_colour_pre_excitatory_and_inhibitory()
        )

    if cfg.synaptic_input_cooler is None:
        input_vars_sub = None
        input_vars_sign_add = None
        input_vars_sign_mul = None
    else:
        pre_points = synaptic_input_vars[cfg.synaptic_input_cooler.get_var_pre_name()]
        post_points = synaptic_input_vars[cfg.synaptic_input_cooler.get_var_post_name()]
        assert len(pre_points) == len(post_points)
        input_vars_sub = [(pre_points[i][0], post_points[i][1] - pre_points[i][1]) for i in range(len(pre_points))]
        input_vars_sign_add = [(pre_points[i][0], math.copysign(post_points[i][1] + pre_points[i][1],
                                                                post_points[i][1] - pre_points[i][1]))
                               for i in range(len(pre_points))]
        input_vars_sign_mul = [(pre_points[i][0], math.copysign(post_points[i][1] * pre_points[i][1],
                                                                post_points[i][1] - pre_points[i][1]))
                               for i in range(len(pre_points))]

    if cfg.save_per_partes_plots:
        save_spikes_board_per_partes(
            cfg,
            [pre_spike_train.get_spikes()],
            [],
            [[1.0 for _ in range(len(pre_spike_train.get_spikes()))]],
            [],
            post_spike_train.get_spikes(),
            "",
            "spikes_board"
            )
        plot.curve_per_partes(
            delta_post_pre,
            os.path.join(cfg.output_dir, "delta_post_pre", "delta_post_pre" + cfg.plot_files_extension),
            cfg.start_time,
            cfg.start_time + cfg.nsteps * cfg.dt,
            cfg.plot_time_step,
            lambda p: print("    Saving plot " + p),
            marker="x"
            )
        for var, points in synaptic_input_vars.items():
            plot.curve_per_partes(
                points,
                os.path.join(cfg.output_dir, "synaptic_" + var, "synaptic_" + var + cfg.plot_files_extension),
                cfg.start_time,
                cfg.start_time + cfg.nsteps * cfg.dt,
                cfg.plot_time_step,
                lambda p: print("    Saving plot " + p),
                title="VAR=" + var + ", " + cfg.synaptic_input_cooler.get_short_description()
                )
        if cfg.synaptic_input_cooler is not None:
            plot.curve_per_partes(
                input_vars_sub,
                os.path.join(cfg.output_dir, "input_vars_sub", "input_vars_sub" + cfg.plot_files_extension),
                cfg.start_time,
                cfg.start_time + cfg.nsteps * cfg.dt,
                cfg.plot_time_step,
                lambda p: print("    Saving plot " + p),
                title="[input_post-input_pre], " + cfg.synaptic_input_cooler.get_short_description()
                )
            plot.curve_per_partes(
                input_vars_sign_add,
                os.path.join(cfg.output_dir, "input_vars_sign_add", "input_vars_sign_add" + cfg.plot_files_extension),
                cfg.start_time,
                cfg.start_time + cfg.nsteps * cfg.dt,
                cfg.plot_time_step,
                lambda p: print("    Saving plot " + p),
                title="[sgn(X-Y)*(Y+X),X=input_pre,Y=input_post], " + cfg.synaptic_input_cooler.get_short_description()
                )
            plot.curve_per_partes(
                input_vars_sign_mul,
                os.path.join(cfg.output_dir, "input_vars_sign_mul", "input_vars_sign_mul" + cfg.plot_files_extension),
                cfg.start_time,
                cfg.start_time + cfg.nsteps * cfg.dt,
                cfg.plot_time_step,
                lambda p: print("    Saving plot " + p),
                title="[sgn(X-Y)*(Y*X),X=input_pre,Y=input_post], " + cfg.synaptic_input_cooler.get_short_description()
                )

    pathname = os.path.join(cfg.output_dir, "delta_post_pre_hist" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    delta_post_pre_values = [p[1] for p in delta_post_pre]
    delta_post_pre_min = min(delta_post_pre_values)
    delta_post_pre_max = max(delta_post_pre_values)
    plot.histogram(
        distribution.make_counts_histogram(
            delta_post_pre_values,
            0.0,
            (delta_post_pre_max - delta_post_pre_min) / 500.0
            ),
        pathname,
        normalised=False
        )

    if cfg.synaptic_input_cooler is not None:
        pathname = os.path.join(cfg.output_dir, "input_vars_sub" + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        input_vars_values = [p[1] for p in input_vars_sub]
        input_vars_min = min(input_vars_values)
        input_vars_max = max(input_vars_values)
        plot.histogram(
            distribution.make_counts_histogram(
                input_vars_values,
                0.0,
                (input_vars_max - input_vars_min) / 500.0
                ),
            pathname,
            normalised=False
            )

        pathname = os.path.join(cfg.output_dir, "input_vars_sign_add" + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        input_vars_values = [p[1] for p in input_vars_sign_add]
        input_vars_min = min(input_vars_values)
        input_vars_max = max(input_vars_values)
        input_vars_counts_hist =\
            distribution.make_counts_histogram(
                input_vars_values,
                0.0,
                (input_vars_max - input_vars_min) / 500.0
                )
        plot.histogram(input_vars_counts_hist, pathname, normalised=False)

        pathname = os.path.join(cfg.output_dir, "input_vars_sign_add_times_var" + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        plot.curve([(v, abs(v) * input_vars_counts_hist[v]) for v in sorted(input_vars_counts_hist.keys())], pathname)

        pathname = os.path.join(cfg.output_dir, "input_vars_sign_mul" + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        input_vars_values = [p[1] for p in input_vars_sign_mul]
        input_vars_min = min(input_vars_values)
        input_vars_max = max(input_vars_values)
        plot.histogram(
            distribution.make_counts_histogram(
                input_vars_values,
                0.0,
                (input_vars_max - input_vars_min) / 500.0
                ),
            pathname,
            normalised=False
            )

    print("  Done.")


def main(cmdline):
    if cmdline.test is not None and cmdline.evaluate is None:
        for tst in tests.get_registered_tests():
            if tst["name"] == cmdline.test:
                tests.run_test(tst)
                return 0
        print("ERROR: There is no test of the name '" + str(cmdline.test) + "'. Use the option "
              "'--help' to list all available tests.")
        return 1
    for cfg in config.get_registered_configurations():
        if cmdline.evaluate is None or cfg["name"] == cmdline.evaluate:
            if cfg["class_name"] == config.NeuronWithInputSynapses.__name__:
                evaluate_neuron_with_input_synapses(config.construct_experiment(cfg))
            elif cfg["class_name"] == config.SynapseAndSpikeNoise.__name__:
                evaluate_synapse_and_spike_noise(config.construct_experiment(cfg))
            elif cfg["class_name"] == config.PrePostSpikeNoisesDifferences.__name__:
                evaluate_pre_post_spike_noises_differences(config.construct_experiment(cfg))
            else:
                print("ERROR: There is not defined the evaluation function for configuration class '" +
                      cfg["class_name"] + "'.")
                return 1
            if cmdline.evaluate:
                return 0
    if cmdline.evaluate is not None:
        print("ERROR: Unknown configuration '" + str(cmdline.evaluate) + "'. Use the option "
              "'--help' to list all available configurations.")
        return 1
    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="This module provides evaluation of spiking models of neurons and synapses.\n"
                    "Individual experiments are fully defined in so called 'configurations'.\n"
                    "To evaluate an experiment specify the name of its configuration.",
        epilog="Here is the list of all available configurations:\n\n" +
               "\n\n".join(["* " + cfg["name"] + "\n\n" + cfg["description"]
                            for cfg in config.get_registered_configurations()]) +
               "\n\nHere is the list of all tests:\n\n" +
               "\n\n".join(["* " + tst["name"] + "\n\n" + tst["description"]
                            for tst in tests.get_registered_tests()])
        )
    parser.add_argument(
        "--evaluate", type=str, default=None,
        help="Evaluate the experiment defined in the configuration of the passed name. Use the option "
             "--help to list all available configurations. If you omit this option, then all configurations "
             "will be evaluated in a sequence."
        )
    parser.add_argument(
        "--test", type=str,
        help="Runs the test of the passed name. Use the option --help to list all available tests."
        )
    exit(main(parser.parse_args()))
