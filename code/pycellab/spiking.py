import os
import argparse
import config
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
    if cfg.are_equal_noise_distributions:
        pathname = os.path.join(cfg.output_dir, "pre_isi_all" + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        plot.histogram(
            cfg.excitatory_noise_distributions[0],
            pathname,
            markers="-",
            colours=get_colour_pre_excitatory_and_inhibitory()
            )
    else:
        if cfg.are_equal_excitatory_noise_distributions:
            pathname = os.path.join(cfg.output_dir, "pre_isi_excitatory" + cfg.plot_files_extension)
            print("    Saving plot " + pathname)
            plot.histogram(
                cfg.excitatory_noise_distributions[0],
                pathname,
                markers="-",
                colours=get_colour_pre_excitatory()
                )
        if cfg.are_equal_inhibitory_noise_distributions:
            pathname = os.path.join(cfg.output_dir, "pre_isi_inhibitory" + cfg.plot_files_extension)
            print("    Saving plot " + pathname)
            plot.histogram(
                cfg.inhibitory_noise_distributions[0],
                pathname,
                markers="-",
                colours=get_colour_pre_inhibitory()
                )


def save_post_isi_distribution(cfg, post_spikes, subdir):
    pathname = os.path.join(cfg.output_dir, subdir, "post_isi" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(post_spikes, cfg.dt),
        pathname,
        normalised=False,
        colours=get_colour_post()
        )


def save_spikes_board(cfg, pre_spikes_excitatory, pre_spikes_inhibitory, post_spikes, soma_names):
    pathname = os.path.join(cfg.output_dir, "pre_spikes" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)

    stride = -max(int((len(pre_spikes_excitatory) + len(pre_spikes_inhibitory))/100), 5)
    base_shift = -max(int((len(pre_spikes_excitatory) + len(pre_spikes_inhibitory))/10 + stride / 2), 5)

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
        colours=[get_colour_pre_excitatory() for spikes in pre_spikes_excitatory for _ in spikes] +
                [get_colour_pre_inhibitory() for spikes in pre_spikes_inhibitory for _ in spikes] +
                [get_colour_post() for spikes in post_spikes for _ in spikes],
        title=(
            "pre-total=" +
                    str(len(pre_spikes_excitatory) + len(pre_spikes_inhibitory)) +
            ", pre-excitatory[0," +
                    str(len(pre_spikes_excitatory)) + ")=" +
                    str(len(pre_spikes_excitatory)) +
            ", pre-inhibitory[" +
                    str(len(pre_spikes_excitatory)) + "," +
                    str(len(pre_spikes_excitatory) + len(pre_spikes_inhibitory)) + ")=" +
                    str(len(pre_spikes_inhibitory)) + ", " +
            "".join(filter(lambda _: len(soma_names) > 1, ["\n"])) +
            "".join(map(lambda x: "post[" + x[0] + "][" + x[1] + "]=" + x[2] + ", ",
                        zip([name for name in soma_names],
                            [str(stride * i + base_shift) for i in range(len(post_spikes))],
                            [str(len(spikes)) for spikes in post_spikes])))
            )
        )


def save_pre_spike_counts_histograms(cfg, pre_spikes_excitatory, pre_spikes_inhibitory):
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


def save_pre_spike_counts_curves(cfg, pre_spikes_excitatory, pre_spikes_inhibitory):
    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_curve" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.curve(
        distribution.make_counts_curve(
            sorted([t for spikes in pre_spikes_excitatory + pre_spikes_inhibitory for t in spikes]),
            dx=cfg.dt),
        pathname,
        colours=get_colour_pre_excitatory_and_inhibitory()
        )

    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_curve_excitatory" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.curve(
        distribution.make_counts_curve(
            sorted([t for spikes in pre_spikes_excitatory for t in spikes]),
            dx=cfg.dt),
        pathname,
        colours=get_colour_pre_excitatory()
        )

    pathname = os.path.join(cfg.output_dir, "pre_spike_counts_curve_inhibitory" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.curve(
        distribution.make_counts_curve(
            sorted([t for spikes in pre_spikes_inhibitory for t in spikes]),
            dx=cfg.dt),
        pathname,
        colours=get_colour_pre_inhibitory()
        )


def save_soma_recording(cfg, data, title, subdir):
    for key, points in data.items():
        pathname = os.path.join(cfg.output_dir, subdir, "soma_" + key + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        plot.curve(points, pathname, colours=get_colour_soma(), title=title)


def evaluate(cfg):
    assert isinstance(cfg, config.configuration)

    print("Evaluating the configuration '" + cfg.name + "'.")

    print("  Constructing and initialising data structures,")
    excitatory_spike_trains = [spike_train.spike_train(noise, [], cfg.start_time)
                               for noise in cfg.excitatory_noise_distributions]
    inhibitory_spike_trains = [spike_train.spike_train(noise, [], cfg.start_time)
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
    os.makedirs(cfg.output_dir, exist_ok=True)
    save_pre_isi_distributions(cfg)
    save_spikes_board(cfg, [train.get_spikes() for train in excitatory_spike_trains],
                           [train.get_spikes() for train in inhibitory_spike_trains],
                           [cell.get_spikes() for cell in cells],
                           [cell.get_soma().get_name() for cell in cells])
    save_pre_spike_counts_histograms(cfg, [train.get_spikes() for train in excitatory_spike_trains],
                                          [train.get_spikes() for train in inhibitory_spike_trains])
    save_pre_spike_counts_curves(cfg, [train.get_spikes() for train in excitatory_spike_trains],
                                      [train.get_spikes() for train in inhibitory_spike_trains])
    if len(cells) == 1:
        subdirs = [""]
    else:
        subdirs = [cell.get_soma().get_name() for cell in cells]
    for i in range(len(cells)):
        os.makedirs(os.path.join(cfg.output_dir, subdirs[i]), exist_ok=True)
        save_post_isi_distribution(cfg, cells[i].get_spikes(), subdirs[i])
        save_soma_recording(cfg, cells[i].get_soma_recording(), cells[i].get_soma().get_short_description(), subdirs[i])

    print("  Done.")


def main(cmdline):
    if cmdline.configurations:
        print("Listing available configurations:\n")
        for cfg in config.get_registered_configurations():
            print("* " + cfg.name + "\n\n" + cfg.description + "\n\n")
        return 0

    if cmdline.eval:
        for cfg in config.get_registered_configurations():
            if cfg.name == cmdline.eval:
                evaluate(cfg)
                return 0
        print("ERROR: unknown configuration '" + cmdline.eval + "'. Use the option "
              "'--configurations' to list all available configurations.")
        return 1

    for cfg in config.get_registered_configurations():
        evaluate(cfg)

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="This is evaluation system of spiking models of neurons. "
                    "Individual experiments are defined in so called 'configurations'. "
                    "You can list all available configuration using '--configurations' "
                    "option. Then you can pass the name of a chosen configuration to the "
                    "system via the option '--eval' and that configuration will be evaluated. "
                    "If no configuration is specified (i.e. the option '--eval' is not present), "
                    "then the system evaluates all available configurations."
        )
    parser.add_argument(
        "--configurations", action="store_true",
        help="Prints names and descriptions of all all available configurations. "
             "A configuration holds data to initialise and execute the corresponding experiment. "
             "Use the option '--eval' with the name of one of the listed configurations to run it."
        )
    parser.add_argument(
        "--eval", type=str, default=None,
        help="The system evaluates the configuration of the passed name. Use the option "
             "'--configurations' to list all available."
        )
    # config.__dbg()
    exit(main(parser.parse_args()))
