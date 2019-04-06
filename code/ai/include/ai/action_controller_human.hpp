#ifndef AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED

#   include <ai/action_controller.hpp>
#   include <ai/blackboard.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace ai {


struct  action_controller_human : public action_controller
{
    action_controller_human(blackboard_ptr const  blackboard_)
        : action_controller(blackboard_)
    {}

    void  next_round(float_32_bit const  time_step_in_seconds) override;
};


}

#endif
