#include <ai/cortex.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex::cortex(agent const*  myself_)
    : m_myself(myself_)
    , m_motion_desire_props()
{
    ASSUMPTION(m_myself != nullptr);
}


cortex::mock_input_props::mock_input_props(
        osi::keyboard_props const* const  keyboard_,
        osi::mouse_props const* const  mouse_,
        osi::window_props const* const  window_,
        gfx::viewport const* const  viewport_
        )
    : keyboard(keyboard_)
    , mouse(mouse_)
    , window(window_)
    , viewport(viewport_)
{
    ASSUMPTION(keyboard != nullptr && mouse != nullptr && window != nullptr && viewport != nullptr);
}


void  cortex::next_round(float_32_bit const  time_step_in_seconds, mock_input_props const* const  mock_input_ptr)
{
    if (mock_input_ptr == nullptr)
        next_round(time_step_in_seconds);
    else
        next_round(time_step_in_seconds, *mock_input_ptr);
}


}
