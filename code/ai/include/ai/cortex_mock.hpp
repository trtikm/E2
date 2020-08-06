#ifndef AI_CORTEX_MOCK_HPP_INCLUDED
#   define AI_CORTEX_MOCK_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>

namespace ai {


struct  cortex_mock : public cortex
{
    void  next_round(
            float_32_bit const  time_step_in_seconds,
            osi::keyboard_props const&  keyboard,
            osi::mouse_props const&  mouse,
            osi::window_props const&  window
            ) override;
};


}

#endif
