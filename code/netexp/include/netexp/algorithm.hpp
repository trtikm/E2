#ifndef NETEXP_ALGORITHM_HPP_INCLUDED
#   define NETEXP_ALGORITHM_HPP_INCLUDED

#   include <netlab/network.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>

namespace netexp {


netlab::layer_index_type  compute_layer_index_for_area_center(
        std::vector<natural_64_bit>&  counts_of_centers_into_layers,
        random_generator_for_natural_32_bit&  generator
        );


//void  compute_extreme_sector_coordinates_for_center_of_movement_area(
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
//        netlab::network_layer_props const&  layer_props,
//        vector3 const&  size_of_ship_movement_area_in_meters,
//        netlab::sector_coordinate_type const  max_distance_x,
//        netlab::sector_coordinate_type const  max_distance_y,
//        netlab::sector_coordinate_type const  max_distance_c,
//        netlab::sector_coordinate_type&  lo_x,
//        netlab::sector_coordinate_type&  lo_y,
//        netlab::sector_coordinate_type&  lo_c,
//        netlab::sector_coordinate_type&  hi_x,
//        netlab::sector_coordinate_type&  hi_y,
//        netlab::sector_coordinate_type&  hi_c
//        );
//
//void  ensure_whole_movement_area_is_inside_layer(
//        vector3 const&  low_corner_of_ships,
//        vector3 const&  high_corner_of_ships,
//        vector3 const&  size_of_ship_movement_area_in_meters,
//        vector3&  area_center
//        );
//
//
//void  compute_center_of_movement_area_for_ships_of_spiker(
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
//        netlab::network_layer_props const&  layer_props,
//        vector3 const&  size_of_ship_movement_area_in_meters,
//        netlab::sector_coordinate_type const  max_distance_x,
//        netlab::sector_coordinate_type const  max_distance_y,
//        netlab::sector_coordinate_type const  max_distance_c,
//        random_generator_for_natural_32_bit&  position_generator,
//        vector3&  area_center
//        );
//
//
//void  compute_center_of_movement_area_for_ships_of_spiker(
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
//        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
//        netlab::network_layer_props const&  spiker_layer_props,
//        netlab::network_layer_props const&  area_layer_props,
//        vector3 const&  size_of_ship_movement_area_in_meters,
//        netlab::sector_coordinate_type const  max_distance_x,
//        netlab::sector_coordinate_type const  max_distance_y,
//        netlab::sector_coordinate_type const  max_distance_c,
//        random_generator_for_natural_32_bit&  position_generator,
//        vector3&  area_center
//        );


vector3  compute_spiker_position_projected_to_area_layer(
        netlab::layer_index_type const  spiker_layer_index,
        netlab::layer_index_type const&  area_layer_index,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
        netlab::network_props const&  props
        );


void  compute_maximal_bbox_for_storing_movement_area_of_spiker_into(
        vector3 const&  spiker_position_projected_to_area_layer,
        netlab::layer_index_type const  area_layer_index,
        netlab::sector_coordinate_type const  max_distance_x,
        netlab::sector_coordinate_type const  max_distance_y,
        netlab::sector_coordinate_type const  max_distance_c,
        netlab::network_props const&  props,
        vector3&  low_corner,
        vector3&  high_corner
        );


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
        );


void  compute_random_ship_position_in_movement_area(
        vector3 const&  area_center,
        float_32_bit const  half_size_of_ship_movement_area_along_x_axis_in_meters,
        float_32_bit const  half_size_of_ship_movement_area_along_y_axis_in_meters,
        float_32_bit const  half_size_of_ship_movement_area_along_c_axis_in_meters,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  ship_position
        );


void  compute_random_ship_velocity_in_movement_area(
        float_32_bit const  min_speed_of_ship_in_movement_area,
        float_32_bit const  max_speed_of_ship_in_movement_area,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  ship_velocity
        );


/**
* It is a function F(x) such that
*      F(x) = x <= 0.0   ? 0.0   :
*             x >= X_MAX ? 1.0   :
*             POW > 1.0  ? y^POW :
*                          y     ;
*  where y = (e^(x / X_MAX) - 1.0) / (e - 1.0).
*/
float_32_bit  exponential_increase_from_zero_to_one(
    float_32_bit const  x,
    float_32_bit const  X_MAX = 1.0f,
    float_32_bit const  POW = 3.0f
    );


/**
 * It is a function F(x) such that
 *      F(x) = x <= 0.0   ? 1.0   :
 *             x >= X_MAX ? 0.0   :
 *             POW > 1.0  ? y^POW : 
 *                          y     ;
 *  where y = (e^(1.0 - x / X_MAX) - 1.0) / (e - 1.0).
 */
float_32_bit  exponential_decrease_from_one_to_zero(
    float_32_bit const  x,
    float_32_bit const  X_MAX = 1.0f,
    float_32_bit const  POW = 3.0f
    );


}


#endif
