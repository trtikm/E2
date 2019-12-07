#include <netlab/simple_network.hpp>

namespace netlab { namespace simple {


const std::array<network::config::data, network::config::NUM_CONFIGURATIONS>  network::config::configurations {
    // UNIVERSAL
    {
        1.0f        // EVENT_POTENTIAL_MAGNITUDE            Must be > 0.0f
    }
};


const std::array<network_layer::config::events::data, network_layer::config::events::NUM_CONFIGURATIONS>
network_layer::config::configurations_of_events{
    // UNIVERSAL
    {
        5.0f,       // EVENT_TREASHOLD                      Must be > 0.0f
        0.0f,       // EVENT_RECOVERY_POTENTIAL             Must be < EVENT_TREASHOLD
        0.98f       // EVENT_POTENTIAL_DECAY_COEF           Must be > 0.0f and < 1.0f
    }
};


const std::array<network_layer::config::weights::data, network_layer::config::weights::NUM_CONFIGURATIONS>
network_layer::config::configurations_of_weights{
    // UNIVERSAL
    {
        0.025f,     // WEIGHT_PER_POTENTIAL                 Must be > 0.0f
        0.75f       // WEIGHT_EQUILIBRIUM_TREASHOLD         Must be > 0.0f
    }
};


const std::array<network_layer::config::sockets::data, network_layer::config::sockets::NUM_CONFIGURATIONS>
network_layer::config::configurations_of_sockets{
    // UNIVERSAL
    {
        1.0f,       // SOCKET_CONNECTION_TREASHOLD          Must be > 0.0f
        0.25f       // SOCKET_DISCONNECTION_TREASHOLD       Must be > 0.0f and <= SOCKET_CONNECTION_TREASHOLD
    }
};



}}
