#include <netlab/statistics_of_densities_of_ships_in_layers.hpp>
#include <angeo/collide.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <limits>

namespace netlab {


statistics_of_densities_of_ships_in_layers::statistics_of_densities_of_ships_in_layers(
        std::vector<float_32_bit>&  ideal_densities,
        std::vector<float_32_bit>&  minimal_densities,
        std::vector<float_32_bit>&  maximal_densities,
        std::vector<float_32_bit>&  average_densities,
        std::vector<distribution_of_spikers_by_density_of_ships>&  distribution_of_spikers_by_densities_of_ships
        )
    : m_ideal_densities()
    , m_minimal_densities()
    , m_maximal_densities()
    , m_average_densities()
    , m_distribution_of_spikers_by_densities_of_ships()
{
    m_ideal_densities.swap(ideal_densities);
    m_minimal_densities.swap(minimal_densities);
    m_maximal_densities.swap(maximal_densities);
    m_average_densities.swap(average_densities);
    m_distribution_of_spikers_by_densities_of_ships.swap(distribution_of_spikers_by_densities_of_ships);

    ASSUMPTION(m_ideal_densities.size() != 0ULL);
    ASSUMPTION(m_ideal_densities.size() <= max_number_of_layers());
    ASSUMPTION(m_ideal_densities.size() == m_minimal_densities.size());
    ASSUMPTION(m_ideal_densities.size() == m_maximal_densities.size());
    ASSUMPTION(m_ideal_densities.size() == m_average_densities.size());
    ASSUMPTION(m_ideal_densities.size() == m_distribution_of_spikers_by_densities_of_ships.size());
    ASSUMPTION(
            [](std::vector<float_32_bit> const&  ideal,
               std::vector<float_32_bit> const&  minimal,
               std::vector<float_32_bit> const&  maximal,
               std::vector<float_32_bit> const&  average ) -> bool {
                for (std::size_t  i = 0ULL; i != ideal.size(); ++i)
                    if (minimal.at(i) < 0.0f || average.at(i) < minimal.at(i) || maximal.at(i) < average.at(i))
                        return false;
                return true;
            }(m_ideal_densities,m_minimal_densities,m_maximal_densities,m_average_densities)
            );
}


float_32_bit  compute_ideal_density_of_ships_in_layer(
        natural_32_bit const  num_docks_per_spiker_in_layer,
        natural_32_bit const  num_ships_per_spiker_in_layer
        )
{
    ASSUMPTION(num_docks_per_spiker_in_layer != 0U);
    return (float_32_bit)num_ships_per_spiker_in_layer / (float_32_bit)num_docks_per_spiker_in_layer;
}


void  compute_ideal_densities_of_ships_in_layers(
        network_props const&  props,
        std::vector<float_32_bit>&  ideal_densities
        )
{
    ideal_densities.reserve(props.layer_props().size());
    for (layer_index_type  i = 0U; i != props.layer_props().size(); ++i)
        ideal_densities.push_back( compute_ideal_density_of_ships_in_layer(props.layer_props().at(i).num_docks_per_spiker(),
                                                                           props.layer_props().at(i).num_ships_per_spiker()) );
}


void  initialise_densities_of_ships_per_spiker_in_layers(
        network_props const&  props,
        accessor_to_extra_data_for_spikers_in_layers&  extra_data_accessor
        )
{
    for (layer_index_type i = 0U, m = (layer_index_type)props.layer_props().size(); i != m; ++i)
        for (object_index_type j = 0ULL, n = props.layer_props().at(i).num_spikers(); j != n; ++j)
            extra_data_accessor.set_extra_data_of_spiker(i,j,0.0f);
}


void  compute_densities_of_ships_per_spiker_in_layers(
        network_props const&  props,
        access_to_movement_area_centers const&  movement_area_centers,
        accessor_to_extra_data_for_spikers_in_layers&  extra_data_accessor
        )
{
    for (layer_index_type layer_index = 0U; layer_index < props.layer_props().size(); ++layer_index)
    {
        network_layer_props const&  layer_props = props.layer_props().at(layer_index);
        for (object_index_type spiker_index = 0ULL; spiker_index != layer_props.num_spikers(); ++spiker_index)
        {
            vector3 const&  area_center = movement_area_centers.area_center(layer_index, spiker_index);

            layer_index_type const  area_layer_index = props.find_layer_index(area_center(2));
            network_layer_props const&  area_layer_props = props.layer_props().at(area_layer_index);

            float_32_bit const  from_meters_to_num_docks = 1.0f / area_layer_props.distance_of_docks_in_meters();

            vector3 const  corner_shift = 0.5f * layer_props.size_of_ship_movement_area_in_meters(area_layer_index);
            vector3 const  area_lo_corner = area_center - corner_shift;
            vector3 const  area_hi_corner = area_center + corner_shift;
            vector3 const  area_size = from_meters_to_num_docks * (area_hi_corner - area_lo_corner);
            float_32_bit const  area_volume = area_size(0) * area_size(1) * area_size(2);
            ASSUMPTION(area_volume >= 1e-3f);
            float_32_bit const  area_density = (float_32_bit)layer_props.num_ships_per_spiker() / area_volume;

            vector3 const  sector_corner_shift = 0.5f * area_layer_props.distance_of_spikers_in_meters();
            sector_coordinate_type  x_lo,y_lo,c_lo;
            area_layer_props.spiker_sector_coordinates(area_lo_corner,x_lo,y_lo,c_lo);
            sector_coordinate_type  x_hi,y_hi,c_hi;
            area_layer_props.spiker_sector_coordinates(area_hi_corner,x_hi,y_hi,c_hi);

            for (sector_coordinate_type c = c_lo; c <= c_hi; ++c)
                for (sector_coordinate_type y = y_lo; y < y_hi; ++y)
                    for (sector_coordinate_type x = x_lo; x < x_hi; ++x)
                    {
                        vector3 const  sector_center = area_layer_props.spiker_sector_centre(x,y,c);
                        vector3 const  sector_lo_corner = sector_center - sector_corner_shift;
                        vector3 const  sector_hi_corner = sector_center + sector_corner_shift;

                        vector3 intersection_lo_corner;
                        vector3 intersection_hi_corner;
                        if (angeo::collision_bbox_bbox(
                                    area_lo_corner,
                                    area_hi_corner,
                                    sector_lo_corner,
                                    sector_hi_corner,
                                    intersection_lo_corner,
                                    intersection_hi_corner
                                    ))
                        {
                            vector3 const  sector_size = sector_hi_corner - sector_lo_corner;
                            float_32_bit const  sector_volume = sector_size(0) * sector_size(1) * sector_size(2);
                            ASSUMPTION(sector_volume >= 1e-3f);

                            vector3 const  intersection_size = intersection_hi_corner - intersection_lo_corner;
                            float_32_bit const  intersection_volume =
                                std::fabs(intersection_size(0) * intersection_size(1) * intersection_size(2));

                            float_32_bit const  scale = std::min(std::max(0.0f,intersection_volume / sector_volume),1.0f);
                            float_32_bit const  sector_density =
                                std::fabs(scale * ((float_32_bit)layer_props.num_ships_per_spiker() / area_volume));

                            object_index_type const  area_spiker_index = area_layer_props.spiker_sector_index(x,y,c);
                            extra_data_accessor.add_value_to_extra_data_of_spiker(area_layer_index,area_spiker_index,sector_density);
                        }
                        else
                        {
                            UNREACHABLE();
                        }
                    }
        }
    }
}


void  compute_statistics_of_density_of_ships_in_layers(
        network_props const&  props,
        accessor_to_extra_data_for_spikers_in_layers const&  extra_data_accessor,
        std::vector<float_32_bit> const&  ideal_densities,
        std::vector<float_32_bit>&  minimal_densities,
        std::vector<float_32_bit>&  maximal_densities,
        std::vector<float_32_bit>&  average_densities,
        std::vector<distribution_of_spikers_by_density_of_ships>&  distribution_of_spikers
        )
{
    ASSUMPTION(ideal_densities.size() == props.layer_props().size());

    minimal_densities.clear();
    minimal_densities.resize(props.layer_props().size(),std::numeric_limits<float_32_bit>::max());

    maximal_densities.clear();
    maximal_densities.resize(props.layer_props().size(),0.0f);

    average_densities.clear();
    average_densities.resize(props.layer_props().size(),0.0f);

    distribution_of_spikers.clear();
    {
        distribution_of_spikers_by_density_of_ships  zeros;
        zeros.fill(0ULL);
        distribution_of_spikers.resize(props.layer_props().size(),zeros);
    }

    for (layer_index_type layer_index = 0U; layer_index < props.layer_props().size(); ++layer_index)
    {
        network_layer_props const&  layer_props = props.layer_props().at(layer_index);
        float_32_bit const  max_density = 2.0f * ideal_densities.at(layer_index);
        ASSUMPTION(max_density > 1e-5f);
        for (object_index_type spiker_index = 0ULL; spiker_index != layer_props.num_spikers(); ++spiker_index)
        {
            float_32_bit const  density = extra_data_accessor.get_extra_data_of_spiker(layer_index,spiker_index);
            if (density < minimal_densities.at(layer_index))
                minimal_densities.at(layer_index) = density;
            if (density > maximal_densities.at(layer_index))
                maximal_densities.at(layer_index) = density;
            average_densities.at(layer_index) += density;

            float_32_bit const  param = std::min(std::max(0.0f, density / max_density),1.0f);
            natural_64_bit const  slot_index =
                std::min((natural_64_bit)(param  * (float_32_bit)distribution_of_spikers_by_density_of_ships().size()),
                         distribution_of_spikers_by_density_of_ships().size() - 1ULL);
            ++distribution_of_spikers.at(layer_index).at(slot_index);
        }
        INVARIANT(layer_props.num_spikers() != 0ULL);
        average_densities.at(layer_index) /= (float_32_bit)layer_props.num_spikers();
    }
}


}
