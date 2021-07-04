#ifndef AI_SIMULATOR_HPP_INCLUDED
#   define AI_SIMULATOR_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <ai/agent_config.hpp>
#   include <ai/agent.hpp>
#   include <ai/navigation.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>
#   include <utility/dynamic_array.hpp>

namespace ai {


struct simulator
{
    simulator();

    agent_id  insert_agent(
            agent_config const  config,
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding
            );
    void  erase_agent(agent_id const  id);

    agent const&  get_agent(agent_id const  id) const;

    void  initialise_navsystem(simulation_context_const_ptr const  context_) { ensure_navsystem_exists(context_); }
    navsystem_const_ptr  get_navsystem() const { return m_navsystem; }
    naveditor_ptr  get_naveditor() { return m_naveditor; }

    void  clear(bool const  also_navsystem = true);

    void  next_round(float_32_bit const  time_step_in_seconds, cortex::mock_input_props const* const  mock_input_ptr);

private:
    void  ensure_navsystem_exists(simulation_context_const_ptr const  context_);

    dynamic_array<agent_ptr, agent_id>  m_agents;
    navsystem_ptr  m_navsystem;
    naveditor_ptr  m_naveditor;
};


}

#endif
