#include <ai/cortex_mock.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  cortex_mock::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // Screen space: Origin is in the center of the screen, x-axis goes to the right, and y-axis goes up.
    //               Range of coords along both axes is <-1,1>.

    auto const  clip = [](float_32_bit const  x) -> float_32_bit { return std::max(-1.0f, std::min(x, 1.0f)); };

    vector2 const  mouse_pos {
        clip(2.0f * (m_input_devices->mouse.x() / (float_32_bit)m_input_devices->window.width_in_pixels()) - 1.0f),
        clip(2.0f * (1.0f - m_input_devices->mouse.y() / (float_32_bit)m_input_devices->window.height_in_pixels()) - 1.0f)
    };

    // speed --------------------

    if (m_input_devices->mouse.is_pressed(qtgl::LEFT_MOUSE_BUTTON()) && !m_input_devices->mouse.is_pressed(qtgl::RIGHT_MOUSE_BUTTON()))
    {
        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_LSHIFT()) || m_input_devices->keyboard.is_pressed(qtgl::KEY_RSHIFT()))
        {
            m_motion_desire_props.move.left = mouse_pos(0);
            m_motion_desire_props.move.forward = mouse_pos(1);
        }
        else if (m_input_devices->keyboard.is_pressed(qtgl::KEY_LCTRL()) || m_input_devices->keyboard.is_pressed(qtgl::KEY_RCTRL()))
            m_motion_desire_props.move.up = mouse_pos(1);
        else
        {
            m_motion_desire_props.move.turn_ccw = -mouse_pos(0);
            m_motion_desire_props.move.forward = mouse_pos(1);
        }
    }

    // subject & sign -----------------------

    if (m_input_devices->mouse.is_pressed(qtgl::LEFT_MOUSE_BUTTON()) && m_input_devices->mouse.is_pressed(qtgl::RIGHT_MOUSE_BUTTON()))
    {
        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_LSHIFT()) || m_input_devices->keyboard.is_pressed(qtgl::KEY_RSHIFT()))
            m_motion_desire_props.guesture.sign.intensity = mouse_pos(1);
        else if (m_input_devices->keyboard.is_pressed(qtgl::KEY_LCTRL()) || m_input_devices->keyboard.is_pressed(qtgl::KEY_RCTRL()))
        {
            m_motion_desire_props.guesture.sign.head = mouse_pos(0);
            m_motion_desire_props.guesture.sign.tail = mouse_pos(1);
        }
        else
        {
            m_motion_desire_props.guesture.subject.head = mouse_pos(0);
            m_motion_desire_props.guesture.subject.tail = mouse_pos(1);
        }
    }

    // look/aim_at_target -----------------------

    if (!m_input_devices->mouse.is_pressed(qtgl::LEFT_MOUSE_BUTTON()) && m_input_devices->mouse.is_pressed(qtgl::RIGHT_MOUSE_BUTTON()))
    {
        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_LCTRL()) || m_input_devices->keyboard.is_pressed(qtgl::KEY_RCTRL()))
            m_motion_desire_props.aim_at = m_motion_desire_props.look_at;
        else
        {
            if (m_input_devices->keyboard.is_pressed(qtgl::KEY_LSHIFT()) || m_input_devices->keyboard.is_pressed(qtgl::KEY_RSHIFT()))
                m_motion_desire_props.look_at.magnitude = mouse_pos(1);
            else
            {
                m_motion_desire_props.look_at.longitude = -mouse_pos(0);
                m_motion_desire_props.look_at.altitude = mouse_pos(1);
            }
        }
    }

    SLOG(
        "speed={" << m_motion_desire_props.move.forward << ","
                  << m_motion_desire_props.move.left << ","
                  << m_motion_desire_props.move.up << ","
                  << m_motion_desire_props.move.turn_ccw << "}\n"
        "subject={" << m_motion_desire_props.guesture.subject.head << ","
                    << m_motion_desire_props.guesture.subject.tail << "}\n"
        "sign={" << m_motion_desire_props.guesture.sign.head << ","
                 << m_motion_desire_props.guesture.sign.tail << ","
                 << m_motion_desire_props.guesture.sign.intensity << "}\n"
        "look_at_target={" << m_motion_desire_props.look_at.longitude << ","
                           << m_motion_desire_props.look_at.altitude << ","
                           << m_motion_desire_props.look_at.magnitude << "}\n"
        "aim_at_target={" << m_motion_desire_props.aim_at.longitude << ","
                          << m_motion_desire_props.aim_at.altitude << ","
                          << m_motion_desire_props.aim_at.magnitude << "}\n"
    );
}


}
