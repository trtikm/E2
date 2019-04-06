#ifndef AI_AGENTS_HPP_INCLUDED
#   define AI_AGENTS_HPP_INCLUDED

#   include <ai/agent.hpp>
#   include <ai/agent_id.hpp>
#   include <ai/input_devices.hpp>
#   include <ai/environment_models.hpp>
#   include <ai/skeleton_composition.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/action_controller.hpp>
#   include <memory>
#   include <vector>
#   include <functional>

namespace ai {


struct agents
{
    agents(environment_models::collision_scene_ptr const  collisions,
           environment_models::rigid_body_simulator_ptr const  physics,
           environment_models::create_scene_object_callback const&  create_scene_object_handler,
           environment_models::destroy_scene_object_callback const&  destroy_scene_object_handler
           );

    agent_id  insert(
            skeleton_composition_const_ptr const  skeleton,
            skeletal_motion_templates_const_ptr const  motion_templates
            );
    void  erase(agent_id const  id) { m_agents.at(id) = nullptr; }
    
    agent&  at(agent_id const  id) { return *m_agents.at(id); }
    agent const&  at(agent_id const  id) const { return *m_agents.at(id); }

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            input_devices::keyboard_props const&  keyboard,
            input_devices::mouse_props const&  mouse,
            input_devices::window_props const&  window
            );

private:
    std::vector<std::shared_ptr<agent> >  m_agents; // Should be 'std::vector<std::unique_ptr<agent> >', but does not compile :-(
    environment_models_ptr  m_environment_models;
    input_devices_ptr  m_input_devices;
    std::vector<std::unique_ptr<std::function<void()> > >  m_scene_update_events;
    environment_models::create_scene_object_callback   m_create_scene_object_handler;
    environment_models::destroy_scene_object_callback  m_destroy_scene_object_handler;
};


}

#endif
