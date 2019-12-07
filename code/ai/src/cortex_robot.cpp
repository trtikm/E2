#include <ai/cortex_robot.hpp>
#include <ai/action_controller.hpp>
#include <ai/detail/guarded_motion_actions_processor.hpp>
#include <ai/detail/ideal_velocity_buider.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex_robot::cortex_robot(blackboard_weak_ptr const  blackboard_)
    : cortex(blackboard_)
    , m_snapshots_cache(env::create_snapshots_cache(blackboard_, {}))
{}


cortex_robot::~cortex_robot()
{}


void  cortex_robot::initialise()
{
    cortex::initialise();
}


void  cortex_robot::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_snapshots_cache->next_round(time_step_in_seconds);

    cortex::next_round(time_step_in_seconds);
}


}