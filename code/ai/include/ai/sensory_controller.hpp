#ifndef AI_SENSORY_CONTROLLER_HPP_INCLUDED
#   define AI_SENSORY_CONTROLLER_HPP_INCLUDED

#   include <ai/blackboard_agent.hpp>
#   include <ai/sensory_controller_collision_contacts.hpp>
#   include <ai/sensory_controller_sight.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace ai {


struct  sensory_controller
{
    explicit sensory_controller(
            blackboard_agent_weak_ptr const  blackboard_,
            sensory_controller_collision_contacts_ptr const  collision_contacts_, // Can NOT be nullptr
            sensory_controller_sight_ptr  const  sight_ // Can be nullptr
            );

    virtual ~sensory_controller() {}

    virtual void  initialise() {}
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    blackboard_agent_ptr  get_blackboard() const { return m_blackboard.lock(); }
    sensory_controller_collision_contacts_ptr  get_collision_contacts() const { return m_collision_contacts; }
    sensory_controller_sight_ptr  get_sight() const { return m_sight; }

private:
    blackboard_agent_weak_ptr  m_blackboard;
    sensory_controller_collision_contacts_ptr  m_collision_contacts;
    sensory_controller_sight_ptr  m_sight;
};


using sensory_controller_ptr = std::shared_ptr<sensory_controller>;


}

#endif
