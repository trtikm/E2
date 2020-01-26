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


snapshot::snapshot(blackboard_agent_weak_const_ptr const  blackboard_ptr)
{
    TMPROF_BLOCK();

    blackboard_agent_const_ptr const  bb = blackboard_ptr.lock();
    ASSUMPTION(bb != nullptr);
    action_controller const&  actions = *bb->m_action_controller;
    detail::rigid_body_motion const&  motion = actions.get_motion_object_motion();
    ASSUMPTION(bb->m_sensory_controller->get_sight() != nullptr);
    sensory_controller_ray_cast_sight_const_ptr const  sight_ptr = as<sensory_controller_ray_cast_sight const>(bb->m_sensory_controller->get_sight());
    ASSUMPTION(sight_ptr != nullptr);
    angeo::coordinate_system const&  camera_frame = *sight_ptr->get_camera()->coordinate_system();
    matrix44  to_agent_space_matrix;
    angeo::to_base_matrix(motion.frame, to_agent_space_matrix);

    desire_computed_by_cortex = bb->m_cortex->get_motion_desire_props();
    regulated_desire = actions.get_regulated_motion_desire_props();

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

    auto const  begin_and_end = bb->m_sensory_controller->get_collision_contacts()->get_collision_contacts_map().equal_range(motion.nid);
    for (auto it = begin_and_end.first; it != begin_and_end.second; ++it)
        collision_contact_events.push_back({ it->second.cell_x, it->second.cell_y, it->second.contact_point_in_local_space });

    if (!sight_ptr->get_ray_casts_in_time().empty())
    {
        auto const  range = sight_ptr->get_ray_casts_in_time().equal_range(sight_ptr->get_ray_casts_in_time().crbegin()->first);
        for (auto  it = range.first; it != range.second; ++it)
        {
            auto const&  info = it->second;
            ray_cast_events.push_back({
                    info.cell_x,
                    info.cell_y,
                    transform_point(info.ray_origin_in_world_space + info.parameter_to_coid * info.ray_unit_direction_in_world_space,
                                    to_agent_space_matrix)
                    });
        }
    }
}


snapshot_const_ptr  create_snapshot(blackboard_agent_weak_const_ptr const  blackboard_ptr)
{
    return std::make_shared<snapshot const>(blackboard_ptr);
}


}}
