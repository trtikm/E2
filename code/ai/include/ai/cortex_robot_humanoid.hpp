#ifndef AI_CORTEX_ROBOT_HUMANOID_HPP_INCLUDED
#   define AI_CORTEX_ROBOT_HUMANOID_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <ai/blackboard.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/motion_desire_props.hpp>
#   include <ai/env/snapshots_cache.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace ai {


struct  cortex_robot_humanoid : public cortex
{
    explicit cortex_robot_humanoid(blackboard_weak_ptr const  blackboard_);
    ~cortex_robot_humanoid();

    void  initialise() override;
    void  next_round(float_32_bit const  time_step_in_seconds) override;

private:
    env::snapshots_cache_ptr  m_snapshots_cache;
};


}

#endif
