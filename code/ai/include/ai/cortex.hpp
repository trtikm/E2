#ifndef AI_CORTEX_HPP_INCLUDED
#   define AI_CORTEX_HPP_INCLUDED

#   include <ai/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>

namespace ai {


struct  cortex
{
    cortex();
    virtual ~cortex() {}

    motion_desire_props const&  get_motion_desire_props() const { return m_motion_desire_props; }
    motion_desire_props&  motion_desire_props_ref() { return m_motion_desire_props; }

    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    // Only "MOCK" cortices should override this version of the next_step function.
    virtual void  next_round(
            float_32_bit const  time_step_in_seconds,
            osi::keyboard_props const&,
            osi::mouse_props const&,
            osi::window_props const&
            )
    { next_round(time_step_in_seconds); }

private:
    motion_desire_props  m_motion_desire_props;
};


}

#endif
