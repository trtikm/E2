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

    float_32_bit&  rot_speed = m_motion_desire_props.speed(DESIRE_COORD::TURN_CCW);
    {
        rot_speed = 0.0f;
        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_J()))
            rot_speed += 1.0f;
        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_L()))
            rot_speed -= 1.0f;
    }

    float_32_bit&  fwd_speed = m_motion_desire_props.speed(DESIRE_COORD::FORWARD);
    {
        fwd_speed = 0.0f;
        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_I()))
        {
            fwd_speed += 0.5f;
            if (m_input_devices->keyboard.is_pressed(qtgl::KEY_Z()))
                fwd_speed += 0.5f;
        }
    }

    m_motion_desire_props.speed(DESIRE_COORD::LEFT) = 0.0f;

    float_32_bit& up_speed = m_motion_desire_props.speed(DESIRE_COORD::UP);
    {
        up_speed = 0.0f;
        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_X()))
            up_speed += 1.0f;
    }

    m_motion_desire_props.look_at_target = 5.0f * vector3_unit(DESIRE_COORD::FORWARD);
}


}
