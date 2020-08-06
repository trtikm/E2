#ifndef AI_CORTEX_ROBOT_HPP_INCLUDED
#   define AI_CORTEX_ROBOT_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai {


struct  cortex_robot : public cortex
{
    cortex_robot();
    void  next_round(float_32_bit const  time_step_in_seconds) override;

private:
};


}

#endif
