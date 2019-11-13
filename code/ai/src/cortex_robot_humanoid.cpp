#include <ai/cortex_robot_humanoid.hpp>
#include <ai/action_controller.hpp>
#include <ai/detail/guarded_motion_actions_processor.hpp>
#include <ai/detail/ideal_velocity_buider.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex_robot_humanoid::cortex_robot_humanoid(blackboard_weak_ptr const  blackboard_)
    : cortex(blackboard_)
    , m_snapshots_cache(env::create_snapshots_cache(blackboard_, {}))
{}


cortex_robot_humanoid::~cortex_robot_humanoid()
{}


void  cortex_robot_humanoid::initialise()
{
    cortex::initialise();
}


void  cortex_robot_humanoid::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_snapshots_cache->next_round(time_step_in_seconds);

    // TODO
}


vector3  cortex_robot_humanoid::get_look_at_target_in_world_space() const
{
    // TODO
    return cortex::get_look_at_target_in_world_space();
}


skeletal_motion_templates::cursor_and_transition_time  cortex_robot_humanoid::choose_next_motion_action(
        std::vector<skeletal_motion_templates::cursor_and_transition_time> const&  possibilities
        ) const
{
    TMPROF_BLOCK();

    // TODO
    return cortex::choose_next_motion_action(possibilities);
}


}
