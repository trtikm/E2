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
    virtual void  on_next_area(layer_index_type const  layer_index, object_index_type const  spiker_index,
                               network_props const&  props) {}

    virtual void  compute_ship_position_and_velocity_in_movement_area(
            vector3 const&  center_of_movement_area,
                    //!< The center appears in the layer at index 'area_layer_index'.
            natural_32_bit const  ship_index_in_the_area,
                    //!< In the range [0,props.layer_props().at(home_layer_index).num_ships_per_spiker()).
            layer_index_type const  home_layer_index,
                    //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,
                    //!< Index of layer where is the movement area in which the ship moves.
            network_props const&  props,
            vector3&  ship_position,
                    //!< A reference to the ship's position which should be computed. The position
                    //!< must be computed within the area.
            vector3&  ship_velocity 
                    //!< A reference to the ship's velocity which should be computed. The length of
                    //!< the velocity should be within limits:
                    //!< props.layer_props().at(home_layer_index).speed_limits_of_ship_in_meters_per_second(area_layer_index)
            ) = 0;
};


}

#endif
