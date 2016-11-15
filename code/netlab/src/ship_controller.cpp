#include <netlab/ship_controller.hpp>
#include <netlab/network_props.hpp>
#include <netlab/network_layer_props.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


vector3  ship_controller::accelerate_into_space_box(
            vector3 const&  ship_position,
            vector3 const&  ship_velocity,
            vector3 const&  space_box_center,
            network_layer_props const&  layer_props,
            network_props const&  props
            ) const
{
    TMPROF_BLOCK();

    vector3 const  delta = 2.0f * (ship_position - space_box_center);
    float_32_bit const  two_times_max_speed = 2.0f * layer_props.max_speed_of_ship_in_meters_per_second();

    float_32_bit const  ax =
        delta(0) >  layer_props.size_of_ship_movement_area_along_x_axis_in_meters() ? -two_times_max_speed :
        delta(0) < -layer_props.size_of_ship_movement_area_along_x_axis_in_meters() ? two_times_max_speed :
        0.0f;

    float_32_bit const  ay =
        delta(1) >  layer_props.size_of_ship_movement_area_along_y_axis_in_meters() ? -two_times_max_speed :
        delta(1) < -layer_props.size_of_ship_movement_area_along_y_axis_in_meters() ? two_times_max_speed :
        0.0f;

    float_32_bit const  ac =
        delta(2) >  layer_props.size_of_ship_movement_area_along_c_axis_in_meters() ? -two_times_max_speed :
        delta(2) < -layer_props.size_of_ship_movement_area_along_c_axis_in_meters() ? two_times_max_speed :
        0.0f;

    return{ ax, ay, ac };
}


}
