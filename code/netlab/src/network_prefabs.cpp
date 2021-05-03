#include <netlab/network.hpp>

namespace netlab {


const std::array<network::prefab::data, network::prefab::NUM_PREFABS>  network::prefab::prefabs {
    // UNIVERSAL
    {
        network::config{ network::config::NAME::UNIVERSAL },
        {
            network_layer::config {
                true,       // is_excitatory
                500U,       // num_units
                200U,       // num_sockets_per_unit
                network_layer::config::events::NAME::UNIVERSAL,
                network_layer::config::weights::NAME::UNIVERSAL,
                network_layer::config::sockets::NAME::UNIVERSAL
            },
            network_layer::config {
                false,      // is_excitatory
                500U,       // num_units
                200U,       // num_sockets_per_unit
                network_layer::config::events::NAME::UNIVERSAL,
                network_layer::config::weights::NAME::UNIVERSAL,
                network_layer::config::sockets::NAME::UNIVERSAL
            }
        }
    }
};


}
