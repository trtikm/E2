#ifndef AI_CORTEX_ROBOT_HUMANOID_HPP_INCLUDED
#   define AI_CORTEX_ROBOT_HUMANOID_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <ai/blackboard.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/detail/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace ai {


struct  cortex_robot_humanoid : public cortex
{
    explicit cortex_robot_humanoid(blackboard_weak_ptr const  blackboard_);
    ~cortex_robot_humanoid();

    void  initialise() override;
    void  next_round(float_32_bit const  time_step_in_seconds) override;

    // Services for the action controller:

    vector3  get_look_at_target_in_world_space() const override;

    skeletal_motion_templates::cursor_and_transition_time  choose_next_motion_action(
            std::vector<skeletal_motion_templates::cursor_and_transition_time> const&  possibilities
            ) const override;
};


}

#endif