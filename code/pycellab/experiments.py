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


num_seconds = 3
simulation_frequency = 600
num_trains = 5000
num_excitatory_per_inhibitory = 4
frequency_excitatory = 15
train_recovery_milliseconds = 2

spiking_threshold = 200
decay_coef_on_position_current = 0.98
decay_coef_on_negative_current = 0.25
after_spike_current = -10
spike_current = 500

if len(sys.argv) == 3:
    with open(sys.argv[2], "r") as f:
        config = json.load(f)
        num_seconds = config["num_seconds"]
        simulation_frequency = config["simulation_frequency"]
        num_trains = config["num_trains"]
        num_excitatory_per_inhibitory = config["num_excitatory_per_inhibitory"]
        frequency_excitatory = config["frequency_excitatory"]
        train_recovery_milliseconds = config["train_recovery_milliseconds"]
        spiking_threshold = config["spiking_threshold"]
        decay_coef_on_position_current = config["decay_coef_on_position_current"]
        decay_coef_on_negative_current = config["decay_coef_on_negative_current"]
        after_spike_current = config["after_spike_current"]
        spike_current = config["spike_current"]


# --- INFERRED VARS -------------------------------------------------------------------


num_simulation_steps = num_seconds * simulation_frequency

num_inhibitory = num_trains // (num_excitatory_per_inhibitory + 1)
num_excitatory = (num_trains - (num_trains % (num_excitatory_per_inhibitory + 1))) - num_inhibitory
frequency_inhibitory = frequency_excitatory * num_excitatory_per_inhibitory

num_spikes_excitatory = (num_seconds * frequency_excitatory) * num_excitatory
num_spikes_inhibitory = (num_seconds * frequency_inhibitory) * num_inhibitory
assert num_spikes_excitatory == num_spikes_inhibitory

train_recovery_steps = int((simulation_frequency * train_recovery_milliseconds) / 1000 + 0.5)

config_id = (
    str(num_seconds) + "s_" +
    str(simulation_frequency) + "Hz_" +
    str(num_excitatory) + "e" + str(frequency_excitatory) + "Hz_" +
    str(num_inhibitory) + "i" + str(frequency_inhibitory) + "Hz_" +
    str(spiking_threshold) + "thd"
)


# --- THE COMPUTATION ----------------------------------------------------


print("Generating input current...")
input_current = [0 for _ in range(num_simulation_steps)]
def generate_input_train(num_spikes_to_generate, dI):
    global input_current
    locked = set()
    i = 0
    while i < num_spikes_to_generate:
        idx = numpy.random.randint(0, num_simulation_steps)
        if idx not in locked:
            input_current[idx] += dI
            for j in range(max(0, idx - train_recovery_steps), min(num_simulation_steps, idx + train_recovery_steps + 1)):
                locked.add(j)
            i += 1
for _ in range(num_excitatory):
    generate_input_train(num_seconds * frequency_excitatory, 1)
for _ in range(num_inhibitory):
    generate_input_train(num_seconds * frequency_inhibitory, -1)

# def generate_input_current():
#     global input_current
#     for _ in range(num_excitatory):
#         locked = set()
#         i = 0
#         n = num_seconds * frequency_excitatory
#         while i < n:
#             idx = numpy.random.randint(0, len(input_current))
#             if idx not in locked:
#                 input_current[idx] += 1
#                 for j in range(max(0, idx - train_recovery_steps), min(n, idx + train_recovery_steps + 1)):
#                     locked.add(j)
#                 i += 1
#     for _ in range(num_spikes_excitatory):
#         input_current[numpy.random.randint(0, len(input_current))] += 1
#     for _ in range(num_spikes_inhibitory):
#         input_current[numpy.random.randint(0, len(input_current))] -= 1
# generate_input_current()

print("Computing output current...")
output_current = []
def compute_output_current():
    global output_current
    s = 0.0
    for c in input_current:
        if s < 0.0:
            s *= decay_coef_on_negative_current
        else:
            s *= decay_coef_on_position_current
        s += c
        if s < spiking_threshold:
            output_current.append(int(s + 0.5))
        else:
            output_current.append(spike_current)
            s = after_spike_current
compute_output_current()

assert num_simulation_steps == len(output_current)

print("Computing output current without decay...")
output_current_without_decay = []
for c in input_current:
    try:
        output_current_without_decay.append(output_current_without_decay[-1] + c)
    except:
        output_current_without_decay.append(c)

assert num_simulation_steps == len(output_current_without_decay)

def compute_current_histogram(current):
    histogram = {}
    for c in current:
        if c in histogram:
            histogram[c] += 1
        else:
            histogram[c] = 1
    return histogram
print("Computing histogram of the input current...")
input_current_histogram = compute_current_histogram(input_current)
print("Computing histogram of the output current...")
output_current_histogram = compute_current_histogram(output_current)
print("Computing histogram of the output current...")
output_current_without_decay_histogram = compute_current_histogram(output_current_without_decay)

print("Counting output spikes...")
num_output_spikes = 0
for c in output_current:
    if c >= spike_current:
        num_output_spikes += 1


# --- SAVING & PRINTING RESULTS ----------------------------------------------------


print("Saving experiment's info...")
with open(os.path.join(sys.argv[1], "spiking_" + config_id + "_info.txt"), "w") as f:
    f.write("num_seconds: " + str(num_seconds) + "\n")
    f.write("simulation_frequency: " + str(simulation_frequency) + "\n")
    f.write("num_simulation_steps: " + str(num_simulation_steps) + "\n")
    f.write("\n")
    f.write("num_trains: " + str(num_trains) + "\n")
    f.write("num_excitatory: " + str(num_excitatory) + "\n")
    f.write("num_inhibitory: " + str(num_inhibitory) + "\n")
    f.write("num_excitatory_per_inhibitory: " + str(num_excitatory_per_inhibitory) + "\n")
    f.write("frequency_excitatory: " + str(frequency_excitatory) + "\n")
    f.write("frequency_inhibitory: " + str(frequency_inhibitory) + "\n")
    f.write("train_recovery_milliseconds: " + str(train_recovery_milliseconds) + "\n")
    f.write("train_recovery_steps: " + str(train_recovery_steps) + "\n")
    f.write("\n")
    f.write("decay_coef_on_position_current: " + str(decay_coef_on_position_current) + "\n")
    f.write("decay_coef_on_negative_current: " + str(decay_coef_on_negative_current) + "\n")
    f.write("spiking_threshold: " + str(spiking_threshold) + "\n")
    f.write("spike_current: " + str(spike_current) + "\n")
    f.write("after_spike_current: " + str(after_spike_current) + "\n")
    f.write("\n")
    f.write("num_spikes_excitatory: " + str(num_spikes_excitatory) + "\n")
    f.write("num_spikes_inhibitory: " + str(num_spikes_inhibitory) + "\n")
    f.write("num_output_spikes: " + str(num_output_spikes) + "\n")
    f.write("output_spiking_frequency: " + str(float(num_output_spikes) / num_seconds) + "\n")

print("Saving input and output currents...")
with open(os.path.join(sys.argv[1], "spiking_" + config_id + "_currents.txt"), "w") as f:
    f.write("# Column 0 - indices of simulation time steps.\n")
    f.write("# Column 1 - input current (sum of excitatory and inhibitory input spikes)\n")
    f.write("# Column 2 - output current (excitation level of the neuron's body)\n")
    f.write("# Column 3 - output current without decay (excitation level of the neuron's body from the last time step is increased by the sum of excitatory and inhibitory input spikes in the current time step)\n")
    for i in range(num_simulation_steps):
        f.write(
            str(i) + " " +
            str(input_current[i]) + " " +
            str(output_current[i]) + " " +
            str(output_current_without_decay[i]) +
            "\n")

print("Saving input current histogram...")
with open(os.path.join(sys.argv[1], "spiking_" + config_id + "_input_hist.txt"), "w") as f:
    f.write("# Column 0 - input current level.\n")
    f.write("# Column 1 - number of time steps producing that input current.\n")
    for k in sorted(input_current_histogram.keys()):
        f.write(str(k) + " " + str(input_current_histogram[k]) + "\n")

print("Saving output current histogram...")
with open(os.path.join(sys.argv[1], "spiking_" + config_id + "_output_hist.txt"), "w") as f:
    f.write("# Column 0 - output current level.\n")
    f.write("# Column 1 - number of time steps producing that output current.\n")
    for k in sorted(output_current_histogram.keys()):
        f.write(str(k) + " " + str(output_current_histogram[k]) + "\n")

print("Done.")



# print_current(input_current, simulation_frequency, num_seconds_to_print)
# print_current_histogram(input_current)

# print_current(output_current, simulation_frequency, num_seconds_to_print)
# print_current_histogram(output_current)

# print_spiking_frequency(output_current, spike_current, simulation_frequency, num_seconds_to_print)




# def print_current(current, simulation_frequency, num_seconds=-1):
#     for i, c in enumerate(current):
#         if num_seconds > 0 and i // simulation_frequency >= num_seconds:
#             break
#         print(str(c))


# def print_spiking_frequency(current, spike_current, simulation_frequency, num_seconds=-1):
#     num_spikes = 0
#     num_steps = 0
#     for i, c in enumerate(current):
#         if num_seconds > 0 and i // simulation_frequency >= num_seconds:
#             break
#         if c >= spike_current:
#             num_spikes += 1
#         num_steps += 1
#     print(str(float(num_spikes) / (float(num_steps) / simulation_frequency)))
#
#
# def print_current_histogram(current):
#     histogram = {}
#     for c in current:
#         if c in histogram:
#             histogram[c] += 1
#         else:
#             histogram[c] = 1
#     for k in sorted(histogram.keys()):
#         print(str(k) + " " + str(histogram[k]))
#
#
# # print(str((1-60/600)**100))
# # exit(1)
# #
# # F = 600
# # f = 15
# # n = 10
# # z = 0
# # h = 0
# # for i in range(n * F):
# #     if i % F == 0:
# #         print(str(h))
# #         h = 0
# #     if numpy.random.random_sample() < f / F:
# #         z += 1
# #         h += 1
# # print(str(h))
# # print(str(z / n))
# # print(str(f / F))
# #
# # for i in range(100):
# #     x = i / 10.0
# #     y = x / (x + 1.0)
# #     print(str(x) + " " + str(y))
#
#
#
