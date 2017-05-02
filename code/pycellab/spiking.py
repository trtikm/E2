import os
import argparse
import config
import neuron
import distribution
import plot


def evaluate(cfg):
    assert isinstance(cfg, config.configuration)

    print("Evaluating the configuration '" + cfg.name + "'.")

    print("  Constructing the neuron.")
    cell = neuron.neuron(
        cfg.cell_soma,
        cfg.excitatory_noise_distributions,
        cfg.inhibitory_noise_distributions,
        cfg.excitatory_weights,
        cfg.inhibitory_weights
        )

    print("  Starting simulation.")
    for step in range(cfg.nsteps):
        print("    " + format(100.0 * step / float(cfg.nsteps), '.1f') + "%", end='\r')
        cell.integrate(cfg.dt, step + 1 == cfg.nsteps)
    print("    There has been performed " + str(cfg.nsteps) + " steps, each " + str(cfg.dt) + "s long.")

    print("  Saving results.")

    for key, points in cell.get_soma_recording().items():
        pathname = os.path.join(cfg.output_dir, "soma_" + key + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        plot.curve(points, pathname, title=cell.get_soma().get_short_description())

    def is_excitatory_spike(spike_train_id):
        return spike_train_id in range(cell.get_interval_of_excitatory_spike_trains()[0],
                                       cell.get_interval_of_excitatory_spike_trains()[1])

    def is_inhibitory_spike(spike_train_id):
        return spike_train_id in range(cell.get_interval_of_inhibitory_spike_trains()[0],
                                       cell.get_interval_of_inhibitory_spike_trains()[1])

    pathname = os.path.join(cfg.output_dir, "pre_spikes" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.scatter(
        cell.get_pre_spikes() + [(t, -1) for t in cell.get_post_spikes()],
        pathname,
        title=(
            "total pre-spikes=" + str(len(cell.get_pre_spikes())) +
            ", excitatory pre-spikes" + str(cell.get_interval_of_excitatory_spike_trains()) + "=" +
                str(len([1 for _, i in cell.get_pre_spikes() if is_excitatory_spike(i)])) +
            ", inhibitory pre-spikes" + str(cell.get_interval_of_inhibitory_spike_trains()) + "=" +
                str(len([1 for _, i in cell.get_pre_spikes() if is_inhibitory_spike(i)])) +
            ", post-spikes(-1)=" + str(len(cell.get_post_spikes()))
            )
        )

    pathname = os.path.join(cfg.output_dir, "pre_spike_counts" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_counts_histogram(
            [c for t, c in distribution.make_counts_histogram(
                                [t for t, n in cell.get_pre_spikes()], bin_size=cfg.dt).items()]),
        pathname,
        normalised=False
        )

    pathname = os.path.join(cfg.output_dir, "pre_isi" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram([t for t, n in cell.get_pre_spikes()], cfg.dt),
        pathname
        )

    pathname = os.path.join(cfg.output_dir, "post_isi" + cfg.plot_files_extension)
    print("    Saving plot " + pathname)
    plot.histogram(
        distribution.make_isi_histogram(cell.get_post_spikes(), cfg.dt),
        pathname
        )

    if cfg.are_equal_noise_distributions:
        pathname = os.path.join(cfg.output_dir, "trains_noise" + cfg.plot_files_extension)
        print("    Saving plot " + pathname)
        plot.histogram(
            cfg.excitatory_noise_distributions[0],
            pathname,
            markers="-"
            )

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
