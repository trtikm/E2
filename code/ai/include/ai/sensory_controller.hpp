#ifndef AI_SENSORY_CONTROLLER_HPP_INCLUDED
#   define AI_SENSORY_CONTROLLER_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/sensory_controller_sight.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace ai {


struct  sensory_controller
{
    sensory_controller(
            blackboard_weak_ptr const  blackboard_,
            sensory_controller_sight_ptr  const  sight_ = nullptr)
        : m_blackboard(blackboard_)
        , m_sight(sight_)
    {}

    virtual ~sensory_controller() {}

    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    blackboard_ptr  get_blackboard() const { return m_blackboard.lock(); }

    sensory_controller_sight_ptr  get_sight() const { return m_sight; }

private:
    blackboard_weak_ptr  m_blackboard;
    sensory_controller_sight_ptr  m_sight;
};


using sensory_controller_ptr = std::shared_ptr<sensory_controller>;


}

#endif
