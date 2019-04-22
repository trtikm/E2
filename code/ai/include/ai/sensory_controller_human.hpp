#ifndef AI_SENSORY_CONTROLLER_HUMAN_HPP_INCLUDED
#   define AI_SENSORY_CONTROLLER_HUMAN_HPP_INCLUDED

#   include <ai/sensory_controller.hpp>
#   include <ai/blackboard_human.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai {


struct  sensory_controller_human : public sensory_controller
{
    sensory_controller_human(blackboard_ptr const  blackboard_)
        : sensory_controller(blackboard_)
    {}

    blackboard_human_ptr  get_blackboard() const { return as<blackboard_human>(sensory_controller::get_blackboard()); }

    void  next_round(float_32_bit const  time_step_in_seconds) override;
};


}

#endif
