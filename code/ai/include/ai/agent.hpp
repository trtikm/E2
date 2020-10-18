#ifndef AI_AGENT_HPP_INCLUDED
#   define AI_AGENT_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <ai/agent_config.hpp>
#   include <ai/agent_state_variables.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <ai/action_controller.hpp>
#   include <ai/sight_controller.hpp>
#   include <ai/cortex.hpp>
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
        scene_binding_ptr const  binding
        );

    agent_state_variables const&  get_state_variables() const { return *m_state_variables; }
    agent_state_variables&  state_variables_ref() { return *m_state_variables; }

    skeletal_motion_templates  get_motion_templates() const { return m_motion_templates; }
    scene_binding_ptr  get_binding() const { return m_binding; }

    action_controller const&  get_action_controller() const { return m_action_controller; }
    sight_controller const&  get_sight_controller() const { return m_sight_controller; }

    cortex const&  get_cortex() const { return *m_cortex; }

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            osi::keyboard_props const&  keyboard,
            osi::mouse_props const&  mouse,
            osi::window_props const&  window
            );

private:
    agent_state_variables_ptr  m_state_variables;
    skeletal_motion_templates  m_motion_templates;
    scene_binding_ptr  m_binding;

    action_controller  m_action_controller;
    sight_controller  m_sight_controller;

    std::shared_ptr<cortex>  m_cortex;
};


using  agent_ptr = std::shared_ptr<agent>;
using  agent_const_ptr = std::shared_ptr<agent const>;


}

#endif
