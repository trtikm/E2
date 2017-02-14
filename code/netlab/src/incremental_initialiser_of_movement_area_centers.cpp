#include <netlab/initialiser_of_movement_area_centers.hpp>
#include <netlab/statistics_of_densities_of_ships_in_layers.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>

namespace netlab {


void  incremental_initialiser_of_movement_area_centers::prepare_for_shifting_movement_area_centers_in_layers(
        access_to_movement_area_centers const&  movement_area_centers,
        network_props const&  props,
        accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
        )
{
    TMPROF_BLOCK();

    compute_densities_of_ships_per_spiker_in_layers(props, movement_area_centers, extra_data_for_spikers);
    m_sources.resize(props.layer_props().size(),true);
    m_updated.resize(props.layer_props().size(),true);
    m_solved.resize(props.layer_props().size(),false);
}

void  incremental_initialiser_of_movement_area_centers::get_indices_of_layers_where_to_apply_movement_area_centers_migration(
        network_props const&  props,
        accessor_to_extra_data_for_spikers_in_layers const&  extra_data_for_spikers,
        std::vector<layer_index_type>&  layers_to_update
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(m_sources.size() == props.layer_props().size());

    for (layer_index_type layer_index = 0U; layer_index != m_sources.size(); ++layer_index)
        if (m_sources.at(layer_index) == true)
            layers_to_update.push_back(layer_index);

    std::fill(m_sources.begin(), m_sources.end(),false);

    for (layer_index_type layer_index = 0U; layer_index != m_updated.size(); ++layer_index)
        if (m_updated.at(layer_index) == false)
            m_solved.at(layer_index) = true;

    std::fill(m_updated.begin(), m_updated.end(),false);
}

void  incremental_initialiser_of_movement_area_centers::on_shift_movement_area_center_in_layer(
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
{
    TMPROF_BLOCK();

    ASSUMPTION(spiker_layer_index < m_sources.size());
    ASSUMPTION(area_layer_index < m_sources.size());
    ASSUMPTION(m_sources.size() == props.layer_props().size());
    ASSUMPTION(spiker_index_into_layer < props.layer_props().at(spiker_layer_index).num_spikers());

    if (m_solved.at(area_layer_index) == true)
        return;

    // TODO: here implement an algorithm which shifts the spiker's movement area to better possition, if exists!

    // If the center of the spiker's movement area was updated (shifted), then execute these 2 assignments:
    //      m_sources.at(spiker_layer_index) = true;
    //      m_updated.at(area_layer_index) = true;
    // Otherwise, make no changes.
}


}
