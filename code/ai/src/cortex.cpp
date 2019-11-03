#include <ai/cortex.hpp>
#include <ai/action_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex::cortex(blackboard_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)
    , m_motion_desire_props()
{}


void  cortex::initialise()
{
    detail::rigid_body_motion const&  motion = get_blackboard()->m_action_controller->get_motion_object_motion();
    m_motion_desire_props.forward_unit_vector_in_world_space = motion.forward;
    m_motion_desire_props.linear_velocity_unit_direction_in_world_space = m_motion_desire_props.forward_unit_vector_in_world_space;
    m_motion_desire_props.linear_speed = 0.0f;
    m_motion_desire_props.angular_velocity_unit_axis_in_world_space = motion.up;
    m_motion_desire_props.angular_speed = 0.0f;
}


void  cortex::next_round(float_32_bit const  time_step_in_seconds)
{
    // Nothing to do.
}


vector3  cortex::get_look_at_target_in_world_space() const
{
    return get_blackboard()->m_action_controller->get_motion_object_motion().frame.origin() +
                10.0f * get_blackboard()->m_action_controller->get_motion_object_motion().forward;
}


skeletal_motion_templates::cursor_and_transition_time  cortex::choose_next_motion_action(
        std::vector<skeletal_motion_templates::cursor_and_transition_time> const&  possibilities
        ) const
{
    return possibilities.front();
}


}
