#ifndef AI_AGENT_HPP_INCLUDED
#   define AI_AGENT_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <ai/agent_config.hpp>
#   include <ai/agent_system_state.hpp>
#   include <ai/agent_system_variables.hpp>
#   include <ai/agent_state_variables.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <ai/action_controller.hpp>
#   include <ai/sight_controller.hpp>
#   include <ai/cortex.hpp>
#   include <ai/navigation.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace com { struct  simulation_context; }

namespace ai {


struct agent
{
    agent(
        agent_config const  config,
        skeletal_motion_templates const  motion_templates,
        navsystem_const_ptr const  navsystem_,
        scene_binding_ptr const  binding
        );

    agent_state_variables const&  get_state_variables() const { return m_state_variables; }
    agent_state_variables&  state_variables_ref() { return m_state_variables; }

    agent_system_variables const&  get_system_variables() const { return m_system_variables; }
    agent_system_variables&  system_variables_ref() { return m_system_variables; }

    agent_system_state const&  get_system_state() const { return m_system_state; }
    agent_system_state&  system_state_ref() { return m_system_state; }

    skeletal_motion_templates  get_motion_templates() const { return m_motion_templates; }
    scene_binding_ptr  get_binding() const { return m_binding; }

    action_controller const&  get_action_controller() const { return m_action_controller; }
    sight_controller const&  get_sight_controller() const { return m_sight_controller; }

    cortex const&  get_cortex() const { return *m_cortex; }

    void  next_round(float_32_bit const  time_step_in_seconds, cortex::mock_input_props const* const  mock_input_ptr);

private:
    void  update_system_state();

    agent_system_state  m_system_state;
    agent_system_variables  m_system_variables;
    agent_state_variables  m_state_variables;

    skeletal_motion_templates  m_motion_templates;
    scene_binding_ptr  m_binding;

    action_controller  m_action_controller;
    sight_controller  m_sight_controller;

    std::shared_ptr<cortex>  m_cortex;

    navsystem_const_ptr  m_navsystem;
};


using  agent_ptr = std::shared_ptr<agent>;
using  agent_const_ptr = std::shared_ptr<agent const>;


}

#endif
