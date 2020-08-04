#include <aiold/env/snapshot.hpp>
#include <aiold/cortex.hpp>
#include <aiold/sensory_controller.hpp>
#include <aiold/sensory_controller_sight.hpp>
#include <aiold/sensory_controller_ray_cast_sight.hpp>
#include <aiold/action_controller.hpp>
#include <angeo/tensor_math.hpp>
#include <angeo/coordinate_system.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <vector>
#include <algorithm>

namespace aiold { namespace env {


snapshot::snapshot(blackboard_agent_weak_const_ptr const  blackboard_ptr)
{
    TMPROF_BLOCK();

    blackboard_agent_const_ptr const  bb = blackboard_ptr.lock();
    ASSUMPTION(bb != nullptr);
    action_controller const&  actions = *bb->m_action_controller;
    ASSUMPTION(bb->m_sensory_controller->get_sight() != nullptr);
    sensory_controller_ray_cast_sight_const_ptr const  sight_ptr = as<sensory_controller_ray_cast_sight const>(bb->m_sensory_controller->get_sight());
    ASSUMPTION(sight_ptr != nullptr);
    angeo::coordinate_system const&  camera_frame = *sight_ptr->get_camera()->coordinate_system();
    matrix44  to_agent_space_matrix;
    angeo::to_base_matrix(actions.get_agent_frame(), to_agent_space_matrix);

    desire_computed_by_cortex = bb->m_cortex->get_motion_desire_props();

    camera_origin = transform_point(camera_frame.origin(), to_agent_space_matrix);
    camera_x_axis = transform_vector(angeo::axis_x(camera_frame), to_agent_space_matrix);
    camera_y_axis = transform_vector(angeo::axis_y(camera_frame), to_agent_space_matrix);
    camera_z_axis = transform_vector(angeo::axis_z(camera_frame), to_agent_space_matrix);

    auto  begin_and_end = bb->m_sensory_controller->get_collision_contacts()->get_collision_contacts_map().equal_range(actions.get_roller_nid());
    for (auto it = begin_and_end.first; it != begin_and_end.second; ++it)
        collision_contact_events.push_back({ it->second.cell_x, it->second.cell_y, it->second.contact_point_in_local_space });

    begin_and_end = bb->m_sensory_controller->get_collision_contacts()->get_collision_contacts_map().equal_range(actions.get_body_nid());
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
