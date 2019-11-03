#ifndef AI_AGENT_HPP_INCLUDED
#   define AI_AGENT_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai {


struct agent
{
    agent() {}

    agent(blackboard_ptr const  blackboard_);
    virtual  ~agent();

    virtual void  next_round(float_32_bit const  time_step_in_seconds);
    
    blackboard_ptr  get_blackboard() { return m_blackboard; }
    blackboard_const_ptr  get_blackboard() const { return m_blackboard; }

    void  set_use_cortex_mock(bool const  state);
    bool  uses_cortex_mock() const;

    action_controller const&  get_action_controller() const { return *get_blackboard()->m_action_controller; }
    sensory_controller const&  get_sensory_controller() const { return *get_blackboard()->m_sensory_controller; }

private:
    blackboard_ptr  m_blackboard;
};


}

#endif
