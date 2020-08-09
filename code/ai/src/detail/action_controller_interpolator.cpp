#include <ai/detail/action_controller_interpolator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_base::action_controller_interpolator_base(
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : m_motion_templates(motion_templates)
    , m_binding(binding)
{
    ASSUMPTION(m_motion_templates.loaded_successfully() && m_binding->frame_guids_of_bones.size() == m_motion_templates.names().size());
}


action_controller_interpolator_time::action_controller_interpolator_time(
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : action_controller_interpolator_base(motion_templates, binding)
    , m_total_interpolation_time_in_seconds(0.0f)
    , m_consumed_time_in_seconds(0.0f)
{}


void  action_controller_interpolator_time::add_time(float_32_bit const  time_step_in_seconds)
{
    m_consumed_time_in_seconds = std::min(m_total_interpolation_time_in_seconds, m_consumed_time_in_seconds + time_step_in_seconds);
}


void  action_controller_interpolator_time::reset_time(float_32_bit const  total_interpolation_time_in_seconds)
{
    ASSUMPTION(total_interpolation_time_in_seconds >= 0.0f);
    m_total_interpolation_time_in_seconds = total_interpolation_time_in_seconds;
    m_consumed_time_in_seconds = 0.0f;
}


float_32_bit  action_controller_interpolator_time::get_interpolation_parameter() const
{
    return m_total_interpolation_time_in_seconds < 0.0001f ?
                1.0f :
                std::min(m_consumed_time_in_seconds / m_total_interpolation_time_in_seconds, 1.0f);
}


}}