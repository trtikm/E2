#ifndef NETLAB_INITIALISER_OF_MOVEMENT_AREA_CENTERS_HPP_INCLUDED
#   define NETLAB_INITIALISER_OF_MOVEMENT_AREA_CENTERS_HPP_INCLUDED

#   include <netlab/network_props.hpp>
#   include <netlab/network_indices.hpp>
#   include <netlab/extra_data_for_spikers.hpp>
#   include <netlab/access_to_movement_area_centers.hpp>
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
            access_to_movement_area_centers const&  movement_area_centers,
            network_props const&  props,
            accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
            )
    {}

    virtual void  get_indices_of_layers_where_to_apply_movement_area_centers_migration(
            network_props const&  props,
            accessor_to_extra_data_for_spikers_in_layers const&  extra_data_for_spikers,
            std::vector<layer_index_type>&  layers_to_update
            )
    {}

    virtual void  on_shift_movement_area_center_in_layer(
            layer_index_type const  spiker_layer_index,
            object_index_type const  spiker_index_into_layer,
            sector_coordinate_type const  spiker_sector_coordinate_x,
            sector_coordinate_type const  spiker_sector_coordinate_y,
            sector_coordinate_type const  spiker_sector_coordinate_c,
            layer_index_type const  area_layer_index,
            network_props const&  props,
            access_to_movement_area_centers const&  movement_area_centers,
            vector3&  area_center,
            accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
            )
    {}

    virtual bool  do_extra_data_hold_densities_of_ships_per_spikers_in_layers() { return false; }
};


struct  incremental_initialiser_of_movement_area_centers : public initialiser_of_movement_area_centers
{
    virtual void  prepare_for_shifting_movement_area_centers_in_layers(
            access_to_movement_area_centers const&  movement_area_centers,
            network_props const&  props,
            accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
            ) override;

    virtual void  get_indices_of_layers_where_to_apply_movement_area_centers_migration(
            network_props const&  props,
            accessor_to_extra_data_for_spikers_in_layers const&  extra_data_for_spikers,
            std::vector<layer_index_type>&  layers_to_update
            ) override;

    virtual void  on_shift_movement_area_center_in_layer(
            layer_index_type const  spiker_layer_index,
            object_index_type const  spiker_index_into_layer,
            sector_coordinate_type const  spiker_sector_coordinate_x,
            sector_coordinate_type const  spiker_sector_coordinate_y,
            sector_coordinate_type const  spiker_sector_coordinate_c,
            layer_index_type const  area_layer_index,
            network_props const&  props,
            access_to_movement_area_centers const&  movement_area_centers,
            vector3&  area_center,
            accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
            ) override;

    virtual bool  do_extra_data_hold_densities_of_ships_per_spikers_in_layers() override { return true; }

private:
    std::vector<bool>  m_sources;
    std::vector<bool>  m_updated;
    std::vector<bool>  m_solved;
};


}

#endif
