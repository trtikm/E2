#ifndef AI_AGENT_HPP_INCLUDED
#   define AI_AGENT_HPP_INCLUDED

#   include <ai/blackboard_agent.hpp>
#   include <ai/input_devices.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai {


struct agent
{
    static blackboard_agent_ptr  create_blackboard(AGENT_KIND const  agent_kind);
    static void  create_modules(blackboard_agent_ptr const  bb, input_devices_ptr const  idev);

    explicit agent(blackboard_agent_ptr const  blackboard_);
    ~agent();

    void  next_round(float_32_bit const  time_step_in_seconds);

    blackboard_agent_ptr  get_blackboard() { return m_blackboard; }
    blackboard_agent_const_ptr  get_blackboard() const { return m_blackboard; }

    AGENT_KIND  get_kind() const { return get_blackboard()->m_agent_kind; }

    // Access to agent's modules:

    cortex const&  get_cortex() const { return *get_blackboard()->m_cortex; }
    sensory_controller const&  get_sensory_controller() const { return *get_blackboard()->m_sensory_controller; }
    action_controller const&  get_action_controller() const { return *get_blackboard()->m_action_controller; }

    // Move the following somewhere else:
    retina const* get_retina() const { return get_blackboard()->m_retina_ptr == nullptr ? nullptr : get_blackboard()->m_retina_ptr.get(); }

private:
    blackboard_agent_ptr  m_blackboard;
};


}

#endif
