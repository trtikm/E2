#ifndef AI_DETAIL_MOTION_DESIRE_PROPS_HPP_INCLUDED
#   define AI_DETAIL_MOTION_DESIRE_PROPS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace ai { namespace detail {


struct  motion_desire_props
{
    motion_desire_props();

    vector3  forward_unit_vector_in_world_space;
    vector3  linear_velocity_unit_direction_in_world_space;
    float_32_bit  linear_speed;
    vector3  angular_velocity_unit_axis_in_world_space;
    float_32_bit  angular_speed;
};


}}

#endif
