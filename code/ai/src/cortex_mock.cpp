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

    m_motion_desire_props.speed = vector4_zero();
    if (m_input_devices->keyboard.is_pressed(qtgl::KEY_I()))
    {
        m_motion_desire_props.speed(DESIRE_COORD::FORWARD) += 0.5f;
        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_Z()))
            m_motion_desire_props.speed(DESIRE_COORD::FORWARD) += 0.5f;
    }
    if (m_input_devices->keyboard.is_pressed(qtgl::KEY_X()))
        m_motion_desire_props.speed(DESIRE_COORD::UP) += 1.0f;
    if (m_input_devices->keyboard.is_pressed(qtgl::KEY_J()))
        m_motion_desire_props.speed(DESIRE_COORD::TURN_CCW) += 1.0f;
    if (m_input_devices->keyboard.is_pressed(qtgl::KEY_L()))
        m_motion_desire_props.speed(DESIRE_COORD::TURN_CCW) -= 1.0f;

    using  region_id = std::pair<natural_32_bit, natural_32_bit>;

    auto const  get_region_center = [this](region_id const&  id) -> vector2 {
        return vector2{
            (float_32_bit)(id.first + 1U) * 1.0f / 3.0f - 1.0f / 6.0f,
            (float_32_bit)(id.second + 1U) * 1.0f / 3.0f - 1.0f / 6.0f
        };
    };

    auto const  get_mouse_pos = [this]() -> vector2 {
        return vector2{
            (float_32_bit)m_input_devices->mouse.x() / (float_32_bit)m_input_devices->window.width_in_pixels(),
            1.0f - (float_32_bit)m_input_devices->mouse.y() / (float_32_bit)m_input_devices->window.height_in_pixels()
        };
    };

    auto const  get_closest_region_id_ref =
        [this, &get_region_center](std::vector<region_id> const&  ids, vector2 const&  mouse) -> region_id const& {
            float_32_bit  min_dist = std::numeric_limits<float_32_bit>::max();
            natural_32_bit  best_i = (natural_32_bit)ids.size();
            for (natural_32_bit  i = 0; i != ids.size(); ++i)
            {
                float_32_bit  dist = length_2d(get_region_center(ids.at(i)) - mouse);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    best_i = i;
                }
            }
            return ids.at(best_i);
        };

    vector2 const  mouse = get_mouse_pos();

    if (m_input_devices->mouse.was_just_released(qtgl::LEFT_MOUSE_BUTTON()))
    {
        static std::vector<region_id> const  GRID31 = {
            { 1U, 0U },
            { 1U, 1U },
            { 1U, 2U },
        };
        static std::vector<region_id> const  GRID33 = {
            { 0U, 0U }, { 1U, 0U }, { 2U, 0U },
            { 0U, 1U }, { 1U, 1U }, { 2U, 1U },
            { 0U, 2U }, { 1U, 2U }, { 2U, 2U }
        };

        if (m_input_devices->keyboard.is_pressed(qtgl::KEY_R()))
        {
            region_id const&  id = get_closest_region_id_ref(GRID33, mouse);
            m_motion_desire_props.guesture_subject(DESIRE_COORD::HEAD) = (float_32_bit)id.first * 0.5f;
            m_motion_desire_props.guesture_subject(DESIRE_COORD::TAIL) = (float_32_bit)id.second * 0.5f;
        }
        else if (m_input_devices->keyboard.is_pressed(qtgl::KEY_T()))
        {
            natural_32_bit const  idx = get_closest_region_id_ref(GRID31, mouse).second;
            m_motion_desire_props.guesture_sign(DESIRE_COORD::HEAD) = (float_32_bit)idx * 0.5f;
        }
        else if (m_input_devices->keyboard.is_pressed(qtgl::KEY_Y()))
        {
            natural_32_bit const  idx = get_closest_region_id_ref(GRID31, mouse).second;
            m_motion_desire_props.guesture_sign(DESIRE_COORD::TAIL) = (float_32_bit)idx * 0.5f;
        }
        else if (m_input_devices->keyboard.is_pressed(qtgl::KEY_U()))
        {
            natural_32_bit const  idx = get_closest_region_id_ref(GRID31, mouse).second;
            m_motion_desire_props.guesture_intensity = (float_32_bit)idx * 0.5f;
        }
    }

    //SLOG(
    //    "subject={" << m_motion_desire_props.guesture_subject(0) << "," << m_motion_desire_props.guesture_subject(1) << "}, "
    //    "sign={" << m_motion_desire_props.guesture_sign(0) << "," << m_motion_desire_props.guesture_sign(1) << "}, "
    //    "intensity=" << m_motion_desire_props.guesture_intensity
    //);

    m_motion_desire_props.look_at_target = 5.0f * vector3_unit(DESIRE_COORD::FORWARD);
    m_motion_desire_props.aim_at_target = 0.3f * vector3_unit(DESIRE_COORD::FORWARD) + 0.3f * vector3_unit(DESIRE_COORD::UP);
}


}
