#include <netexp/algorithm.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace netexp {


void  compute_extreme_sector_coordinates_for_center_of_movement_area(
        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
        netlab::network_layer_props const&  layer_props,
        vector3 const&  size_of_ship_movement_area_in_meters,
        netlab::sector_coordinate_type const  max_distance_x,
        netlab::sector_coordinate_type const  max_distance_y,
        netlab::sector_coordinate_type const  max_distance_c,
        netlab::sector_coordinate_type&  lo_x,
        netlab::sector_coordinate_type&  lo_y,
        netlab::sector_coordinate_type&  lo_c,
        netlab::sector_coordinate_type&  hi_x,
        netlab::sector_coordinate_type&  hi_y,
        netlab::sector_coordinate_type&  hi_c
        )
{
    vector3 const  area_shift = 0.5f * size_of_ship_movement_area_in_meters;
    vector3 const  spiker_shift = 0.5f * vector3(layer_props.distance_of_spikers_along_x_axis_in_meters() -
                                                        layer_props.distance_of_docks_in_meters(),
                                                 layer_props.distance_of_spikers_along_y_axis_in_meters() -
                                                        layer_props.distance_of_docks_in_meters(),
                                                 layer_props.distance_of_spikers_along_c_axis_in_meters() -
                                                        layer_props.distance_of_docks_in_meters());

    {
        netlab::sector_coordinate_type  min_x, min_y, min_c;
        layer_props.spiker_sector_coordinates(layer_props.low_corner_of_ships() + area_shift - spiker_shift, min_x, min_y, min_c);
        lo_x = std::max(min_x, spiker_sector_coordinate_x <= max_distance_x ? 0U : spiker_sector_coordinate_x - max_distance_x);
        lo_y = std::max(min_y, spiker_sector_coordinate_y <= max_distance_y ? 0U : spiker_sector_coordinate_y - max_distance_y);
        lo_c = std::max(min_c, spiker_sector_coordinate_c <= max_distance_c ? 0U : spiker_sector_coordinate_c - max_distance_c);
    }

    {
        netlab::sector_coordinate_type  max_x, max_y, max_c;
        layer_props.spiker_sector_coordinates(layer_props.high_corner_of_ships() - area_shift + spiker_shift, max_x, max_y, max_c);
        hi_x = std::max(lo_x, std::min(max_x, layer_props.num_spikers_along_x_axis() - spiker_sector_coordinate_x <= max_distance_x ?
                                                    layer_props.num_spikers_along_x_axis() - 1U :
                                                    spiker_sector_coordinate_x + max_distance_x));
        hi_y = std::max(lo_y, std::min(max_y, layer_props.num_spikers_along_y_axis() - spiker_sector_coordinate_y <= max_distance_y ?
                                                    layer_props.num_spikers_along_y_axis() - 1U :
                                                    spiker_sector_coordinate_y + max_distance_y));
        hi_c = std::max(lo_c, std::min(max_c, layer_props.num_spikers_along_c_axis() - spiker_sector_coordinate_c <= max_distance_c ?
                                                    layer_props.num_spikers_along_c_axis() - 1U :
                                                    spiker_sector_coordinate_c + max_distance_c));
    }
}


void  ensure_whole_movement_area_is_inside_layer(
        vector3 const&  low_corner_of_ships,
        vector3 const&  high_corner_of_ships,
        vector3 const&  size_of_ship_movement_area_in_meters,
        vector3&  area_center
        )
{
    ASSUMPTION(
        size_of_ship_movement_area_in_meters(0) <= high_corner_of_ships(0) - low_corner_of_ships(0) + 0.001f &&
        size_of_ship_movement_area_in_meters(1) <= high_corner_of_ships(1) - low_corner_of_ships(1) + 0.001f &&
        size_of_ship_movement_area_in_meters(2) <= high_corner_of_ships(2) - low_corner_of_ships(2) + 0.001f
        );

    vector3 const  area_shift = 0.5f * size_of_ship_movement_area_in_meters;
    for (int i = 0; i != 3; ++i)
    {
        if (area_center(i) - area_shift(i) < low_corner_of_ships(i))
            area_center(i) = low_corner_of_ships(i) + area_shift(i);
        else if (area_center(i) + area_shift(i) > high_corner_of_ships(i))
            area_center(i) = high_corner_of_ships(i) - area_shift(i);
    }
}


void  compute_center_of_movement_area_for_ships_of_spiker(
        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
        netlab::network_layer_props const&  layer_props,
        vector3 const&  size_of_ship_movement_area_in_meters,
        netlab::sector_coordinate_type const  max_distance_x,
        netlab::sector_coordinate_type const  max_distance_y,
        netlab::sector_coordinate_type const  max_distance_c,
        random_generator_for_natural_32_bit&  position_generator,
        vector3&  area_center
        )
{
    TMPROF_BLOCK();

    netlab::sector_coordinate_type  lo_x, lo_y, lo_c;
    netlab::sector_coordinate_type  hi_x, hi_y, hi_c;
    compute_extreme_sector_coordinates_for_center_of_movement_area(
            spiker_sector_coordinate_x,
            spiker_sector_coordinate_y,
            spiker_sector_coordinate_c,
            layer_props,
            size_of_ship_movement_area_in_meters,
            max_distance_x,
            max_distance_y,
            max_distance_c,
            lo_x, lo_y, lo_c,
            hi_x, hi_y, hi_c
            );

    vector3 const  spiker_position = layer_props.spiker_sector_centre(spiker_sector_coordinate_x,
                                                                      spiker_sector_coordinate_y,
                                                                      spiker_sector_coordinate_c);
    vector3 const  shift = 0.5f * size_of_ship_movement_area_in_meters;

    for (int i = 0; i < 10; ++i)
    {
        area_center = layer_props.spiker_sector_centre(get_random_natural_32_bit_in_range(lo_x, hi_x, position_generator),
                                                       get_random_natural_32_bit_in_range(lo_y, hi_y, position_generator),
                                                       get_random_natural_32_bit_in_range(lo_c, hi_c, position_generator));
        vector3 const  delta = area_center - spiker_position;
        if (std::abs(delta(0)) >= shift(0) + 0.5f * layer_props.distance_of_spikers_along_x_axis_in_meters() ||
            std::abs(delta(1)) >= shift(1) + 0.5f * layer_props.distance_of_spikers_along_y_axis_in_meters() ||
            std::abs(delta(2)) >= shift(2) + 0.5f * layer_props.distance_of_spikers_along_c_axis_in_meters() )
            break;
    }

    ensure_whole_movement_area_is_inside_layer(
            layer_props.low_corner_of_ships(),
            layer_props.high_corner_of_ships(),
            size_of_ship_movement_area_in_meters,
            area_center
            );
}


void  compute_center_of_movement_area_for_ships_of_spiker(
        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
        netlab::network_layer_props const&  spiker_layer_props,
        netlab::network_layer_props const&  area_layer_props,
        vector3 const&  size_of_ship_movement_area_in_meters,
        netlab::sector_coordinate_type const  max_distance_x,
        netlab::sector_coordinate_type const  max_distance_y,
        netlab::sector_coordinate_type const  max_distance_c,
        random_generator_for_natural_32_bit&  position_generator,
        vector3&  area_center
        )
{
    TMPROF_BLOCK();

    netlab::sector_coordinate_type  lo_x, lo_y, lo_c;
    netlab::sector_coordinate_type  hi_x, hi_y, hi_c;
    {
         netlab::sector_coordinate_type const  projected_spiker_sector_coordinate_x =
                static_cast<netlab::sector_coordinate_type>(
                        (static_cast<float_64_bit>(spiker_sector_coordinate_x) /
                                static_cast<float_64_bit>(spiker_layer_props.num_spikers_along_x_axis() - 1U)) *
                                        static_cast<float_64_bit>(area_layer_props.num_spikers_along_x_axis() - 1U)
                        );
        ASSUMPTION(projected_spiker_sector_coordinate_x < area_layer_props.num_spikers_along_x_axis());

        netlab::sector_coordinate_type const  projected_spiker_sector_coordinate_y =
                static_cast<netlab::sector_coordinate_type>(
                        (static_cast<float_64_bit>(spiker_sector_coordinate_y) /
                                static_cast<float_64_bit>(spiker_layer_props.num_spikers_along_y_axis() - 1U)) *
                                        static_cast<float_64_bit>(area_layer_props.num_spikers_along_y_axis() - 1U)
                        );
        ASSUMPTION(projected_spiker_sector_coordinate_y < area_layer_props.num_spikers_along_y_axis());

        netlab::sector_coordinate_type const  projected_spiker_sector_coordinate_c =
                static_cast<netlab::sector_coordinate_type>(
                        (static_cast<float_64_bit>(spiker_sector_coordinate_c) /
                                static_cast<float_64_bit>(spiker_layer_props.num_spikers_along_c_axis() - 1U)) *
                                        static_cast<float_64_bit>(area_layer_props.num_spikers_along_c_axis() - 1U)
                        );
        ASSUMPTION(projected_spiker_sector_coordinate_c < area_layer_props.num_spikers_along_c_axis());

        compute_extreme_sector_coordinates_for_center_of_movement_area(
                projected_spiker_sector_coordinate_x,
                projected_spiker_sector_coordinate_y,
                projected_spiker_sector_coordinate_c,
                area_layer_props,
                size_of_ship_movement_area_in_meters,
                max_distance_x,
                max_distance_y,
                max_distance_c,
                lo_x, lo_y, lo_c,
                hi_x, hi_y, hi_c
                );
    }
    area_center = area_layer_props.spiker_sector_centre(get_random_natural_32_bit_in_range(lo_x, hi_x, position_generator),
                                                        get_random_natural_32_bit_in_range(lo_y, hi_y, position_generator),
                                                        get_random_natural_32_bit_in_range(lo_c, hi_c, position_generator));

    ensure_whole_movement_area_is_inside_layer(
            area_layer_props.low_corner_of_ships(),
            area_layer_props.high_corner_of_ships(),
            size_of_ship_movement_area_in_meters,
            area_center
            );
}



//void  compute_center_of_movement_area_for_ships_of_spiker(
//        netlab::layer_index_type const  spiker_layer_index,
//        vector3 const&  spiker_position,
//        netlab::network_props const&  props,
//        bar_random_distribution const&  distribution_of_spiker_layer,
//        random_generator_for_natural_32_bit&  generator_of_spiker_layer,
//        std::vector<float_32_bit> const&  max_distance_x,
//        std::vector<float_32_bit> const&  max_distance_y,
//        std::vector<float_32_bit> const&  max_distance_c,
//        random_generator_for_natural_32_bit&  position_generator,
//        netlab::layer_index_type&  area_layer_index,
//        vector3&  area_center
//        )
//{
//    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == props.layer_props().size());
//    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_x.size());
//    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_y.size());
//    ASSUMPTION(get_num_bars(distribution_of_spiker_layer) == max_distance_c.size());
//
//    area_layer_index = static_cast<netlab::layer_index_type>(
//                            get_random_bar_index(distribution_of_spiker_layer,generator_of_spiker_layer)
//                            );
//    netlab::network_layer_props const&  spiker_layer_props = props.layer_props().at(spiker_layer_index);
//
//
//    vector3 const layer_lo =
//        spiker_layer_props.low_corner_of_ships() + 0.5f * spiker_layer_props.size_of_ship_movement_area_in_meters(area_layer_index);
//    vector3 const layer_hi =
//        spiker_layer_props.high_corner_of_ships() - 0.5f * spiker_layer_props.size_of_ship_movement_area_in_meters(area_layer_index);
//
//    vector3 const  max_shift(max_distance_x.at(area_layer_index),max_distance_y.at(area_layer_index),max_distance_c.at(area_layer_index));
//
//    if (area_layer_index == spiker_layer_index)
//    {
//        //vector3 const distance_lo = spiker_position - max_shift;
//        //vector3 const distance_hi = spiker_position + max_shift;
//
//        //vector3 const lo(std::max(distance_lo(0),layer_lo(0)),
//        //                 std::max(distance_lo(1),layer_lo(1)),
//        //                 std::max(distance_lo(2),layer_lo(2)));
//        //vector3 const hi(std::min(distance_hi(0),layer_hi(0)),
//        //                 std::min(distance_hi(1),layer_hi(1)),
//        //                 std::min(distance_hi(2),layer_hi(2)));
//
//        netlab::sector_coordinate_type  lo_x, lo_y, lo_c;
//        spiker_layer_props.spiker_sector_coordinates(layer_lo,lo_x,lo_y,lo_c);
//
//        //netlab::sector_coordinate_type const  min_x =
//
//        vector3 const  min_shift = 0.5f * spiker_layer_props.size_of_ship_movement_area_in_meters(area_layer_index);
//
//        for (int  i = 0; i < 10; ++i)
//        {
//            netlab::sector_coordinate_type const  spiker_x =
//                get_random_natural_32_bit_in_range(0U, spiker_layer_props.num_spikers_along_x_axis() - 1U, position_generator);
//            netlab::sector_coordinate_type const  spiker_y =
//                get_random_natural_32_bit_in_range(0U, spiker_layer_props.num_spikers_along_y_axis() - 1U, position_generator);
//            netlab::sector_coordinate_type const  spiker_c =
//                get_random_natural_32_bit_in_range(0U, spiker_layer_props.num_spikers_along_c_axis() - 1U, position_generator);
//
//            area_center = spiker_layer_props.spiker_sector_centre(spiker_x, spiker_y, spiker_c);
//
//            //area_center = vector3(lo(0) < hi(0) ? get_random_float_32_bit_in_range(lo(0), hi(0), position_generator) : 0.0f,
//            //                      lo(1) < hi(1) ? get_random_float_32_bit_in_range(lo(1), hi(1), position_generator) : 0.0f,
//            //                      lo(2) < hi(2) ? get_random_float_32_bit_in_range(lo(2), hi(2), position_generator) : 0.0f);
//
//            vector3 const  delta = area_center - spiker_position;
//            if (std::abs(delta(0)) >= min_shift(0) && std::abs(delta(1)) >= min_shift(1) && std::abs(delta(2)) >= min_shift(2))
//                break;
//        }
//    }
//    else
//    {
//        float_32_bit const coef_x =
//                (spiker_position(0) - spiker_layer_props.low_corner_of_ships()(0)) /
//                    (spiker_layer_props.high_corner_of_ships()(0) - spiker_layer_props.low_corner_of_ships()(0));
//        float_32_bit const coef_y =
//                (spiker_position(1) - spiker_layer_props.low_corner_of_ships()(1)) /
//                    (spiker_layer_props.high_corner_of_ships()(1) - spiker_layer_props.low_corner_of_ships()(1));
//
//        netlab::network_layer_props const&  area_layer_props = props.layer_props().at(area_layer_index);
//
//        float_32_bit const  projected_spiker_position_x =
//                area_layer_props.low_corner_of_ships()(0) + coef_x *
//                    (area_layer_props.high_corner_of_ships()(0) - area_layer_props.low_corner_of_ships()(0));
//        float_32_bit const  projected_spiker_position_y =
//                area_layer_props.low_corner_of_ships()(1) + coef_y *
//                    (area_layer_props.high_corner_of_ships()(1) - area_layer_props.low_corner_of_ships()(1));
//
//        float_32_bit const  low_center_x =
//                std::max(area_layer_props.low_corner_of_ships()(0)
//                            + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index),
//                         projected_spiker_position_x - max_distance_x.at(area_layer_index)
//                         );
//        float_32_bit const  high_center_x =
//                std::min(area_layer_props.high_corner_of_ships()(0)
//                            - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index),
//                         projected_spiker_position_x + max_distance_x.at(area_layer_index)
//                         );
//
//        float_32_bit const  low_center_y =
//                std::max(area_layer_props.low_corner_of_ships()(1)
//                            + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index),
//                         projected_spiker_position_y - max_distance_y.at(area_layer_index)
//                         );
//        float_32_bit const  high_center_y =
//                std::min(area_layer_props.high_corner_of_ships()(1)
//                            - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index),
//                         projected_spiker_position_y + max_distance_y.at(area_layer_index)
//                         );
//
//        float_32_bit const  low_center_c =
//                area_layer_props.low_corner_of_ships()(2)
//                    + 0.5f * spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index);
//
//        float_32_bit const  high_center_c =
//                area_layer_props.high_corner_of_ships()(2)
//                    - 0.5f * spiker_layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index);
//
//        area_center(0) = get_random_float_32_bit_in_range(low_center_x,high_center_x,position_generator);
//        area_center(1) = get_random_float_32_bit_in_range(low_center_y,high_center_y,position_generator);
//        area_center(2) = get_random_float_32_bit_in_range(low_center_c,high_center_c,position_generator);
//    }
//}


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
