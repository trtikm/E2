#include <ai/agents.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {


agents::agents(simulator* const  simulator_, scene_ptr const  scene_)
    : m_agents()
    , m_simulator(simulator_)
    , m_scene(scene_)
    , m_input_devices(std::make_shared<input_devices>())
{
    ASSUMPTION(m_simulator != nullptr && m_scene != nullptr);
}


agent_id  agents::insert(
        scene::node_id const&  agent_nid,
        skeletal_motion_templates const  motion_templates,
        AGENT_KIND const  agent_kind,
        from_sensor_event_to_sensor_action_map const&  sensor_actions,
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
    props->m_sensor_actions = std::make_shared<from_sensor_event_to_sensor_action_map>(sensor_actions);
    props->retina_ptr = retina_or_null;

    if (id == m_agents.size())
        m_agents.resize(m_agents.size() + 1U, nullptr);
    m_agents.at(id) = props;

    return id;
}


void  agents::construct_agent(agent_id const  id, agent_props&  props)
{
    TMPROF_BLOCK();

    blackboard_agent_ptr const  bb = agent::create_blackboard(props.agent_kind);
    {
        // General blackboard setup
        bb->m_self_id = agent_to_object_id(id);
        bb->m_motion_templates = props.motion_templates;
        bb->m_scene = m_scene;
        bb->m_self_nid = props.agent_nid;
        bb->initialise_bone_nids();
        bb->m_state = 0U;
        bb->m_sensor_actions = props.m_sensor_actions;
        bb->m_simulator_ptr = m_simulator;

        // Agent's blackboard setup
        bb->m_agent_kind = props.agent_kind;
        bb->m_retina_ptr = props.retina_ptr;
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
        agent_id const  id,
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info const&  contact_info,
        object_id const&  other_id,
        scene::node_id const&  other_collider_nid
        )
{
    m_agents.at(id)->agent_ptr->get_blackboard()->m_sensory_controller->get_collision_contacts()->on_collision_contact(
            collider_nid,
            contact_info
            );
}


void  agents::on_sensor_event(agent_id const  id, sensor const&  s)
{
    m_agents.at(id)->agent_ptr->on_sensor_event(s);
}


}
