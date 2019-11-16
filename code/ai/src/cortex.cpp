#include <ai/cortex.hpp>
#include <ai/action_controller.hpp>
#include <ai/detail/guarded_motion_actions_processor.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex::cortex(blackboard_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)
    , m_motion_desire_props()
    , m_ranks_of_motion_actions()
    , m_look_at_target_in_local_space(vector3_unit_y())
{}


void  cortex::initialise()
{
    set_stationary_desire();
    set_stationary_ranks_of_motion_actions();
}


void  cortex::next_round(float_32_bit const  time_step_in_seconds)
{
    set_stationary_desire();
    set_stationary_ranks_of_motion_actions();
    set_indifferent_look_at_target();
}


skeletal_motion_templates::cursor_and_transition_time  cortex::choose_next_motion_action(
        std::vector<skeletal_motion_templates::cursor_and_transition_time> const&  possibilities
        ) const
{
    float_32_bit  best_rank = -1.0f;
    natural_32_bit  best_index = 0U;
    for (natural_32_bit  i = 0U; i != possibilities.size(); ++i)
    {
        float_32_bit  rank = 0.0f;
        for (auto const guarded_actions_ptr : get_blackboard()->m_motion_templates.at(possibilities.at(i).first.motion_name).actions
                                                                                  .at(possibilities.at(i).first.keyframe_index))
            for (auto const action_ptr : guarded_actions_ptr->actions)
                rank += m_ranks_of_motion_actions.at(action_ptr->get_unique_name());
        if (best_rank < rank)
        {
            best_rank = rank;
            best_index = i;
        }
    }
    return possibilities.at(best_index);
}


void  cortex::set_stationary_desire()
{
    detail::rigid_body_motion const& motion = get_blackboard()->m_action_controller->get_motion_object_motion();
    m_motion_desire_props.forward_unit_vector_in_world_space = motion.forward;
    m_motion_desire_props.linear_velocity_unit_direction_in_world_space = m_motion_desire_props.forward_unit_vector_in_world_space;
    m_motion_desire_props.linear_speed = 0.0f;
    m_motion_desire_props.angular_velocity_unit_axis_in_world_space = motion.up;
    m_motion_desire_props.angular_speed = 0.0f;
}


void  cortex::set_stationary_ranks_of_motion_actions()
{
    for (auto const&  action_name : detail::get_all_action_unique_names())
        m_ranks_of_motion_actions[action_name] = detail::get_stationary_rank(action_name);
}


void  cortex::set_indifferent_look_at_target()
{
    detail::rigid_body_motion const&  motion = get_blackboard()->m_action_controller->get_motion_object_motion();
    matrix44  T;
    angeo::to_base_matrix(motion.frame, T);
    m_look_at_target_in_local_space = transform_point(motion.frame.origin() + 10.0f * motion.forward, T);
}


}
