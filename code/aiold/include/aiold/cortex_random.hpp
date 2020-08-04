#ifndef AIOLD_CORTEX_RANDOM_HPP_INCLUDED
#   define AIOLD_CORTEX_RANDOM_HPP_INCLUDED

#   include <aiold/cortex.hpp>
#   include <aiold/blackboard_agent.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>

namespace aiold {


struct  cortex_random : public cortex
{
    explicit cortex_random(blackboard_agent_weak_ptr const  blackboard_);

    void  next_round(float_32_bit const  time_step_in_seconds);

private:
    float_32_bit  m_seconds_till_change;
    random_generator_for_natural_32_bit  m_generator;
};


}

#endif
