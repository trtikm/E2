#include <netexp/incremental_initialiser_of_movement_area_centers.hpp>
#include <netexp/algorithm.hpp>
#include <netlab/statistics_of_densities_of_ships_in_layers.hpp>
#include <angeo/collide.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>

namespace netexp { namespace detail { namespace {


void  compute_both_change_bboxes(
        vector3 const&  area_low_corner,
        vector3 const&  area_high_corner,
        int const  coord_index,
        float_32_bit const  coord_shift,
        vector3&  low_corner_0,
        vector3&  high_corner_0,
        vector3&  low_corner_1,
        vector3&  high_corner_1
        )
{
    low_corner_0 = area_low_corner;
    high_corner_1 = area_high_corner;
    if (coord_shift < 0.0f)
        low_corner_0(coord_index) += coord_shift;
    else
        high_corner_1(coord_index) += coord_shift;

    high_corner_0 = high_corner_1;
    high_corner_0(coord_index) -= area_high_corner(coord_index) - area_low_corner(coord_index);

    low_corner_1 = low_corner_0;
    low_corner_1(coord_index) += area_high_corner(coord_index) - area_low_corner(coord_index);
}


float_32_bit  compute_score(
        vector3 const&  low_corner,
        vector3 const&  high_corner,
        float_32_bit const  volume_mult,
        float_32_bit const  ideal_average_density_in_layer,
        netlab::layer_index_type const  layer_index,
        netlab::network_layer_props const&  area_layer_props,
        netlab::accessor_to_extra_data_for_spikers_in_layers const&  extra_data_for_spikers
        )
{
    float_32_bit const  volume_scale =
            1.0f / (area_layer_props.distance_of_docks_in_meters() * 
                    area_layer_props.distance_of_docks_in_meters() * 
                    area_layer_props.distance_of_docks_in_meters() );
    float_32_bit  score = 0.0f;
    netlab::sector_coordinate_type  x_lo, y_lo, c_lo;
    area_layer_props.spiker_sector_coordinates(low_corner,x_lo,y_lo,c_lo);
    netlab::sector_coordinate_type  x_hi, y_hi, c_hi;
    area_layer_props.spiker_sector_coordinates(high_corner,x_hi,y_hi,c_hi);
    for (netlab::sector_coordinate_type x = x_lo; x <= x_hi; ++x)
        for (netlab::sector_coordinate_type y = y_lo; y <= y_hi; ++y)
            for (netlab::sector_coordinate_type c = c_lo; c <= c_hi; ++c)
            {
                    vector3 const  spiker_position = area_layer_props.spiker_sector_centre(x,y,c);
                    netlab::object_index_type const  spiker_index = area_layer_props.spiker_sector_index(x,y,c);

                    vector3  intersection_low_corner, intersection_high_corner;
                    bool const  do_intersect = angeo::collision_bbox_bbox(
                            spiker_position - 0.5f * area_layer_props.distance_of_spikers_in_meters(),
                            spiker_position + 0.5f * area_layer_props.distance_of_spikers_in_meters(),
                            low_corner,
                            high_corner,
                            intersection_low_corner,
                            intersection_high_corner
                            );
                    INVARIANT(do_intersect);

                    vector3 const  size = intersection_high_corner - intersection_low_corner;

                    float_32_bit const  volume_delta = volume_mult * volume_scale * size(0) * size(1) * size(2);
                    float_32_bit const  sector_density = extra_data_for_spikers.get_extra_data_of_spiker(layer_index,spiker_index);
                    ASSUMPTION(sector_density >= 0.0f);
                    float_32_bit const  sector_score = volume_delta * (1.0f - sector_density / ideal_average_density_in_layer);
                    INVARIANT(
                        (volume_mult * sector_score <= 0.0f && sector_density >= ideal_average_density_in_layer)
                        ||
                        (volume_mult * sector_score >= 0.0f && sector_density <= ideal_average_density_in_layer)
                        );

                    score += sector_score;
            }
    return score;
}


float_32_bit  compute_score(
        vector3 const&  area_low_corner,
        vector3 const&  area_high_corner,
        int const  coord_index,
        float_32_bit const  coord_shift,
        float_32_bit const  ideal_average_density_in_layer,
        netlab::layer_index_type const  layer_index,
        netlab::network_layer_props const&  area_layer_props,
        netlab::accessor_to_extra_data_for_spikers_in_layers const&  extra_data_for_spikers
        )
{
    vector3  low_corner_0, high_corner_0;
    vector3  low_corner_1, high_corner_1;
    compute_both_change_bboxes(
            area_low_corner,
            area_high_corner,
            coord_index,
            coord_shift,
            low_corner_0,
            high_corner_0,
            low_corner_1,
            high_corner_1
            );
    float_32_bit const  volume_mult_0 = coord_shift < 0.0f ? 1.0f : -1.0f;
    float_32_bit const  volume_mult_1 = -volume_mult_0;
    return compute_score(
                low_corner_0,
                high_corner_0,
                volume_mult_0,
                ideal_average_density_in_layer,
                layer_index,
                area_layer_props,
                extra_data_for_spikers
                )
            +
            compute_score(
                low_corner_1,
                high_corner_1,
                volume_mult_1,
                ideal_average_density_in_layer,
                layer_index,
                area_layer_props,
                extra_data_for_spikers
                );
}


float_32_bit  update_densities_in_bbox(
        vector3 const&  low_corner,
        vector3 const&  high_corner,
        float_32_bit const  volume_mult,
        float_32_bit const  density_of_ships_in_movement_area,
        netlab::layer_index_type const  layer_index,
        netlab::network_layer_props const&  area_layer_props,
        netlab::accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
        )
{
    float_32_bit  total_density_delta = 0.0f;
    float_32_bit const  spiker_sector_volume = 
            area_layer_props.distance_of_spikers_in_meters()(0) *
            area_layer_props.distance_of_spikers_in_meters()(1) *
            area_layer_props.distance_of_spikers_in_meters()(2) ;
    netlab::sector_coordinate_type  x_lo, y_lo, c_lo;
    area_layer_props.spiker_sector_coordinates(low_corner,x_lo,y_lo,c_lo);
    netlab::sector_coordinate_type  x_hi, y_hi, c_hi;
    area_layer_props.spiker_sector_coordinates(high_corner,x_hi,y_hi,c_hi);
    for (netlab::sector_coordinate_type x = x_lo; x <= x_hi; ++x)
        for (netlab::sector_coordinate_type y = y_lo; y <= y_hi; ++y)
            for (netlab::sector_coordinate_type c = c_lo; c <= c_hi; ++c)
            {
                    vector3 const  spiker_position = area_layer_props.spiker_sector_centre(x,y,c);
                    netlab::object_index_type const  spiker_index = area_layer_props.spiker_sector_index(x,y,c);

                    vector3  intersection_low_corner, intersection_high_corner;
                    bool const  do_intersect = angeo::collision_bbox_bbox(
                            spiker_position - 0.5f * area_layer_props.distance_of_spikers_in_meters(),
                            spiker_position + 0.5f * area_layer_props.distance_of_spikers_in_meters(),
                            low_corner,
                            high_corner,
                            intersection_low_corner,
                            intersection_high_corner
                            );
                    INVARIANT(do_intersect);

                    vector3 const  size = intersection_high_corner - intersection_low_corner;

                    float_32_bit const  volume = volume_mult * size(0) * size(1) * size(2);
                    float_32_bit const  density_delta = density_of_ships_in_movement_area * (volume / spiker_sector_volume);
                    float_32_bit const  orig_sector_density = extra_data_for_spikers.get_extra_data_of_spiker(layer_index,spiker_index);
                    ASSUMPTION(orig_sector_density >= 0.0f);
                    ASSUMPTION(orig_sector_density + density_delta >= 0.0f);

                    extra_data_for_spikers.set_extra_data_of_spiker(layer_index,spiker_index,orig_sector_density + density_delta);
                    total_density_delta += density_delta;
            }
    return total_density_delta;
}


void  shift_movement_area_center(
        vector3 const&  size_of_area,
        natural_32_bit const  num_ships_in_area,
        int const  coord_index,
        float_32_bit const  coord_shift,
        netlab::layer_index_type const  layer_index,
        netlab::network_layer_props const&  area_layer_props,
        netlab::accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers,
        vector3&  area_center
        )
{
    vector3 const  area_low_corner = area_center - 0.5f * size_of_area;
    vector3 const  area_high_corner = area_center + 0.5f * size_of_area;
    vector3  low_corner_0, high_corner_0;
    vector3  low_corner_1, high_corner_1;
    compute_both_change_bboxes(
            area_low_corner,
            area_high_corner,
            coord_index,
            coord_shift,
            low_corner_0,
            high_corner_0,
            low_corner_1,
            high_corner_1
            );
    float_32_bit const  volume_mult_0 = coord_shift < 0.0f ? 1.0f : -1.0f;
    float_32_bit const  volume_mult_1 = -volume_mult_0;

    vector3 const  scaled_size_of_area = (1.0f / area_layer_props.distance_of_docks_in_meters()) * size_of_area;
    float_32_bit const  volume_of_area = scaled_size_of_area(0) * scaled_size_of_area(1) * scaled_size_of_area(2);
    float_32_bit const  density_of_ships = (float_32_bit)num_ships_in_area / volume_of_area;

    float_32_bit  total_density_delta = 
        update_densities_in_bbox(
                low_corner_0,
                high_corner_0,
                volume_mult_0,
                density_of_ships,
                layer_index,
                area_layer_props,
                extra_data_for_spikers
                );
    total_density_delta += 
        update_densities_in_bbox(
                low_corner_1,
                high_corner_1,
                volume_mult_1,
                density_of_ships,
                layer_index,
                area_layer_props,
                extra_data_for_spikers
                );
    INVARIANT(std::fabsf(total_density_delta) < 1e-3f);

    area_center(coord_index) += coord_shift;
}


}}}



namespace netexp {


void  incremental_initialiser_of_movement_area_centers::prepare_for_shifting_movement_area_centers_in_layers(
        netlab::access_to_movement_area_centers const&  movement_area_centers,
        netlab::network_props const&  props,
        netlab::accessor_to_extra_data_for_spikers_in_layers&  extra_data_for_spikers
        )
{
    TMPROF_BLOCK();

    compute_densities_of_ships_per_spiker_in_layers(props, movement_area_centers, extra_data_for_spikers);
    m_sources.resize(props.layer_props().size(),true);
    m_updated.resize(props.layer_props().size(),true);
    m_solved.resize(props.layer_props().size(),false);
m_scores.resize(props.layer_props().size(),0.0f);
    compute_ideal_densities_of_ships_in_layers(props,m_ideal_average_ship_densities_in_layers);
}

void  incremental_initialiser_of_movement_area_centers::get_indices_of_layers_where_to_apply_movement_area_centers_migration(
        netlab::network_props const&  props,
        netlab::accessor_to_extra_data_for_spikers_in_layers const&  extra_data_for_spikers,
        std::vector<netlab::layer_index_type>&  layers_to_update
        )
{
    TMPROF_BLOCK();

    if (m_max_iterations >= 50U)
        return;
    ++m_max_iterations;

    ASSUMPTION(m_sources.size() == props.layer_props().size());

    for (netlab::layer_index_type layer_index = 0U; layer_index != m_sources.size(); ++layer_index)
        if (m_sources.at(layer_index) == true)
            layers_to_update.push_back(layer_index);

    std::fill(m_sources.begin(),m_sources.end(),false);

    for (netlab::layer_index_type layer_index = 0U; layer_index != m_updated.size(); ++layer_index)
        if (m_updated.at(layer_index) == false)
            m_solved.at(layer_index) = true;

    std::fill(m_updated.begin(),m_updated.end(),false);

std::fill(m_scores.begin(),m_scores.end(),0.0f);
}

void  incremental_initialiser_of_movement_area_centers::on_shift_movement_area_center_in_layer(
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
        )
{
    TMPROF_BLOCK();

//if (spiker_layer_index == 1U && spiker_index_into_layer == 5ULL && area_layer_index == 6U)
//{
//    int iii = 0;
//}

    ASSUMPTION(spiker_layer_index < m_sources.size());
    ASSUMPTION(area_layer_index < m_sources.size());
    ASSUMPTION(m_sources.size() == props.layer_props().size());
    ASSUMPTION(spiker_index_into_layer < props.layer_props().at(spiker_layer_index).num_spikers());

    if (m_solved.at(area_layer_index) == true)
        return;

    vector3 const  spiker_position =
            compute_spiker_position_projected_to_area_layer(
                    spiker_layer_index,
                    area_layer_index,
                    spiker_sector_coordinate_x,
                    spiker_sector_coordinate_y,
                    spiker_sector_coordinate_c,
                    props
                    );

    std::array<natural_32_bit, 3ULL> const&  max_distances =
            m_get_max_area_distance_from_spiker(spiker_layer_index,area_layer_index);

    netlab::network_layer_props const&  spiker_layer_props = props.layer_props().at(spiker_layer_index);
    netlab::network_layer_props const&  area_layer_props = props.layer_props().at(area_layer_index);

    vector3 const&  size_of_area = spiker_layer_props.size_of_ship_movement_area_in_meters(area_layer_index);

    vector3  low_corner, high_corner;
    compute_maximal_bbox_for_storing_movement_area_of_spiker_into(
            spiker_position,
            size_of_area,
            max_distances.at(0ULL),
            max_distances.at(1ULL),
            max_distances.at(2ULL),
            area_layer_props,
            low_corner,
            high_corner
            );

    vector3 const  center_low_corner = low_corner + 0.5f * size_of_area;
    vector3 const  center_high_corner = high_corner - 0.5f * size_of_area;

    float_32_bit const  score_limit = 0.0f;
    std::vector<std::pair<int,float_32_bit> > const  directions = {
        { 0,  0.5f * area_layer_props.distance_of_spikers_along_x_axis_in_meters() },
        { 0, -0.5f * area_layer_props.distance_of_spikers_along_x_axis_in_meters() },

        { 1,  0.5f * area_layer_props.distance_of_spikers_along_y_axis_in_meters() },
        { 1, -0.5f * area_layer_props.distance_of_spikers_along_y_axis_in_meters() },

        { 2,  0.5f * area_layer_props.distance_of_spikers_along_c_axis_in_meters() },
        { 2, -0.5f * area_layer_props.distance_of_spikers_along_c_axis_in_meters() },
    };
    float_32_bit  best_score = score_limit;
    std::pair<int,float_32_bit>  best_shift;

    for (auto const&  index_shift : directions)
    {
        vector3 direction = vector3_zero();
        direction(index_shift.first) = index_shift.second;
        vector3  moved_area_center;
        angeo::closest_point_of_bbox_to_point(center_low_corner,center_high_corner,area_center + direction,moved_area_center);
        if (std::fabs(moved_area_center(index_shift.first) - area_center(index_shift.first)) <
                0.5f * area_layer_props.distance_of_docks_in_meters())
            continue;
        if (spiker_layer_index == area_layer_index &&
                angeo::collision_bbox_bbox(
                        moved_area_center - 0.5f * size_of_area,
                        moved_area_center + 0.5f * size_of_area,
                        spiker_position - 0.5f * area_layer_props.distance_of_spikers_in_meters(),
                        spiker_position + 0.5f * area_layer_props.distance_of_spikers_in_meters()
                        ))
            continue;
        float_32_bit const  score =
                detail::compute_score(
                        area_center - 0.5f * size_of_area,
                        area_center + 0.5f * size_of_area,
                        index_shift.first,
                        moved_area_center(index_shift.first) - area_center(index_shift.first),
                        m_ideal_average_ship_densities_in_layers.at(area_layer_index),
                        area_layer_index,
                        area_layer_props,
                        extra_data_for_spikers
                        );
        if (score > score_limit + 1e-3f && score > best_score)
        {
            best_score = score;
            best_shift = { index_shift.first, moved_area_center(index_shift.first) - area_center(index_shift.first) };
        }
    }

    if (best_score > score_limit)
    {
//if (m_max_iterations == 9U)
//{
//    int iii = 0;
//}

//std::vector<float_32_bit>  minimal_densities_1;
//std::vector<float_32_bit>  maximal_densities_1;
//std::vector<float_32_bit>  average_densities_1;
//std::vector<netlab::distribution_of_spikers_by_density_of_ships>  distribution_of_spikers_1;
//netlab::compute_statistics_of_density_of_ships_in_layers(
//        props,
//        extra_data_for_spikers,
//        m_ideal_average_ship_densities_in_layers,
//        minimal_densities_1,
//        maximal_densities_1,
//        average_densities_1,
//        distribution_of_spikers_1
//        );

        detail::shift_movement_area_center(
                size_of_area,
                spiker_layer_props.num_ships_per_spiker(),
                best_shift.first,
                best_shift.second,
                area_layer_index,
                area_layer_props,
                extra_data_for_spikers,
                area_center
                );

//std::vector<float_32_bit>  minimal_densities_2;
//std::vector<float_32_bit>  maximal_densities_2;
//std::vector<float_32_bit>  average_densities_2;
//std::vector<netlab::distribution_of_spikers_by_density_of_ships>  distribution_of_spikers_2;
//netlab::compute_statistics_of_density_of_ships_in_layers(
//        props,
//        extra_data_for_spikers,
//        m_ideal_average_ship_densities_in_layers,
//        minimal_densities_2,
//        maximal_densities_2,
//        average_densities_2,
//        distribution_of_spikers_2
//        );
//ASSUMPTION(average_densities_1.size() == average_densities_2.size());
//for (natural_64_bit i = 0ULL; i != average_densities_1.size(); ++i)
//{
//    if (std::fabsf(average_densities_2.at(i) - average_densities_1.at(i)) > 1e-3f)
//    {
//        int iii = 0;
//    }
//    if (minimal_densities_2.at(i) < minimal_densities_1.at(i))
//    {
//        int iii = 0;
//    }
//    if (maximal_densities_2.at(i) > maximal_densities_1.at(i))
//    {
//        int iii = 0;
//    }
//}

        m_sources.at(spiker_layer_index) = true;
        m_updated.at(area_layer_index) = true;
m_scores.at(area_layer_index) += best_score;
    }
}


}
