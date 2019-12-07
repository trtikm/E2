#ifndef NETLAB_SIMPLE_NETWORK_HPP_INCLUDED
#   define NETLAB_SIMPLE_NETWORK_HPP_INCLUDED

#   include <utility/array_of_derived.hpp>
#   include <utility/random.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <memory>
#   include <string>
#   include <deque>
#   include <unordered_set>

namespace netlab { namespace simple {


struct uid
{
    natural_32_bit  layer :  6, // 64
                    unit  : 14, // 16384
                    socket: 12; // 4096
};
static_assert(sizeof(uid) == sizeof(natural_32_bit), "The uid must exactly fit to 32 bits.");


struct  input_socket
{
    uid  other;
    float_32_bit  weight;
};


struct  output_socket
{
    uid  other;
};


struct  computation_unit
{
    float_32_bit  event_potential;

    std::vector<input_socket>  inputs;
    std::vector<output_socket>  outputs;

};


struct  network_layer
{
    // CONSTANTS:

    float_32_bit  EVENT_TREASHOLD;                      // Must be > 0.0f
    float_32_bit  EVENT_RECOVERY_POTENTIAL;             // Must be < EVENT_TREASHOLD
    float_32_bit  EVENT_POTENTIAL_DECAY_COEF;           // Must be > 0.0f and < 1.0f
    float_32_bit  EVENT_POTENTIAL_SIGN;                 // 1.0f for an excitatory layer and -1.0f for an inhibitory layer

    float_32_bit  WEIGHT_PER_POTENTIAL;                 // Must be > 0.0f
    float_32_bit  WEIGHT_EQUILIBRIUM_TREASHOLD;         // Must be > 0.0f

    float_32_bit  SOCKET_CONNECTION_TREASHOLD;          // Must be > 0.0f
    float_32_bit  SOCKET_DISCONNECTION_TREASHOLD;       // Must be > 0.0f and <= SOCKET_CONNECTION_TREASHOLD

    // DATA:

    std::vector<computation_unit>  units;               // Cannot be empty.
};


struct  network
{
    // CONSTANTS:

    float_32_bit  EVENT_POTENTIAL_MAGNITUDE;            // Must be > 0.0f

    // DATA:

    std::vector<network_layer>  layers;                 // Cannot be empty.

    std::vector<uid>  events;
    std::vector<uid>  next_events;

    // Both vectors must always have the same size.
    std::vector<uid>  open_inputs;
    std::vector<uid>  open_outputs;

    random_generator_for_natural_32_bit  random_generator;

    // FUNCTIONS:

    void  next_round();

    void  update_computation_units();
    void  deliver_events();
    void  update_open_sockets();

    bool  connect(uid  iid, uid  oid);
    void  disconnect(
        std::vector<input_socket>&  inputs,
        natural_16_bit const  input_socket,
        std::vector<output_socket>&  outputs,
        natural_16_bit const  output_socket
        );
};


}}


#endif
