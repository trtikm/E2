#ifndef NETLAB_SIMPLE_NETWORK_HPP_INCLUDED
#   define NETLAB_SIMPLE_NETWORK_HPP_INCLUDED

#   include <netlab/simple_network_uid.hpp>
#   include <utility/random.hpp>
#   include <vector>
#   include <array>
#   include <deque>
#   include <unordered_map>

namespace netlab { namespace simple {


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
    // CONFIGURATIONS & STATISTICS:

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

    struct prefab
    {
        enum NAME
        {
            UNIVERSAL = 0
        };
        static constexpr int NUM_PREFABS = 1;
        struct data
        {
            config  network_config;
            std::vector<network_layer::config>  layers_configs;
        };
        static const std::array<prefab::data, NUM_PREFABS>  prefabs;
    };

    struct  statistics
    {
        struct  config
        {
            natural_32_bit  NUM_ROUNDS_PER_SNAPSHOT;        // When == 0, then statistics are NOT collected/updated at all!
            natural_32_bit  SNAPSHOTS_HISTORY_SIZE;
            float_32_bit  RATIO_OF_PROBED_UNITS_PER_LAYER;  // Must be >= 0.0f and <= 1.0f
            config(natural_32_bit const  num_rounds_per_snapshot,
                   natural_32_bit const  snapshots_history_size = 1U,
                   float_32_bit const  ratio_of_probed_units_per_layer = 0.1f);
        };

        struct  probe
        {
            natural_32_bit  num_events_received;
            natural_32_bit  num_events_produced;
            natural_32_bit  num_connected_input_sockets;
            natural_32_bit  num_connected_output_sockets;
            natural_32_bit  num_disconnected_input_sockets;
            natural_32_bit  num_disconnected_output_sockets;
            probe();
        };

        struct  overall
        {
            natural_32_bit  num_events;
            natural_32_bit  num_connected_sockets;
            natural_32_bit  num_disconnected_sockets;
            overall();
        };

        // CONSTANTS:

        natural_32_bit  NUM_ROUNDS_PER_SNAPSHOT;            // When == 0, then statistics are NOT collected/updated!
        natural_32_bit  SNAPSHOTS_HISTORY_SIZE;

        // DATA:

        natural_64_bit  num_passed_rounds;
        natural_32_bit  num_passed_rounds_in_current_snapshot;
        std::unordered_map<uid, std::deque<probe> >  probes_history;    // For each uid, the size of the deque is > 0
                                                                        // and <= SNAPSHOTS_HISTORY_SIZE + 1.
                                                                        // The front element in each deque is the probe of the
                                                                        // currently processed snapshot, for the related unit's uid.
        std::deque<overall>  overall_history;   // The size is > 0 and <= SNAPSHOTS_HISTORY_SIZE + 1. The front element
                                                // is the one of the currently processed snapshot.

        // FUNCTIONS:

        statistics(config const&  cfg, std::vector<network_layer::config> const&  layers_configs);
        bool  enabled() const { return NUM_ROUNDS_PER_SNAPSHOT != 0U; }
        void  on_next_round();
        void  on_event_received(uid const  id);
        void  on_event_produced(uid const  id);
        void  on_connect(uid const  iid, uid const  oid);
        void  on_disconnect(
                std::vector<input_socket> const&  inputs,
                natural_16_bit const  input_socket,
                std::vector<output_socket> const&  outputs,
                natural_16_bit const  output_socket
                );
    };

    // CONSTANTS:

    float_32_bit  EVENT_POTENTIAL_MAGNITUDE;            // Must be > 0.0f

    // DATA:

    std::vector<network_layer>  layers;                 // Cannot be empty.

    std::vector<uid>  events;
    std::vector<uid>  next_events;

    std::vector<uid>  open_inputs;
    std::vector<uid>  open_outputs;                     // INVARIANT: open_outputs.size() == open_inputs.size()

    random_generator_for_natural_32_bit  random_generator;

    statistics  stats;

    // FUNCTIONS:

    network(config const&  network_config,
            std::vector<network_layer::config> const&  layers_configs,
            statistics::config const&  stats_config,
            natural_32_bit const  seed = 0U);

    network(prefab::NAME const  prefab_name, statistics::config const&  stats_config, natural_32_bit const  seed = 0U)
        : network(prefab::prefabs.at(prefab_name).network_config, prefab::prefabs.at(prefab_name).layers_configs, stats_config, seed)
    {}

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
