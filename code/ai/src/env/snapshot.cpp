#include <ai/env/snapshot.hpp>
#include <ai/cortex.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/sensory_controller_sight.hpp>
#include <ai/sensory_controller_ray_cast_sight.hpp>
#include <ai/action_controller.hpp>
#include <angeo/tensor_math.hpp>
#include <angeo/coordinate_system.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <vector>
#include <algorithm>

namespace ai { namespace env {


snapshot::snapshot(blackboard_weak_const_ptr const  blackboard_ptr)
{
    TMPROF_BLOCK();

    blackboard_const_ptr const  bb = blackboard_ptr.lock();
    ASSUMPTION(bb != nullptr);
    motion_desire_props const&  desire = bb->m_cortex->get_motion_desire_props();
    action_controller const&  actions = *bb->m_action_controller;
    detail::rigid_body_motion const&  motion = actions.get_motion_object_motion();
    ASSUMPTION(bb->m_sensory_controller->get_sight() != nullptr);
    sensory_controller_ray_cast_sight_const_ptr const  sight_ptr = as<sensory_controller_ray_cast_sight const>(bb->m_sensory_controller->get_sight());
    ASSUMPTION(sight_ptr != nullptr);
    angeo::coordinate_system const&  camera_frame = *sight_ptr->get_camera()->coordinate_system();
    matrix44  to_agent_space_matrix;
    angeo::to_base_matrix(motion.frame, to_agent_space_matrix);

    desired_forward_unit_vector = transform_vector(desire.forward_unit_vector_in_local_space, to_agent_space_matrix);
    desired_linear_velocity_unit_direction = transform_vector(desire.linear_velocity_unit_direction_in_local_space, to_agent_space_matrix);
    desired_linear_speed = desire.linear_speed;
    desired_angular_velocity_unit_axis = transform_vector(desire.angular_velocity_unit_axis_in_local_space, to_agent_space_matrix);
    desired_angular_speed = desire.angular_speed;

    forward = transform_vector(motion.forward, to_agent_space_matrix);
    up = transform_vector(motion.up, to_agent_space_matrix);

    linear_velocity = transform_vector(motion.velocity.m_linear, to_agent_space_matrix);
    angular_velocity = transform_vector(motion.velocity.m_angular, to_agent_space_matrix);
    linear_acceleration = transform_vector(motion.acceleration.m_linear, to_agent_space_matrix);
    angular_acceleration = transform_vector(motion.acceleration.m_angular, to_agent_space_matrix);

    gravity = transform_vector(actions.get_gravity_acceleration(), to_agent_space_matrix);

    ideal_linear_velocity = transform_vector(actions.get_ideal_linear_velocity_in_world_space(), to_agent_space_matrix);
    ideal_angular_velocity = transform_vector(actions.get_ideal_angular_velocity_in_world_space(), to_agent_space_matrix);

    interpolation_param_till_destination_cursor =
            actions.get_consumed_time_till_destination_cursor_in_seconds() / actions.get_total_time_till_destination_cursor_in_seconds();
    destination_cursor = actions.get_destination_cursor();

    camera_origin = transform_point(camera_frame.origin(), to_agent_space_matrix);
    camera_x_axis = transform_vector(angeo::axis_x(camera_frame), to_agent_space_matrix);
    camera_y_axis = transform_vector(angeo::axis_y(camera_frame), to_agent_space_matrix);
    camera_z_axis = transform_vector(angeo::axis_z(camera_frame), to_agent_space_matrix);

    for (auto const& time_and_ray_cast_info : sight_ptr->get_ray_casts_in_time())
    {
        auto const&  info = time_and_ray_cast_info.second;
        ray_cast_targets.push_back(
                transform_point(info.ray_origin_in_world_space + info.parameter_to_coid * info.ray_unit_direction_in_world_space,
                                to_agent_space_matrix)
                );
    }
    auto const  begin_and_end = bb->m_collision_contacts.equal_range(motion.nid);
    for (auto it = begin_and_end.first; it != begin_and_end.second; ++it)
        contact_points.push_back(transform_point(it->second.contact_point, to_agent_space_matrix));
}


snapshot_const_ptr  create_snapshot(blackboard_weak_const_ptr const  blackboard_ptr)
{
    return std::make_shared<snapshot const>(blackboard_ptr);
}


}}
