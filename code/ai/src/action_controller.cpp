#include <ai/action_controller.hpp>
#include <ai/cortex.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/skeleton_utils.hpp>
#include <ai/detail/interpolation_speed_solver.hpp>
#include <angeo/linear_segment_curve.hpp>
#include <utility/std_pair_hash.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <queue>
#include <functional>
#include <algorithm>

namespace ai { namespace detail { namespace {


struct  motion_object_props
{
    explicit motion_object_props(detail::action_controller_roller const& roller)
        : forward(-roller.get_body_frame().basis_vector_y())
        , left(roller.get_body_frame().basis_vector_x())
        , up(roller.get_body_frame().basis_vector_z())
        , seconds_since_last_contact(roller.get_seconds_since_last_contact())
        , external_accel(roller.get_external_linear_acceleration())
        , angle_to_straight_pose(roller.get_angle_to_straight_pose())
        , linear_velocity(roller.get_linear_velocity())
        , angular_velocity(roller.get_angular_velocity())
    {}

    vector3  forward;
    vector3  left;
    vector3  up;
    float_32_bit  seconds_since_last_contact;
    vector3  external_accel;
    float_32_bit  angle_to_straight_pose;
    vector3  linear_velocity;
    vector3  angular_velocity;
};


float_32_bit  compute_penalty(
        std::string const&  name,
        skeletal_motion_templates::motion_object_binding::binding_info const&  info,
        skeletal_motion_templates::motion_object_binding::transition_penalties_map const&  transition_penalties,
        skeletal_motion_templates::motion_template_cursor const&  cursor,
        motion_object_props const&  motion,
        motion_desire_props const&  desire
        )
{
    float_32_bit  penalty = 0.0f;

    auto const  it = transition_penalties.find({ cursor.motion_name, name });
    if (it != transition_penalties.cend())
        penalty += it->second;

    penalty += info.motion_matcher.seconds_since_last_contact(motion.seconds_since_last_contact);
    penalty += info.motion_matcher.angle_to_straight_pose(motion.angle_to_straight_pose);

    vector3  ideal_linear_velocity_dir;
    {
        ideal_linear_velocity_dir =
                info.motion_matcher.ideal_linear_velocity_dir(DESIRE_COORD::FORWARD) * motion.forward +
                info.motion_matcher.ideal_linear_velocity_dir(DESIRE_COORD::LEFT) * motion.left +
                info.motion_matcher.ideal_linear_velocity_dir(DESIRE_COORD::UP) * motion.up
                ;
        float_32_bit const  len = length(ideal_linear_velocity_dir);
        if (len > 0.0001f)
            ideal_linear_velocity_dir *= (1.0f / len);
    }

    penalty += info.motion_matcher.linear_velocity_dir(angle(ideal_linear_velocity_dir, motion.linear_velocity));
    penalty += info.motion_matcher.linear_velocity_mag(length(motion.linear_velocity));
    penalty += info.motion_matcher.angular_speed(dot_product(motion.up, motion.angular_velocity));

    penalty += info.desire_matcher.speed_forward(desire.speed(DESIRE_COORD::FORWARD));
    penalty += info.desire_matcher.speed_left(desire.speed(DESIRE_COORD::LEFT));
    penalty += info.desire_matcher.speed_up(desire.speed(DESIRE_COORD::UP));
    penalty += info.desire_matcher.speed_turn_ccw(desire.speed(DESIRE_COORD::TURN_CCW));

    return penalty;
}


}}}

namespace ai {


action_controller::action_controller(blackboard_agent_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)

    , m_roller(blackboard_)

    , m_template_cursor(get_blackboard()->m_motion_templates.initial_motion_template())
    , m_interpolator(blackboard_, m_template_cursor, m_roller.get_config().AGENT_FRAME_ORIGIN_Z_OFFSET_FROM_BOTTOM)
{}


void  action_controller::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    motion_desire_props const&  desire = get_blackboard()->m_cortex->get_motion_desire_props();

    bool  disable_upper_joint = false;
    skeletal_motion_templates::motion_object_binding::desire_to_speed_convertor_curves const*  desire_to_speed_convertors_ptr;
    {
        static skeletal_motion_templates::motion_object_binding::desire_to_speed_convertor_curves const
        zero_desire_to_speed_convertors{
            {{ // DESIRE_COORD::FORWARD
            }},
            {{ // DESIRE_COORD::LEFT
            }},
            {{ // DESIRE_COORD::UP
            }},
            {{ // DESIRE_COORD::TURN_CCW
            }}
        };
        desire_to_speed_convertors_ptr = &zero_desire_to_speed_convertors;

        skeletal_motion_templates::motion_template_cursor const  old_template_cursor = m_template_cursor;
        std::unordered_set<natural_32_bit>  excluded;
        detail::motion_object_props  motion(m_roller);
        detail::interpolation_speed_solver  speed_solver(get_blackboard()->m_motion_templates,
                                                         length(motion.linear_velocity),
                                                         length(motion.angular_velocity));
        float_32_bit  current_remaining_time = m_interpolator.get_remaining_time();
        float_32_bit  total_remaining_time = 0.0f;
        while (true)
        {
            auto const  binding_it = get_blackboard()->m_motion_templates.motion_object_bindings().find(m_roller.get_name());
            if (binding_it == get_blackboard()->m_motion_templates.motion_object_bindings().cend())
                break;
            skeletal_motion_templates::motion_object_binding const&  motion_bindings = binding_it->second;

            natural_32_bit  best_template_usage = (natural_32_bit)motion_bindings.binding_infos.size();
            {
                float_32_bit  smallest_penalty = std::numeric_limits<float_32_bit>::max();
                for (natural_32_bit  i = 0U; i != motion_bindings.binding_infos.size(); ++i)
                    if (excluded.count(i) == 0UL)
                    {
                        float_32_bit  penalty =
                                compute_penalty(
                                        motion_bindings.binding_infos.at(i).first,
                                        motion_bindings.binding_infos.at(i).second,
                                        motion_bindings.transition_penalties,
                                        m_template_cursor,
                                        motion,
                                        desire
                                        );
                        if (penalty < smallest_penalty)
                        {
                            best_template_usage = i;
                            smallest_penalty = penalty;
                        }
                    }
                }
            INVARIANT(best_template_usage < motion_bindings.binding_infos.size());

            std::string const&  name = motion_bindings.binding_infos.at(best_template_usage).first;
            skeletal_motion_templates::motion_object_binding::binding_info const&  info =
                    motion_bindings.binding_infos.at(best_template_usage).second;
            if (name != m_template_cursor.motion_name)
            {
                std::pair<natural_32_bit, float_32_bit> const  transition_props =
                        get_motion_template_transition_props(
                                    get_blackboard()->m_motion_templates.transitions(),
                                    m_template_cursor,
                                    name,
                                    get_blackboard()->m_motion_templates.default_transition_props()
                                    );
                m_template_cursor.motion_name = name;
                m_template_cursor.keyframe_index = transition_props.first;
                current_remaining_time = transition_props.second;
            }

            skeletal_motion_templates::keyframes const  keyframes =
                    get_blackboard()->m_motion_templates.at(m_template_cursor.motion_name).keyframes;
            auto const  loop_target_it = get_blackboard()->m_motion_templates.loop_targets().find(m_template_cursor.motion_name);

            bool  success = true;
            total_remaining_time += current_remaining_time;
            float_32_bit const  old_total_remaining_time = total_remaining_time;
            while (time_step_in_seconds >= total_remaining_time)
            {
                if (m_template_cursor.keyframe_index + 1U < keyframes.num_keyframes())
                {
                    float_32_bit const  dt = keyframes.time_point_at(m_template_cursor.keyframe_index + 1U) -
                                             keyframes.time_point_at(m_template_cursor.keyframe_index);
                    float_32_bit const  speed_mult =
                            speed_solver.compute_transition_time_scale(speed_solver.compute_interpolation_speed(
                                    m_template_cursor,
                                    { m_template_cursor.motion_name, m_template_cursor.keyframe_index + 1U },
                                    dt,
                                    info.speedup_sensitivity
                                    ));
                    total_remaining_time += speed_mult * dt;
                    m_template_cursor.keyframe_index += 1U;
                }
                else if (loop_target_it != get_blackboard()->m_motion_templates.loop_targets().cend())
                {
                    float_32_bit const  dt = loop_target_it->second.seconds_to_interpolate;
                    float_32_bit const  speed_mult =
                            speed_solver.compute_transition_time_scale(speed_solver.compute_interpolation_speed(
                                    m_template_cursor,
                                    { m_template_cursor.motion_name, loop_target_it->second.index },
                                    dt,
                                    info.speedup_sensitivity
                                    ));
                    total_remaining_time += speed_mult * dt;
                    m_template_cursor.keyframe_index = loop_target_it->second.index;
                }
                else
                {
                    success = false;
                    excluded.insert(best_template_usage);
                    break;
                }
            }
            if (success)
            {
                disable_upper_joint = info.disable_upper_joint;
                desire_to_speed_convertors_ptr = &motion_bindings.desire_to_speed_convertors;
                if (m_template_cursor != old_template_cursor)
                {
                    m_interpolator.reset_time(total_remaining_time);
                    m_interpolator.set_target(m_template_cursor);
                }
                break;
            }
            if (total_remaining_time > old_total_remaining_time)
                total_remaining_time -= current_remaining_time;
        }
    }

    m_roller.apply_desire(
            time_step_in_seconds,
            desire_to_speed_convertors_ptr->at(DESIRE_COORD::TURN_CCW)(desire.speed(DESIRE_COORD::TURN_CCW)),
            desire_to_speed_convertors_ptr->at(DESIRE_COORD::LEFT)(desire.speed(DESIRE_COORD::LEFT)) * vector3_unit_x()
                - desire_to_speed_convertors_ptr->at(DESIRE_COORD::FORWARD)(desire.speed(DESIRE_COORD::FORWARD)) * vector3_unit_y(),
            desire_to_speed_convertors_ptr->at(DESIRE_COORD::UP)(desire.speed(DESIRE_COORD::UP)) * vector3_unit_z(),
            disable_upper_joint
            );

    m_interpolator.next_round(
            time_step_in_seconds,
            desire.look_at_target(DESIRE_COORD::LEFT) * vector3_unit_x()
                - desire.look_at_target(DESIRE_COORD::FORWARD) * vector3_unit_y()
                + desire.look_at_target(DESIRE_COORD::UP) * vector3_unit_y(),
            m_roller.get_agent_frame()
            );
    m_interpolator.commit();
}


}
