import os
import soma
import integrator


def __get_my_dir():
    return os.path.dirname(__file__)


def __output_root_dir():
    if str(__get_my_dir()).replace("\\", "/").endswith("E2/code/pycellab"):
        return os.path.normpath(os.path.join(__get_my_dir(), "..", "..", "dist", "evaluation", "pycellab"))
    else:
        return __get_my_dir()


class configuration:
    def __init__(self,
                 dt,
                 nsteps,
                 cell_soma,
                 excitatory_noise_distributions,
                 inhibitory_noise_distributions,
                 excitatory_weights,
                 inhibitory_weights,
                 output_dir,
                 description
                 ):
        self.dt = dt
        self.nsteps = nsteps
        self.cell_soma = cell_soma
        self.excitatory_noise_distributions = excitatory_noise_distributions
        self.inhibitory_noise_distributions = inhibitory_noise_distributions
        self.excitatory_weights = excitatory_weights
        self.inhibitory_weights = inhibitory_weights
        self.output_dir = output_dir
        self.name = os.path.basename(self.output_dir)
        self.description = description


def development():
    num_excitatory = 8
    num_inhibitory = 2
    excitatory_noise = {0.05: 1, 0.1: 0.5, 0.02: 2}
    # excitatory_noise = {10: 60, 20: 100, 30: 65, 40: 35, 50: 20, 60: 10, 70: 5, 80: 3, 90: 2, 100: 1},
    inhibitory_noise = excitatory_noise
    return configuration(
        dt=0.001,
        nsteps=1000,
        cell_soma=soma.leaky_integrate_and_fire(
            initial_potential=-65,
            initial_input=0.0,
            resting_potential=-65,
            firing_potential=-55,
            saturation_potential=-70,
            potential_cooling_coef=-0.1,
            input_cooling_coef=-250.0,
            psp_max_potential=60.0,
            spike_magnitude=1.0,
            integrator_function=integrator.euler
            ),
        excitatory_noise_distributions=[excitatory_noise for _ in range(num_excitatory)],
        inhibitory_noise_distributions=[inhibitory_noise for _ in range(num_inhibitory)],
        excitatory_weights=[1.0 for _ in range(num_excitatory)],
        inhibitory_weights=[1.0 for _ in range(num_inhibitory)],
        output_dir=os.path.join(__output_root_dir(), "development"),
        description="This is not a genuine configuration. It serves only for development, testing, and "
                    "bug-fixing of this evaluation system."
        )


def get_registered_configurations():
    return [
        development()
        ]
