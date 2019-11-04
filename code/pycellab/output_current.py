# import random
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
numpy.random.seed(0)


# --- CONFIG -------------------------------------------------------------------------


num_seconds = 1
simulation_frequency_list = [100, 500, 1000]
num_trains_list = [100, 500, 1000]
num_excitatory_per_inhibitory = 4
frequency_excitatory = 15
train_recovery_milliseconds = 2

if len(sys.argv) == 3:
    with open(sys.argv[2], "r") as f:
        config = json.load(f)
        num_seconds = config["num_seconds"]
        simulation_frequency_list = config["simulation_frequency"]
        num_trains_list = config["num_trains"]
        num_excitatory_per_inhibitory = config["num_excitatory_per_inhibitory"]
        frequency_excitatory = config["frequency_excitatory"]
        train_recovery_milliseconds = config["train_recovery_milliseconds"]


# --- COMPUTATION -------------------------------------------------------------------


# def get_rand_int(lo, hi):
#     while True:
#         s = numpy.random.random_sample()
#         if s >= 0.25 and s <= 0.75:
#             t = min(1.0, max(0.0, (s - 0.25) / (0.75 - 0.25)))
#             return int(lo + t * (hi - lo))

for num_trains in num_trains_list:

    num_inhibitory = num_trains // (num_excitatory_per_inhibitory + 1)
    num_excitatory = (num_trains - (num_trains % (num_excitatory_per_inhibitory + 1))) - num_inhibitory
    frequency_inhibitory = frequency_excitatory * num_excitatory_per_inhibitory

    num_spikes_excitatory = (num_seconds * frequency_excitatory) * num_excitatory
    num_spikes_inhibitory = (num_seconds * frequency_inhibitory) * num_inhibitory
    assert num_spikes_excitatory == num_spikes_inhibitory

    output_current_without_decay = {}
    input_current_excitatory = {}
    input_current_inhibitory = {}
    for simulation_frequency in simulation_frequency_list:

        print("--- trains=" + str(num_trains) + ", simulation_frequency=" + str(simulation_frequency) + " ---")

        num_simulation_steps = num_seconds * simulation_frequency
        train_recovery_steps = int((simulation_frequency * train_recovery_milliseconds) / 1000 + 0.5)

        print("Generating input current...")
        input_current = [0 for _ in range(num_simulation_steps)]
        input_current_excitatory[simulation_frequency] = {x: 0 for x in range(num_simulation_steps)}
        input_current_inhibitory[simulation_frequency] = {x: 0 for x in range(num_simulation_steps)}
        def generate_input_train(num_spikes_to_generate, dI):
            global input_current
            locked = set()
            i = 0
            while i < num_spikes_to_generate:
                # idx = get_rand_int(0, num_simulation_steps - 1)
                # idx = int(random.rand() * 1) % num_simulation_steps
                idx = numpy.random.randint(0, num_simulation_steps)
                if dI > 0:
                    generated_indices_excitatory[simulation_frequency][idx] += 1
                else:
                    generated_indices_inhibitory[simulation_frequency][idx] += 1
                if idx not in locked:
                    input_current[idx] += dI
                    for j in range(max(0, idx - train_recovery_steps), min(num_simulation_steps, idx + train_recovery_steps + 1)):
                        locked.add(j)
                    i += 1
        for _ in range(num_excitatory):
            generate_input_train(num_seconds * frequency_excitatory, 1)
        for _ in range(num_inhibitory):
            generate_input_train(num_seconds * frequency_inhibitory, -1)

        # for _ in range(num_spikes_excitatory):
        #     input_current[numpy.random.randint(0, num_simulation_steps)] += 1
        # for _ in range(num_spikes_inhibitory):
        #     input_current[numpy.random.randint(0, num_simulation_steps)] -= 1

        print("Computing output current without decay...")
        output_current_without_decay[simulation_frequency] = []
        for c in input_current:
            try:
                output_current_without_decay[simulation_frequency].append(output_current_without_decay[simulation_frequency][-1] + c)
            except:
                output_current_without_decay[simulation_frequency].append(c)

    max_simulation_frequency = max(simulation_frequency_list)
    max_num_simulation_steps = num_seconds * max_simulation_frequency

    print("Saving output currents without decay...")
    with open(os.path.join(sys.argv[1], "output_current_without_decay_" + str(num_trains) + "_trains.txt"), "w") as f:
        f.write("# Column 0 - indices of simulation time steps.\n")
        for i, simulation_frequency in enumerate(simulation_frequency_list):
            f.write("# Column " + str(i+1) + " - output current without decay; for simulation frequency " + str(simulation_frequency) + ".\n")
        for step in range(max_num_simulation_steps):
            f.write(str(step))
            for simulation_frequency in simulation_frequency_list:
                num_simulation_steps = num_seconds * simulation_frequency
                s = int((step / max_num_simulation_steps) * num_simulation_steps)
                f.write(" " + str(output_current_without_decay[simulation_frequency][s]))
            f.write("\n")

    print("Saving excitatory input current...")
    with open(os.path.join(sys.argv[1], "rand_hist_excitatory" + str(num_trains) + "_trains.txt"), "w") as f:
        f.write("# Column 0 - indices of simulation time steps.\n")
        for i, simulation_frequency in enumerate(simulation_frequency_list):
            f.write("# Column " + str(i+1) + " - counts of spikes; for simulation frequency " + str(simulation_frequency) + ".\n")
        for step in range(max_num_simulation_steps):
            f.write(str(step))
            for simulation_frequency in simulation_frequency_list:
                num_simulation_steps = num_seconds * simulation_frequency
                s = int((step / max_num_simulation_steps) * num_simulation_steps)
                f.write(" " + str(generated_indices_excitatory[simulation_frequency][s]))
            f.write("\n")

print("Done.")
