#ifndef AI_AGENTS_HPP_INCLUDED
#   define AI_AGENTS_HPP_INCLUDED

#   include <ai/agent.hpp>
#   include <ai/agent_id.hpp>
#   include <ai/input_devices.hpp>
#   include <ai/scene.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/blackboard.hpp>
#   include <ai/cortex.hpp>
#   include <ai/sensory_controller.hpp>
#   include <ai/action_controller.hpp>
#   include <ai/retina.hpp>
#   include <memory>
#   include <vector>
#   include <functional>

namespace ai {


struct agents
{
    explicit agents(scene_ptr const  scene_);

    agent_id  insert(
            scene::node_id const&  agent_nid,
            skeletal_motion_templates const  motion_templates,
            AGENT_KIND const  agent_kind,
            retina_ptr const  retina_or_null
            );
    void  erase(agent_id const  id) { m_agents.at(id) = nullptr; }

    void  clear() { m_agents.clear(); }

    bool  ready(agent_id const  id) { return m_agents.at(id)->agent_ptr.operator bool(); }
    agent&  at(agent_id const  id) { return *m_agents.at(id)->agent_ptr; }
    agent const&  at(agent_id const  id) const { return *m_agents.at(id)->agent_ptr; }

    scene_ptr  get_scene_ptr() const { return m_scene; }

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            input_devices::keyboard_props const&  keyboard,
            input_devices::mouse_props const&  mouse,
            input_devices::window_props const&  window
            );

    void  on_collision_contact(
            agent_id const  agent_id,
            scene::node_id const&  collider_nid,
            scene::collicion_contant_info const&  contact_info
            );

private:

    struct  agent_props
    {
        std::unique_ptr<agent>  agent_ptr;
        scene::node_id  agent_nid;
        skeletal_motion_templates  motion_templates;
        AGENT_KIND  agent_kind;
        retina_ptr  retina_ptr;
    };

    void  construct_agent(agent_id const  id, agent_props&  props);

    std::vector<std::shared_ptr<agent_props> >  m_agents; // Should be 'std::vector<std::unique_ptr<agent_props> >', but does not compile :-(
    scene_ptr  m_scene;
    input_devices_ptr  m_input_devices;
    std::vector<std::unique_ptr<std::function<void()> > >  m_scene_update_events;
};


}

#endif
