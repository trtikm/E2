#ifndef AI_DETAIL_INTERPOLATION_SPEED_SOLVER_HPP_INCLUDED
#   define AI_DETAIL_INTERPOLATION_SPEED_SOLVER_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai { namespace detail {


struct  interpolation_speed_solver
{
    interpolation_speed_solver(
            skeletal_motion_templates const  motion_templates,
            float_32_bit const  agent_linear_speed,
            float_32_bit const  agent_angular_speed,
            float_32_bit const  min_speed_coef = 0.1f,
            float_32_bit const  max_speed_coef = 10.0f
            );

    float_32_bit  compute_interpolation_speed(
            skeletal_motion_templates::motion_template_cursor const&  src_cursor,
            skeletal_motion_templates::motion_template_cursor const&  dst_cursor,
            float_32_bit const  transition_time,
            skeletal_motion_templates::motion_object_binding::speed_sensitivity const&  speed_sensitivity
            ) const;

    float_32_bit  compute_transition_time_scale(float_32_bit const  interpolation_speed) const;

private:
    skeletal_motion_templates  m_motion_templates;
    float_32_bit  m_agent_linear_speed;
    float_32_bit  m_agent_angular_speed;
    float_32_bit  MIN_SPEED_COEF;
    float_32_bit  MAX_SPEED_COEF;
};


}}

#endif
