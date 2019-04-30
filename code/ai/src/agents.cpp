#include <ai/agents.hpp>
#include <ai/blackboard_human.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {


agents::agents(scene_ptr const  scene_)
    : m_agents()
    , m_scene(scene_)
    , m_input_devices(std::make_shared<input_devices>())
    , m_scene_update_events()
{
    ASSUMPTION(m_scene != nullptr);
}


agent_id  agents::insert(
        scene::node_id const&  agent_nid,
        skeletal_motion_templates::motion_template_cursor const&  start_pose,
        skeleton_composition_const_ptr const  skeleton,
        skeletal_motion_templates_const_ptr const  motion_templates
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(skeleton != nullptr);
    ASSUMPTION(
        !skeleton->pose_frames.empty() &&
        skeleton->pose_frames.size() == skeleton->names.size() &&
        skeleton->pose_frames.size() == skeleton->parents.size() &&
        (skeleton->children.empty() || skeleton->pose_frames.size() == skeleton->children.size())
        );
    ASSUMPTION(skeleton->parents.at(0U) == -1);
    ASSUMPTION(motion_templates != nullptr);

    if (skeleton->children.empty())
        angeo::skeleton_compute_child_bones(skeleton->parents, std::const_pointer_cast<skeleton_composition>(skeleton)->children);

    blackboard_ptr const  bb = std::make_shared<blackboard_human>();
    bb->m_skeleton_composition = skeleton;
    bb->m_motion_templates = motion_templates;
    bb->m_agent_id = 0U; // Is computed below.
    bb->m_scene = m_scene;
    bb->m_agent_nid = agent_nid;
    bb->m_bone_nids.resize(skeleton->pose_frames.size());
    for (natural_32_bit  bone = 0U; bone != bb->m_bone_nids.size(); ++bone)
    {
        scene::node_id::path_type  path;
        for (integer_32_bit parent_bone = (integer_32_bit)bone;
                parent_bone >= 0;
                parent_bone = bb->m_skeleton_composition->parents.at(parent_bone))
            path.push_back(bb->m_skeleton_composition->names.at(parent_bone));
        std::reverse(path.begin(), path.end());
        bb->m_bone_nids.at(bone) = agent_nid / scene::node_id(path);
    }

    for (; bb->m_agent_id != m_agents.size(); ++bb->m_agent_id)
        if (m_agents.at(bb->m_agent_id) == nullptr)
            break;

    auto  agent_ptr = std::make_shared<agent>(bb, m_input_devices, start_pose);

    if (bb->m_agent_id == m_agents.size())
        m_agents.resize(m_agents.size() + 1U, nullptr);
    m_agents.at(bb->m_agent_id) = agent_ptr;
    
    return bb->m_agent_id;
}


void  agents::next_round(
        float_32_bit const  time_step_in_seconds,
        input_devices::keyboard_props const&  keyboard,
        input_devices::mouse_props const&  mouse,
        input_devices::window_props const&  window
        )
{
    TMPROF_BLOCK();

    for (auto const&  event_handler_ptr : m_scene_update_events)
        (*event_handler_ptr)();
    m_scene_update_events.clear();

    m_input_devices->keyboard = keyboard;
    m_input_devices->mouse = mouse;
    m_input_devices->window = window;

    for (auto&  agent_ptr : m_agents)
        if (agent_ptr != nullptr)
        {
            agent_ptr->next_round(time_step_in_seconds);
            agent_ptr->get_blackboard()->m_collision_contacts.clear();
        }
}


void  agents::on_collision_contact(
        agent_id const  agent_id,
        scene::node_id const&  collider_nid,
        vector3 const&  contact_point,
        vector3 const&  unit_normal
        )
{
    m_agents.at(agent_id)->get_blackboard()->m_collision_contacts.insert({collider_nid, {contact_point, unit_normal}});
}


}
