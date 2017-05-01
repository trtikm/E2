import integrator


class leaky_integrate_and_fire:
    def __init__(self,
                 initial_potential=-70,
                 initial_input=40.0,
                 resting_potential=-65,
                 firing_potential=-55,
                 saturation_potential=-70,
                 potential_cooling_coef=-58.0,
                 input_cooling_coef=-250.0,
                 psp_max_potential=-50.0,
                 spike_magnitude=1.0,
                 integrator_function=integrator.midpoint
                 ):
        self.constants = {
            "resting_potential": resting_potential,
            "firing_potential": firing_potential,
            "saturation_potential": saturation_potential,
            "potential_cooling_coef": potential_cooling_coef,
            "spike_magnitude": spike_magnitude,
            "input_cooling_coef": input_cooling_coef,
            "psp_max_potential": psp_max_potential
            }
        self.variables = {
            "potential": initial_potential,
            "input": initial_input
            }
        assert callable(integrator_function)
        self.integrator = integrator_function

    def derivatives(self, var):
        psp_voltage = var["input"] * (self.constants["psp_max_potential"] - self.constants["resting_potential"])
        # * (self.constants["psp_max_potential"] - var["potential"])
        # / (self.constants["psp_max_potential"] - self.constants["resting_potential"])
        return {
            "potential":
                psp_voltage
                    + (var["potential"] - self.constants["resting_potential"])
                        * self.constants["potential_cooling_coef"],
            "input":
                self.constants["input_cooling_coef"] * var["input"]
            }

    def integrate(self, dt):
        if self.is_spiking():
            self.variables["potential"] = self.constants["saturation_potential"]
            self.variables["input"] = 0.0
        else:
            self.integrator(dt, self.variables, self.derivatives)

    def ranges_of_variables(self, num_spike_trains):
        return {
            "potential":
                (self.constants["saturation_potential"], self.constants["firing_potential"]),
            "input":
                (-num_spike_trains * self.constants["spike_magnitude"],
                 +num_spike_trains * self.constants["spike_magnitude"])
            }

    def is_spiking(self):
        return self.variables["potential"] >= self.constants["firing_potential"]

    def on_excitatory_spike(self, weight):
        assert weight >= 0.0
        self.variables["input"] += weight * self.constants["spike_magnitude"]

    def on_inhibitory_spike(self, weight):
        assert weight >= 0.0
        self.variables["input"] -= weight * self.constants["spike_magnitude"]

    def get_on_spike_variable_names(self):
        return ["input"]


class izhikevich:
    def __init__(self,
                 initial_potential_U,
                 initial_potential_V,
                 initial_input,
                 resting_potential_U,
                 resting_potential_V,
                 firing_potential,
                 coef_a,
                 coef_b,
                 spike_magnitude=1.0,
                 integrator_function=integrator.midpoint
                 ):
        self.constants = {
            "resting_potential_U": resting_potential_U,
            "resting_potential_V": resting_potential_V,
            "firing_potential": firing_potential,
            "coef_a": coef_a,
            "coef_b": coef_b,
            "spike_magnitude": spike_magnitude
            }
        self.variables = {
            "U": initial_potential_U,
            "V": initial_potential_V,
            "input": initial_input
            }
        assert callable(integrator_function)
        self.integrator = integrator_function

    @staticmethod
    def default(initial_potential_U=-14.0,
                initial_potential_V=-70.0,
                initial_input=0.0,
                spike_magnitude=1.0,
                integrator_function=integrator.midpoint
                ):
        return izhikevich(
                    initial_potential_U=initial_potential_U,
                    initial_potential_V=initial_potential_V,
                    initial_input=initial_input,
                    resting_potential_U=2.0,
                    resting_potential_V=-65.0,
                    firing_potential=30.0,
                    coef_a=0.02,
                    coef_b=0.2,
                    spike_magnitude=spike_magnitude,
                    integrator_function=integrator_function
                    )

    @staticmethod
    def fast_spiking(initial_potential_U=-14.0,
                     initial_potential_V=-70.0,
                     initial_input=0.0,
                     spike_magnitude=1.0,
                     integrator_function=integrator.midpoint
                     ):
        return izhikevich(
                    initial_potential_U=initial_potential_U,
                    initial_potential_V=initial_potential_V,
                    initial_input=initial_input,
                    resting_potential_U=2.0,
                    resting_potential_V=-65.0,
                    firing_potential=30.0,
                    coef_a=0.1,
                    coef_b=0.2,
                    spike_magnitude=spike_magnitude,
                    integrator_function=integrator_function
                    )

    @staticmethod
    def regular_spiking(initial_potential_U=-14.0,
                        initial_potential_V=-70.0,
                        initial_input=0.0,
                        spike_magnitude=1.0,
                        integrator_function=integrator.midpoint
                        ):
        return izhikevich(
                    initial_potential_U=initial_potential_U,
                    initial_potential_V=initial_potential_V,
                    initial_input=initial_input,
                    resting_potential_U=8.0,
                    resting_potential_V=-65.0,
                    firing_potential=30.0,
                    coef_a=0.02,
                    coef_b=0.2,
                    spike_magnitude=spike_magnitude,
                    integrator_function=integrator_function
                    )

    @staticmethod
    def bursting(initial_potential_U=-14.0,
                 initial_potential_V=-70.0,
                 initial_input=0.0,
                 spike_magnitude=1.0,
                 integrator_function=integrator.midpoint
                 ):
        return izhikevich(
                    initial_potential_U=initial_potential_U,
                    initial_potential_V=initial_potential_V,
                    initial_input=initial_input,
                    resting_potential_U=2.0,
                    resting_potential_V=-50.0,
                    firing_potential=30.0,
                    coef_a=0.02,
                    coef_b=0.2,
                    spike_magnitude=spike_magnitude,
                    integrator_function=integrator_function
                    )

    def derivatives(self, var):
        psp_voltage = var["input"] * (self.constants["firing_potential"] - self.constants["resting_potential_V"])
        return {
            "U":
                self.constants["coef_a"] * (self.constants["coef_b"] * var["V"] - var["U"]),
            "V":
                0.04 * var["V"]**2.0 + 5.0 * var["V"] + 140 - var["U"] + psp_voltage,
            "input":
                self.constants["input_cooling_coef"] * var["input"]
            }

    def integrate(self, dt):
        if self.is_spiking():
            self.variables["U"] += self.constants["resting_potential_U"]
            self.variables["V"] = self.constants["resting_potential_V"]
            self.variables["input"] = 0.0
        else:
            self.integrator(dt, self.variables, self.derivatives)

    def ranges_of_variables(self, num_spike_trains):
        return {
            "U":
                (self.constants["resting_potential_V"] - 20.0, self.constants["firing_potential"] + 100.0),
            "V":
                (self.constants["resting_potential_V"] - 20.0, self.constants["firing_potential"] + 100.0),
            "input":
                (-num_spike_trains * self.constants["spike_magnitude"],
                 +num_spike_trains * self.constants["spike_magnitude"])
            }

    def is_spiking(self):
        return self.variables["V"] > self.constants["firing_potential"]

    def on_excitatory_spike(self, weight):
        assert weight >= 0.0
        self.variables["input"] += weight * self.constants["spike_magnitude"]

    def on_inhibitory_spike(self, weight):
        assert weight >= 0.0
        self.variables["input"] -= weight * self.constants["spike_magnitude"]

    def get_on_spike_variable_names(self):
        return ["input"]
