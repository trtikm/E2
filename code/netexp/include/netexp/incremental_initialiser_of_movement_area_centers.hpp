#ifndef NETEXP_INITIALISER_OF_MOVEMENT_AREA_CENTERS_HPP_INCLUDED
#   define NETEXP_INITIALISER_OF_MOVEMENT_AREA_CENTERS_HPP_INCLUDED

#   include <netlab/initialiser_of_movement_area_centers.hpp>
#   include <netlab/network_props.hpp>
#   include <netlab/network_indices.hpp>
#   include <netlab/extra_data_for_spikers.hpp>
#   include <netlab/access_to_movement_area_centers.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <functional>
#   include <array>

namespace netexp {


struct  incremental_initialiser_of_movement_area_centers : public netlab::initialiser_of_movement_area_centers
{
    using max_area_distance_from_spiker_callback_function =
                std::function<std::array<natural_32_bit, 3ULL>( //!< Returns distances in axes x,y,c.
                        netlab::layer_index_type,   //!< spiker layer index
                        netlab::layer_index_type    //!< area layer index
                        )>;

    incremental_initialiser_of_movement_area_centers(
            max_area_distance_from_spiker_callback_function const&  get_max_area_distance_from_spiker
            )
        : netlab::initialiser_of_movement_area_centers()
        , m_get_max_area_distance_from_spiker(get_max_area_distance_from_spiker)
        , m_sources()
        , m_updated()
        , m_solved()
        , m_ideal_average_ship_densities_in_layers()
        , m_max_iterations(0U)
, m_scores()
    {}

    virtual void  prepare_for_shifting_movement_area_centers_in_layers(
            netlab::access_to_movement_area_centers const&  movement_area_centers,
            netlab::network_props const&  props,
            netlab::accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
            ) override;

    virtual void  get_indices_of_layers_where_to_apply_movement_area_centers_migration(
            netlab::network_props const&  props,
            netlab::accessor_to_extra_data_for_spikers_in_layers const&  extra_data_for_spikers,
            std::vector<netlab::layer_index_type>&  layers_to_update
            ) override;

    virtual void  on_shift_movement_area_center_in_layer(
            netlab::layer_index_type const  spiker_layer_index,
            netlab::object_index_type const  spiker_index_into_layer,
            netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
            netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
            netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
            netlab::layer_index_type const  area_layer_index,
            netlab::network_props const&  props,
            netlab::access_to_movement_area_centers const&  movement_area_centers,
            vector3&  area_center,
            netlab::accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
            ) override;

    virtual bool  do_extra_data_hold_densities_of_ships_per_spikers_in_layers() override { return true; }

private:
    max_area_distance_from_spiker_callback_function  m_get_max_area_distance_from_spiker;
    std::vector<bool>  m_sources;
    std::vector<bool>  m_updated;
    std::vector<bool>  m_solved;
    std::vector<float_32_bit>  m_ideal_average_ship_densities_in_layers;
    natural_32_bit  m_max_iterations;
std::vector<float_32_bit>  m_scores;
};


}

#endif
