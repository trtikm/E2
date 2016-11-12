#ifndef NETEXP_ALGORITHM_HPP_INCLUDED
#   define NETEXP_ALGORITHM_HPP_INCLUDED

#   include <netlab/network.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>

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

void  compute_random_vector_of_magnitude(
        float_32_bit const  magnitude,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  resulting_vector
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
