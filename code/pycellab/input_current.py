import numpy
import os
import sys
import json


# --- SETUP AND PRECONDITIONS --------------------------------------------------------


if len(sys.argv) < 2:
    print("ERROR: Too few arguments.")
    print("       Mandatory argument 1 must be an output directory.")
    print("       Optional argument 2 must be a JSON config file.")
    exit(1)
if len(sys.argv) > 3:
    print("ERROR: Too many arguments.")
    print("       Mandatory argument 1 must be an output directory.")
    print("       Optional argument 2 must be a JSON config file.")
    exit(1)
os.makedirs(sys.argv[1], exist_ok=True)


# --- CONFIG -------------------------------------------------------------------------


num_seconds = 1
simulation_frequency_list = [100, 500, 1000]
num_trains_list = [100, 500, 1000]
num_excitatory_per_inhibitory = 4
frequency_excitatory = 15
train_recovery_milliseconds = 2
random_seed = 0

if len(sys.argv) == 3:
    with open(sys.argv[2], "r") as f:
        config = json.load(f)
        num_seconds = config["num_seconds"]
        simulation_frequency_list = config["simulation_frequency"]
        num_trains_list = config["num_trains"]
        num_excitatory_per_inhibitory = config["num_excitatory_per_inhibitory"]
        frequency_excitatory = config["frequency_excitatory"]
        train_recovery_milliseconds = config["train_recovery_milliseconds"]
        random_seed = config["random_seed"]


# --- COMPUTATION -------------------------------------------------------------------


numpy.random.seed(random_seed)

for num_trains in num_trains_list:

    num_inhibitory = num_trains // (num_excitatory_per_inhibitory + 1)
    num_excitatory = (num_trains - (num_trains % (num_excitatory_per_inhibitory + 1))) - num_inhibitory
    frequency_inhibitory = frequency_excitatory * num_excitatory_per_inhibitory

    num_spikes_excitatory = (num_seconds * frequency_excitatory) * num_excitatory
    num_spikes_inhibitory = (num_seconds * frequency_inhibitory) * num_inhibitory
    assert num_spikes_excitatory == num_spikes_inhibitory

    current_excitatory = {}
    current_inhibitory = {}
    current_composed = {}
    current_accumulated = {}
    histograms_excitatory = {}
    histograms_inhibitory = {}
    histograms_composed = {}
    for simulation_frequency in simulation_frequency_list:

        print("--- trains=" + str(num_trains) + ", simulation_frequency=" + str(simulation_frequency) + " ---")

        num_simulation_steps = num_seconds * simulation_frequency
        train_recovery_steps = int((simulation_frequency * train_recovery_milliseconds) / 1000 + 0.5)

        print("Generating input current...")
        current_composed[simulation_frequency] = [0 for _ in range(num_simulation_steps)]
        current_excitatory[simulation_frequency] = [0 for _ in range(num_simulation_steps)]
        current_inhibitory[simulation_frequency] = [0 for _ in range(num_simulation_steps)]
        def generate_input_train(num_spikes_to_generate, output_composed, output_dedicated, dI):
            locked = set()
            i = 0
            while i < 2*num_spikes_to_generate:
                idx = numpy.random.randint(0, 2*num_simulation_steps)
                if idx not in locked:
                    if idx < num_simulation_steps:
                        output_composed[idx] += dI
                        output_dedicated[idx] += dI
                    for j in range(idx - train_recovery_steps, idx + train_recovery_steps + 1):
                        locked.add(j)
                    i += 1
        for _ in range(num_excitatory):
            generate_input_train(num_seconds * frequency_excitatory, current_composed[simulation_frequency], current_excitatory[simulation_frequency], 1)
        for _ in range(num_inhibitory):
            generate_input_train(num_seconds * frequency_inhibitory, current_composed[simulation_frequency], current_inhibitory[simulation_frequency], -1)

        print("Computing accumulative input current...")
        current_accumulated[simulation_frequency] = []
        for c in current_composed[simulation_frequency]:
            try:
                current_accumulated[simulation_frequency].append(current_accumulated[simulation_frequency][-1] + c)
            except:
                current_accumulated[simulation_frequency].append(c)

        print("Computing histograms of the input current...")
        def compute_current_histogram(current):
            histogram = {}
            for c in current:
                if c in histogram:
                    histogram[c] += 1
                else:
                    histogram[c] = 1
            return histogram
        histograms_composed[simulation_frequency] = compute_current_histogram(current_composed[simulation_frequency])
        histograms_excitatory[simulation_frequency] = compute_current_histogram(current_excitatory[simulation_frequency])
        histograms_inhibitory[simulation_frequency] = compute_current_histogram(current_inhibitory[simulation_frequency])

    eval_id = "input_current_" + str(num_seconds) + "sec_" + str(random_seed) + "seed_" + str(num_trains) + "trains"

    max_simulation_frequency = max(simulation_frequency_list)
    max_num_simulation_steps = num_seconds * max_simulation_frequency

    print("Saving excitatory and inhibitory input current...")
    with open(os.path.join(sys.argv[1], eval_id + "_dedicated.txt"), "w") as f:
        f.write("# Column 0 - indices of simulation time steps.\n")
        for i, simulation_frequency in enumerate(simulation_frequency_list):
            f.write("# Column " + str(2*i+1) + " - counts of excitatory spikes; for simulation frequency " + str(simulation_frequency) + ".\n")
            f.write("# Column " + str(2*i+2) + " - counts of inhibitory spikes; for simulation frequency " + str(simulation_frequency) + ".\n")
        for step in range(max_num_simulation_steps):
            f.write(str(step))
            for simulation_frequency in simulation_frequency_list:
                num_simulation_steps = num_seconds * simulation_frequency
                s = int((step / max_num_simulation_steps) * num_simulation_steps)
                f.write(" " + str(current_excitatory[simulation_frequency][s]))
                f.write(" " + str(current_inhibitory[simulation_frequency][s]))
            f.write("\n")

    print("Saving composed input current...")
    with open(os.path.join(sys.argv[1], eval_id + "_composed.txt"), "w") as f:
        f.write("# Column 0 - indices of simulation time steps.\n")
        for i, simulation_frequency in enumerate(simulation_frequency_list):
            f.write("# Column " + str(i+1) + " - composed input current (excitatory + inhibitory current); for simulation frequency " + str(simulation_frequency) + ".\n")
        for step in range(max_num_simulation_steps):
            f.write(str(step))
            for simulation_frequency in simulation_frequency_list:
                num_simulation_steps = num_seconds * simulation_frequency
                s = int((step / max_num_simulation_steps) * num_simulation_steps)
                f.write(" " + str(current_composed[simulation_frequency][s]))
            f.write("\n")

    print("Saving accumulated input current...")
    with open(os.path.join(sys.argv[1], eval_id + "_accumulated.txt"), "w") as f:
        f.write("# Column 0 - indices of simulation time steps.\n")
        for i, simulation_frequency in enumerate(simulation_frequency_list):
            f.write("# Column " + str(i+1) + " - accumulated input current (excitatory + inhibitory current adds to current from the previous step); for simulation frequency " + str(simulation_frequency) + ".\n")
        for step in range(max_num_simulation_steps):
            f.write(str(step))
            for simulation_frequency in simulation_frequency_list:
                num_simulation_steps = num_seconds * simulation_frequency
                s = int((step / max_num_simulation_steps) * num_simulation_steps)
                f.write(" " + str(current_accumulated[simulation_frequency][s]))
            f.write("\n")

    print("Saving histograms of excitatory and inhibitory input current...")
    with open(os.path.join(sys.argv[1], eval_id + "_histogram_dedicated.txt"), "w") as f:
        f.write("# Column 0 - input current level.\n")
        for i, simulation_frequency in enumerate(simulation_frequency_list):
            f.write("# Column " + str(2*i+1) + " - number of time steps producing that excitatory input current; for simulation frequency " + str(simulation_frequency) + ".\n")
            f.write("# Column " + str(2*i+2) + " - number of time steps producing that inhibitory input current; for simulation frequency " + str(simulation_frequency) + ".\n")
        min_level = None
        max_level = None
        for _, hist in histograms_excitatory.items():
            x = min(hist.keys())
            min_level = x if min_level is None or x < min_level else min_level
            x = max(hist.keys())
            max_level = x if max_level is None or max_level < x else max_level
        for _, hist in histograms_inhibitory.items():
            x = min(hist.keys())
            min_level = x if min_level is None or x < min_level else min_level
            x = max(hist.keys())
            max_level = x if max_level is None or max_level < x else max_level
        for level in range(min_level, max_level + 1):
            f.write(str(level))
            for simulation_frequency in simulation_frequency_list:
                f.write(" " + str(histograms_excitatory[simulation_frequency][level] if level in histograms_excitatory[simulation_frequency] else 0))
                f.write(" " + str(histograms_inhibitory[simulation_frequency][level] if level in histograms_inhibitory[simulation_frequency] else 0))
            f.write("\n")

    print("Saving histograms of composed input current...")
    with open(os.path.join(sys.argv[1], eval_id + "_histogram_composed.txt"), "w") as f:
        f.write("# Column 0 - input current level.\n")
        for i, simulation_frequency in enumerate(simulation_frequency_list):
            f.write("# Column " + str(i+1) + " - number of time steps producing that input current; for simulation frequency " + str(simulation_frequency) + ".\n")
        min_level = None
        max_level = None
        for _, hist in histograms_composed.items():
            x = min(hist.keys())
            min_level = x if min_level is None or x < min_level else min_level
            x = max(hist.keys())
            max_level = x if max_level is None or max_level < x else max_level
        for level in range(min_level, max_level + 1):
            f.write(str(level))
            for simulation_frequency in simulation_frequency_list:
                f.write(" " + str(histograms_composed[simulation_frequency][level] if level in histograms_composed[simulation_frequency] else 0))
            f.write("\n")

print("Done.")
