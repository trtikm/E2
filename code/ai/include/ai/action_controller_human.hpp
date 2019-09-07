#ifndef AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED

#   include <ai/action_controller.hpp>
#   include <ai/blackboard_human.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai {


struct  action_controller_human : public action_controller
{
    explicit action_controller_human(blackboard_ptr const  blackboard_);
    ~action_controller_human();

    blackboard_human_ptr  get_blackboard() const { return as<blackboard_human>(action_controller::get_blackboard()); }

    void  next_round_internal(float_32_bit  time_step_in_seconds) override;
};


}

#endif
