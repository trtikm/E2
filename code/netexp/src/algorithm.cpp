#include <netexp/algorithm.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace netexp {


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
