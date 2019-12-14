#include <ai/agents.hpp>
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
        skeletal_motion_templates const  motion_templates,
        AGENT_KIND const  agent_kind,
        retina_ptr const  retina_or_null
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(agent_nid.valid() && !motion_templates.empty());

    agent_id  id = 0U;
    for (; id != m_agents.size(); ++id)
        if (m_agents.at(id) == nullptr)
            break;

    auto const  props = std::make_shared<agent_props>();
    props->agent_ptr = nullptr;
    props->agent_nid = agent_nid;
    props->motion_templates = motion_templates;
    props->agent_kind = agent_kind;
    props->retina_ptr = retina_or_null;

    if (id == m_agents.size())
        m_agents.resize(m_agents.size() + 1U, nullptr);
    m_agents.at(id) = props;

    return id;
}


void  agents::construct_agent(agent_id const  id, agent_props&  props)
{
    TMPROF_BLOCK();

    blackboard_ptr const  bb = agent::create_blackboard(props.agent_kind);
    {
        bb->m_motion_templates = props.motion_templates;
        bb->m_agent_id = id;
        bb->m_agent_kind = props.agent_kind;
        bb->m_retina_ptr = props.retina_ptr;
        bb->m_scene = m_scene;
        bb->m_agent_nid = props.agent_nid;
        bb->m_bone_nids.resize(props.motion_templates.pose_frames().size());
        for (natural_32_bit bone = 0U; bone != bb->m_bone_nids.size(); ++bone)
        {
            scene::node_id::path_type  path;
            for (integer_32_bit parent_bone = (integer_32_bit)bone;
                    parent_bone >= 0;
                    parent_bone = bb->m_motion_templates.hierarchy().parents().at(parent_bone))
                path.push_back(bb->m_motion_templates.names().at(parent_bone));
            std::reverse(path.begin(), path.end());
            bb->m_bone_nids.at(bone) = props.agent_nid / scene::node_id(path);
        }
        agent::create_modules(bb, m_input_devices);
    }
    props.agent_ptr = std::make_unique<agent>(bb);
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

    for (natural_32_bit  id = 0U; id != m_agents.size(); ++id)
    {
        auto const  props = m_agents.at(id);
        if (props != nullptr)
            if (props->agent_ptr != nullptr)
                props->agent_ptr->next_round(time_step_in_seconds);
            else if (props->motion_templates.loaded_successfully())
                construct_agent(id, *props);
    }
}


void  agents::on_collision_contact(
        agent_id const  agent_id,
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info const&  contact_info
        )
{
    m_agents.at(agent_id)->agent_ptr->get_blackboard()->m_sensory_controller->get_collision_contacts()->on_collision_contact(
            collider_nid,
            contact_info
            );
}


}
