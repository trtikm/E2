#include <ai/action_regulator.hpp>
#include <ai/action_controller.hpp>
#include <ai/skeletal_motion_templates.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai {


action_regulator::action_regulator(action_controller const* const  controller_ptr)
    : m_controller(controller_ptr)
{
    ASSUMPTION(controller_ptr != nullptr);
}


natural_32_bit  action_regulator::choose_next_motion_action(
        natural_32_bit const  index_chosen_by_cortex,
        std::vector<skeletal_motion_templates::transition_info> const&  possibilities)
{
    TMPROF_BLOCK();

    // TODO!
    return index_chosen_by_cortex;
}


}
