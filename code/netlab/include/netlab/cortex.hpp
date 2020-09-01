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

    struct  neuron
    {
        struct constant_data
        {
            float_32_bit  excitation_initial; // in <-1,1>; default 0.0f
            float_32_bit  spiking_excitation_initial; // in <-1,1>; default 0.9f
            float_32_bit  excitation_recovery; // in <-1,1>; default 0.0f
            float_32_bit  ln_of_excitation_decay_coef; // <=0; default -3.0f
            float_32_bit  mean_spiking_frequency; // >0; default 10.0f
            float_32_bit  mean_input_spiking_frequency; // >0; default is computed.
            float_32_bit  max_input_signal_magnitude; // >0; default is computed.
        };

        float_32_bit  excitation; // in <-1, 1>
        float_32_bit  spiking_excitation; // in <0, 1>
        float_32_bit  input_signal; // in <-D,D>, where D is number of dendrites of the neuron
    };

    struct  synapse // Hebbian pasticity model; weight treashold used for disconnection
    {
        struct constant_data
        {
            float_32_bit  weight_initial; // in <0,1>; default 0.5
            float_32_bit  weight_disconnection; // in <0,1>; default 0.1
            float_32_bit  weight_delta_per_second; // in <0,1>; default 0.25
            float_32_bit  weight_decay_delta_per_second; // in <0,1>; default 0.01
        };

        neuron_guid  from;
        neuron_guid  to;
        float_32_bit  weight; // in <0,1>
    };

    struct  layer
    {
        struct  constant_data
        {
            neuron::constant_data  neuron;

            float_32_bit  sign; // Sign of the output action potential: 1.0f - excitatory, -1.0f - inhibitory.
            natural_16_bit  num_axons_per_neuron;
            natural_16_bit  num_dendrites_per_neuron;

            struct  to_layer_data
            {
                float_32_bit  connection_probability; // in <0,1>
                float_32_bit  accumulated_connection_probability; // >= 0

                synapse::constant_data  synapse;

            };
            std::vector<to_layer_data>  to_layer;

        } constants;

        std::vector<neuron>  neurons; // Fixed size.
        std::vector<neuron_index>  free_axons;
        std::vector<neuron_index>  free_dendrites;
    };

    struct  constant_data
    {
        float_32_bit  simulation_fequency; // >0.0f default 100.0f
        //float_32_bit  expected_spiking_frequency;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    // CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////////////////

    cortex();

    void  reset_random_generator(natural_32_bit const  seed) { random_generator.seed(seed); }

    void  clear();

    layer_index  add_layer(
                natural_16_bit const  num_neurons,
                natural_16_bit const  num_axons_per_neuron,
                natural_16_bit const  num_dendrites_per_neuron,
                bool const  excitatory
                );

    //void  set_constant_expected_spiking_frequency(float_32_bit const  value);

    void  set_constant_connection_probability(layer_index const  from, layer_index const  to, float_32_bit const  value);

    void  set_constant_neuron_excitation_initial(layer_index const  index, float_32_bit const  value);
    void  set_constant_neuron_spiking_excitation_initial(layer_index const  index, float_32_bit const  value);
    void  set_constant_neuron_excitation_recovery(layer_index const  index, float_32_bit const  value);
    void  set_constant_neuron_ln_of_excitation_decay_coef(layer_index const  index, float_32_bit const  value);
    void  set_constant_neuron_mean_spiking_frequency(layer_index const  index, float_32_bit const  value);
    void  set_constant_neuron_mean_input_spiking_frequency(layer_index const  index, float_32_bit const  value);
    void  set_constant_neuron_max_input_signal_magnitude(layer_index const  index, float_32_bit const  value);

    void  set_constant_synapse_weight_initial(layer_index const  from, layer_index const  to, float_32_bit const  value);
    void  set_constant_synapse_weight_disconnection(layer_index const  from, layer_index const  to, float_32_bit const  value);
    void  set_constant_synapse_weight_delta_per_second(layer_index const  from, layer_index const  to, float_32_bit const  value);
    void  set_constant_synapse_weight_decay_delta_per_second(layer_index const  from, layer_index const  to, float_32_bit const  value);
    
    void  build_new_synapses();

    ////////////////////////////////////////////////////////////////////////////////////////
    // GENERAL GETTERS
    ////////////////////////////////////////////////////////////////////////////////////////

    natural_16_bit  num_layers() const { return (natural_16_bit)layers.size(); }
    natural_32_bit  num_synapses() const { return (natural_32_bit)synapses.size(); }

    layer const&  get_layer(layer_index const  index) const { return layers.at(index); }
    layer&  layer_ref(layer_index const  index) { return layers.at(index); }

    synapse const&  get_synapse(synapse_index const  index) const { return synapses.at(index); }
    synapse&  synapse_ref(synapse_index const  index) { return synapses.at(index); }

    neuron const&  get_neuron(neuron_guid const  guid) const { return layers.at(guid.layer).neurons.at(guid.neuron); }
    neuron&  neuron_ref(neuron_guid const  guid) { return layers.at(guid.layer).neurons.at(guid.neuron); }

    constant_data const&  get_constants() const { return constants; }
    layer::constant_data const&  get_layer_constants(layer_index const  index) const { return layers.at(index).constants; }

    ////////////////////////////////////////////////////////////////////////////////////////
    // I/O
    ////////////////////////////////////////////////////////////////////////////////////////

    void  add_to_input_signal(neuron_guid const  guid, float_32_bit const  input_signal_delta)
    { layers.at(guid.layer).neurons.at(guid.neuron).input_signal += input_signal_delta; }

    float_32_bit  neuron_excitation(neuron_guid const  guid) const { return get_neuron(guid).excitation; }
    bool  is_neuron_spiking(neuron_guid const  guid) const
    { neuron const&  n = get_neuron(guid); return n.excitation >=n.spiking_excitation ; }

    ////////////////////////////////////////////////////////////////////////////////////////
    // SIMULATION
    ////////////////////////////////////////////////////////////////////////////////////////

    void  set_constant_simulation_frequency(float_32_bit const  frequency);

    void  next_round();

    void  update_neurons();
    void  update_neurons(layer&  l);
    void  clear_input_signal_of_neurons();
    void  update_existing_synapses();
    void  disconnect_weak_synapses();

private:
    void  recompute_accumulated_connection_probabilities(layer&  l);
    void  recompute_mean_input_spiking_frequency(layer_index const  layer_idx);
    void  recompute_mean_input_spiking_frequencies();
    void  recompute_max_input_signal_magnitude(layer_index const  layer_idx);
    void  recompute_max_input_signal_magnitudes();

    constant_data  constants;
    std::vector<layer>  layers;
    std::vector<synapse>  synapses;
    random_generator_for_natural_32_bit  random_generator;
};


}

#endif
