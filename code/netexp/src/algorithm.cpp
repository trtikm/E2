#include <netexp/algorithm.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace netexp {


void  compute_random_ship_position_and_velocity_in_movement_area(
        vector3 const&  area_center,
        float_32_bit const  size_of_ship_movement_area_along_x_axis_in_meters,
        float_32_bit const  size_of_ship_movement_area_along_y_axis_in_meters,
        float_32_bit const  size_of_ship_movement_area_along_c_axis_in_meters,
        netlab::ship&  ship_reference,
        random_generator_for_natural_32_bit&   position_generator,
        random_generator_for_natural_32_bit&   velocity_generator
        )
{

}


}
