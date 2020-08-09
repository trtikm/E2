#include <ai/action_controller.hpp>
#include <ai/cortex.hpp>
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
                info.motion_matcher.ideal_linear_velocity_dir(0) * motion.forward +
                info.motion_matcher.ideal_linear_velocity_dir(1) * motion.left +
                info.motion_matcher.ideal_linear_velocity_dir(2) * motion.up
                ;
        float_32_bit const  len = length(ideal_linear_velocity_dir);
        if (len > 0.0001f)
            ideal_linear_velocity_dir *= (1.0f / len);
    }

    penalty += info.motion_matcher.linear_velocity_dir(angle(ideal_linear_velocity_dir, motion.linear_velocity));
    penalty += info.motion_matcher.linear_velocity_mag(length(motion.linear_velocity));
    penalty += info.motion_matcher.angular_speed(dot_product(motion.up, motion.angular_velocity));

    penalty += info.desire_matcher.speed_forward(desire.move.forward);
    penalty += info.desire_matcher.speed_left(desire.move.left);
    penalty += info.desire_matcher.speed_up(desire.move.up);
    penalty += info.desire_matcher.speed_turn_ccw(desire.move.turn_ccw);

    return penalty;
}


}}}

namespace ai {


action_controller::action_controller(
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : m_roller(motion_templates, binding)
    , m_template_cursor(motion_templates.initial_motion_template())
    , m_interpolator(
            motion_templates,
            binding,
            m_template_cursor,
            m_roller.get_config().AGENT_FRAME_ORIGIN_Z_OFFSET_FROM_BOTTOM
            )
{
    m_interpolator.commit();
}


void  action_controller::next_round(
        float_32_bit const  time_step_in_seconds,
        motion_desire_props const&  desire
        )
{
    TMPROF_BLOCK();

    m_roller.synchronise_with_scene();

    bool  disable_upper_joint = false;
    skeletal_motion_templates::motion_object_binding::desire_values_intepretation_curves const*  desire_values_interpreters_ptr;
    {
        static skeletal_motion_templates::motion_object_binding::desire_values_intepretation_curves const  zero_desire_values_interpreters;
        desire_values_interpreters_ptr = &zero_desire_values_interpreters;

        skeletal_motion_templates::motion_template_cursor const  old_template_cursor = m_template_cursor;
        std::unordered_set<natural_32_bit>  excluded;
        detail::motion_object_props  motion(m_roller);
        detail::interpolation_speed_solver  speed_solver(get_motion_templates(),
                                                         length(motion.linear_velocity),
                                                         length(motion.angular_velocity));
        float_32_bit  current_remaining_time = m_interpolator.get_remaining_time();
        float_32_bit  total_remaining_time = 0.0f;
        while (true)
        {
            auto const  binding_it = get_motion_templates().motion_object_bindings().find(m_roller.get_name());
            if (binding_it == get_motion_templates().motion_object_bindings().cend())
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
                                    get_motion_templates().transitions(),
                                    m_template_cursor,
                                    name,
                                    get_motion_templates().default_transition_props()
                                    );
                float_32_bit const  speed_mult =
                        speed_solver.compute_transition_time_scale(speed_solver.compute_interpolation_speed(
                                m_template_cursor,
                                { name, transition_props.first },
                                transition_props.second,
                                info.speedup_sensitivity
                                ));
                m_template_cursor.motion_name = name;
                m_template_cursor.keyframe_index = transition_props.first;
                current_remaining_time = transition_props.second;
            }

            skeletal_motion_templates::keyframes const  keyframes =
                    get_motion_templates().at(m_template_cursor.motion_name).keyframes;
            auto const  loop_target_it = get_motion_templates().loop_targets().find(m_template_cursor.motion_name);

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
                else if (loop_target_it != get_motion_templates().loop_targets().cend())
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
                desire_values_interpreters_ptr = &motion_bindings.desire_values_interpreters;
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
            desire_values_interpreters_ptr->move.turn_ccw(desire.move.turn_ccw),
            desire_values_interpreters_ptr->move.left(desire.move.left) * vector3_unit_x()
                - desire_values_interpreters_ptr->move.forward(desire.move.forward) * vector3_unit_y(),
            desire_values_interpreters_ptr->move.up(desire.move.up) * vector3_unit_z(),
            disable_upper_joint
            );


    auto const  get_look_or_aim_target =
            []( skeletal_motion_templates::motion_object_binding::desire_values_intepretation_curves::target_curves const&  curves,
                motion_desire_props::target_props const&  props
            ) -> vector3
            {
                float_32_bit const  lon_angle = curves.longitude(props.longitude);
                float_32_bit const  alt_angle = curves.altitude(props.altitude);
                float_32_bit const  mag = curves.magnitude(props.magnitude);
                float_32_bit const  cos_alt_angle = std::cosf(alt_angle);
                vector3 const  result {
                    mag * cos_alt_angle * std::cosf(lon_angle - PI() / 2.0f),
                    mag * cos_alt_angle * std::sinf(lon_angle - PI() / 2.0f),
                    mag * std::sinf(alt_angle),
                };
                return result;
            };

    auto const  get_origin_shift = [this](natural_32_bit const  bone) -> vector3 {
        vector3 const  origin_in_bone_space = 0.5f * get_motion_templates().lengths().at(bone) * vector3_unit_y();
        vector3 const  origin_in_template_space =
                transform_point(origin_in_bone_space, get_motion_templates().from_pose_matrices().at(bone));
        vector3 const  origin_in_reference_frame =
                angeo::point3_to_coordinate_system(
                        origin_in_template_space,        
                        get_motion_templates().at(m_template_cursor.motion_name).reference_frames.at(m_template_cursor.keyframe_index)
                        );
        return origin_in_reference_frame;
    };

    m_interpolator.next_round(
            time_step_in_seconds,
            get_look_or_aim_target(desire_values_interpreters_ptr->look_at, desire.look_at)
                + get_origin_shift(get_motion_templates().look_at_origin_bone()),
            get_look_or_aim_target(desire_values_interpreters_ptr->aim_at, desire.aim_at)
                + get_origin_shift(get_motion_templates().aim_at_origin_bone())
            );
    m_interpolator.commit();
}


}
