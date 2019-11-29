#ifndef AI_ENV_SNAPSHOT_HPP_INCLUDED
#   define AI_ENV_SNAPSHOT_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <angeo/tensor_math.hpp>
#   include <memory>

namespace ai { namespace env {


struct  snapshot
{
    explicit snapshot(blackboard_weak_const_ptr const  blackboard_ptr);

    // All vectors are in agent's local frame!

    vector3  desired_forward_unit_vector;
    vector3  desired_linear_velocity_unit_direction;
    float_32_bit  desired_linear_speed;
    vector3  desired_angular_velocity_unit_axis;
    float_32_bit  desired_angular_speed;

    vector3  forward;
    vector3  up;

    vector3  linear_velocity;
    vector3  angular_velocity;
    vector3  linear_acceleration;
    vector3  angular_acceleration;

    vector3  gravity;

    vector3  ideal_linear_velocity;
    vector3  ideal_angular_velocity;

    float_32_bit  interpolation_param_till_destination_cursor;
    skeletal_motion_templates::motion_template_cursor  destination_cursor;

    vector3  camera_origin;
    vector3  camera_x_axis;
    vector3  camera_y_axis;
    vector3  camera_z_axis;

    struct  ray_cast_event
    {
        natural_32_bit  cell_x;
        natural_32_bit  cell_y;
        vector3  target_in_local_space;
    };

    std::vector<ray_cast_event>  ray_cast_targets;  // Order is given by the order of performed raycasts.
    std::vector<vector3>  contact_points;           // Order does NOT matter.
};


using  snapshot_const_ptr = std::shared_ptr<snapshot const>;


snapshot_const_ptr  create_snapshot(blackboard_weak_const_ptr const  blackboard_ptr);


}}

#endif
