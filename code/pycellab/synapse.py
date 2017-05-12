
class synapse:

    def __init__(self, initial_weight=1.0):
        self._weight = initial_weight

    def get_weight(self):
        return self._weight

    def on_pre_synaptic_spike(self):
        pass

    def on_post_synaptic_spike(self):
        pass

    def integrate(self, dt):
        pass
