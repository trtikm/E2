#ifndef AI_CORTEX_HPP_INCLUDED
#   define AI_CORTEX_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/detail/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace ai {


struct  cortex
{
    explicit cortex(blackboard_weak_ptr const  blackboard_);
    virtual ~cortex() {}

    blackboard_ptr  get_blackboard() const { return m_blackboard.lock(); }

    virtual void  initialise();
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    // Services for the action controller:

    detail::motion_desire_props const&  get_motion_desire_props() const { return m_motion_desire_props; }

    virtual vector3  get_look_at_target_in_world_space() const;

    virtual skeletal_motion_templates::cursor_and_transition_time  choose_next_motion_action(
            std::vector<skeletal_motion_templates::cursor_and_transition_time> const&  possibilities
            ) const;

protected:
    detail::motion_desire_props  m_motion_desire_props;

private:
    blackboard_weak_ptr  m_blackboard;
};


}

#endif
