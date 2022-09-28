#ifndef AI_CORTEX_HPP_INCLUDED
#   define AI_CORTEX_HPP_INCLUDED

#   include <ai/motion_desire_props.hpp>
#   include <ai/agent_system_state.hpp>
#   include <ai/agent_system_variables.hpp>
#   include <ai/agent_state_variables.hpp>
#   include <angeo/tensor_math.hpp>
#   include <gfx/viewport.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>

namespace ai { struct  agent; }

namespace ai {


struct  cortex
{
    struct  mock_input_props
    {
        mock_input_props(
                osi::keyboard_props const* const  keyboard_,
                osi::mouse_props const* const  mouse_,
                osi::window_props const* const  window_,
                gfx::viewport const* const  viewport_
                );
        osi::keyboard_props const*  keyboard;
        osi::mouse_props const*  mouse;
        osi::window_props const*  window;
        gfx::viewport const*  viewport;
    };

    cortex(agent const*  myself_);
    virtual ~cortex() {}

    agent const&  myself() const { return *m_myself; }
    motion_desire_props const&  get_motion_desire_props() const { return m_motion_desire_props; }
    motion_desire_props&  motion_desire_props_ref() { return m_motion_desire_props; }

    void  next_round(float_32_bit const  time_step_in_seconds, mock_input_props const* const  mock_input_ptr);

    virtual void  next_round(float_32_bit const  time_step_in_seconds) {}

    // Only "MOCK" cortices should override this version of the next_step function (instead of the one above!).
    virtual void  next_round(float_32_bit const  time_step_in_seconds, mock_input_props const&  mock_input)
    { next_round(time_step_in_seconds); }

private:
    agent const*  m_myself;
    motion_desire_props  m_motion_desire_props;
};


}

#endif
