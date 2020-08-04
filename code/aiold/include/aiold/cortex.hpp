#ifndef AIOLD_CORTEX_HPP_INCLUDED
#   define AIOLD_CORTEX_HPP_INCLUDED

#   include <aiold/blackboard_agent.hpp>
#   include <aiold/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>

namespace aiold {


struct  cortex
{
    explicit cortex(blackboard_agent_weak_ptr const  blackboard_);
    virtual ~cortex() {}

    blackboard_agent_ptr  get_blackboard() const { return m_blackboard.lock(); }
    motion_desire_props const&  get_motion_desire_props() const { return m_motion_desire_props; }

    // For initialisation steps which cannot be performed/completed in a constructor.
    virtual void  initialise();

    // Any overriden method is required to update the following field in each call:
    //      cortex::m_motion_desire_props
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

protected:
    // This filed have to be updated in each call to the method 'next_round', including any of its overrides:
    motion_desire_props  m_motion_desire_props;

private:
    blackboard_agent_weak_ptr  m_blackboard;
};


void  set_stationary_desire(motion_desire_props&  desire_props);


}

#endif
