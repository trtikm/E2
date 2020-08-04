#include <aiold/sensory_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace aiold {


sensory_controller::sensory_controller(
        blackboard_agent_weak_ptr const  blackboard_,
        sensory_controller_collision_contacts_ptr const  collision_contacts_,
        sensory_controller_sight_ptr  const  sight_
        )
    : m_blackboard(blackboard_)
    , m_collision_contacts(collision_contacts_)
    , m_sight(sight_)
{
    ASSUMPTION(m_collision_contacts != nullptr);
}


void  sensory_controller::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    get_collision_contacts()->next_round();
    if (get_sight() != nullptr)
        get_sight()->next_round(time_step_in_seconds);
}


}
