#ifndef AI_CORTEX_ROBOT_HPP_INCLUDED
#   define AI_CORTEX_ROBOT_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <ai/blackboard.hpp>
#   include <netlab/simple_network.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai {


struct  cortex_robot : public cortex
{
    explicit cortex_robot(blackboard_weak_ptr const  blackboard_);
    ~cortex_robot();

    void  initialise() override;
    void  next_round(float_32_bit const  time_step_in_seconds) override;

private:

    static constexpr float_32_bit  NETWORK_TIMES_STEP_DURATION_IN_SECONDS = 0.01f;

    netlab::simple::network  network;
    float_32_bit  time_buffer_in_seconds;
};


}

#endif
