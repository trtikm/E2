#include <ai/detail/action_controller_interpolator_composed.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_composed::action_controller_interpolator_composed(
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding,
        skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor,
        float_32_bit const  reference_offset
        )
    : action_controller_interpolator(motion_templates, binding)
    , m_animation(this, initial_template_cursor, reference_offset)
    , m_look_at(this, initial_template_cursor)
    , m_aim_at(this, initial_template_cursor)
{}


void  action_controller_interpolator_composed::next_round(
        float_32_bit const  time_step_in_seconds,
        vector3 const&  look_at_target_in_agent_space,
        vector3 const&  aim_at_target_in_agent_space
        )
{
    add_time(time_step_in_seconds);

    float_32_bit const  interpolation_param = get_interpolation_parameter();

    m_animation.interpolate(interpolation_param);
    m_look_at.interpolate(
            time_step_in_seconds,
            interpolation_param,
            look_at_target_in_agent_space,
            m_animation.get_current_frames_ref()
            );
    m_aim_at.interpolate(
            time_step_in_seconds,
            interpolation_param,
            aim_at_target_in_agent_space,
            m_animation.get_current_frames_ref()
            );
}


void  action_controller_interpolator_composed::set_target(skeletal_motion_templates::motion_template_cursor const&  cursor)
{
    m_animation.set_target(cursor);
    m_look_at.set_target(cursor);
    m_aim_at.set_target(cursor);
}


void  action_controller_interpolator_composed::commit() const
{
    m_animation.commit();
    m_look_at.commit();
    m_aim_at.commit();
}


}}
