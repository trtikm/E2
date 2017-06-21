import distribution


class DataSignal:
    def __init__(self, phase_distribution, spiking_distributions, selector_distribution, start_time):
        assert isinstance(phase_distribution, distribution.distribution)
        assert isinstance(spiking_distributions, list)
        assert len(spiking_distributions) > 0
        assert all(isinstance(dist, distribution.distribution) for dist in spiking_distributions)
        assert isinstance(selector_distribution, distribution.distribution)
        assert selector_distribution.get_events() == [i for i in range(len(spiking_distributions))]
        assert isinstance(start_time, float)
        self._phase_distribution = phase_distribution
        self._spiking_distributions = spiking_distributions
        self._selector_distribution = selector_distribution
        self._phase_end_time = start_time + self._phase_distribution.next_event()
        self._spiking_index = self._selector_distribution.next_event()
        self._last_spike_time = start_time + self._spiking_distributions[self._spiking_index].next_event()

    def on_time_step(self, t, dt):
        if self._phase_end_time <= t + dt:
            self._phase_end_time = t + dt + self._phase_distribution.next_event()
            self._spiking_index = self._selector_distribution.next_event()
            self._last_spike_time = t + dt + self._spiking_distributions[self._spiking_index].next_event()
            return False
        elif self._last_spike_time <= t + dt:
            self._last_spike_time = t + dt + self._spiking_distributions[self._spiking_index].next_event()
            return True
        else:
            return False

    @staticmethod
    def constant_isi(inter_spikes_interval, start_time=0.0):
        assert isinstance(inter_spikes_interval, float) and inter_spikes_interval > 0.00001
        return DataSignal(
            phase_distribution=distribution.distribution(None),
            spiking_distributions=[distribution.distribution({inter_spikes_interval: 1})],
            selector_distribution=distribution.distribution({0: 1}),
            start_time=start_time
        )

    @staticmethod
    def default_excitatory(start_time=0.0):
        return DataSignal(
            phase_distribution=distribution.distribution(distribution.mkhist_by_linear_interpolation([
                # (ISI, count)
                (0.010, 600),
                (0.020, 800),
                (0.030, 600),
                (0.040, 400),
                (0.050, 300),
                (0.060, 200),
                (0.070, 100),
                (0.080, 50),
                (0.090, 25),
                (0.100, 10),
                ], 0.001)),
            spiking_distributions=[
                distribution.distribution(distribution.mkhist_by_linear_interpolation([
                    # (ISI, count)
                    (0.005, 10),
                    (0.010, 100),
                    ], 0.001)),
                distribution.distribution(distribution.mkhist_by_linear_interpolation([
                    # (ISI, count)
                    (0.010, 100),
                    (0.050, 10),
                    (0.100, 0),
                    ], 0.001)),
                ],
            selector_distribution=distribution.distribution(dict([(i, 1.0) for i in range(2)])),
            start_time=start_time
        )



