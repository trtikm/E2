#ifndef AI_CORTEX_RANDOM_HPP_INCLUDED
#   define AI_CORTEX_RANDOM_HPP_INCLUDED

#   include <ai/cortex_mock.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>

namespace ai {


struct  cortex_random : public cortex_mock_optional
{
    cortex_random(agent const*  myself_, bool const  use_mock_);
    void  next_round(float_32_bit const  time_step_in_seconds);
private:
    float_32_bit  m_seconds_till_change;
    random_generator_for_natural_32_bit  m_generator;
};


}

#endif
