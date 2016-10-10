#ifndef NETLAB_INITIALISER_OF_MOVEMENT_AREA_CENTERS_HPP_INCLUDED
#   define NETLAB_INITIALISER_OF_MOVEMENT_AREA_CENTERS_HPP_INCLUDED

#   include <netlab/network_props.hpp>
#   include <utility/tensor_math.hpp>

namespace netlab {


struct  initialiser_of_movement_area_centers
{
    virtual ~initialiser_of_movement_area_centers() {}

    virtual void  on_next_layer(natural_8_bit const  layer_index, network_props const&  props) {}

    virtual void  compute_movement_area_center_for_ships_of_spiker(
            natural_8_bit const  spiker_layer_index,
            natural_64_bit const  spiker_index_into_layer,
            natural_32_bit const  spiker_sector_coordinate_x,
            natural_32_bit const  spiker_sector_coordinate_y,
            natural_32_bit const  spiker_sector_coordinate_c,
            network_props const&  props,
            natural_8_bit&  area_layer_index,
            vector3&  area_center
            ) = 0;
};


}

#endif
