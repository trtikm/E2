#ifndef NETLAB_SIMPLE_NETWORK_HPP_INCLUDED
#   define NETLAB_SIMPLE_NETWORK_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>
#   include <vector>
#   include <array>

namespace netlab { namespace simple {


struct uid
{
    static constexpr natural_8_bit  NUM_LAYER_BITS  = 6;
    static constexpr natural_8_bit  NUM_UNIT_BITS   = 14;
    static constexpr natural_8_bit  NUM_SOCKET_BITS = 12;

    static constexpr natural_8_bit   MAX_NUM_LAYERS             = 1 << NUM_LAYER_BITS;
    static constexpr natural_16_bit  MAX_NUM_UNITS_PER_LAYER    = 1 << NUM_UNIT_BITS;
    static constexpr natural_16_bit  MAX_NUM_SOCKETS_PER_UNIT   = 1 << NUM_SOCKET_BITS;

    static_assert(uid::MAX_NUM_LAYERS           == 64, "");
    static_assert(uid::MAX_NUM_UNITS_PER_LAYER  == 16384, "");
    static_assert(uid::MAX_NUM_SOCKETS_PER_UNIT == 4096, "");

    natural_32_bit  layer : NUM_LAYER_BITS,
                    unit  : NUM_UNIT_BITS,
                    socket: NUM_SOCKET_BITS;
};
static_assert(sizeof(uid) == sizeof(natural_32_bit), "");


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
    // CONFIGURATIONS:

    struct  config
    {
        struct  events
        {
            enum NAME
            {
                UNIVERSAL = 0
            };
            static constexpr int NUM_CONFIGURATIONS = 1;
            struct data
            {
                float_32_bit  EVENT_TREASHOLD;                      // Must be > 0.0f
                float_32_bit  EVENT_RECOVERY_POTENTIAL;             // Must be < EVENT_TREASHOLD
                float_32_bit  EVENT_POTENTIAL_DECAY_COEF;           // Must be > 0.0f and < 1.0f
            };
        };
        struct  weights
        {
            enum NAME
            {
                UNIVERSAL = 0
            };
            static constexpr int NUM_CONFIGURATIONS = 1;
            struct data
            {
                float_32_bit  WEIGHT_PER_POTENTIAL;                 // Must be > 0.0f
                float_32_bit  WEIGHT_EQUILIBRIUM_TREASHOLD;         // Must be > 0.0f
            };
        };
        struct  sockets
        {
            enum NAME
            {
                UNIVERSAL = 0
            };
            static constexpr int NUM_CONFIGURATIONS = 1;
            struct data
            {
                float_32_bit  SOCKET_CONNECTION_TREASHOLD;          // Must be > 0.0f
                float_32_bit  SOCKET_DISCONNECTION_TREASHOLD;       // Must be > 0.0f and <= SOCKET_CONNECTION_TREASHOLD
            };
        };

        static const std::array<config::events::data, config::events::NUM_CONFIGURATIONS>  configurations_of_events;
        static const std::array<config::weights::data, config::weights::NUM_CONFIGURATIONS>  configurations_of_weights;
        static const std::array<config::sockets::data, config::sockets::NUM_CONFIGURATIONS>  configurations_of_sockets;

        bool  is_excitatory;
        natural_16_bit  num_units;
        natural_16_bit  num_sockets_per_unit;
        events::NAME  events_config_name;
        weights::NAME  weithts_config_name;
        sockets::NAME  sockets_config_name;
    };

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
    // CONFIGURATIONS:

    struct  config
    {
        enum NAME
        {
            UNIVERSAL = 0
        };
        static constexpr int NUM_CONFIGURATIONS = 1;
        struct data
        {
            float_32_bit  EVENT_POTENTIAL_MAGNITUDE;     // Must be > 0.0f
        };

        static const std::array<config::data, config::NUM_CONFIGURATIONS>  configurations;

        NAME  config_name;
    };

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

    network(config const&  network_config, std::vector<network_layer::config> const&  layers_configs, natural_32_bit const  seed = 0U);

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
