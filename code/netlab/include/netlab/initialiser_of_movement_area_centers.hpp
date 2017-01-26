#ifndef NETLAB_INITIALISER_OF_MOVEMENT_AREA_CENTERS_HPP_INCLUDED
#   define NETLAB_INITIALISER_OF_MOVEMENT_AREA_CENTERS_HPP_INCLUDED

#   include <netlab/network_props.hpp>
#   include <netlab/network_indices.hpp>
#   include <netlab/extra_data_for_spikers.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace netlab {


struct  initialiser_of_movement_area_centers
{
    virtual ~initialiser_of_movement_area_centers() {}

    virtual void  on_next_layer(layer_index_type const  layer_index, network_props const&  props) {}

    virtual void  compute_initial_movement_area_center_for_ships_of_spiker(
            layer_index_type const  spiker_layer_index,
            object_index_type const  spiker_index_into_layer,
            sector_coordinate_type const  spiker_sector_coordinate_x,
            sector_coordinate_type const  spiker_sector_coordinate_y,
            sector_coordinate_type const  spiker_sector_coordinate_c,
            network_props const&  props,
            layer_index_type&  area_layer_index,
            vector3&  area_center
            ) = 0;

    virtual void  prepare_for_shifting_movement_area_centers_in_layers(
            extra_data_for_spikers_in_layers&  extra_data_for_spikers,
            network_props const&  props
            )
    {}

    virtual bool  start_next_iteration_of_shifting_movement_area_centers_in_layers(
            extra_data_for_spikers_in_layers const&  extra_data_for_spikers,
            network_props const&  props
            )
    { return false; }

    virtual void  on_shift_movement_area_center_in_layers(
            layer_index_type const  spiker_layer_index,
            object_index_type const  spiker_index_into_layer,
            sector_coordinate_type const  spiker_sector_coordinate_x,
            sector_coordinate_type const  spiker_sector_coordinate_y,
            sector_coordinate_type const  spiker_sector_coordinate_c,
            layer_index_type const  area_layer_index,
            network_props const&  props,
            vector3&  area_center,
            extra_data_for_spikers_in_layers&  extra_data_for_spikers
            )
    {}

    virtual bool  do_extra_data_hold_densities_of_ships_per_spikers_in_layers() { return false; }
};


}

#endif
