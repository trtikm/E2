#ifndef NETLAB_NETWORK_HPP_INCLUDED
#   define NETLAB_NETWORK_HPP_INCLUDED

#   include <netlab/uid.hpp>
#   include <netlab/sockets.hpp>
#   include <netlab/unit.hpp>
#   include <netlab/layer.hpp>
#   include <netlab/statistics.hpp>
#   include <netlab/builder.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/random.hpp>
#   include <vector>
#   include <array>
#   include <mutex>

namespace netlab {


struct  network
{
    network();
    network(
        natural_8_bit const  NUM_INPUT_LAYERS_,
        natural_8_bit const  NUM_OUTPUT_LAYERS_,
        natural_32_bit const  random_generator_seed,
        statistics const&  stats_
        );

    void  set_spiking_input_unit(uid const  input_unit_id);
    std::vector<uid> const&  get_spiking_output_units() const { return spiking_output_units; }
    statistics const&  get_statisitcs() const { return stats; }

    void  next_round();

    network_layer const&  get_layer(uid const  id) const { return layers.at(id.layer); }
    network_layer&  layer_ref(uid const  id) { return layers.at(id.layer); }
    computation_unit const&  get_unit(uid const  id) const { return get_layer(id).units.at(id.unit); }
    computation_unit&  unit_ref(uid const  id) { return layer_ref(id).units.at(id.unit); }
    input_socket const&  get_input_socket(uid const  id) const { return get_unit(id).inputs.at(id.socket); }
    input_socket&  input_socket_ref(uid const  id) { return unit_ref(id).inputs.at(id.socket); }
    output_socket const&  get_output_socket(uid const  id) const { return get_unit(id).outputs.at(id.socket); }
    output_socket&  output_socket_ref(uid const  id) { return unit_ref(id).outputs.at(id.socket); }

private:

    void  update_input_sockets_of_spiking_units();
    void  update_output_sockets_of_spiking_units();
    void  propagate_charge_of_spikes();
    void  discharge_spiking_units();
    void  update_charge_of_units();
    void  connect_open_sockets();
    bool  connect(
            natural_32_bit const  input_idx,
            natural_32_bit const  output_idx
            );
    void  disconnect(
            std::vector<input_socket>&  inputs,
            uid const  input_socket_id,
            std::vector<output_socket>&  outputs,
            uid const  output_socket_id
            );

    friend struct  netlab::builder;

    // CONSTANTS:

    natural_8_bit  NUM_INPUT_LAYERS;        // Input layers are: 0,...,NUM_INPUT_LAYERS-1
    natural_8_bit  NUM_OUTPUT_LAYERS;       // Output layers are: layers.size()-NUM_OUTPUT_LAYERS,...,layers.size()-1

    // DATA:

    std::vector<network_layer>  layers;

    std::vector<uid>  open_inputs;
    std::vector<uid>  open_outputs;
    std::mutex  open_io_mutex;

    std::vector<uid>  spiking_units;
    std::vector<uid>  spiking_output_units;
    std::mutex  spiking_mutex;

    random_generator_for_natural_32_bit  random_generator;

    statistics  stats;
};


}


#endif
