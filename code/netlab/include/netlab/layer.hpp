#ifndef NETLAB_LAYER_HPP_INCLUDED
#   define NETLAB_LAYER_HPP_INCLUDED

#   include <netlab/uid.hpp>
#   include <netlab/sockets.hpp>
#   include <netlab/unit.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>

namespace netlab {


struct  network_layer
{
    // CONSTANTS:

    float_32_bit  SPIKE_SIGN;                           // 1.0f for an excitatory layer and -1.0f for an inhibitory layer
    float_32_bit  CHARGE_SPIKE;                         // Must be > 0.0f
    float_32_bit  CHARGE_RECOVERY;                      // Must be >= 0.0f and < CHARGE_SPIKE
    float_32_bit  CHARGE_DECAY_COEF;                    // Must be > 0.0f and < 1.0f
    float_32_bit  WEIGHT_NEUTRAL_CHARGE;                // Must be > CHARGE_RECOVERY and < CHARGE_SPIKE
    float_32_bit  WEIGHT_DELTA_PER_SPIKE;               // Must be > 0.0f
    float_32_bit  WEIGHT_CONNECTION;                    // Must be > 0.0f
    float_32_bit  WEIGHT_DISCONNECTION;                 // Must be >= 0.0f and < WEIGHT_CONNECTION
    float_32_bit  WEIGHT_MAXIMAL;                       // Must be > WEIGHT_CONNECTION
    float_32_bit  SPIKE_MAGNITUDE;                      // Must be > 0.0f

    // DATA:

    std::vector<computation_unit>  units;               // Cannot be empty.
};


}


#endif
