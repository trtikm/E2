#ifndef AI_ENV_SINET_SNAPSHOT_ENCODER_HPP_INCLUDED
#   define AI_ENV_SINET_SNAPSHOT_ENCODER_HPP_INCLUDED

#   include <ai/env/snapshot.hpp>
#   include <ai/env/sinet/number_encoder.hpp>
#   include <ai/blackboard.hpp>
#   include <netlab/simple_network.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai { namespace env { namespace sinet {


struct  snapshot_encoder
{
    snapshot_encoder(blackboard_const_ptr const  blackboard_ptr, natural_8_bit const  start_layer_index);

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            snapshot const&  ss,
            std::unordered_set<netlab::simple::uid>&  spiking_units
            );

    void  get_layers_configs(std::vector<netlab::simple::network_layer::config::sign_and_geometry>&  configs) const;

private:

    struct  layer_props
    {
        // CONSTANTS:

        netlab::simple::network_layer::config::sign_and_geometry  SIGN_AND_GEOMETRY_INFO;
        natural_8_bit  INDEX;
        natural_8_bit  NUM_ENCODERS_PER_GRID_CELL;      // set to 0U, when the layer does NOT have encoders arranged in a grid.
        natural_16_bit  NUM_GRID_CELLS_ALONG_ANY_AXIS;  // set to 0U, when the layer does NOT have encoders arranged in a grid.

        // DATA:

        std::vector<sinet::number_binary_encoder>  encoders;    // When encoders are arranged in a grid, then all must use the
                                                                // same number of units.
    };

    // CONSTANTS:

    natural_8_bit   START_LAYER_INDEX;

    // DATA:

    layer_props  desire_computed_by_cortex;
    layer_props  regulated_desire;
    layer_props  motion;
    layer_props  camera;
    layer_props  collision_contacts;
    layer_props  ray_casts;
};


}}}

#endif
