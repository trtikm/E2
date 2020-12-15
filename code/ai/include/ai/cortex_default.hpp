#ifndef AI_CORTEX_DEFAULT_HPP_INCLUDED
#   define AI_CORTEX_DEFAULT_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai {


struct  cortex_default : public cortex
{
    cortex_default(agent const*  myself_);
    void  next_round(float_32_bit const  time_step_in_seconds) override;

private:
};


}

#endif
