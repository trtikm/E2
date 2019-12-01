#include <ai/action_regulator.hpp>
#include <ai/action_controller.hpp>
#include <ai/cortex.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai {


action_regulator::action_regulator(action_controller const* const  controller_ptr)
    : m_controller(controller_ptr)
    , m_motion_desire_props()
{
    ASSUMPTION(controller_ptr != nullptr);
}


void  action_regulator::initialise()
{
    set_stationary_desire(m_motion_desire_props, m_controller->get_blackboard());
}


void  action_regulator::next_round(motion_desire_props const&  motion_desire_props_of_the_cortex)
{
    TMPROF_BLOCK();

    m_motion_desire_props = motion_desire_props_of_the_cortex;

    // TODO!
}


}
