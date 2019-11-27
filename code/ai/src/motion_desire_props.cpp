#include <ai/motion_desire_props.hpp>

namespace ai {


motion_desire_props::motion_desire_props()
    : forward_unit_vector_in_local_space(vector3_unit_y())
    , linear_velocity_unit_direction_in_local_space(vector3_unit_y())
    , linear_speed(0.0f)
    , angular_velocity_unit_axis_in_local_space(vector3_unit_z())
    , angular_speed(0.0f)
    , look_at_target_in_local_space(vector3_unit_y())
{}


}
