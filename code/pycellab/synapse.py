import math
import integrator


class synapse:
    def __init__(self,
                 initial_input_pre,
                 initial_input_post,
                 initial_weight,
                 weight_neutral,
                 weight_epsilon,
                 weight_power,
                 weight_dt,
                 weight_cooling_coef,
                 input_pre_cooling_coef,
                 input_post_cooling_coef,
                 spike_magnitude,
                 integrator_function,
                 name="synapse_custom"
                 ):
        assert weight_epsilon > 0.0001
        assert weight_power >= 2
        assert weight_dt > 0.0
        assert weight_cooling_coef <= 0.0
        assert input_pre_cooling_coef <= 0.0
        assert input_post_cooling_coef <= 0.0
        assert spike_magnitude >= 0.0
        self._variables = {
            "input_pre": initial_input_pre,
            "input_post": initial_input_post,
            "weight": initial_weight
            }
        self._weight_neutral = weight_neutral
        self._weight_epsilon = weight_epsilon
        self._weight_power = weight_power
        self._weight_dt = weight_dt
        self._weight_cooling_coef = weight_cooling_coef
        self._input_pre_cooling_coef = input_pre_cooling_coef
        self._input_post_cooling_coef = input_post_cooling_coef
        self._spike_magnitude = spike_magnitude
        self._integrator = integrator_function
        self._name = name

    @staticmethod
    def constant(initial_weight=1.0
                 ):
        return synapse(
            initial_input_pre=0.0,
            initial_input_post=0.0,
            initial_weight=initial_weight,
            weight_neutral=1.0,
            weight_epsilon=1.0,
            weight_power=2.0,
            weight_dt=1e23,
            weight_cooling_coef=0.0,
            input_pre_cooling_coef=-200.0,
            input_post_cooling_coef=-200.0,
            spike_magnitude=1.0,
            integrator_function=integrator.euler,
            name="synapse_constant"
            )

    @staticmethod
    def plastic(initial_input_pre=0.0,
                initial_input_post=0.0,
                initial_weight=1.0,
                weight_dt=0.1,
                weight_cooling_coef=-0.01,
                input_pre_cooling_coef=-200.0,
                input_post_cooling_coef=-200.0,
                spike_magnitude=1.0,
                integrator_function=integrator.midpoint
                ):
        return synapse(
            initial_input_pre=initial_input_pre,
            initial_input_post=initial_input_post,
            initial_weight=initial_weight,
            weight_neutral=1.0,
            weight_epsilon=1.0,
            weight_power=2.0,
            weight_dt=weight_dt,
            weight_cooling_coef=weight_cooling_coef,
            input_pre_cooling_coef=input_pre_cooling_coef,
            input_post_cooling_coef=input_post_cooling_coef,
            spike_magnitude=spike_magnitude,
            integrator_function=integrator_function,
            name="synapse_plastic"
            )

    def get_name(self):
        return self._name

    def derivatives(self, var):
        weight_slope_scale = math.copysign(
            self._variables["input_post"]
            * self._variables["input_pre"]
            * math.exp(-abs(self._variables["input_post"] - self._variables["input_pre"]))
            * max(0.0, 1.0 - (abs(var["weight"] - self._weight_neutral) / self._weight_epsilon)**self._weight_power),
            self._variables["input_post"] - self._variables["input_pre"])
        if abs(self._variables["input_post"] - self._variables["input_pre"]) < 0.001:
            weight_slope_scale = 0.0
        return {
            "input_pre": self._input_pre_cooling_coef * var["input_pre"],
            "input_post": self._input_post_cooling_coef * var["input_post"],
            "weight": (
                (2.0 * self._weight_epsilon / self._weight_dt) * weight_slope_scale +
                self._weight_cooling_coef * (var["weight"] - self._weight_neutral)
                )
            }

    def get_weight(self):
        return self._variables["weight"]

    def on_pre_synaptic_spike(self):
        self._variables["input_pre"] += self._spike_magnitude

    def on_post_synaptic_spike(self):
        self._variables["input_post"] += self._spike_magnitude

    def integrate(self, dt):
        self._integrator(dt, self._variables, self.derivatives)

    def get_variables(self):
        return self._variables

    def ranges_of_variables(self):
        return {
            "weight":
                (self._weight_neutral - self._weight_epsilon, self._weight_neutral + self._weight_epsilon),
            "input_pre":
                (0.0, self._spike_magnitude),
            "input_post":
                (0.0, self._spike_magnitude)
            }

    @staticmethod
    def get_weight_variable_name():
        return "weight"

    def get_short_description(self):
        return (
            self.get_name() +
            ", w0=" + str(self._weight_neutral) +
            ", eW=" + str(self._weight_epsilon) +
            ", m=" + str(self._weight_power) +
            ", wdt=" + str(self._weight_dt) +
            ", cool_w=" + str(self._weight_cooling_coef) +
            ", cool_pre=" + str(self._input_pre_cooling_coef) +
            ", cool_post=" + str(self._input_post_cooling_coef) +
            ", spike magnitude=" + str(self._spike_magnitude) +
            ", integrator=" + str(integrator.get_name(self._integrator))
            )


#########################################################################################
#########################################################################################
#########################################################################################


import os
import distribution
import spike_train
import plot
import config


def __test_synapse():
    print("Starting 'synapse.__test_synapse()':")

    start_time = 0.0
    dt = 0.001
    nsteps = 1000

    pre_spikes_train = spike_train.spike_train(distribution.get_standard_spike_noise(), [], start_time)
    post_spikes_train = spike_train.spike_train(distribution.get_standard_spike_noise(), [], start_time)
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

    the_synapse = synapse.plastic()
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

    output_dir = os.path.join(config.__output_root_dir(), "__test_synapse")
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

    # weights_delta = []
    # w0 = None
    # for i in range(len(synapse_recording[the_synapse.get_weight_variable_name()])):
    #     t, w = synapse_recording[the_synapse.get_weight_variable_name()][i]
    #     pre_t = synapse_recording["last_pre_times"][i]
    #     post_t = synapse_recording["last_post_times"][i]
    #     if t == pre_t or t == post_t:
    #         w0 = synapse_recording[the_synapse.get_weight_variable_name()][i - 1][1]
    #     if w0:
    #         weights_delta.append((post_t - pre_t, w - w0))
    # weights_delta.sort(key=lambda pair: pair[0])
    #
    # pathname = os.path.join(output_dir, "plasticity.png")
    # print("    Saving plot " + pathname)
    # plot.scatter(
    #     distribution.make_sum_points(weights_delta, dt),
    #     pathname,
    #     xaxis_name="post_t - pre_t",
    #     faxis_name="weight delta"
    #     )


if __name__ == "__main__":
    __test_synapse()
