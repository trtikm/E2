#ifndef NETEXP_ALGORITHM_HPP_INCLUDED
#   define NETEXP_ALGORITHM_HPP_INCLUDED

#   include <netlab/network.hpp>
#   include <utility/tensor_math.hpp>
#   include <utility/random.hpp>

namespace netexp {


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


}


#endif
