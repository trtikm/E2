#ifndef AI_CORTEX_NETLAB_HPP_INCLUDED
#   define AI_CORTEX_NETLAB_HPP_INCLUDED

#   include <ai/cortex_mock.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai {


struct  cortex_netlab : public cortex_mock_optional
{
    cortex_netlab(agent const*  myself_, bool const  use_mock_);
    void  next_round(float_32_bit const  time_step_in_seconds) override;

private:
};


}

#endif
