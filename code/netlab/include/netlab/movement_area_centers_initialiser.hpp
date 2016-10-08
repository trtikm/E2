#ifndef NETLAB_MOVEMENT_AREA_CENTERS_INITIALISER_HPP_INCLUDED
#   define NETLAB_MOVEMENT_AREA_CENTERS_INITIALISER_HPP_INCLUDED

#   include <netlab/network_props.hpp>
#   include <utility/tensor_math.hpp>

namespace netlab {


struct  movement_area_centers_initialiser
{
    virtual ~movement_area_centers_initialiser() {}

    virtual void  compute_movement_area_center_for_ships_of_spiker(
            natural_8_bit const  spiker_layer_index,
            natural_64_bit const  spiker_index_into_layer,
            natural_32_bit const  spiker_sector_coordinate_x,
            natural_32_bit const  spiker_sector_coordinate_y,
            natural_32_bit const  spiker_sector_coordinate_c,
            network_props const&  props,
            natural_8_bit&  area_layer_index,
            vector3&  area_center
            );
};


}

#endif
