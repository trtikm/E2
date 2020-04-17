#ifndef AI_AGENTS_HPP_INCLUDED
#   define AI_AGENTS_HPP_INCLUDED

#   include <ai/agent.hpp>
#   include <ai/agent_id.hpp>
#   include <ai/input_devices.hpp>
#   include <ai/sensor.hpp>
#   include <ai/sensor_id.hpp>
#   include <ai/scene.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/blackboard_agent.hpp>
#   include <ai/cortex.hpp>
#   include <ai/sensory_controller.hpp>
#   include <ai/action_controller.hpp>
#   include <ai/sensor_action.hpp>
#   include <ai/retina.hpp>
#   include <memory>
#   include <vector>

namespace ai {


struct  simulator;


struct  agents
{
    explicit agents(simulator* const  simulator_, scene_ptr const  scene_);

    agent_id  insert(
            scene::record_id const&  agent_rid,
            skeletal_motion_templates const  motion_templates,
            AGENT_KIND const  agent_kind,
            from_sensor_record_to_sensor_action_map const&  sensor_actions,
            retina_ptr const  retina_or_null
            );
    void  erase(agent_id const  id) { m_agents.at(id) = nullptr; }

    void  clear() { m_agents.clear(); }

    natural_32_bit  size() const { return (natural_32_bit)m_agents.size(); }

    bool  valid(agent_id const  id) const { return id < m_agents.size() && m_agents.at(id).operator bool(); }
    bool  ready(agent_id const  id) const { return valid(id) && m_agents.at(id)->agent_ptr.operator bool(); }
    agent&  at(agent_id const  id) { return *m_agents.at(id)->agent_ptr; }
    agent const&  at(agent_id const  id) const { return *m_agents.at(id)->agent_ptr; }

    scene_ptr  get_scene_ptr() const { return m_scene; }

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            input_devices::keyboard_props const&  keyboard,
            input_devices::mouse_props const&  mouse,
            input_devices::window_props const&  window
            );

    void  on_collision_contact(agent_id const  id, scene::collicion_contant_info_ptr const  contact_info, object_id const&  other_id);

    void  on_sensor_event(agent_id const  id, sensor const& s, sensor::other_object_info const&  other);

private:

    struct  agent_props
    {
        std::unique_ptr<agent>  agent_ptr;
        scene::record_id  agent_rid;
        skeletal_motion_templates  motion_templates;
        AGENT_KIND  agent_kind;
        std::shared_ptr<from_sensor_record_to_sensor_action_map>  m_sensor_actions;
        retina_ptr  retina_ptr;
    };

    void  construct_agent(agent_id const  id, agent_props&  props);

    std::vector<std::shared_ptr<agent_props> >  m_agents; // Should be 'std::vector<std::unique_ptr<agent_props> >', but does not compile :-(
    simulator*  m_simulator;
    scene_ptr  m_scene;
    input_devices_ptr  m_input_devices;
};


}

#endif
