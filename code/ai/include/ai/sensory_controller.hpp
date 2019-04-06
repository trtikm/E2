#ifndef AI_SENSORY_CONTROLLER_HPP_INCLUDED
#   define AI_SENSORY_CONTROLLER_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai {


struct  sensory_controller
{
    sensory_controller(blackboard_ptr const  blackboard_)
        : m_blackboard(blackboard_)
    {}

    virtual ~sensory_controller() {}

    virtual void  next_round(float_32_bit const  time_step_in_seconds) {}

    blackboard_ptr  get_blackboard() const { return m_blackboard; }

private:
    blackboard_ptr  m_blackboard;
};


}

#endif
