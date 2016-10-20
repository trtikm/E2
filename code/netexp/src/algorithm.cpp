#include <netexp/algorithm.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace netexp {


void  compute_center_of_movement_area_for_ships_of_spiker(
        natural_8_bit const  spiker_layer_index,
        vector3 const&  spiker_position,
        netlab::network_props const&  props,
        bar_random_distribution const&  distribution_of_spiker_layer,
        random_generator_for_natural_32_bit&  generator_of_spiker_layer,
        std::vector<float_32_bit> const&  max_distance_x,
        std::vector<float_32_bit> const&  max_distance_y,
        std::vector<float_32_bit> const&  max_distance_c,
        random_generator_for_natural_32_bit&  position_generator,
        natural_8_bit&  area_layer_index,
        vector3&  area_center
        )
{
    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) < props.layer_props().size());
    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_x.size());
    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_y.size());
    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_c.size());

    area_layer_index = static_cast<natural_8_bit>(get_random_bar_index(distribution_of_spiker_layer,generator_of_spiker_layer));
    netlab::network_layer_props const&  spiker_layer_props = props.layer_props().at(spiker_layer_index);
    if (area_layer_index == spiker_layer_index)
    {
        {
            float_32_bit const  low_center_x =
                    spiker_layer_props.low_corner_of_ships()(0)
                        + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters();
            float_32_bit const  high_center_x =
                    spiker_layer_props.high_corner_of_ships()(0)
                        - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters();

            float_32_bit const  raw_low0_x = spiker_position(0) - max_distance_x.at(area_layer_index);
            float_32_bit const  raw_high0_x =
                    spiker_position(0) - 0.5f * (spiker_layer_props.distance_of_spikers_along_x_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters());

            ASSUMPTION(raw_low0_x <= raw_high0_x);

            float_32_bit const  low0_x = (raw_low0_x < low_center_x)  ? low_center_x  :
                                         (raw_low0_x > high_center_x) ? high_center_x :
                                                                        raw_low0_x    ;
            float_32_bit const  high0_x = (raw_high0_x < low_center_x)  ? low_center_x  :
                                          (raw_high0_x > high_center_x) ? high_center_x :
                                                                          raw_high0_x   ;

            float_32_bit const  raw_low1_x =
                    spiker_position(0) + 0.5f * (spiker_layer_props.distance_of_spikers_along_x_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters());
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
                    spiker_layer_props.low_corner_of_ships()(0)
                        + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters();
            float_32_bit const  high_center_y =
                    spiker_layer_props.high_corner_of_ships()(0)
                        - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters();

            float_32_bit const  raw_low0_y = spiker_position(1) - max_distance_y.at(area_layer_index);
            float_32_bit const  raw_high0_y =
                    spiker_position(1) - 0.5f * (spiker_layer_props.distance_of_spikers_along_y_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters());

            ASSUMPTION(raw_low0_y <= raw_high0_y);

            float_32_bit const  low0_y = (raw_low0_y < low_center_y)  ? low_center_y  :
                                         (raw_low0_y > high_center_y) ? high_center_y :
                                                                        raw_low0_y    ;
            float_32_bit const  high0_y = (raw_high0_y < low_center_y)  ? low_center_y  :
                                          (raw_high0_y > high_center_y) ? high_center_y :
                                                                          raw_high0_y   ;

            float_32_bit const  raw_low1_y =
                    spiker_position(1) + 0.5f * (spiker_layer_props.distance_of_spikers_along_y_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters());
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
                    spiker_layer_props.low_corner_of_ships()(0)
                        + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters();
            float_32_bit const  high_center_c =
                    spiker_layer_props.high_corner_of_ships()(0)
                        - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters();

            float_32_bit const  raw_low0_c = spiker_position(2) - max_distance_c.at(area_layer_index);
            float_32_bit const  raw_high0_c =
                    spiker_position(2) - 0.5f * (spiker_layer_props.distance_of_spikers_along_c_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters());

            ASSUMPTION(raw_low0_c <= raw_high0_c);

            float_32_bit const  low0_c = (raw_low0_c < low_center_c)  ? low_center_c  :
                                         (raw_low0_c > high_center_c) ? high_center_c :
                                                                        raw_low0_c    ;
            float_32_bit const  high0_c = (raw_high0_c < low_center_c)  ? low_center_c  :
                                          (raw_high0_c > high_center_c) ? high_center_c :
                                                                          raw_high0_c   ;

            float_32_bit const  raw_low1_c =
                    spiker_position(2) + 0.5f * (spiker_layer_props.distance_of_spikers_along_c_axis_in_meters() +
                                                 spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters());
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

        float_32_bit const  raw_area_center_x =
                projected_spiker_position_x +
                get_random_float_32_bit_in_range(
                        -max_distance_x.at(area_layer_index),
                        max_distance_x.at(area_layer_index),
                        position_generator
                        );
        float_32_bit const  raw_area_center_y =
                projected_spiker_position_y +
                get_random_float_32_bit_in_range(
                        -max_distance_y.at(area_layer_index),
                        max_distance_y.at(area_layer_index),
                        position_generator
                        );

        float_32_bit const  low_center_x =
                area_layer_props.low_corner_of_ships()(0)
                    + 0.5f * area_layer_props.size_of_ship_movement_area_along_x_axis_in_meters();
        float_32_bit const  high_center_x =
                area_layer_props.high_corner_of_ships()(0)
                    - 0.5f * area_layer_props.size_of_ship_movement_area_along_x_axis_in_meters();

        float_32_bit const  low_center_y =
                area_layer_props.low_corner_of_ships()(1)
                    + 0.5f * area_layer_props.size_of_ship_movement_area_along_y_axis_in_meters();
        float_32_bit const  high_center_y =
                area_layer_props.high_corner_of_ships()(1)
                    - 0.5f * area_layer_props.size_of_ship_movement_area_along_y_axis_in_meters();

        area_center(0) = (raw_area_center_x < low_center_x)  ? low_center_x      :
                         (raw_area_center_x > high_center_x) ? high_center_x     :
                                                               raw_area_center_x ;
        area_center(1) = (raw_area_center_y < low_center_y)  ? low_center_y      :
                         (raw_area_center_y > high_center_y) ? high_center_y     :
                                                               raw_area_center_y ;
        area_center(2) =
                get_random_float_32_bit_in_range(
                        area_layer_props.low_corner_of_ships()(2)
                            + 0.5f * area_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(),
                        area_layer_props.high_corner_of_ships()(2)
                            - 0.5f * area_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(),
                        position_generator
                        );
    }
}


void  compute_random_ship_position_and_velocity_in_movement_area(
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

    compute_random_vector_of_magnitude(
                get_random_float_32_bit_in_range(
                        min_speed_of_ship_in_movement_area,
                        max_speed_of_ship_in_movement_area,
                        random_generator
                        ),
                random_generator,
                ship_velocity
                );
}

void  compute_random_vector_of_magnitude(
        float_32_bit const  magnitude,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  resulting_vector
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(magnitude >= 0.0f);

    float_32_bit const  phi = get_random_float_32_bit_in_range(0.0f,2.0f * PI(),random_generator);
    float_32_bit const  sin_phi = std::sinf(phi);
    float_32_bit const  cos_phi = std::cosf(phi);

    float_32_bit const  psi = get_random_float_32_bit_in_range(-PI(),PI(),random_generator);
    float_32_bit const  sin_psi = std::sinf(psi);
    float_32_bit const  cos_psi = std::cosf(psi);

    resulting_vector = magnitude * vector3(cos_psi * cos_phi, cos_psi * sin_phi, sin_psi);
}


}
