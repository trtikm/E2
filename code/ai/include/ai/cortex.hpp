#ifndef AI_CORTEX_HPP_INCLUDED
#   define AI_CORTEX_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/detail/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <string>
#   include <unordered_map>

namespace ai {


struct  cortex
{
    explicit cortex(blackboard_weak_ptr const  blackboard_);
    virtual ~cortex() {}

    blackboard_ptr  get_blackboard() const { return m_blackboard.lock(); }

    // For initialisation steps which cannot be performed/completed in a constructor.
    virtual void  initialise();

    // Any overriden method is required to update the following fields in each call:
    //      cortex::m_motion_desire_props
    //      cortex::m_ranks_of_motion_actions
    //      cortex::m_look_at_target_in_local_space
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    // Services for the action controller:

    detail::motion_desire_props const&  get_motion_desire_props() const { return m_motion_desire_props; }
    vector3 const&  get_look_at_target_in_local_space() const { return m_look_at_target_in_local_space; }
    virtual skeletal_motion_templates::cursor_and_transition_time  choose_next_motion_action(
                std::vector<skeletal_motion_templates::cursor_and_transition_time> const&  possibilities
                ) const;

protected:
    void  set_stationary_desire();
    void  set_stationary_ranks_of_motion_actions();
    void  set_indifferent_look_at_target();

    // The three fileds below have to be updated in each call to the method 'next_round', including any of its overrides:
    detail::motion_desire_props  m_motion_desire_props;
    std::unordered_map<std::string, float_32_bit>  m_ranks_of_motion_actions;
    vector3  m_look_at_target_in_local_space;

private:
    blackboard_weak_ptr  m_blackboard;
};


}

#endif
