#include <ai/cortex_mock_human.hpp>
#include <ai/action_controller.hpp>
#include <ai/detail/guarded_motion_actions_processor.hpp>
#include <ai/detail/ideal_velocity_buider.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex_mock_human::cortex_mock_human(blackboard_weak_ptr const  blackboard_, input_devices_const_ptr const  input_devices_)
    : cortex_mock(blackboard_, input_devices_)
    , m_move_intensity(0.0f)
    , m_turn_intensity(0.0f)
    , m_elevation_intensity(0.0f)
    , m_max_forward_speed_in_meters_per_second(7.5f)
    , m_max_turn_speed_in_radians_per_second(PI() * 1.0f)
{}


void  cortex_mock_human::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    update_motion_intensities(time_step_in_seconds);
    update_motion_desire_props();
    update_look_at_target_in_local_space();
}


void  cortex_mock_human::update_motion_intensities(float_32_bit const  time_step_in_seconds)
{
    m_move_intensity = 0.0f;
    m_turn_intensity = 0.0f;
    m_elevation_intensity = 0.0f;

    if (get_input_devices()->keyboard.is_pressed(qtgl::KEY_UP()))
        if (get_input_devices()->keyboard.is_pressed(qtgl::KEY_LSHIFT()) || get_input_devices()->keyboard.is_pressed(qtgl::KEY_RSHIFT()))
            m_move_intensity = 0.56f;
        else
            m_move_intensity = 0.2f;

    bool const  turn_left = get_input_devices()->keyboard.is_pressed(qtgl::KEY_LEFT());
    bool const  turn_right = get_input_devices()->keyboard.is_pressed(qtgl::KEY_RIGHT());
    if (turn_left == true && turn_right == false)
        m_turn_intensity = 1.0f * time_step_in_seconds;
    else if (turn_left == false && turn_right == true)
        m_turn_intensity = -1.0f * time_step_in_seconds;

    if (get_input_devices()->keyboard.is_pressed(qtgl::KEY_LCTRL()) || get_input_devices()->keyboard.is_pressed(qtgl::KEY_RCTRL()))
        m_elevation_intensity = 0.5f;
}


void  cortex_mock_human::update_motion_desire_props()
{
    m_motion_desire_props.linear_velocity_unit_direction_in_world_space =
            normalised(
                    quaternion_to_rotation_matrix(
                            angle_axis_to_quaternion(m_turn_intensity * m_max_turn_speed_in_radians_per_second, vector3_unit_z())
                            )
                    * vector3(m_motion_desire_props.linear_velocity_unit_direction_in_world_space(0),
                              m_motion_desire_props.linear_velocity_unit_direction_in_world_space(1),
                              0.0f)
                    );
    float_32_bit const  elevation_angle = m_elevation_intensity * 0.5f * PI();
    m_motion_desire_props.linear_velocity_unit_direction_in_world_space *= std::cosf(elevation_angle);
    m_motion_desire_props.linear_velocity_unit_direction_in_world_space(2) = std::sinf(elevation_angle);

    m_motion_desire_props.linear_speed = m_move_intensity * m_max_forward_speed_in_meters_per_second;

    // Currently we do not support motions where m_desired_forward_unit_vector_in_world_space and
    // m_desired_linear_velocity_unit_direction_in_world_space could be different.
    m_motion_desire_props.forward_unit_vector_in_world_space = m_motion_desire_props.linear_velocity_unit_direction_in_world_space;

    // Currently we do not support rotation motions desires => the following assignments are intentionally commented.
    //m_motion_desire_props.angular_velocity_unit_axis_in_world_space = ...;
    //m_motion_desire_props.angular_speed = ...;
}


void  cortex_mock_human::update_look_at_target_in_local_space()
{
    detail::rigid_body_motion const&  motion = get_blackboard()->m_action_controller->get_motion_object_motion();
    matrix44  T;
    angeo::to_base_matrix(motion.frame, T);

    m_look_at_target_in_local_space =
            transform_point(
                    motion.frame.origin() + std::max(1.0f, m_motion_desire_props.linear_speed) *
                                            m_motion_desire_props.linear_velocity_unit_direction_in_world_space,
                    T
                    );
}


skeletal_motion_templates::cursor_and_transition_time  cortex_mock_human::choose_next_motion_action(
        std::vector<skeletal_motion_templates::cursor_and_transition_time> const&  possibilities
        ) const
{
    TMPROF_BLOCK();

    std::vector<natural_32_bit>  satisfied_transitions;
    for (natural_32_bit  i = 0U; i != possibilities.size(); ++i)
    {
        if (detail::get_satisfied_motion_guarded_actions(
                    get_blackboard()->m_motion_templates.motions_map().at(possibilities.at(i).first.motion_name).actions
                                                                      .at(possibilities.at(i).first.keyframe_index),
                    get_blackboard()->m_collision_contacts,
                    get_blackboard()->m_action_controller->get_motion_object_motion(),
                    get_motion_desire_props(),
                    get_blackboard()->m_action_controller->get_gravity_acceleration(),
                    nullptr
                    ))
            satisfied_transitions.push_back(i);
    }
    if (satisfied_transitions.size() == 1UL)
        return possibilities.at(satisfied_transitions.front());

    natural_32_bit  best_index;
    {
        best_index = std::numeric_limits<natural_32_bit>::max();
        float_32_bit  best_cost = std::numeric_limits<float_32_bit>::max();
        float_32_bit const  time_horizon = 0.2f;
        float_32_bit const  integration_time_step = 0.1f;
        detail::motion_action_persistent_data_map  motion_action_data;
        for (natural_32_bit i = 0U; i != satisfied_transitions.size(); ++i)
        {
            detail::rigid_body_motion  motion = get_blackboard()->m_action_controller->get_motion_object_motion();
            float_32_bit  consumed_time = 0.0f;
            {
                skeletal_motion_templates::motion_template_cursor  cursor = possibilities.at(satisfied_transitions.at(i)).first;
                detail::ideal_velocity_buider  ideal_velocity_buider(cursor, get_blackboard()->m_motion_templates );
                float_32_bit  integration_time = 0.0f;
                bool  passed_branching = false;
                while (consumed_time < time_horizon || !passed_branching)
                {
                    if (consumed_time >= time_horizon && get_blackboard()->m_motion_templates.is_branching_keyframe(cursor))
                        passed_branching = true;

                    get_blackboard()->m_motion_templates.for_each_successor_keyframe(
                            cursor,
                            [&cursor, &ideal_velocity_buider, &consumed_time, &integration_time]
                            (skeletal_motion_templates::motion_template_cursor const&  target_cursor, float_32_bit const  transition_time)
                            {
                                cursor = target_cursor;
                                consumed_time += transition_time;
                                integration_time += transition_time;
                                ideal_velocity_buider.extend(target_cursor, transition_time);
                                return false;   // We need only the first successor to stay in the initial animation as long as possible.
                                                // When all possibilities goes out from the initial animation, then take any => the first is ok.
                            });
                    if (integration_time > integration_time_step || consumed_time >= time_horizon)
                    {
                        detail::motion_action_persistent_data_map  discardable_motion_action_data;
                        motion.acceleration.m_linear = motion.acceleration.m_angular = vector3_zero();
                        vector3  ideal_linear_velocity_in_world_space, ideal_angular_velocity_in_world_space;
                        ideal_velocity_buider.close(
                                motion.frame,
                                ideal_linear_velocity_in_world_space,
                                ideal_angular_velocity_in_world_space
                                );
                        ideal_velocity_buider.reset(cursor);
                        execute_satisfied_motion_guarded_actions(
                                get_blackboard()->m_motion_templates.motions_map().at(cursor.motion_name).actions.at(cursor.keyframe_index),
                                integration_time,
                                ideal_linear_velocity_in_world_space,
                                ideal_angular_velocity_in_world_space,
                                vector3_zero(),
                                get_motion_desire_props(),
                                motion_action_data,
                                motion,
                                discardable_motion_action_data
                                );
                        motion.integrate(integration_time);
                        integration_time = 0.0f;
                    }
                }
            }

            float_32_bit  cost;
            {
                vector3 const&  current_origin = get_blackboard()->m_action_controller->get_motion_object_motion().frame.origin();

                float_32_bit const  d_pos =
                    length(
                        current_origin +
                        consumed_time *
                        get_motion_desire_props().linear_speed *
                        get_motion_desire_props().linear_velocity_unit_direction_in_world_space - motion.frame.origin());
                float_32_bit const  d_fwd = angle(get_motion_desire_props().forward_unit_vector_in_world_space, motion.forward) / PI();
                
                cost = 1.0f * d_pos + 2.0f * d_fwd;
            }

            if (cost < best_cost)
            {
                best_cost = cost;
                best_index = satisfied_transitions.at(i);
            }
        }
    }

    return  possibilities.at(best_index);
}


}
