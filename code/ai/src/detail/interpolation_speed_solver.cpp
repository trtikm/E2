#include <ai/detail/interpolation_speed_solver.hpp>
#include <utility/development.hpp>
#include <memory>

namespace ai { namespace detail {


float_32_bit  interpolation_speed_solver::compute_interpolation_speed(
        skeletal_motion_templates::motion_template_cursor const&  src_cursor,
        skeletal_motion_templates::motion_template_cursor const&  dst_cursor,
        float_32_bit const  transition_time,
        skeletal_motion_templates::motion_object_binding::speed_sensitivity const&  speed_sensitivity
        ) const
{
    if (src_cursor.motion_name != dst_cursor.motion_name || transition_time < 0.001f)
        return 1.0f;

    angeo::coordinate_system const&  src_frame =
            m_motion_templates.at(src_cursor.motion_name).reference_frames.at(src_cursor.keyframe_index);
    angeo::coordinate_system const&  dst_frame =
            m_motion_templates.at(dst_cursor.motion_name).reference_frames.at(dst_cursor.keyframe_index);

    float_32_bit  ideal_linear_speed, ideal_angular_speed;
    {
        vector3  pos_delta;
        vector3  rot_axis;
        float_32_bit  rot_angle;
        {
            matrix44  F, T;
            angeo::from_base_matrix(dst_frame, F);
            angeo::to_base_matrix(src_frame, T);

            matrix33  rot;
            decompose_matrix44(T * F, pos_delta, rot);

            rot_angle = quaternion_to_angle_axis(rotation_matrix_to_quaternion(rot), rot_axis);
        }
        ideal_linear_speed = length(pos_delta) / transition_time;
        ideal_angular_speed = length((rot_angle / transition_time) * rot_axis);
    }

    auto const  compute_speed_coef = [](float_32_bit const  real_speed, float_32_bit const  ideal_speed) {
        return ideal_speed < 0.0001f ? 1.0f + real_speed : real_speed / ideal_speed;
    };

    float_32_bit const  linear_speed_coef = compute_speed_coef(m_agent_linear_speed, ideal_linear_speed);
    float_32_bit const  angular_speed_coef = compute_speed_coef(m_agent_angular_speed, ideal_angular_speed);

    skeletal_motion_templates::motion_object_binding::speed_sensitivity  normalised_sensitivity = speed_sensitivity;
    if (normalised_sensitivity.sum() > 1.0f)
    {
        float_32_bit const  denom = normalised_sensitivity.sum();
        normalised_sensitivity.linear /= denom;
        normalised_sensitivity.angular /= denom;
    }

    float_32_bit const  raw_interpolation_speed =
            normalised_sensitivity.linear * linear_speed_coef +
            normalised_sensitivity.angular * angular_speed_coef +
            (1.0f - normalised_sensitivity.sum()) * 1.0f
            ;

    float_32_bit const  interpolation_speed = std::max(MIN_SPEED_COEF, std::min(raw_interpolation_speed, MAX_SPEED_COEF));

    return interpolation_speed;
}


float_32_bit  interpolation_speed_solver::compute_transition_time_scale(float_32_bit const  interpolation_speed) const
{
    float_32_bit const  MIDDLE = 0.5f * (MIN_SPEED_COEF + MAX_SPEED_COEF);
    return MIDDLE - (interpolation_speed - MIDDLE);
}


}}
