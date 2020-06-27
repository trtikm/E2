#include <ai/detail/interpolation_speed_solver.hpp>
#include <utility/development.hpp>
#include <memory>

namespace ai { namespace detail {


interpolation_speed_solver::interpolation_speed_solver(
        skeletal_motion_templates const  motion_templates,
        float_32_bit const  agent_linear_speed,
        float_32_bit const  agent_angular_speed,
        float_32_bit const  min_speed_coef,
        float_32_bit const  max_speed_coef
        )
    : m_motion_templates(motion_templates)
    , m_agent_linear_speed(agent_linear_speed)
    , m_agent_angular_speed(agent_angular_speed)
    , MIN_SPEED_COEF(min_speed_coef)
    , MAX_SPEED_COEF(max_speed_coef)
{
    ASSUMPTION(MIN_SPEED_COEF > 0.0001f && MAX_SPEED_COEF > MIN_SPEED_COEF + 0.0001f);
}

float_32_bit  interpolation_speed_solver::compute_interpolation_speed(
        skeletal_motion_templates::motion_template_cursor const&  src_cursor,
        skeletal_motion_templates::motion_template_cursor const&  dst_cursor,
        float_32_bit const  transition_time,
        skeletal_motion_templates::motion_object_binding::speed_sensitivity const&  speed_sensitivity
        ) const
{
    if (transition_time < 0.001f)
        return 1.0f;

    if (src_cursor.motion_name != dst_cursor.motion_name)
    {
        float_32_bit const  is1 = compute_interpolation_speed(src_cursor, src_cursor, transition_time, speed_sensitivity);
        float_32_bit const  is2 = compute_interpolation_speed(dst_cursor, dst_cursor, transition_time, speed_sensitivity);
        return 0.5f * (is1 + is2);
    }

    if (src_cursor.keyframe_index == dst_cursor.keyframe_index)
    {
        if (src_cursor.keyframe_index > 0U)
            return compute_interpolation_speed(
                        { src_cursor.motion_name, src_cursor.keyframe_index - 1U },
                        src_cursor,
                        transition_time,
                        speed_sensitivity
                        );
        if (src_cursor.keyframe_index + 1U < m_motion_templates.at(src_cursor.motion_name).keyframes.num_keyframes())
            return compute_interpolation_speed(
                        src_cursor,
                        { src_cursor.motion_name, src_cursor.keyframe_index + 1U },
                        transition_time,
                        speed_sensitivity
                        );
        return 1.0f;
    }

    if (dst_cursor.keyframe_index < src_cursor.keyframe_index)
        return compute_interpolation_speed(
                    { src_cursor.motion_name, src_cursor.keyframe_index - 1U },
                    src_cursor,
                    transition_time,
                    speed_sensitivity
                    );

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
    ASSUMPTION(MIN_SPEED_COEF <= interpolation_speed && interpolation_speed <= MAX_SPEED_COEF);
    return 1.0f / interpolation_speed;
}


}}
