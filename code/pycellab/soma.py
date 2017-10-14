import numpy
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
        self._integrator = integrator_function

    @staticmethod
    def get_name():
        return "leaky_integrate_and_fire"

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
            self._integrator(dt, self.variables, self.derivatives)

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

    def get_variables(self):
        return self.variables

    def get_key_variables(self):
        return {"potential": self.get_variables()["potential"]}

    @staticmethod
    def get_on_spike_variable_names():
        return ["input"]

    def get_short_description(self):
        return (
            "Leaky IF"
            ", V[resting]=" + str(self.constants["resting_potential"]) +
            ", V[firing]=" + str(self.constants["firing_potential"]) +
            ", V[saturation]=" + str(self.constants["saturation_potential"]) +
            ", V[psp_max]=" + str(self.constants["psp_max_potential"]) +
            ", V[cooling]=" + str(self.constants["potential_cooling_coef"]) +
            ", I[cooling]=" + str(self.constants["input_cooling_coef"]) +
            ", spike magnitude=" + str(self.constants["spike_magnitude"]) +
            ", intergator=" + str(integrator.get_name(self._integrator))
            )


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
                 input_cooling_coef,
                 spike_magnitude=1.0,
                 integrator_function=integrator.midpoint,
                 name="izhikevich_custom"
                 ):
        self.constants = {
            "resting_potential_U": resting_potential_U,
            "resting_potential_V": resting_potential_V,
            "firing_potential": firing_potential,
            "coef_a": coef_a,
            "coef_b": coef_b,
            "input_cooling_coef": input_cooling_coef,
            "spike_magnitude": spike_magnitude
            }
        self.variables = {
            "U": initial_potential_U,
            "V": initial_potential_V,
            "input": initial_input
            }
        assert callable(integrator_function)
        self._integrator = integrator_function
        self._name = name

    def get_name(self):
        return self._name

    @staticmethod
    def default(initial_potential_U=-14.0,
                initial_potential_V=-70.0,
                initial_input=0.0,
                input_cooling_coef=-0.25,
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
                    input_cooling_coef=input_cooling_coef,
                    spike_magnitude=spike_magnitude,
                    integrator_function=integrator_function,
                    name="izhikevich_default"
                    )

    @staticmethod
    def regular_spiking(initial_potential_U=-14.0,
                        initial_potential_V=-70.0,
                        initial_input=0.0,
                        input_cooling_coef=-0.25,
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
                    input_cooling_coef=input_cooling_coef,
                    spike_magnitude=spike_magnitude,
                    integrator_function=integrator_function,
                    name="izhikevich_regular_spiking"
                    )

    @staticmethod
    def chattering(initial_potential_U=-14.0,
                   initial_potential_V=-70.0,
                   initial_input=0.0,
                   input_cooling_coef=-0.25,
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
                    input_cooling_coef=input_cooling_coef,
                    spike_magnitude=spike_magnitude,
                    integrator_function=integrator_function,
                    name="izhikevich_chattering"
                    )

    @staticmethod
    def fast_spiking(initial_potential_U=-14.0,
                     initial_potential_V=-70.0,
                     initial_input=0.0,
                     input_cooling_coef=-0.25,
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
                    input_cooling_coef=input_cooling_coef,
                    spike_magnitude=spike_magnitude,
                    integrator_function=integrator_function,
                    name="izhikevich_fast_spiking"
                    )

    def derivatives(self, var):
        psp_voltage = var["input"]
        return {
            "U":
                self.constants["coef_a"] * (self.constants["coef_b"] * var["V"] - var["U"]),
            "V":
                0.04 * var["V"]**2.0 + 5.0 * var["V"] + 140.0 - var["U"] + psp_voltage,
            "input":
                self.constants["input_cooling_coef"] * var["input"]
            }

    def integrate(self, dt):
        if self.is_spiking():
            self.variables["U"] += self.constants["resting_potential_U"]
            self.variables["V"] = self.constants["resting_potential_V"]
            self.variables["input"] = 0.0
        else:
            # Derivatives are expressed in 'ms' time unit, but we have 'dt' in seconds.
            self._integrator(1000.0 * dt, self.variables, self.derivatives)
            if self.variables["V"] > 300.0:
                self.variables["V"] = 300.0

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

    def get_variables(self):
        return self.variables

    def get_key_variables(self):
        return {"V": self.get_variables()["V"]}

    @staticmethod
    def get_on_spike_variable_names():
        return ["input"]

    def get_short_description(self):
        return (
            self.get_name() +
            ", U[resting]=" + str(self.constants["resting_potential_U"]) +
            ", V[resting]=" + str(self.constants["resting_potential_V"]) +
            ", V[firing]=" + str(self.constants["firing_potential"]) +
            ", a=" + str(self.constants["coef_a"]) +
            ", b=" + str(self.constants["coef_b"]) +
            ", I[cooling]=" + str(self.constants["input_cooling_coef"]) +
            ", spike magnitude=" + str(self.constants["spike_magnitude"]) +
            ", intergator=" + str(integrator.get_name(self._integrator))
            )


class hodgkin_huxley:
    def __init__(self,
                 initial_V=0.0,
                 initial_n=0.317729,
                 initial_m=0.052955,
                 initial_h=0.595945,
                 initial_input=0.0,
                 g_K=36.0,
                 g_Na=120.0,
                 g_L=0.3,
                 E_K=-12.0,
                 E_Na=115.0,
                 E_L=10.613,
                 C=1.0,
                 input_cooling_coef=-0.25,
                 spike_magnitude=1.0,
                 integrator_function=integrator.euler,
                 ):
        self.constants = {
            "g_K": g_K,
            "g_Na": g_Na,
            "g_L": g_L,
            "E_K": E_K,
            "E_Na": E_Na,
            "E_L": E_L,
            "C": C,
            "firing_potential": 50.0,
            "spike_magnitude": spike_magnitude,
            "input_cooling_coef": input_cooling_coef,
            }
        self.variables = {
            "V": initial_V,
            "n": initial_n,
            "m": initial_m,
            "h": initial_h,
            "input": initial_input
            }
        assert callable(integrator_function)
        self._integrator = integrator_function

    @staticmethod
    def get_name():
        return "hodgkin_huxley"

    def derivatives(self, var):
        psp_voltage = var["input"]
        return {
            "V":
                (- self.constants["g_K"] * var["n"]**4.0 * (var["V"] - self.constants["E_K"])
                 - self.constants["g_Na"] * var["m"]**3.0 * var["h"] * (var["V"] - self.constants["E_Na"])
                 - self.constants["g_L"] * (var["V"] - self.constants["E_L"])
                 + psp_voltage
                 ) / self.constants["C"],
            "n":
                ((0.1 - 0.01 * var["V"]) / (numpy.exp(1.0 - 0.1 * var["V"]) - 1.0)) * (1.0 - var["n"])
                - (0.125 * numpy.exp(-var["V"] / 80.0)) * var["n"],
            "m":
                ((2.5 - 0.1 * var["V"]) / (numpy.exp(2.5 - 0.1 * var["V"]) - 1.0)) * (1.0 - var["m"])
                - (4.0 * numpy.exp(-var["V"] / 18.0)) * var["m"],
            "h":
                (0.07 * numpy.exp(-var["V"] / 20.0)) * (1.0 - var["h"])
                - (1.0 / (numpy.exp(3.0 - 0.1 * var["V"]) + 1.0)) * var["h"],
            "input":
                self.constants["input_cooling_coef"] * var["input"]
            }

    def integrate(self, dt):
        if self.is_spiking():
            self.variables["input"] = 0.0
        # Derivatives are expressed in 'ms' time unit, but we have 'dt' in seconds.
        self._integrator(1000.0 * dt, self.variables, self.derivatives)

    def ranges_of_variables(self, num_spike_trains):
        return {
            "V":
                (-50.0, 150.0),
            "n":
                (0.0, 1.0),
            "m":
                (0.0, 1.0),
            "h":
                (0.0, 1.0),
            "input":
                (-num_spike_trains * self.constants["spike_magnitude"],
                 +num_spike_trains * self.constants["spike_magnitude"])
            }

    def is_spiking(self):
        return self.variables["V"] >= self.constants["firing_potential"]

    def on_excitatory_spike(self, weight):
        assert weight >= 0.0
        self.variables["input"] += weight * self.constants["spike_magnitude"]

    def on_inhibitory_spike(self, weight):
        assert weight >= 0.0
        self.variables["input"] -= weight * self.constants["spike_magnitude"]

    def get_variables(self):
        return self.variables

    def get_key_variables(self):
        return {"V": self.get_variables()["V"]}

    @staticmethod
    def get_on_spike_variable_names():
        return ["input"]

    def get_short_description(self):
        return (
            self.get_name() +
            ", g_K=" + str(self.constants["g_K"]) +
            ", g_Na=" + str(self.constants["g_Na"]) +
            ", g_L=" + str(self.constants["g_L"]) +
            ", E_K=" + str(self.constants["E_K"]) +
            ", E_Na=" + str(self.constants["E_Na"]) +
            ", E_L=" + str(self.constants["E_L"]) +
            ", C=" + str(self.constants["C"]) +
            ", V[firing]=" + str(self.constants["firing_potential"]) +
            ", I[cooling]=" + str(self.constants["input_cooling_coef"]) +
            ", spike magnitude=" + str(self.constants["spike_magnitude"]) +
            ", intergator=" + str(integrator.get_name(self._integrator))
            )


class wilson:
    def __init__(self,
                 initial_V,
                 initial_R,
                 initial_T,
                 initial_H,
                 initial_input,
                 g_K,
                 g_T,
                 g_H,
                 E_K,
                 E_Na,
                 E_T,
                 E_H,
                 tau_R,
                 tau_T,
                 tau_H,
                 C,
                 input_cooling_coef,
                 spike_magnitude,
                 integrator_function,
                 name="wilson_custom"
                 ):
        self.constants = {
            "g_K": g_K,
            "g_T": g_T,
            "g_H": g_H,
            "E_K": E_K,
            "E_Na": E_Na,
            "E_T": E_T,
            "E_H": E_H,
            "tau_R": tau_R,
            "tau_T": tau_T,
            "tau_H": tau_H,
            "C": C,
            "input_cooling_coef": input_cooling_coef,
            }
        self.variables = {
            "V": initial_V,
            "R": initial_R,
            "T": initial_T,
            "H": initial_H,
            "input": initial_input
            }
        self._firing_potential = -30.0
        self._spike_magnitude = spike_magnitude
        assert callable(integrator_function)
        self._integrator = integrator_function
        self._name = name

    @staticmethod
    def regular_spiking(
                initial_V=-70.0,
                initial_R=0.0,
                initial_T=0.0,
                initial_H=0.0,
                initial_input=0.0,
                input_cooling_coef=-0.25,
                spike_magnitude=1.0,
                integrator_function=integrator.euler
                ):
        return wilson(
                initial_V=initial_V,
                initial_R=initial_R,
                initial_T=initial_T,
                initial_H=initial_H,
                initial_input=initial_input,
                g_K=26.0,
                g_T=0.1,
                g_H=5.0,
                E_K=-95.0,
                E_Na=50.0,
                E_T=120.0,
                E_H=-95.0,
                tau_R=4.2,
                tau_T=14.0,
                tau_H=45.0,
                C=1.0,
                input_cooling_coef=input_cooling_coef,
                spike_magnitude=spike_magnitude,
                integrator_function=integrator_function,
                name="wilson_regular_spiking"
                )

    @staticmethod
    def fast_spiking(
                initial_V=-70.0,
                initial_R=0.0,
                initial_T=0.0,
                initial_H=0.0,
                initial_input=0.0,
                input_cooling_coef=-0.25,
                spike_magnitude=1.0,
                integrator_function=integrator.euler
                ):
        return wilson(
                initial_V=initial_V,
                initial_R=initial_R,
                initial_T=initial_T,
                initial_H=initial_H,
                initial_input=initial_input,
                g_K=26.0,
                g_T=0.25,
                g_H=0.0,
                E_K=-95.0,
                E_Na=50.0,
                E_T=120.0,
                E_H=-95.0,
                tau_R=1.5,
                tau_T=14.0,
                tau_H=45.0,
                C=1.0,
                input_cooling_coef=input_cooling_coef,
                spike_magnitude=spike_magnitude,
                integrator_function=integrator_function,
                name="wilson_fast_spiking"
                )

    @staticmethod
    def bursting(
                initial_V=-70.0,
                initial_R=0.0,
                initial_T=0.0,
                initial_H=0.0,
                initial_input=0.0,
                input_cooling_coef=-0.25,
                spike_magnitude=1.0,
                integrator_function=integrator.euler
                ):
        return wilson(
                initial_V=initial_V,
                initial_R=initial_R,
                initial_T=initial_T,
                initial_H=initial_H,
                initial_input=initial_input,
                g_K=26.0,
                g_T=2.25,
                g_H=9.5,
                E_K=-95.0,
                E_Na=50.0,
                E_T=120.0,
                E_H=-95.0,
                tau_R=4.2,
                tau_T=14.0,
                tau_H=45.0,
                C=1.0,
                input_cooling_coef=input_cooling_coef,
                spike_magnitude=spike_magnitude,
                integrator_function=integrator_function,
                name="wilson_bursting"
                )

    def get_name(self):
        return self._name

    def derivatives(self, var):
        psp_voltage = var["input"]
        return {
            "V":
                (- (17.8 + 0.476*var["V"] + 0.00338*var["V"]*var["V"]) * (var["V"] - self.constants["E_Na"])
                 - self.constants["g_K"] * var["R"] * (var["V"] - self.constants["E_K"])
                 - self.constants["g_T"] * var["T"] * (var["V"] - self.constants["E_T"])
                 - self.constants["g_H"] * var["H"] * (var["V"] - self.constants["E_H"])
                 + psp_voltage
                 ) / self.constants["C"],
            "R":
                -(var["R"] - (1.24 + 0.037*var["V"] + 0.00032*var["V"]*var["V"])) / self.constants["tau_R"],
            "T":
                -(var["T"] - (4.205 + 0.116*var["V"] + 0.0008*var["V"]*var["V"])) / self.constants["tau_T"],
            "H":
                -(var["H"] - 3.0 * var["T"]) / self.constants["tau_H"],
            # "V":
            #     (- (17.8 + 47.6*var["V"] + 33.8*var["V"]*var["V"]) * (var["V"] - self.constants["E_Na"])
            #      - self.constants["g_K"] * var["R"] * (var["V"] - self.constants["E_K"])
            #      - self.constants["g_T"] * var["T"] * (var["V"] - self.constants["E_T"])
            #      - self.constants["g_H"] * var["H"] * (var["V"] - self.constants["E_H"])
            #      + psp_voltage
            #      ) / self.constants["C"],
            # "R":
            #     -(var["R"] - (1.24 + 3.7*var["V"] + 3.2*var["V"]*var["V"])) / self.constants["tau_R"],
            # "T":
            #     -(var["T"] - (4.205 + 11.6*var["V"] + 8.0*var["V"]*var["V"])) / self.constants["tau_T"],
            # "H":
            #     -(var["H"] - 3.0 * var["T"]) / self.constants["tau_H"],
            "input":
                self.constants["input_cooling_coef"] * var["input"]
            }

    def integrate(self, dt):
        if self.is_spiking():
            self.variables["input"] = 0.0
        # Derivatives are expressed in 'ms' time unit, but we have 'dt' in seconds.
        self._integrator(1000.0 * dt, self.variables, self.derivatives)

    def ranges_of_variables(self, num_spike_trains):
        return {
            "V":
                (-80.0, 60.0),
            "R":
                (0.0, 10.0),
            "T":
                (0.0, 10.0),
            "H":
                (0.0, 10.0),
            "input":
                (-num_spike_trains * self._spike_magnitude,
                 +num_spike_trains * self._spike_magnitude)
            }

    def is_spiking(self):
        return self.variables["V"] >= self._firing_potential

    def on_excitatory_spike(self, weight):
        assert weight >= 0.0
        self.variables["input"] += weight * self._spike_magnitude

    def on_inhibitory_spike(self, weight):
        assert weight >= 0.0
        self.variables["input"] -= weight * self._spike_magnitude

    def get_variables(self):
        return self.variables

    def get_key_variables(self):
        return {"V": self.get_variables()["V"]}

    @staticmethod
    def get_on_spike_variable_names():
        return ["input"]

    def get_short_description(self):
        return (
            self.get_name() +
            ", g_K=" + str(self.constants["g_K"]) +
            ", g_T=" + str(self.constants["g_T"]) +
            ", g_H=" + str(self.constants["g_H"]) +
            ", E_K=" + str(self.constants["E_K"]) +
            ", E_Na=" + str(self.constants["E_Na"]) +
            ", E_T=" + str(self.constants["E_T"]) +
            ", E_H=" + str(self.constants["E_H"]) +
            ", tau_R=" + str(self.constants["tau_R"]) +
            ", tau_T=" + str(self.constants["tau_T"]) +
            ", tau_H=" + str(self.constants["tau_H"]) +
            ", C=" + str(self.constants["C"]) +
            "\nV[firing]=" + str(self._firing_potential) +
            ", I[cooling]=" + str(self.constants["input_cooling_coef"]) +
            ", spike magnitude=" + str(self._spike_magnitude) +
            ", intergator=" + str(integrator.get_name(self._integrator))
            )
