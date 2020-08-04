#ifndef AIOLD_CORTEX_ROBOT_HPP_INCLUDED
#   define AIOLD_CORTEX_ROBOT_HPP_INCLUDED

#   include <aiold/cortex.hpp>
#   include <aiold/env/sinet/snapshot_encoder.hpp>
#   include <aiold/env/sinet/motion_desire_decoder.hpp>
#   include <aiold/blackboard_agent.hpp>
#   include <netlab/simple_network.hpp>
#   include <angeo/tensor_math.hpp>
#   include <memory>

namespace aiold {


struct  cortex_robot : public cortex
{
    explicit cortex_robot(blackboard_agent_weak_ptr const  blackboard_);
    ~cortex_robot();

    void  initialise() override;
    void  next_round(float_32_bit const  time_step_in_seconds) override;

private:

    static constexpr float_32_bit  NETWORK_TIME_STEP_DURATION_IN_SECONDS = 0.01f;

    std::unique_ptr<env::sinet::snapshot_encoder>  snapshot_encoder;
    std::unique_ptr<env::sinet::motion_desire_decoder>  motion_desire_decoder;
    std::unique_ptr<netlab::simple::network>  network;
    float_32_bit  time_buffer_in_seconds;
};


}

#endif
