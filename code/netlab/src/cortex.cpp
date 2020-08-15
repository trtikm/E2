#include <netlab/cortex.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>

namespace netlab {


cortex::cortex()
    : layers()
    , synapses()
    , random_generator(1U)
{}


void cortex::set_connection_probability(layer_index const  from, layer_index const  to, float_32_bit const  value)
{
    ASSUMPTION(value >= 0.0f);

    layer&  l = layers.at(from);

    l.connection_probabilities.at(to) = value;

    float_32_bit  sum = 0.0f;
    for (layer_index  i = 0U, n =(synapse_index)layers.size(); i != n; ++i, sum = l.accumulated_connection_probabilities.at(i))
        l.accumulated_connection_probabilities.at(i) = sum + l.connection_probabilities.at(i);
    if (sum > 1e-6f)
        for (float_32_bit&  ap : l.accumulated_connection_probabilities)
            ap /= sum;
    l.accumulated_connection_probabilities.back() = 1.0f;
}


cortex::layer_index  cortex::add_layer(
        natural_16_bit const  num_neurons,
        natural_16_bit const  num_axons_per_neuron,
        natural_16_bit const  num_dendrites_per_neuron,
        bool const  excitatory
        )
{
    for (layer&  l : layers)
    {
        l.conductances.push_back(1.0f);
        l.weight_resistances.push_back(1.0f);
        l.disconnection_weights.push_back(0.0f);
        l.initial_weights.push_back(1.0f);
        l.accumulated_connection_probabilities.push_back(l.accumulated_connection_probabilities.back());
    }

    layers.push_back({});

    layer&  l = layers.back();

    // Setup for Izhikevich regular spiking neuron.
    l.integration_constants.treashold = 30.0f;
    l.integration_constants.a = 0.02f;
    l.integration_constants.b = 0.2f;
    l.integration_constants.c = -65.0f;
    l.integration_constants.d = 8.0f;

    l.sign = excitatory ? 1.0f : -1.0f;

    for (layer_index  i = 0U, n =(synapse_index)layers.size(); i != n; ++i)
    {
        l.conductances.push_back(1.0f);
        l.weight_resistances.push_back(1.0f);
        l.disconnection_weights.push_back(0.0f);
        l.initial_weights.push_back(1.0f);
        l.accumulated_connection_probabilities.push_back(1.0f);
    }

    l.neurons.resize(num_neurons);
    for (neuron&  n : l.neurons)
    {
        n.membrane_voltage = l.integration_constants.c;
        n.membrane_voltage_derivative = 0.0f;
        n.recovery_voltage = -0.5f * l.integration_constants.treashold;
        n.recovery_voltage_derivative = 0.0f;
        n.input_current = 0.0f;
    }

    l.free_axons.reserve(num_neurons * num_axons_per_neuron);
    for (neuron_index  i = 0U, n =(neuron_index)l.neurons.size(); i != n; ++i)
        for (axon_index  j = 0U; i != num_axons_per_neuron; ++j)
            l.free_axons.push_back(i);

    l.free_dendrites.reserve(num_neurons * num_dendrites_per_neuron);
    for (neuron_index  i = 0U, n =(neuron_index)l.neurons.size(); i != n; ++i)
        for (axon_index  j = 0U; i != num_dendrites_per_neuron; ++j)
            l.free_dendrites.push_back(i);

    return (layer_index)layers.size() - 1U;
}


void  cortex::next_round(float_32_bit const  round_seconds)
{
    TMPROF_BLOCK();

    update_neurons(round_seconds);
    update_existing_synapses(round_seconds);
    build_new_synapses();
}


void  cortex::update_neurons(float_32_bit const  round_seconds)
{
    TMPROF_BLOCK();

    // Izhikevich's differential equations are expressed in 'ms' time unit.
    float_32_bit const  round_milli_seconds = 1000.0f * round_seconds;

    for (layer&  l : layers)
        for (neuron&  n : l.neurons)
        {
            // Integrate Izhikevich's differential equations.

            n.membrane_voltage_derivative =
                    0.04f * n.membrane_voltage * n.membrane_voltage
                    + 5.0f * n.membrane_voltage
                    + 140.0f
                    - n.recovery_voltage
                    + n.input_current
                    ;
            n.recovery_voltage_derivative = 
                    l.integration_constants.a * (l.integration_constants.b * n.membrane_voltage - n.recovery_voltage)
                    ;

            n.membrane_voltage += n.membrane_voltage_derivative * round_milli_seconds; // Euler integrator
            n.recovery_voltage += n.recovery_voltage_derivative * round_milli_seconds; // Euler integrator

            if (n.recovery_voltage > l.integration_constants.treashold)
            {
                n.membrane_voltage = l.integration_constants.c;
                n.recovery_voltage += l.integration_constants.d;
            }

            // And we reset input current for the next round.

            n.input_current = 0.0f;
        }
}


void  cortex::update_existing_synapses(float_32_bit const  round_seconds)
{
    TMPROF_BLOCK();

    // Voltage derivatives of the Izhikevich's model are expressed in 'ms' time unit.
    float_32_bit const  round_milli_seconds = 1000.0f * round_seconds;

    for (synapse_index  synapse_idx = 0U; synapse_idx < (synapse_index)synapses.size(); )
    {
        synapse&  s = synapses.at(synapse_idx);

        layer&  from_layer = layers.at(s.from.layer);
        neuron const&  from_neuron = from_layer.neurons.at(s.from.neuron);

        layer&  to_layer = layers.at(s.to.layer);
        neuron&  to_neuron = to_layer.neurons.at(s.to.neuron);

        // First we deliver the presynaptic current from the 'from' neuron to the 'to' neuron.

        float_32_bit const  presynaptic_current_magnitude =
                from_layer.conductances.at(s.from.layer) * from_neuron.membrane_voltage_derivative * round_milli_seconds
                ;
        to_neuron.input_current += from_layer.sign * presynaptic_current_magnitude * s.weight;

        // Now we update the weight of the synapse using the Hebbian learning algo.

        float_32_bit const  postsynaptic_current_magnitude =
                to_layer.conductances.at(s.to.layer) * to_neuron.membrane_voltage_derivative * round_milli_seconds
                ;

        s.weight += (from_layer.weight_resistances.at(s.to.layer) * from_layer.sign) *
                    (presynaptic_current_magnitude * postsynaptic_current_magnitude);

        // Finally, we apply the disconnection check.

        if (s.weight <= from_layer.disconnection_weights.at(s.to.layer))
        {
            // Disconnect the synapse -> destroy it.
            from_layer.free_axons.push_back(s.from.neuron);
            to_layer.free_dendrites.push_back(s.to.neuron);
            std::swap(s, synapses.back());
            synapses.pop_back();
        }
        else
            ++synapse_idx; // The synapse survived -> move to the next synapse.
    }
}


void  cortex::build_new_synapses()
{
    TMPROF_BLOCK();

    for (layer_index  axon_layer_idx = 0U; axon_layer_idx < (layer_index)layers.size(); ++axon_layer_idx)
    {
        layer&  axon_layer = layers.at(axon_layer_idx);
        for (axon_index  axon_idx = 0U; axon_idx < (axon_index)axon_layer.free_axons.size(); )
        {
            layer_index  dendrite_layer_idx = 0U;
            {
                float_32_bit const  trial = get_random_float_32_bit_in_range(0.0f, 1.0f, random_generator);
                while (dendrite_layer_idx + 1U < (dendrite_index)axon_layer.accumulated_connection_probabilities.size() &&
                       trial > axon_layer.accumulated_connection_probabilities.at(dendrite_layer_idx))
                    ++dendrite_layer_idx;
            }
            layer&  dendrite_layer = layers.at(dendrite_layer_idx);
            if (!dendrite_layer.free_dendrites.empty())
            {
                dendrite_index const  dendrite_idx =
                    get_random_natural_32_bit_in_range(0U, (natural_32_bit)dendrite_layer.free_dendrites.size() - 1U,
                                                       random_generator);

                // Build a new synapse.

                synapses.push_back(synapse{
                    neuron_guid{ axon_layer_idx, axon_layer.free_axons.at(axon_idx) },                  // from
                    neuron_guid{ dendrite_layer_idx, dendrite_layer.free_dendrites.at(dendrite_idx) },  // to
                    axon_layer.initial_weights.at(dendrite_layer_idx)                                   // weight
                    });

                // Replace the axon by the last free axons in the vector.
                axon_layer.free_axons.at(axon_idx) = axon_layer.free_axons.back();
                axon_layer.free_axons.pop_back();

                // Replace the dendrite by the last free dendrites in the vector.
                dendrite_layer.free_dendrites.at(dendrite_idx) = dendrite_layer.free_dendrites.back();
                dendrite_layer.free_dendrites.pop_back();
            }
            else
                ++axon_idx; // We failed to connect this axon -> mode to the next one.
        }
    }
}


}
