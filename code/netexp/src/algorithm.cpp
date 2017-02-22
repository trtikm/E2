#include <netexp/algorithm.hpp>
#include <angeo/collide.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace netexp {


netlab::layer_index_type  compute_layer_index_for_area_center(
        std::vector<natural_64_bit>&  counts_of_centers_into_layers,
        random_generator_for_natural_32_bit&  generator
        )
{
    netlab::layer_index_type  layer_index =
            static_cast<netlab::layer_index_type>(
                    get_random_natural_32_bit_in_range(
                            0U,
                            static_cast<netlab::layer_index_type>(counts_of_centers_into_layers.size() - 1U),
                            generator
                            )
                    );
    for (netlab::layer_index_type i = 0U;
            i != counts_of_centers_into_layers.size();
            ++i, layer_index = (layer_index + 1U) % counts_of_centers_into_layers.size())
        if (counts_of_centers_into_layers.at(layer_index) != 0ULL)
        {
            --counts_of_centers_into_layers.at(layer_index);
            return layer_index;
        }
    UNREACHABLE();
}


vector3  compute_spiker_position_projected_to_area_layer(
        netlab::layer_index_type const  spiker_layer_index,
        netlab::layer_index_type const&  area_layer_index,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
        netlab::network_props const&  props
        )
{
    TMPROF_BLOCK();

    netlab::network_layer_props const&  spiker_layer_props = props.layer_props().at(spiker_layer_index);
    netlab::network_layer_props const&  layer_props = props.layer_props().at(area_layer_index);
    vector3  spiker_position = spiker_layer_props.spiker_sector_centre(spiker_sector_coordinate_x,
                                                                       spiker_sector_coordinate_y,
                                                                       spiker_sector_coordinate_c);
    if (area_layer_index != spiker_layer_index)
        spiker_position =
            layer_props.low_corner_of_ships()
            + ( ( (spiker_position - spiker_layer_props.low_corner_of_ships()).array() /
                    (spiker_layer_props.high_corner_of_ships() - spiker_layer_props.low_corner_of_ships()).array() )
                * (layer_props.high_corner_of_ships() - layer_props.low_corner_of_ships()).array() ).matrix();

    return spiker_position;
}


void  compute_maximal_bbox_for_storing_movement_area_of_spiker_into(
        vector3 const&  spiker_position_projected_to_area_layer,
        vector3 const&  size_of_area,
        netlab::sector_coordinate_type const  max_distance_x,
        netlab::sector_coordinate_type const  max_distance_y,
        netlab::sector_coordinate_type const  max_distance_c,
        netlab::network_layer_props const&  area_layer_props,
        vector3&  low_corner,
        vector3&  high_corner
        )
{
    vector3 const  max_distance(max_distance_x * area_layer_props.distance_of_spikers_along_x_axis_in_meters(),
                                max_distance_y * area_layer_props.distance_of_spikers_along_y_axis_in_meters(),
                                max_distance_c * area_layer_props.distance_of_spikers_along_c_axis_in_meters());

    vector3 const  docks_distance(area_layer_props.distance_of_docks_in_meters(),
                                  area_layer_props.distance_of_docks_in_meters(),
                                  area_layer_props.distance_of_docks_in_meters());

    low_corner = spiker_position_projected_to_area_layer - max_distance - 0.5f * size_of_area;
    vector3 const  u = (low_corner - area_layer_props.low_corner_of_ships()).array() / docks_distance.array();
    low_corner(0) += (std::floorf(u(0)) - u(0)) * area_layer_props.distance_of_docks_in_meters();
    low_corner(1) += (std::floorf(u(1)) - u(1)) * area_layer_props.distance_of_docks_in_meters();
    low_corner(2) += (std::floorf(u(2)) - u(2)) * area_layer_props.distance_of_docks_in_meters();

    high_corner = low_corner + 2.0f * max_distance + size_of_area;

    bool const  must_intersect =
        angeo::collision_bbox_bbox(
            area_layer_props.low_corner_of_ships(), area_layer_props.high_corner_of_ships(),
            low_corner, high_corner,
            low_corner, high_corner
            );
    INVARIANT(must_intersect);
}


void  compute_initial_movement_area_center_for_ships_of_spiker_XYC(
        netlab::layer_index_type const  spiker_layer_index,
        netlab::object_index_type const  spiker_index_into_layer,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
        netlab::network_props const&  props,
        netlab::layer_index_type const  area_layer_index,
        netlab::sector_coordinate_type const  max_distance_x,
        netlab::sector_coordinate_type const  max_distance_y,
        netlab::sector_coordinate_type const  max_distance_c,
        random_generator_for_natural_32_bit&  position_generator,
        vector3&  area_center
        )
{
    TMPROF_BLOCK();

    vector3 const  spiker_position =
            compute_spiker_position_projected_to_area_layer(
                    spiker_layer_index,
                    area_layer_index,
                    spiker_sector_coordinate_x,
                    spiker_sector_coordinate_y,
                    spiker_sector_coordinate_c,
                    props
                    );

    netlab::network_layer_props const&  area_layer_props = props.layer_props().at(area_layer_index);
    vector3 const&  size_of_area = props.layer_props().at(spiker_layer_index).size_of_ship_movement_area_in_meters(area_layer_index);

    vector3  low_corner, high_corner;
    compute_maximal_bbox_for_storing_movement_area_of_spiker_into(
            spiker_position,
            size_of_area,
            max_distance_x,
            max_distance_y,
            max_distance_c,
            area_layer_props,
            low_corner,
            high_corner
            );

    vector3 const  min_low_pos = low_corner;
    vector3 const  max_low_pos = high_corner - size_of_area;
    INVARIANT(min_low_pos(0) <= max_low_pos(0) && min_low_pos(1) <= max_low_pos(1) && min_low_pos(2) <= max_low_pos(2));

    natural_32_bit const  max_rand_x =
            static_cast<natural_32_bit>(0.5f + (max_low_pos(0) - min_low_pos(0)) / area_layer_props.distance_of_docks_in_meters());
    natural_32_bit const  max_rand_y =
            static_cast<natural_32_bit>(0.5f + (max_low_pos(1) - min_low_pos(1)) / area_layer_props.distance_of_docks_in_meters());
    natural_32_bit const  max_rand_c =
            static_cast<natural_32_bit>(0.5f + (max_low_pos(2) - min_low_pos(2)) / area_layer_props.distance_of_docks_in_meters());

    auto const  generate_random_area_center =
            [ &min_low_pos, &max_low_pos, &size_of_area,
              max_rand_x, max_rand_y, max_rand_c,
              &area_layer_props, &position_generator]() -> vector3 {
                vector3  shift((float_32_bit)get_random_natural_32_bit_in_range(0U,max_rand_x,position_generator),
                                (float_32_bit)get_random_natural_32_bit_in_range(0U,max_rand_y,position_generator),
                                (float_32_bit)get_random_natural_32_bit_in_range(0U,max_rand_c,position_generator));
                shift *= area_layer_props.distance_of_docks_in_meters();
                shift(0) = std::min(shift(0),max_low_pos(0) - min_low_pos(0));
                shift(1) = std::min(shift(1),max_low_pos(1) - min_low_pos(1));
                shift(2) = std::min(shift(2),max_low_pos(2) - min_low_pos(2));
                return min_low_pos + shift + 0.5f * size_of_area;
            };

    if (area_layer_index != spiker_layer_index)
    {
        area_center = generate_random_area_center();
        return;
    }

    for (int i = 0; i < 1000; ++i)
    {
        area_center = generate_random_area_center();
        vector3 const  delta = area_center - spiker_position;
        if (std::abs(delta(0)) >= 0.5f * (size_of_area(0) + area_layer_props.distance_of_spikers_along_x_axis_in_meters()) ||
            std::abs(delta(1)) >= 0.5f * (size_of_area(1) + area_layer_props.distance_of_spikers_along_y_axis_in_meters()) ||
            std::abs(delta(2)) >= 0.5f * (size_of_area(2) + area_layer_props.distance_of_spikers_along_c_axis_in_meters()) )
            return;
    }
    UNREACHABLE(); // We should always succeeed to find a valid center of area. This solution (wiht throwing
                   // exception) is perhaps better then be captured in infinite loop.
}


void  compute_random_ship_position_in_movement_area(
        vector3 const&  area_center,
        float_32_bit const  half_size_of_ship_movement_area_along_x_axis_in_meters,
        float_32_bit const  half_size_of_ship_movement_area_along_y_axis_in_meters,
        float_32_bit const  half_size_of_ship_movement_area_along_c_axis_in_meters,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  ship_position
        )
{
    TMPROF_BLOCK();

    ship_position = {
            get_random_float_32_bit_in_range(
                    area_center(0) - half_size_of_ship_movement_area_along_x_axis_in_meters,
                    area_center(0) + half_size_of_ship_movement_area_along_x_axis_in_meters,
                    random_generator
                    ),
            get_random_float_32_bit_in_range(
                    area_center(1) - half_size_of_ship_movement_area_along_y_axis_in_meters,
                    area_center(1) + half_size_of_ship_movement_area_along_y_axis_in_meters,
                    random_generator
                    ),
            get_random_float_32_bit_in_range(
                    area_center(2) - half_size_of_ship_movement_area_along_c_axis_in_meters,
                    area_center(2) + half_size_of_ship_movement_area_along_c_axis_in_meters,
                    random_generator
                    )
            };
}

void  compute_random_ship_velocity_in_movement_area(
        float_32_bit const  min_speed_of_ship_in_movement_area,
        float_32_bit const  max_speed_of_ship_in_movement_area,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  ship_velocity
        )
{
    TMPROF_BLOCK();

    angeo::get_random_vector_of_magnitude(
                get_random_float_32_bit_in_range(
                        min_speed_of_ship_in_movement_area,
                        max_speed_of_ship_in_movement_area,
                        random_generator
                        ),
                random_generator,
                ship_velocity
                );
}


float_32_bit  exponential_increase_from_zero_to_one(
        float_32_bit const  x,
        float_32_bit const  X_MAX,
        float_32_bit const  POW
        )
{
    if (x <= 0.0f)
        return 0.0f;
    if (x >= X_MAX)
        return 1.0f;
    float_32_bit const  f_x = (std::expf(x / X_MAX) - 1.0f) / (E() - 1.0f);
    return POW > 1.0f ? std::powf(f_x,POW) : f_x;
}


float_32_bit  exponential_decrease_from_one_to_zero(
        float_32_bit const  x,
        float_32_bit const  X_MAX,
        float_32_bit const  POW
        )
{
    if (x <= 0.0f)
        return 1.0f;
    if (x >= X_MAX)
        return 0.0f;
    float_32_bit const  f_x = (std::expf(1.0f - x / X_MAX) - 1.0f) / (E() - 1.0f);
    return POW > 1.0f ? std::powf(f_x,POW) : f_x;
}


}
