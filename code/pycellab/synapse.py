
class synapse:

    def __init__(self,
                 initial_weight=1.0,
                 start_time=0.0
                 ):
        self._weight = initial_weight

    def get_weight(self):
        return self._weight

    def on_pre_synaptic_spike(self):
        pass

    def on_post_synaptic_spike(self):
        pass

    def integrate(self, dt):
        pass


#########################################################################################
#########################################################################################
#########################################################################################


import os
import distribution
import spike_train
import plot
import config


def __test_synapse():
    def x():
        pass

    print("Starting 'synapse.__test_synapse()':")

    start_time = 0.0
    dt = 0.001
    nsteps = 1000

    pre_spikes_train = spike_train.spike_train(distribution.get_standard_spike_noise(), start_time)
    post_spikes_train = spike_train.spike_train(distribution.get_standard_spike_noise(), start_time)

    initial_weight = 1.0
    the_synapse = synapse(initial_weight, start_time)
    synapse_recording = {"weights": [], "last_pre_times": [], "last_post_times": []}

    last_pre_spike_time = start_time
    last_post_spike_time = start_time
    t = start_time
    for step in range(nsteps):
        print("    " + format(100.0 * step / float(nsteps), '.1f') + "%", end='\r')

        was_pre_spike_generated = pre_spikes_train.on_time_step(t, dt)
        if was_pre_spike_generated:
            the_synapse.on_pre_synaptic_spike()
            last_pre_spike_time = t + dt
        was_post_spike_generated = post_spikes_train.on_time_step(t, dt)
        if was_post_spike_generated:
            the_synapse.on_post_synaptic_spike()
            last_post_spike_time = t + dt

        the_synapse.integrate(dt)
        synapse_recording["weights"].append((t + dt, the_synapse.get_weight()))
        synapse_recording["last_pre_times"].append(last_pre_spike_time)
        synapse_recording["last_post_times"].append(last_post_spike_time)

        t += dt

    output_dir = os.path.join(config.__output_root_dir(), "__test_synapse")
    os.makedirs(output_dir, exist_ok=True)

    pathname = os.path.join(output_dir, "weights.png")
    print("    Saving plot " + pathname)
    plot.curve(
        synapse_recording["weights"],
        pathname,
        colours="C1"
        )

    weights_delta = []
    w0 = None
    for i in range(len(synapse_recording["weights"])):
        t, w = synapse_recording["weights"][i]
        pre_t = synapse_recording["last_pre_times"][i]
        post_t = synapse_recording["last_post_times"][i]
        if t == pre_t or t == post_t:
            w0 = synapse_recording["weights"][i - 1][1]
        if w0:
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


if __name__ == "__main__":
    __test_synapse()
