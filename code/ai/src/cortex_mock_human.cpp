#include <ai/cortex_mock_human.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  cortex_mock_human::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    for (float_32_bit& value_ref : get_io()->output)
        value_ref = 0.0f;

    if (get_input_devices()->keyboard.is_pressed(qtgl::KEY_UP()))
        if (get_input_devices()->keyboard.is_pressed(qtgl::KEY_LSHIFT()) || get_input_devices()->keyboard.is_pressed(qtgl::KEY_RSHIFT()))
            get_io()->output.front() = 0.56f;
        else
            get_io()->output.front() = 0.2f;

    bool const  turn_left = get_input_devices()->keyboard.is_pressed(qtgl::KEY_LEFT());
    bool const  turn_right = get_input_devices()->keyboard.is_pressed(qtgl::KEY_RIGHT());
    if (turn_left == true && turn_right == false)
        get_io()->output.back() = -1.0f;
    else if (turn_left == false && turn_right == true)
        get_io()->output.back() = 1.0f;
}


}
