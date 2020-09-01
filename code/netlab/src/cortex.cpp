#include <netlab/cortex.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>

namespace netlab {


cortex::cortex()
    : constants{
        100.0f,     // simulation_fequency
        }
    , layers()
    , synapses()
    , random_generator(1U)
{}


void  cortex::recompute_accumulated_connection_probabilities(layer&  l)
{
    float_32_bit  sum = 0.0f;
    for (layer_index  i = 0U, n =(synapse_index)layers.size(); i != n;
            sum = l.constants.to_layer.at(i).accumulated_connection_probability, ++i)
        l.constants.to_layer.at(i).accumulated_connection_probability = sum + l.constants.to_layer.at(i).connection_probability;
    if (sum > 1e-6f)
        for (layer::constant_data::to_layer_data&  data : l.constants.to_layer)
            data.accumulated_connection_probability /= sum;
    l.constants.to_layer.back().accumulated_connection_probability = 1.0f;
}


void  cortex::recompute_mean_input_spiking_frequency(layer_index const  layer_idx)
{
    layer&  l = layers.at(layer_idx);
    if (l.constants.num_dendrites_per_neuron == 0U)
    {
        l.constants.neuron.mean_input_spiking_frequency = 0.0f;
        return;
    }

    float_32_bit  f = 0.0f;
    for (layer&  other : layers)
        f += other.constants.neuron.mean_spiking_frequency *
             other.constants.to_layer.at(layer_idx).connection_probability *
             (float_32_bit)other.constants.num_axons_per_neuron *
             (float_32_bit)other.neurons.size()                
             ;

    l.constants.neuron.mean_input_spiking_frequency =
            f / ((float_32_bit)l.constants.num_dendrites_per_neuron * (float_32_bit)l.neurons.size());
}


void  cortex::recompute_mean_input_spiking_frequencies()
{
    for (layer_index  i = 0U, n =(layer_index)layers.size(); i != n; ++i)
        recompute_mean_input_spiking_frequency(i);
}


void  cortex::recompute_max_input_signal_magnitude(layer_index const  layer_idx)
{
    float_32_bit  e = 0.0f, i = 0.0f;
    for (layer&  other : layers)
        *(other.constants.sign > 0.0f ? &e : &i) +=
                other.constants.to_layer.at(layer_idx).connection_probability *
                (float_32_bit)other.constants.num_axons_per_neuron *
                (float_32_bit)other.neurons.size()                
                ;
    layer&  l = layers.at(layer_idx);
    l.constants.neuron.max_input_signal_magnitude = std::max(e, i) / (float_32_bit)l.neurons.size();
}


void  cortex::recompute_max_input_signal_magnitudes()
{
    for (layer_index  i = 0U, n =(layer_index)layers.size(); i != n; ++i)
        recompute_max_input_signal_magnitude(i);
}


void  cortex::clear()
{
    layers.clear();
    synapses.clear();
}


cortex::layer_index  cortex::add_layer(
        natural_16_bit const  num_neurons,
        natural_16_bit const  num_axons_per_neuron,
        natural_16_bit const  num_dendrites_per_neuron,
        bool const  excitatory
        )
{
    ASSUMPTION(num_neurons > 0U && (natural_32_bit)num_axons_per_neuron + (natural_32_bit)num_dendrites_per_neuron > 0U);

    for (layer&  l : layers)
    {
        l.constants.to_layer.push_back({});
        layer::constant_data::to_layer_data&  data = l.constants.to_layer.back();

        data.connection_probability = 1.0f;
        recompute_accumulated_connection_probabilities(l);

        data.synapse.weight_initial = 0.5f;
        data.synapse.weight_disconnection = 0.1f;
        data.synapse.weight_delta_per_second = 0.25f;
        data.synapse.weight_decay_delta_per_second = 0.01f;
    }

    layers.push_back({});

    layer&  l = layers.back();

    l.constants.sign = excitatory ? 1.0f : -1.0f;
    l.constants.num_axons_per_neuron = num_axons_per_neuron;
    l.constants.num_dendrites_per_neuron = num_dendrites_per_neuron;
    l.constants.neuron.excitation_initial = 0.0f;
    l.constants.neuron.spiking_excitation_initial = 0.9f;
    l.constants.neuron.excitation_recovery = 0.0f;
    l.constants.neuron.ln_of_excitation_decay_coef = -3.0f;
    l.constants.neuron.mean_spiking_frequency = 10.0f;
    l.constants.neuron.mean_input_spiking_frequency = 0.0f; // is recomputed below
    l.constants.neuron.max_input_signal_magnitude = 0.0f; // is recomputed below

    l.constants.to_layer.reserve(layers.size());
    for (layer_index  i = 0U, n =(synapse_index)layers.size(); i != n; ++i)
    {
        l.constants.to_layer.push_back({});
        layer::constant_data::to_layer_data&  data = l.constants.to_layer.back();

        data.connection_probability = 1.0f;

        data.synapse.weight_initial = 0.5f;
        data.synapse.weight_disconnection = 0.1f;
        data.synapse.weight_delta_per_second = 0.25f;
        data.synapse.weight_decay_delta_per_second = 0.01f;
    }
    recompute_accumulated_connection_probabilities(l);

    l.neurons.resize(num_neurons);
    for (neuron&  n : l.neurons)
    {
        n.excitation = l.constants.neuron.excitation_initial;
        n.spiking_excitation = l.constants.neuron.spiking_excitation_initial;
        n.input_signal = 0.0f;
    }

    recompute_mean_input_spiking_frequencies();
    recompute_max_input_signal_magnitudes();

    l.free_axons.reserve(num_neurons * l.constants.num_axons_per_neuron);
    for (neuron_index  i = 0U, n =(neuron_index)l.neurons.size(); i != n; ++i)
        for (axon_index  j = 0U; j != l.constants.num_axons_per_neuron; ++j)
            l.free_axons.push_back(i);

    l.free_dendrites.reserve(num_neurons * l.constants.num_dendrites_per_neuron);
    for (neuron_index  i = 0U, n =(neuron_index)l.neurons.size(); i != n; ++i)
        for (axon_index  j = 0U; j != l.constants.num_dendrites_per_neuron; ++j)
            l.free_dendrites.push_back(i);

    return (layer_index)layers.size() - 1U;
}


void  cortex::set_constant_simulation_frequency(float_32_bit const  frequency)
{
    ASSUMPTION(frequency > 0.0f);
    constants.simulation_fequency = frequency;
}


void  cortex::set_constant_connection_probability(
        layer_index const  from, layer_index const  to, float_32_bit const  value
        )
{
    ASSUMPTION(value >= 0.0f);
    layer&  l = layers.at(from);
    l.constants.to_layer.at(to).connection_probability = value;
    recompute_accumulated_connection_probabilities(l);
    recompute_mean_input_spiking_frequencies();
    recompute_max_input_signal_magnitudes();
}


void  cortex::set_constant_neuron_excitation_initial(
        layer_index const  index, float_32_bit const  value
        )
{
    ASSUMPTION(-1.0f <= value && value <= 1.0f);
    layers.at(index).constants.neuron.excitation_initial = value;
}


void  cortex::set_constant_neuron_spiking_excitation_initial(
        layer_index const  index, float_32_bit const  value
        )
{
    ASSUMPTION(-1.0f <= value && value <= 1.0f);
    layers.at(index).constants.neuron.spiking_excitation_initial = value;
}


void  cortex::set_constant_neuron_excitation_recovery(
        layer_index const  index, float_32_bit const  value
        )
{
    ASSUMPTION(-1.0f <= value && value <= 1.0f);
    layers.at(index).constants.neuron.excitation_recovery = value;
}


void  cortex::set_constant_neuron_ln_of_excitation_decay_coef(
        layer_index const  index, float_32_bit const  value
        )
{
    ASSUMPTION(value <= 0.0f);
    layers.at(index).constants.neuron.ln_of_excitation_decay_coef = value;
}


void  cortex::set_constant_neuron_mean_spiking_frequency(layer_index const  index, float_32_bit const  value)
{
    ASSUMPTION(value > 0.0f);
    layers.at(index).constants.neuron.mean_spiking_frequency = value;
    recompute_mean_input_spiking_frequencies();
}


void  cortex::set_constant_neuron_mean_input_spiking_frequency(layer_index const  index, float_32_bit const  value)
{
    ASSUMPTION(value > 0.0f);
    layers.at(index).constants.neuron.mean_input_spiking_frequency = value;
}


void  cortex::set_constant_neuron_max_input_signal_magnitude(
        layer_index const  index, float_32_bit const  value
        )
{
    ASSUMPTION(0.0001f < value);
    layers.at(index).constants.neuron.max_input_signal_magnitude = value;
}


void  cortex::set_constant_synapse_weight_initial(
        layer_index const  from, layer_index const  to, float_32_bit const  value
        )
{
    ASSUMPTION(0.0f <= value && value <= 1.0f);
    layers.at(from).constants.to_layer.at(to).synapse.weight_initial = value;
}


void  cortex::set_constant_synapse_weight_disconnection(
        layer_index const  from, layer_index const  to, float_32_bit const  value
        )
{
    ASSUMPTION(0.0f <= value && value <= 1.0f);
    layers.at(from).constants.to_layer.at(to).synapse.weight_disconnection = value;
}


void  cortex::set_constant_synapse_weight_delta_per_second(
        layer_index const  from, layer_index const  to, float_32_bit const  value
        )
{
    ASSUMPTION(0.0f <= value && value <= 1.0f);
    layers.at(from).constants.to_layer.at(to).synapse.weight_delta_per_second = value;
}


void  cortex::set_constant_synapse_weight_decay_delta_per_second(
        layer_index const  from, layer_index const  to, float_32_bit const  value
        )
{
    ASSUMPTION(0.0f <= value && value <= 1.0f);
    layers.at(from).constants.to_layer.at(to).synapse.weight_decay_delta_per_second = value;
}


void  cortex::next_round()
{
    TMPROF_BLOCK();

    update_existing_synapses();
    update_neurons();
    clear_input_signal_of_neurons();
    disconnect_weak_synapses();
    build_new_synapses();
}


void  cortex::update_neurons()
{
    TMPROF_BLOCK();

    for (layer&  l : layers)
        update_neurons(l);
}


void  cortex::update_neurons(layer&  l)
{
    TMPROF_BLOCK();

    float_32_bit const  D = std::max(1.0f, l.constants.neuron.max_input_signal_magnitude);
    float_32_bit const  f_over_s =
            constants.simulation_fequency / std::max(1.0f, l.constants.neuron.mean_input_spiking_frequency);
    float_32_bit const  q = std::powf(D / (f_over_s * 0.028f + 0.3f), 1.0f / 2.21f);
    float_32_bit const  w = 0.5f;
    float_32_bit const  a = ( 1.0f / (D*D * q) ) * ( (w*D*D + q*(q - 2.0f*D)) / (D*D + q*(q - 2.0f*D)) );
    float_32_bit const  log_decay_coef = l.constants.neuron.ln_of_excitation_decay_coef / constants.simulation_fequency;
    //float_32_bit const  log_decay_coef = 1.0f + std::logf(l.constants.neuron.excitation_decay_coef) * round_seconds;
    //float_32_bit const  log_decay_coef = std::logf(l.constants.neuron.excitation_decay_coef) * round_seconds;
    float_32_bit const  neg_mult = 0.1f;

    for (neuron&  n : l.neurons)
    {
        float_32_bit  x, mult;
        if (n.input_signal < 0.0f)
        {
            x = -n.input_signal;
            mult = -neg_mult;
        }
        else
        {
            x = n.input_signal;
            mult = 1.0f;
        }
        float_32_bit const  y = x*( ( a*(x - D)*(x - D) ) - ( (x - 2.0f*D) / (D*D) ) );
        n.input_signal = mult * y;

        if (n.excitation >= n.spiking_excitation)
            n.excitation = l.constants.neuron.excitation_recovery;
        else
        {
            n.excitation += n.excitation * log_decay_coef;
            n.excitation += n.input_signal;
        }
    }
}


void  cortex::clear_input_signal_of_neurons()
{
    TMPROF_BLOCK();

    for (layer&  l : layers)
        for (neuron&  n : l.neurons)
            n.input_signal = 0.0f;
}


void  cortex::update_existing_synapses()
{
    TMPROF_BLOCK();

    float_32_bit const  round_seconds = 1.0f / constants.simulation_fequency;
    for (synapse&  s : synapses)
    {
        layer&  from_layer = layers.at(s.from.layer);
        neuron const&  from_neuron = from_layer.neurons.at(s.from.neuron);

        layer&  to_layer = layers.at(s.to.layer);
        neuron&  to_neuron = to_layer.neurons.at(s.to.neuron);

        // Update input signal of 'from' neuron, if 'to' neuron is spiking.

        bool const  is_from_neuron_spiking = from_neuron.excitation >= from_neuron.spiking_excitation;
        if (is_from_neuron_spiking)
            to_neuron.input_signal += from_layer.constants.sign * s.weight;

        // Now we update the weight of the synapse.

        if (from_layer.constants.sign < 0.0f)
        {   // inhibitory
            layer::constant_data::to_layer_data const&  to_layer_data = from_layer.constants.to_layer.at(s.to.layer);
            float_32_bit const  w = 2.0f * s.weight - 1.0f;
            float_32_bit const  a =
                    to_neuron.excitation *
                    std::max(1.0f - w * w, 0.0001f) *
                    to_layer_data.synapse.weight_delta_per_second;
            s.weight = std::max(
                            0.0f,
                            std::min(
                                s.weight + (a - to_layer_data.synapse.weight_decay_delta_per_second) * round_seconds,
                                1.0f));
        }
        else
        {   // excitatory
            float_32_bit const  weight_delta = 0.0f; // TODO!
            s.weight += weight_delta;

            //float_32_bit const  postsynaptic_current_magnitude =
            //        to_layer.conductances.at(s.to.layer) * to_neuron.membrane_voltage_derivative * round_milli_seconds
            //        ;

            //weight_delta = (from_layer.weight_resistances.at(s.to.layer) * from_layer.sign) *
            //               (presynaptic_current_magnitude * postsynaptic_current_magnitude);

        }
    }
}


void  cortex::disconnect_weak_synapses()
{
    TMPROF_BLOCK();

    for (synapse_index  synapse_idx = 0U; synapse_idx < (synapse_index)synapses.size(); )
    {
        synapse&  s = synapses.at(synapse_idx);
        layer&  from_layer = layers.at(s.from.layer);
        if (s.weight <= from_layer.constants.to_layer.at(s.to.layer).synapse.weight_disconnection)
        {
            // Disconnect the synapse -> destroy it.
            from_layer.free_axons.push_back(s.from.neuron);
            layers.at(s.to.layer).free_dendrites.push_back(s.to.neuron);
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
                while (dendrite_layer_idx + 1U <
                            (dendrite_index)axon_layer.constants.to_layer.size()
                        && trial > axon_layer.constants.to_layer.at(dendrite_layer_idx).accumulated_connection_probability)
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
                    axon_layer.constants.to_layer.at(dendrite_layer_idx).synapse.weight_initial         // weight
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
