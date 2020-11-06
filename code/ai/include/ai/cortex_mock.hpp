#ifndef AI_CORTEX_MOCK_HPP_INCLUDED
#   define AI_CORTEX_MOCK_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>

namespace ai {


struct  cortex_mock : public cortex
{
    void  next_round(float_32_bit const  time_step_in_seconds, mock_input_props const&  mock_input) override;
};


}

#endif
