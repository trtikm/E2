#include <ai/detail/motion_desire_props.hpp>

namespace ai { namespace detail {


motion_desire_props::motion_desire_props()
    : forward_unit_vector_in_world_space(vector3_unit_x())
    , linear_velocity_unit_direction_in_world_space(vector3_unit_x())
    , linear_speed(0.0f)
    , angular_velocity_unit_axis_in_world_space(vector3_unit_z())
    , angular_speed(0.0f)
{}


}}
