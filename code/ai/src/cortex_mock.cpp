#include <ai/cortex_mock.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  cortex_mock::next_round(float_32_bit const  time_step_in_seconds, mock_input_props const&  mock_input)
{
    TMPROF_BLOCK();

    // Screen space: Origin is in the center of the screen, x-axis goes to the right, and y-axis goes up.
    //               Range of coords along both axes is <-1,1>.

    auto const  clip = [](float_32_bit const  x) -> float_32_bit { return std::max(-1.0f, std::min(x, 1.0f)); };

    vector2 const  mouse_pos {
        clip(2.0f * (mock_input.mouse->cursor_x() / (float_32_bit)mock_input.window->window_width()) - 1.0f),
        clip(2.0f * (1.0f - mock_input.mouse->cursor_y() / (float_32_bit)mock_input.window->window_height()) - 1.0f)
    };

    // states of buttons & keys -----------------------

    bool const  lmouse = mock_input.mouse->buttons_pressed().count(osi::LEFT_MOUSE_BUTTON()) != 0UL;
    bool const  mmouse = mock_input.mouse->buttons_pressed().count(osi::MIDDLE_MOUSE_BUTTON()) != 0UL;
    bool const  rmouse = mock_input.mouse->buttons_pressed().count(osi::RIGHT_MOUSE_BUTTON()) != 0UL;
    bool const  shift = mock_input.keyboard->keys_pressed().count(osi::KEY_LSHIFT()) != 0UL ||
                        mock_input.keyboard->keys_pressed().count(osi::KEY_RSHIFT()) != 0UL;
    bool const  ctrl = mock_input.keyboard->keys_pressed().count(osi::KEY_LCTRL()) != 0UL ||
                       mock_input.keyboard->keys_pressed().count(osi::KEY_RCTRL()) != 0UL;
    bool const  alt = mock_input.keyboard->keys_pressed().count(osi::KEY_LALT()) != 0UL ||
                      mock_input.keyboard->keys_pressed().count(osi::KEY_RALT()) != 0UL;

    // speed --------------------

    motion_desire_props_ref().move.clear();
    if (lmouse && !mmouse && !rmouse)
    {
        if (shift)
        {
            motion_desire_props_ref().move.left = mouse_pos(0);
            motion_desire_props_ref().move.forward = mouse_pos(1);
        }
        else if (ctrl)
            motion_desire_props_ref().move.up = mouse_pos(1);
        else
        {
            motion_desire_props_ref().move.turn_ccw = -mouse_pos(0);
            motion_desire_props_ref().move.forward = mouse_pos(1);
        }
    }

    // subject & sign -----------------------

    if (!lmouse && mmouse && !rmouse)
    {
        if (shift)
            motion_desire_props_ref().guesture.sign.intensity = mouse_pos(1);
        else if (ctrl)
        {
            motion_desire_props_ref().guesture.sign.head = mouse_pos(0);
            motion_desire_props_ref().guesture.sign.tail = mouse_pos(1);
        }
        else
        {
            motion_desire_props_ref().guesture.subject.head = mouse_pos(0);
            motion_desire_props_ref().guesture.subject.tail = mouse_pos(1);
        }
    }

    // look/aim_at_target -----------------------

    if (!lmouse && !mmouse && rmouse)
    {
        motion_desire_props_ref().guesture.sign.clear();
        motion_desire_props_ref().guesture.subject.clear();
        if (ctrl)
        {
            if (shift)
                motion_desire_props_ref().aim_at.magnitude = mouse_pos(1);
            else
            {
                motion_desire_props_ref().aim_at.longitude = -mouse_pos(0);
                motion_desire_props_ref().aim_at.altitude = mouse_pos(1);
            }
        }
        else
        {
            if (shift)
                motion_desire_props_ref().look_at.magnitude = mouse_pos(1);
            else
            {
                motion_desire_props_ref().look_at.longitude = -mouse_pos(0);
                motion_desire_props_ref().look_at.altitude = mouse_pos(1);
            }
        }
    }

    //SLOG(
    //    "speed={" << motion_desire_props_ref().move.forward << ","
    //              << motion_desire_props_ref().move.left << ","
    //              << motion_desire_props_ref().move.up << ","
    //              << motion_desire_props_ref().move.turn_ccw << "}\n"
    //    "subject={" << motion_desire_props_ref().guesture.subject.head << ","
    //                << motion_desire_props_ref().guesture.subject.tail << "}\n"
    //    "sign={" << motion_desire_props_ref().guesture.sign.head << ","
    //             << motion_desire_props_ref().guesture.sign.tail << ","
    //             << motion_desire_props_ref().guesture.sign.intensity << "}\n"
    //    "look_at_target={" << motion_desire_props_ref().look_at.longitude << ","
    //                       << motion_desire_props_ref().look_at.altitude << ","
    //                       << motion_desire_props_ref().look_at.magnitude << "}\n"
    //    "aim_at_target={" << motion_desire_props_ref().aim_at.longitude << ","
    //                      << motion_desire_props_ref().aim_at.altitude << ","
    //                      << motion_desire_props_ref().aim_at.magnitude << "}\n"
    //);
}


}
