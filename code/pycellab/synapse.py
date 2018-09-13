import math
import integrator


class SynapticInputCooler:
    def __init__(
            self,
            initial_input_pre,
            initial_input_post,
            input_pre_cooling_coef,
            input_post_cooling_coef,
            spike_magnitude,
            clip_var_to_their_ranges,
            integrator_function,
            ):
        assert input_pre_cooling_coef <= 0.0
        assert input_post_cooling_coef <= 0.0
        assert spike_magnitude >= 0.0
        self._variables = {
            self.get_var_pre_name(): initial_input_pre,
            self.get_var_post_name(): initial_input_post
            }
        self._input_pre_cooling_coef = input_pre_cooling_coef
        self._input_post_cooling_coef = input_post_cooling_coef
        self._spike_magnitude = spike_magnitude
        self._clip_var_to_their_ranges = clip_var_to_their_ranges
        self._integrator = integrator_function
        pass

    @staticmethod
    def default(
            initial_input_pre=0.0,
            initial_input_post=0.0,
            input_pre_cooling_coef=-50.0,
            input_post_cooling_coef=-50.0,
            spike_magnitude=1.0,
            clip_var_to_their_ranges=False,
            integrator_function=integrator.euler
            ):
        return SynapticInputCooler(
            initial_input_pre=initial_input_pre,
            initial_input_post=initial_input_post,
            input_pre_cooling_coef=input_pre_cooling_coef,
            input_post_cooling_coef=input_post_cooling_coef,
            spike_magnitude=spike_magnitude,
            clip_var_to_their_ranges=clip_var_to_their_ranges,
            integrator_function=integrator_function
            )

    @staticmethod
    def get_var_pre_name():
        return "input_pre"

    @staticmethod
    def get_var_post_name():
        return "input_post"

    def derivatives(self, var):
        return {
            self.get_var_pre_name(): self._input_pre_cooling_coef * var[self.get_var_pre_name()],
            self.get_var_post_name(): self._input_post_cooling_coef * var[self.get_var_post_name()]
            }

    def on_pre_synaptic_spike(self):
        self._variables[self.get_var_pre_name()] += self._spike_magnitude
        if self._clip_var_to_their_ranges:
            self._variables[self.get_var_pre_name()] = min(self._variables[self.get_var_pre_name()],
                                                           self.ranges_of_variables()[self.get_var_pre_name()][1])

    def on_post_synaptic_spike(self):
        self._variables[self.get_var_post_name()] += self._spike_magnitude
        if self._clip_var_to_their_ranges:
            self._variables[self.get_var_post_name()] = min(self._variables[self.get_var_post_name()],
                                                            self.ranges_of_variables()[self.get_var_post_name()][1])

    def integrate(self, dt):
        self._integrator(dt, self._variables, self.derivatives)

    def get_variables(self):
        return self._variables

    def ranges_of_variables(self):
        return {self.get_var_pre_name(): (0.0, self._spike_magnitude),
                self.get_var_post_name(): (0.0, self._spike_magnitude)}

    def get_short_description(self):
        return (
            "cool_pre=" + str(self._input_pre_cooling_coef) +
            ", cool_post=" + str(self._input_post_cooling_coef) +
            ", spike magnitude=" + str(self._spike_magnitude) +
            ", clip var=" + str(self._clip_var_to_their_ranges) +
            ", integrator=" + str(integrator.get_name(self._integrator))
            )


class Synapse:
    def __init__(self,
                 initial_input_pre,
                 initial_input_post,
                 initial_weight,
                 weight_neutral,
                 weight_epsilon,
                 weight_zero_level,
                 weight_power,
                 weight_dt,
                 weight_tau_potentiation,
                 weight_tau_depression,
                 weight_sign_function,
                 weight_cooling_coef,
                 input_pre_cooling_coef,
                 input_post_cooling_coef,
                 spike_magnitude,
                 integrator_function,
                 name=None
                 ):
        assert weight_epsilon > 0.0001
        assert weight_power >= 2
        assert weight_dt > 0.0
        assert weight_tau_potentiation > 0.0001
        assert weight_tau_depression > 0.0001
        assert callable(weight_sign_function)
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
        self._weight_zero_level = weight_zero_level
        self._weight_power = weight_power
        self._weight_dt = weight_dt
        self._weight_tau_potentiation = weight_tau_potentiation
        self._weight_tau_depression = weight_tau_depression
        self._weight_sign_function = weight_sign_function
        self._weight_cooling_coef = weight_cooling_coef
        self._input_pre_cooling_coef = input_pre_cooling_coef
        self._input_post_cooling_coef = input_post_cooling_coef
        self._spike_magnitude = spike_magnitude
        self._integrator = integrator_function
        self._name = "synapse_custom" if name is None else name

    @staticmethod
    def constant(initial_weight=1.0
                 ):
        return Synapse(
            initial_input_pre=0.0,
            initial_input_post=0.0,
            initial_weight=initial_weight,
            weight_neutral=1.0,
            weight_epsilon=1.0,
            weight_zero_level=0.0,
            weight_power=2.0,
            weight_dt=1e23,
            weight_tau_potentiation=1.0,
            weight_tau_depression=1.0,
            weight_sign_function=lambda x: 1.0,
            weight_cooling_coef=0.0,
            input_pre_cooling_coef=-200.0,
            input_post_cooling_coef=-200.0,
            spike_magnitude=1.0,
            integrator_function=integrator.euler,
            name="synapse_constant"
            )

    @staticmethod
    def plastic_peek_np(
            initial_input_pre=0.0,
            initial_input_post=0.0,
            initial_weight=1.0,
            weight_neutral=1.0,
            weight_epsilon=1.0,
            weight_zero_level=0.0,
            weight_power=2.0,
            weight_dt=0.1,
            weight_tau_potentiation=1.0,
            weight_tau_depression=1.0,
            weight_cooling_coef=-0.01,
            input_pre_cooling_coef=-50.0,
            input_post_cooling_coef=-50.0,
            spike_magnitude=1.0,
            integrator_function=integrator.euler
            ):
        return Synapse(
            initial_input_pre=initial_input_pre,
            initial_input_post=initial_input_post,
            initial_weight=initial_weight,
            weight_neutral=weight_neutral,
            weight_epsilon=weight_epsilon,
            weight_zero_level=weight_zero_level,
            weight_power=weight_power,
            weight_dt=weight_dt,
            weight_tau_potentiation=weight_tau_potentiation,
            weight_tau_depression=weight_tau_depression,
            weight_sign_function=lambda x: math.copysign(1.0, x),
            weight_cooling_coef=weight_cooling_coef,
            input_pre_cooling_coef=input_pre_cooling_coef,
            input_post_cooling_coef=input_post_cooling_coef,
            spike_magnitude=spike_magnitude,
            integrator_function=integrator_function,
            name="synapse_plastic_peek_np"
            )

    @staticmethod
    def plastic_peek_pn(
            initial_input_pre=0.0,
            initial_input_post=0.0,
            initial_weight=1.0,
            weight_neutral=1.0,
            weight_epsilon=1.0,
            weight_zero_level=0.0,
            weight_power=2.0,
            weight_dt=0.1,
            weight_tau_potentiation=1.0,
            weight_tau_depression=1.0,
            weight_cooling_coef=-0.01,
            input_pre_cooling_coef=-50.0,
            input_post_cooling_coef=-50.0,
            spike_magnitude=1.0,
            integrator_function=integrator.euler
            ):
        return Synapse(
            initial_input_pre=initial_input_pre,
            initial_input_post=initial_input_post,
            initial_weight=initial_weight,
            weight_neutral=weight_neutral,
            weight_epsilon=weight_epsilon,
            weight_zero_level=weight_zero_level,
            weight_power=weight_power,
            weight_dt=weight_dt,
            weight_tau_potentiation=weight_tau_potentiation,
            weight_tau_depression=weight_tau_depression,
            weight_sign_function=lambda x: -math.copysign(1.0, x),
            weight_cooling_coef=weight_cooling_coef,
            input_pre_cooling_coef=input_pre_cooling_coef,
            input_post_cooling_coef=input_post_cooling_coef,
            spike_magnitude=spike_magnitude,
            integrator_function=integrator_function,
            name="synapse_plastic_peek_pn"
            )

    @staticmethod
    def plastic_peek_pp(
            initial_input_pre=0.0,
            initial_input_post=0.0,
            initial_weight=1.0,
            weight_neutral=1.0,
            weight_epsilon=1.0,
            weight_zero_level=0.0,
            weight_power=2.0,
            weight_dt=0.1,
            weight_tau_potentiation=1.0,
            weight_tau_depression=1.0,
            weight_cooling_coef=-0.01,
            input_pre_cooling_coef=-50.0,
            input_post_cooling_coef=-50.0,
            spike_magnitude=1.0,
            integrator_function=integrator.euler
            ):
        return Synapse(
            initial_input_pre=initial_input_pre,
            initial_input_post=initial_input_post,
            initial_weight=initial_weight,
            weight_neutral=weight_neutral,
            weight_epsilon=weight_epsilon,
            weight_zero_level=weight_zero_level,
            weight_power=weight_power,
            weight_dt=weight_dt,
            weight_tau_potentiation=weight_tau_potentiation,
            weight_tau_depression=weight_tau_depression,
            weight_sign_function=lambda _: 1.0,
            weight_cooling_coef=weight_cooling_coef,
            input_pre_cooling_coef=input_pre_cooling_coef,
            input_post_cooling_coef=input_post_cooling_coef,
            spike_magnitude=spike_magnitude,
            integrator_function=integrator_function,
            name="synapse_plastic_peek_pp"
            )

    @staticmethod
    def plastic_peek_nn(
            initial_input_pre=0.0,
            initial_input_post=0.0,
            initial_weight=1.0,
            weight_neutral=1.0,
            weight_epsilon=1.0,
            weight_zero_level=0.0,
            weight_power=2.0,
            weight_dt=0.1,
            weight_tau_potentiation=1.0,
            weight_tau_depression=1.0,
            weight_cooling_coef=-0.01,
            input_pre_cooling_coef=-50.0,
            input_post_cooling_coef=-50.0,
            spike_magnitude=1.0,
            integrator_function=integrator.euler
            ):
        return Synapse(
            initial_input_pre=initial_input_pre,
            initial_input_post=initial_input_post,
            initial_weight=initial_weight,
            weight_neutral=weight_neutral,
            weight_epsilon=weight_epsilon,
            weight_zero_level=weight_zero_level,
            weight_power=weight_power,
            weight_dt=weight_dt,
            weight_tau_potentiation=weight_tau_potentiation,
            weight_tau_depression=weight_tau_depression,
            weight_sign_function=lambda _: -1.0,
            weight_cooling_coef=weight_cooling_coef,
            input_pre_cooling_coef=input_pre_cooling_coef,
            input_post_cooling_coef=input_post_cooling_coef,
            spike_magnitude=spike_magnitude,
            integrator_function=integrator_function,
            name="synapse_plastic_peek_nn"
            )

    def get_name(self):
        return self._name

    def derivatives(self, var):
        if abs(self._variables["input_post"] - self._variables["input_pre"]) < 0.001:
            weight_slope_scale = 0.0
        else:
            dI = self._variables["input_post"] - self._variables["input_pre"]
            q = (self._weight_tau_potentiation + self._weight_tau_depression) / 2.0
            tau = (q - self._weight_tau_depression) * math.copysign(1.0, dI) + q
            weight_slope_scale = (
                self._weight_sign_function(dI)
                * self._variables["input_post"]
                * self._variables["input_pre"]
                * math.exp(-abs(dI / tau))
                * max(0.0, 1.0 - (abs(var["weight"] - self._weight_neutral) / self._weight_epsilon)**self._weight_power)
                )
        return {
            "input_pre": self._input_pre_cooling_coef * var["input_pre"],
            "input_post": self._input_post_cooling_coef * var["input_post"],
            "weight": (
                (2.0 * self._weight_epsilon / self._weight_dt) * weight_slope_scale +
                self._weight_zero_level +
                self._weight_cooling_coef * (var["weight"] - self._weight_neutral)
                )
            }

    def get_weight(self):
        return self._variables["weight"]

    def get_neutral_weight(self):
        return self._weight_neutral

    def get_min_weight(self):
        return self._weight_neutral - self._weight_epsilon

    def get_max_weight(self):
        return self._weight_neutral + self._weight_epsilon

    def on_pre_synaptic_spike(self):
        self._variables["input_pre"] += self._spike_magnitude

    def on_post_synaptic_spike(self):
        self._variables["input_post"] += self._spike_magnitude

    def integrate(self, dt):
        self._integrator(dt, self._variables, self.derivatives)

    def get_variables(self):
        return self._variables

    def get_key_variables(self):
        return {"weight": self.get_weight()}

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
            ", lvl=" + str(self._weight_zero_level) +
            ", m=" + str(self._weight_power) +
            ", wdt=" + str(self._weight_dt) +
            ", taup=" + str(self._weight_tau_potentiation) +
            ", taun=" + str(self._weight_tau_depression) +
            ", cool_w=" + str(self._weight_cooling_coef) +
            ", cool_pre=" + str(self._input_pre_cooling_coef) +
            ", cool_post=" + str(self._input_post_cooling_coef) +
            ", spike magnitude=" + str(self._spike_magnitude) +
            ", integrator=" + str(integrator.get_name(self._integrator))
            )
