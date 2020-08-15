#ifndef NETLAB_CORTEX_HPP_INCLUDED
#   define NETLAB_CORTEX_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/random.hpp>
#   include <vector>

namespace netlab {


struct  cortex
{
    using  layer_index = natural_16_bit;
    using  neuron_index = natural_16_bit;
    using  synapse_index = natural_32_bit;
    using  axon_index = natural_32_bit;
    using  dendrite_index = natural_32_bit;

    struct  neuron_guid
    {
        layer_index  layer;
        neuron_index  neuron;
    };

    struct  neuron // Izhikevich model
    {
        struct integration_constants
        {
            float_32_bit  treashold; // in mV
            float_32_bit  a;
            float_32_bit  b;
            float_32_bit  c;
            float_32_bit  d;
        };

        float_32_bit  membrane_voltage; // in mV
        float_32_bit  membrane_voltage_derivative;
        float_32_bit  recovery_voltage; // in mV
        float_32_bit  recovery_voltage_derivative;
        float_32_bit  input_current; // in mA
    };

    struct  synapse // Hebbian pasticity model; weight treashold used for disconnection
    {
        neuron_guid  from;
        neuron_guid  to;
        float_32_bit  weight;
    };

    struct  layer
    {
        // --- CONSTANTS ------------------------------------------------------------------------
        neuron::integration_constants  integration_constants; // From the Izhikevich model.
        float_32_bit  sign; // Sign of an action potential: 1.0f - excitatory, -1.0f - inhibitory.
        std::vector<float_32_bit>  conductances; // For conversion of voltage derivative to current - for each layer one coef > 0.
        std::vector<float_32_bit>  weight_resistances; // Scale For update of synaptic weight from change in pre- and post- synaptic
                                                       // voltage changes - for each layer one resistance > 0.
        std::vector<float_32_bit>  disconnection_weights; // For diconnection of synapses - For each layer one number > 0.
        std::vector<float_32_bit>  initial_weights; // For newly created synapse - for each layer one weight > corresponding
                                                    // disconnection_weight.
        std::vector<float_32_bit>  connection_probabilities; // For connection of synapses - For each layer one number >= 0.
        std::vector<float_32_bit>  accumulated_connection_probabilities; // i-th number is the normalised sum of all probabilities
                                                                         // connection_probabilities[0..i]. The last number is 1.0f.

        // --- DATA ------------------------------------------------------------------------
        std::vector<neuron>  neurons; // Fixed size.
        std::vector<neuron_index>  free_axons;
        std::vector<neuron_index>  free_dendrites;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    // CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////////////////

    cortex();

    void  reset_random_generator(natural_32_bit const  seed) { random_generator.seed(seed); }

    layer_index  add_layer(
                natural_16_bit const  num_neurons,
                natural_16_bit const  num_axons_per_neuron,
                natural_16_bit const  num_dendrites_per_neuron,
                bool const  excitatory
                );

    void  set_integration_constants(layer_index const  index, neuron::integration_constants const&  constants)
    { layers.at(index).integration_constants = constants;}

    void  set_conductance(layer_index const  from, layer_index const  to, float_32_bit const  value)
    { layers.at(from).conductances.at(to) = value; }

    void  set_weight_resistance(layer_index const  from, layer_index const  to, float_32_bit const  value)
    { layers.at(from).weight_resistances.at(to) = value; }

    void  set_disconnection_weight(layer_index const  from, layer_index const  to, float_32_bit const  value)
    { layers.at(from).disconnection_weights.at(to) = value; }

    void  set_initial_weight(layer_index const  from, layer_index const  to, float_32_bit const  value)
    { layers.at(from).initial_weights.at(to) = value; }

    void  set_connection_probability(layer_index const  from, layer_index const  to, float_32_bit const  value);

    void  build_new_synapses();

    ////////////////////////////////////////////////////////////////////////////////////////
    // GENERAL GETTERS
    ////////////////////////////////////////////////////////////////////////////////////////

    natural_16_bit  num_layers() const { return (natural_16_bit)layers.size(); }
    natural_32_bit  num_synapses() const { return (natural_32_bit)synapses.size(); }

    layer const&  get_layer(layer_index const  index) const { return layers.at(index); }
    synapse const&  get_synapse(synapse_index const  index) const { return synapses.at(index); }

    ////////////////////////////////////////////////////////////////////////////////////////
    // I/O
    ////////////////////////////////////////////////////////////////////////////////////////

    void  add_input_current_in_mA(neuron_guid const  guid, float_32_bit const  input_current_delta)
    { layers.at(guid.layer).neurons.at(guid.neuron).input_current += input_current_delta; }

    float_32_bit  membrane_voltage_in_mV(neuron_guid const  guid) const
    { return layers.at(guid.layer).neurons.at(guid.neuron).membrane_voltage; }

    ////////////////////////////////////////////////////////////////////////////////////////
    // SIMULATION
    ////////////////////////////////////////////////////////////////////////////////////////

    void  next_round(float_32_bit const  round_seconds);

private:
    void  update_neurons(float_32_bit const  round_seconds);
    void  update_existing_synapses(float_32_bit const  round_seconds);

    std::vector<layer>  layers;
    std::vector<synapse>  synapses;
    random_generator_for_natural_32_bit  random_generator;
};


}

#endif
