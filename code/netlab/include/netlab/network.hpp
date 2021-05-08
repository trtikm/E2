#ifndef NETLAB_NETWORK_HPP_INCLUDED
#   define NETLAB_NETWORK_HPP_INCLUDED

#   include <netlab/uid.hpp>
#   include <netlab/sockets.hpp>
#   include <netlab/unit.hpp>
#   include <netlab/layer.hpp>
#   include <netlab/statistics.hpp>
#   include <utility/random.hpp>
#   include <vector>
#   include <array>
#   include <mutex>

namespace netlab {


struct  network
{
    void  set_spiking_input_unit(uid const  input_unit_id);
    std::vector<uid> const&  get_spiking_output_units() const { return spiking_output_units; }
    statistics const&  get_statisitcs() const { return stats; }

    void  next_round();

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

    // CONSTANTS:

    float_32_bit  SPIKE_MAGNITUDE;          // Must be > 0.0f
    natural_8_bit  NUM_INPUT_LAYERS;
    natural_8_bit  NUM_OUTPUT_LAYERS;

    // DATA:

    std::vector<network_layer>  layers;     // Cannot be empty.

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
