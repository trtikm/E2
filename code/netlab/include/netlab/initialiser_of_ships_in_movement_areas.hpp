#ifndef NETLAB_INITIALISER_OF_SHIPS_IN_MOVEMENT_AREAS_HPP_INCLUDED
#   define NETLAB_INITIALISER_OF_SHIPS_IN_MOVEMENT_AREAS_HPP_INCLUDED

#   include <netlab/network_objects.hpp>
#   include <netlab/network_props.hpp>
#   include <netlab/network_indices.hpp>
#   include <netlab/network_layer_props.hpp>
#   include <angeo/tensor_math.hpp>

namespace netlab {


struct  initialiser_of_ships_in_movement_areas
{
    virtual ~initialiser_of_ships_in_movement_areas() {}

    virtual void  on_next_layer(layer_index_type const  layer_index, network_props const&  props) {}
    virtual void  on_next_area(layer_index_type const  layer_index, object_index_type const  spiker_index, network_props const&  props) {}

    virtual void  compute_ship_position_and_velocity_in_movement_area(
            vector3 const&  center,
            natural_32_bit const  ship_index_in_the_area,
            network_layer_props const&  layer_props,
            ship&  ship_reference
            ) = 0;
};


}

#endif
