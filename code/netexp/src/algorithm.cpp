#include <netexp/algorithm.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace netexp {


void  compute_center_of_movement_area_for_ships_of_spiker(
        netlab::layer_index_type const  spiker_layer_index,
        vector3 const&  spiker_position,
        netlab::network_props const&  props,
        bar_random_distribution const&  distribution_of_spiker_layer,
        random_generator_for_natural_32_bit&  generator_of_spiker_layer,
        std::vector<float_32_bit> const&  max_distance_x,
        std::vector<float_32_bit> const&  max_distance_y,
        std::vector<float_32_bit> const&  max_distance_c,
        random_generator_for_natural_32_bit&  position_generator,
        netlab::layer_index_type&  area_layer_index,
        vector3&  area_center
        )
{
    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == props.layer_props().size());
    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_x.size());
    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_y.size());
    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_c.size());

    area_layer_index = static_cast<netlab::layer_index_type>(
                            get_random_bar_index(distribution_of_spiker_layer,generator_of_spiker_layer)
                            );
    netlab::network_layer_props const&  spiker_layer_props = props.layer_props().at(spiker_layer_index);
    if (area_layer_index == spiker_layer_index)
    {
        {
            float_32_bit const  low_center_x =
                    spiker_layer_props.low_corner_of_ships()(0)
                        + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index);
            float_32_bit const  high_center_x =
                    spiker_layer_props.high_corner_of_ships()(0)
                        - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index);

            float_32_bit const  raw_low0_x = spiker_position(0) - max_distance_x.at(area_layer_index);
            float_32_bit const  raw_high0_x =
                    spiker_position(0) - 0.5f * (spiker_layer_props.distance_of_spikers_along_x_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(
                                                        area_layer_index));

            ASSUMPTION(raw_low0_x <= raw_high0_x);

            float_32_bit const  low0_x = (raw_low0_x < low_center_x)  ? low_center_x  :
                                         (raw_low0_x > high_center_x) ? high_center_x :
                                                                        raw_low0_x    ;
            float_32_bit const  high0_x = (raw_high0_x < low_center_x)  ? low_center_x  :
                                          (raw_high0_x > high_center_x) ? high_center_x :
                                                                          raw_high0_x   ;

            float_32_bit const  raw_low1_x =
                    spiker_position(0) + 0.5f * (spiker_layer_props.distance_of_spikers_along_x_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(
                                                        area_layer_index));
            float_32_bit const  raw_high1_x = spiker_position(0) + max_distance_x.at(area_layer_index);

            ASSUMPTION(raw_low1_x <= raw_high1_x);

            float_32_bit const  low1_x = (raw_low1_x < low_center_x)  ? low_center_x  :
                                         (raw_low1_x > high_center_x) ? high_center_x :
                                                                        raw_low1_x    ;
            float_32_bit const  high1_x = (raw_high1_x < low_center_x)  ? low_center_x  :
                                          (raw_high1_x > high_center_x) ? high_center_x :
                                                                          raw_high1_x   ;

            natural_32_bit const  index_x =
                    get_random_bar_index(
                        make_bar_random_distribution_from_size_bars({high0_x - low0_x, high1_x - low1_x}),
                        position_generator
                        );

            area_center(0) = index_x == 0U ? get_random_float_32_bit_in_range(low0_x,high0_x,position_generator) :
                                             get_random_float_32_bit_in_range(low1_x,high1_x,position_generator) ;
        }
        {
            float_32_bit const  low_center_y =
                    spiker_layer_props.low_corner_of_ships()(1)
                        + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index);
            float_32_bit const  high_center_y =
                    spiker_layer_props.high_corner_of_ships()(1)
                        - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index);

            float_32_bit const  raw_low0_y = spiker_position(1) - max_distance_y.at(area_layer_index);
            float_32_bit const  raw_high0_y =
                    spiker_position(1) - 0.5f * (spiker_layer_props.distance_of_spikers_along_y_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(
                                                        area_layer_index));

            ASSUMPTION(raw_low0_y <= raw_high0_y);

            float_32_bit const  low0_y = (raw_low0_y < low_center_y)  ? low_center_y  :
                                         (raw_low0_y > high_center_y) ? high_center_y :
                                                                        raw_low0_y    ;
            float_32_bit const  high0_y = (raw_high0_y < low_center_y)  ? low_center_y  :
                                          (raw_high0_y > high_center_y) ? high_center_y :
                                                                          raw_high0_y   ;

            float_32_bit const  raw_low1_y =
                    spiker_position(1) + 0.5f * (spiker_layer_props.distance_of_spikers_along_y_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(
                                                        area_layer_index));
            float_32_bit const  raw_high1_y = spiker_position(1) + max_distance_y.at(area_layer_index);

            ASSUMPTION(raw_low1_y <= raw_high1_y);

            float_32_bit const  low1_y = (raw_low1_y < low_center_y)  ? low_center_y  :
                                         (raw_low1_y > high_center_y) ? high_center_y :
                                                                        raw_low1_y    ;
            float_32_bit const  high1_y = (raw_high1_y < low_center_y)  ? low_center_y  :
                                          (raw_high1_y > high_center_y) ? high_center_y :
                                                                          raw_high1_y   ;

            natural_32_bit const  index_y =
                    get_random_bar_index(
                        make_bar_random_distribution_from_size_bars({high0_y - low0_y, high1_y - low1_y}),
                        position_generator
                        );

            area_center(1) = index_y == 0U ? get_random_float_32_bit_in_range(low0_y,high0_y,position_generator) :
                                             get_random_float_32_bit_in_range(low1_y,high1_y,position_generator) ;
        }
        {
            float_32_bit const  low_center_c =
                    spiker_layer_props.low_corner_of_ships()(2)
                        + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index);
            float_32_bit const  high_center_c =
                    spiker_layer_props.high_corner_of_ships()(2)
                        - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index);

            float_32_bit const  raw_low0_c = spiker_position(2) - max_distance_c.at(area_layer_index);
            float_32_bit const  raw_high0_c =
                    spiker_position(2) - 0.5f * (spiker_layer_props.distance_of_spikers_along_c_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(
                                                        area_layer_index));

            ASSUMPTION(raw_low0_c <= raw_high0_c);

            float_32_bit const  low0_c = (raw_low0_c < low_center_c)  ? low_center_c  :
                                         (raw_low0_c > high_center_c) ? high_center_c :
                                                                        raw_low0_c    ;
            float_32_bit const  high0_c = (raw_high0_c < low_center_c)  ? low_center_c  :
                                          (raw_high0_c > high_center_c) ? high_center_c :
                                                                          raw_high0_c   ;

            float_32_bit const  raw_low1_c =
                    spiker_position(2) + 0.5f * (spiker_layer_props.distance_of_spikers_along_c_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(
                                                        area_layer_index));
            float_32_bit const  raw_high1_c = spiker_position(2) + max_distance_c.at(area_layer_index);

            ASSUMPTION(raw_low1_c <= raw_high1_c);

            float_32_bit const  low1_c = (raw_low1_c < low_center_c)  ? low_center_c  :
                                         (raw_low1_c > high_center_c) ? high_center_c :
                                                                        raw_low1_c    ;
            float_32_bit const  high1_c = (raw_high1_c < low_center_c)  ? low_center_c  :
                                          (raw_high1_c > high_center_c) ? high_center_c :
                                                                          raw_high1_c   ;

            natural_32_bit const  index_c =
                    get_random_bar_index(
                        make_bar_random_distribution_from_size_bars({high0_c - low0_c, high1_c - low1_c}),
                        position_generator
                        );

            area_center(2) = index_c == 0U ? get_random_float_32_bit_in_range(low0_c,high0_c,position_generator) :
                                             get_random_float_32_bit_in_range(low1_c,high1_c,position_generator) ;
        }
    }
    else
    {
        float_32_bit const coef_x =
                (spiker_position(0) - spiker_layer_props.low_corner_of_ships()(0)) /
                    (spiker_layer_props.high_corner_of_ships()(0) - spiker_layer_props.low_corner_of_ships()(0));
        float_32_bit const coef_y =
                (spiker_position(1) - spiker_layer_props.low_corner_of_ships()(1)) /
                    (spiker_layer_props.high_corner_of_ships()(1) - spiker_layer_props.low_corner_of_ships()(1));

        netlab::network_layer_props const&  area_layer_props = props.layer_props().at(area_layer_index);

        float_32_bit const  projected_spiker_position_x =
                area_layer_props.low_corner_of_ships()(0) + coef_x *
                    (area_layer_props.high_corner_of_ships()(0) - area_layer_props.low_corner_of_ships()(0));
        float_32_bit const  projected_spiker_position_y =
                area_layer_props.low_corner_of_ships()(1) + coef_y *
                    (area_layer_props.high_corner_of_ships()(1) - area_layer_props.low_corner_of_ships()(1));

        float_32_bit const  low_center_x =
                std::max(area_layer_props.low_corner_of_ships()(0)
                            + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index),
                         projected_spiker_position_x - max_distance_x.at(area_layer_index)
                         );
        float_32_bit const  high_center_x =
                std::min(area_layer_props.high_corner_of_ships()(0)
                            - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index),
                         low_center_x + 2.0f * max_distance_x.at(area_layer_index)
                         );

        float_32_bit const  low_center_y =
                std::max(area_layer_props.low_corner_of_ships()(1)
                            + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index),
                         projected_spiker_position_y - max_distance_y.at(area_layer_index)
                         );
        float_32_bit const  high_center_y =
                std::min(area_layer_props.high_corner_of_ships()(1)
                            - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index),
                         low_center_y + 2.0f * max_distance_y.at(area_layer_index)
                         );

        float_32_bit const  low_center_c =
                area_layer_props.low_corner_of_ships()(2)
                    + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index);

        float_32_bit const  high_center_c =
                area_layer_props.high_corner_of_ships()(2)
                    - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index);

        area_center(0) = get_random_float_32_bit_in_range(low_center_x,high_center_x,position_generator);
        area_center(1) = get_random_float_32_bit_in_range(low_center_y,high_center_y,position_generator);
        area_center(2) = get_random_float_32_bit_in_range(low_center_c,high_center_c,position_generator);
    }
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
