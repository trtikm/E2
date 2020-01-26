#include <ai/cortex.hpp>
#include <ai/action_controller.hpp>
#include <ai/detail/rigid_body_motion.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex::cortex(blackboard_agent_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)
    , m_motion_desire_props()
{}


void  cortex::initialise()
{
    set_stationary_desire(m_motion_desire_props, get_blackboard());
}


void  cortex::next_round(float_32_bit const  time_step_in_seconds)
{
    set_stationary_desire(m_motion_desire_props, get_blackboard());
}


}

namespace ai {


void  set_stationary_desire(motion_desire_props&  desire_props, blackboard_agent_ptr const  bb)
{
    detail::rigid_body_motion const&  motion = bb->m_action_controller->get_motion_object_motion();
    desire_props.forward_unit_vector_in_local_space = bb->m_motion_templates.directions().forward();
    desire_props.linear_velocity_unit_direction_in_local_space = desire_props.forward_unit_vector_in_local_space;
    desire_props.linear_speed = 0.0f;
    desire_props.angular_velocity_unit_axis_in_local_space = bb->m_motion_templates.directions().up();
    desire_props.angular_speed = 0.0f;
    desire_props.look_at_target_in_local_space = 10.0f * bb->m_motion_templates.directions().forward();
}


}
