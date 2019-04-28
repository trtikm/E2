#ifndef AI_ACTION_CONTROLLER_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace ai {


struct  action_controller
{
    action_controller(blackboard_ptr const  blackboard_)
        : m_blackboard(blackboard_)
    {}

    virtual ~action_controller() {}

    virtual void  next_round(float_32_bit  time_step_in_seconds) {}

    blackboard_ptr  get_blackboard() const { return m_blackboard; }

private:
    blackboard_ptr  m_blackboard;
};


}

#endif
