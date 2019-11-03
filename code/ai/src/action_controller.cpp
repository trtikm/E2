#include <ai/action_controller.hpp>
#include <ai/skeleton_utils.hpp>
#include <ai/detail/rigid_body_motion.hpp>
#include <ai/detail/ideal_velocity_buider.hpp>
#include <ai/detail/collider_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <queue>
#include <functional>

namespace ai {


action_controller::intepolation_state::intepolation_state()
    : frames()
    , free_bones_look_at(nullptr)
    , collider(nullptr)
    , mass_distribution(nullptr)
    , disjunction_of_guarded_actions()
{}


action_controller::action_controller(blackboard_weak_ptr const  blackboard_)
    : m_motion_desire_props()
    , m_motion_object_motion()
    , m_gravity_acceleration(vector3_zero())

    , m_blackboard(blackboard_)

    , m_total_interpolation_time_in_seconds(0.0f)
    , m_consumed_time_in_seconds(0.0f)

    , m_src_intepolation_state()
    , m_current_intepolation_state()
    , m_use_inverted_collider_center_offset_interpolation(false)

    , m_dst_cursor{ get_blackboard()->m_motion_templates.transitions().initial_motion_name(), 0U }
    , m_dst_frames()
    , m_ideal_linear_velocity_in_world_space(vector3_zero())
    , m_ideal_angular_velocity_in_world_space(vector3_zero())
    , m_collider_center_offset_in_reference_frame(vector3_zero())

    , m_motion_action_data()
{
    TMPROF_BLOCK();

    transform_keyframes_to_reference_frame(
            get_blackboard()->m_motion_templates.at(m_dst_cursor.motion_name).keyframes.get_keyframes()
                                                .at(m_dst_cursor.keyframe_index).get_coord_systems(),
            get_blackboard()->m_motion_templates.at(m_dst_cursor.motion_name).reference_frames
                                                .at(m_dst_cursor.keyframe_index),
            get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
            get_blackboard()->m_motion_templates.hierarchy().parents(),
            m_dst_frames
            );

    auto const  collider = get_blackboard()->m_motion_templates.at(m_dst_cursor.motion_name).colliders
                                                               .at(m_dst_cursor.keyframe_index);
    auto const  mass_distribution =
        get_blackboard()->m_motion_templates.at(m_dst_cursor.motion_name).mass_distributions
                                            .at(m_dst_cursor.keyframe_index);

    angeo::coordinate_system  agent_frame;
    {
        angeo::coordinate_system  tmp;
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_bone_nids.front(), false, tmp);
        agent_frame.set_origin(tmp.origin());
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, tmp);
        agent_frame.set_orientation(tmp.orientation());
    }
    scene::node_id  nid(detail::get_motion_object_nid(get_blackboard()->m_scene, get_blackboard()->m_agent_nid));
    if (!get_blackboard()->m_scene->has_scene_node(nid))
        nid = detail::create_motion_scene_node(get_blackboard()->m_scene, nid, agent_frame, collider, *mass_distribution);

    m_motion_object_motion = detail::rigid_body_motion(get_blackboard()->m_scene, nid, get_blackboard()->m_motion_templates.directions());
    m_gravity_acceleration = get_blackboard()->m_scene->get_gravity_acceleration_at_point(m_motion_object_motion.frame.origin());

    get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_agent_id);

    angeo::coordinate_system  motion_object_frame;
    get_blackboard()->m_scene->get_frame_of_scene_node(m_motion_object_motion.nid, false, motion_object_frame);

    m_current_intepolation_state.frames = m_dst_frames;
    m_current_intepolation_state.free_bones_look_at =
        get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name).free_bones.look_at()
                                                          .at(m_dst_cursor.keyframe_index);
    m_current_intepolation_state.collider = collider;
    m_current_intepolation_state.mass_distribution = mass_distribution;
    m_current_intepolation_state.disjunction_of_guarded_actions =
        get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name).actions
                                                          .at(m_dst_cursor.keyframe_index);

    m_src_intepolation_state = m_current_intepolation_state;

    action_controller::next_round_internal(0.0f);
}


action_controller::~action_controller()
{
    TMPROF_BLOCK();

    if (m_motion_object_motion.nid.valid())
    {
        get_blackboard()->m_scene->unregister_to_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_agent_id);
        detail::destroy_motion_scene_node(get_blackboard()->m_scene, m_motion_object_motion.nid);
    }
}


void  action_controller::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_motion_object_motion.update_frame_with_forward_and_up_directions(get_blackboard()->m_scene, get_blackboard()->m_motion_templates.directions());
    m_motion_object_motion.update_linear_velocity(get_blackboard()->m_scene);
    m_motion_object_motion.update_angular_velocity(get_blackboard()->m_scene);
    m_gravity_acceleration = get_blackboard()->m_scene->get_gravity_acceleration_at_point(m_motion_object_motion.frame.origin());

    // We clear all forces the agent introduced in the previous frame.
    m_motion_object_motion.set_linear_acceleration(m_gravity_acceleration);
    m_motion_object_motion.set_angular_acceleration(vector3_zero());
    m_motion_object_motion.commit_accelerations(get_blackboard()->m_scene, m_motion_object_motion.nid);

    // Synchronise agent's position in the world space according to its motion object in the previous time step
    m_motion_object_motion.commit_frame(get_blackboard()->m_scene, get_blackboard()->m_agent_nid);

    // Virtual call to child controller (if any). The main purpose to update desires of the agent.
    // And in particular to update the member 'm_motion_desire_props'
    next_round_internal(time_step_in_seconds);

    float_32_bit const  interpolation_time_step_in_seconds = compute_interpolation_speed() * time_step_in_seconds;

    if (m_consumed_time_in_seconds + interpolation_time_step_in_seconds >= m_total_interpolation_time_in_seconds)
    {
        m_src_intepolation_state = m_current_intepolation_state;

        m_total_interpolation_time_in_seconds -= m_consumed_time_in_seconds;
        m_consumed_time_in_seconds = 0.0f;

        float_32_bit  consumed_time = 0.0f;
        detail::ideal_velocity_buider  ideal_velocity_buider(m_dst_cursor, get_blackboard()->m_motion_templates);

        while (m_consumed_time_in_seconds + interpolation_time_step_in_seconds >= m_total_interpolation_time_in_seconds)
        {
            std::pair<skeletal_motion_templates::motion_template_cursor, float_32_bit> const  best_transition = choose_transition();
            m_dst_cursor = best_transition.first;
            m_total_interpolation_time_in_seconds += best_transition.second;
            consumed_time += best_transition.second;
            ideal_velocity_buider.extend(best_transition.first, best_transition.second);
        }

        ideal_velocity_buider.close(
                m_motion_object_motion.frame,
                m_ideal_linear_velocity_in_world_space,
                m_ideal_angular_velocity_in_world_space
                );

        transform_keyframes_to_reference_frame(
                get_blackboard()->m_motion_templates.at(m_dst_cursor.motion_name).keyframes.get_keyframes()
                                                    .at(m_dst_cursor.keyframe_index).get_coord_systems(),
                get_blackboard()->m_motion_templates.at(m_dst_cursor.motion_name).reference_frames
                                                    .at(m_dst_cursor.keyframe_index),
                get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
                get_blackboard()->m_motion_templates.hierarchy().parents(),
                m_dst_frames
                );

        m_collider_center_offset_in_reference_frame = 
                detail::compute_offset_for_center_of_second_collider_to_get_surfaces_alignment_in_direction(
                        m_src_intepolation_state.collider,
                        get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name).colliders
                                                                          .at(m_dst_cursor.keyframe_index),
                        -get_blackboard()->m_motion_templates.directions().up()
                        );
        m_use_inverted_collider_center_offset_interpolation = false;
    }

    m_consumed_time_in_seconds += interpolation_time_step_in_seconds;
    INVARIANT(m_consumed_time_in_seconds <= m_total_interpolation_time_in_seconds);

    float_32_bit const  interpolation_param = m_consumed_time_in_seconds / m_total_interpolation_time_in_seconds;

    interpolate(interpolation_param);
    look_at_target(time_step_in_seconds, interpolation_param);

    // Next we update frames of scene nodes corresponding to the interpolated frames of bones.
    {
        vector3 const  offset =
                (interpolation_param - (m_use_inverted_collider_center_offset_interpolation ? 1.0f : 0.0f))
                * m_collider_center_offset_in_reference_frame
                ;
        auto const&  parents = get_blackboard()->m_motion_templates.hierarchy().parents();
        for (natural_32_bit bone = 0; bone != m_current_intepolation_state.frames.size(); ++bone)
            get_blackboard()->m_scene->set_frame_of_scene_node(
                    get_blackboard()->m_bone_nids.at(bone),
                    true,
                    parents.at(bone) < 0 ?
                        angeo::coordinate_system{
                            m_current_intepolation_state.frames.at(bone).origin() + offset,
                            m_current_intepolation_state.frames.at(bone).orientation()
                            }
                        :
                        m_current_intepolation_state.frames.at(bone)
                    );
    }

    // And finally, we update dynamics of agent's motion object (forces and torques).
    skeletal_motion_templates::guarded_actions_ptr const  satisfied_guarded_actions =
        detail::get_first_satisfied_motion_guarded_actions(
                m_current_intepolation_state.disjunction_of_guarded_actions,
                get_blackboard()->m_collision_contacts,
                m_motion_object_motion,
                m_motion_desire_props,
                m_gravity_acceleration
                );
    if (satisfied_guarded_actions != nullptr)
    {
        execute_satisfied_motion_guarded_actions(
                satisfied_guarded_actions->actions,
                time_step_in_seconds,
                m_ideal_linear_velocity_in_world_space,
                m_ideal_angular_velocity_in_world_space,
                m_gravity_acceleration,
                m_motion_desire_props,
                m_motion_action_data,
                m_motion_object_motion,
                m_motion_action_data
                );
        m_motion_object_motion.commit_accelerations(get_blackboard()->m_scene, m_motion_object_motion.nid);
    }
}


void  action_controller::next_round_internal(float_32_bit)
{
    m_motion_desire_props.forward_unit_vector_in_world_space = m_motion_object_motion.forward;
    m_motion_desire_props.linear_velocity_unit_direction_in_world_space = m_motion_desire_props.forward_unit_vector_in_world_space;
    m_motion_desire_props.linear_speed = 0.0f;
    m_motion_desire_props.angular_velocity_unit_axis_in_world_space = m_motion_object_motion.up;
    m_motion_desire_props.angular_speed = 0.0f;
}


float_32_bit  action_controller::compute_interpolation_speed() const
{
    TMPROF_BLOCK();

    detail::importance_of_ideal_velocities_to_guarded_actions  importances;
    detail::compute_importance_of_ideal_velocities_to_guarded_actions(
            m_current_intepolation_state.disjunction_of_guarded_actions,
            importances
            );
    importances.normalise_sum_of_importances_to_range_01();

    float_32_bit const  min_coef_value = 0.5f;
    float_32_bit const  max_coef_value = 1.5f;
    auto const  compute_speed_coef = [min_coef_value, max_coef_value](float_32_bit const  real_speed, float_32_bit const  ideal_speed)
    {
        float_32_bit const  raw_coef = ideal_speed < 0.0001f ? 1.0f + real_speed : real_speed / ideal_speed;
        return std::max(min_coef_value, std::min(raw_coef, max_coef_value));
    };

    float_32_bit const  linear_speed_coef = compute_speed_coef(
            length(m_motion_object_motion.velocity.m_linear),
            length(m_ideal_linear_velocity_in_world_space)
            );
    float_32_bit const  angular_speed_coef = compute_speed_coef(
            length(m_motion_object_motion.velocity.m_angular),
            length(m_ideal_angular_velocity_in_world_space)
            );

    float_32_bit const  raw_interpolation_speed =
            importances.linear * linear_speed_coef +
            importances.angular * angular_speed_coef +
            (1.0f - importances.sum()) * 1.0f
            ;

    float_32_bit const  interpolation_speed = std::max(min_coef_value, std::min(raw_interpolation_speed, max_coef_value));

    return interpolation_speed;
}


void  action_controller::interpolate(float_32_bit const  interpolation_param)
{
    TMPROF_BLOCK();

    skeletal_motion_templates::motion_template const&  dst_motion_template =
            get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name);

    interpolate_keyframes_spherical(m_src_intepolation_state.frames, m_dst_frames, interpolation_param, m_current_intepolation_state.frames);

    skeletal_motion_templates::free_bones_for_look_at_ptr const  dst_free_bones_look_at =
            dst_motion_template.free_bones.look_at().at(m_dst_cursor.keyframe_index);
    m_current_intepolation_state.free_bones_look_at = (interpolation_param < 0.5f) ? m_src_intepolation_state.free_bones_look_at : dst_free_bones_look_at;

    auto const  dst_collider = dst_motion_template.colliders.at(m_dst_cursor.keyframe_index);
    float_32_bit const  motion_object_interpolation_param =
            (m_src_intepolation_state.collider->weight + dst_collider->weight < 0.0001f) ?
                    0.5f :
                    m_src_intepolation_state.collider->weight / (m_src_intepolation_state.collider->weight + dst_collider->weight);

    skeletal_motion_templates::collider_ptr  interpolated_collider;
    skeletal_motion_templates::mass_distribution_ptr  interpolated_mass_distribution;
    if (interpolation_param < motion_object_interpolation_param)
    {
        interpolated_collider = m_src_intepolation_state.collider;
        interpolated_mass_distribution = m_src_intepolation_state.mass_distribution;
    }
    else
    {
        interpolated_collider = dst_collider;
        interpolated_mass_distribution = dst_motion_template.mass_distributions.at(m_dst_cursor.keyframe_index);
    }
    if (*interpolated_collider != *m_current_intepolation_state.collider || *interpolated_mass_distribution != *m_current_intepolation_state.mass_distribution)
    {
        m_current_intepolation_state.collider = interpolated_collider;
        m_current_intepolation_state.mass_distribution = interpolated_mass_distribution;
        m_motion_object_motion.inverted_mass = interpolated_mass_distribution->mass_inverted;
        m_motion_object_motion.inverted_inertia_tensor = interpolated_mass_distribution->inertia_tensor_inverted;
        {
            matrix44  W;
            angeo::from_base_matrix(m_motion_object_motion.frame, W);
            vector3 const  offset = transform_vector(m_collider_center_offset_in_reference_frame, W);
            angeo::translate(m_motion_object_motion.frame, offset);
            m_use_inverted_collider_center_offset_interpolation = true;
        }

        get_blackboard()->m_scene->unregister_to_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_agent_id);
        detail::destroy_collider_and_rigid_bofy_of_motion_scene_node(get_blackboard()->m_scene, m_motion_object_motion.nid);
        detail::create_collider_and_rigid_body_of_motion_scene_node(
                get_blackboard()->m_scene,
                m_motion_object_motion.nid,
                m_current_intepolation_state.collider,
                m_motion_object_motion
                );
        get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_agent_id);
        m_motion_object_motion.commit_frame(get_blackboard()->m_scene, get_blackboard()->m_agent_nid);
    }

    m_current_intepolation_state.disjunction_of_guarded_actions =
            (interpolation_param < motion_object_interpolation_param) ?
                    m_src_intepolation_state.disjunction_of_guarded_actions :
                    dst_motion_template.actions.at(m_dst_cursor.keyframe_index);
}


void  action_controller::look_at_target(float_32_bit const  time_step_in_seconds, float_32_bit const  interpolation_param)
{
    TMPROF_BLOCK();

    skeletal_motion_templates::free_bones_for_look_at_ptr const  dst_look_at_bones =
            get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name).free_bones.look_at()
                                                              .at(m_dst_cursor.keyframe_index);;
    if (m_current_intepolation_state.free_bones_look_at->all_bones.empty() && dst_look_at_bones->all_bones.empty())
        return;

    // First setup data for the look-at algo.

    std::unordered_set<integer_32_bit>  bones_to_consider;
    for (auto bone : m_current_intepolation_state.free_bones_look_at->all_bones)
        bones_to_consider.insert(bone);
    if (m_current_intepolation_state.free_bones_look_at != dst_look_at_bones)
        for (auto bone : dst_look_at_bones->all_bones)
            bones_to_consider.insert(bone);

    auto const& parents = get_blackboard()->m_motion_templates.hierarchy().parents();

    vector3  target;
    {
        target = m_motion_object_motion.frame.origin() +
            std::max(1.0f, m_motion_desire_props.linear_speed) *
            m_motion_desire_props.linear_velocity_unit_direction_in_world_space;

        // The target is now in world space, but we need the target in the space of the bone which is the closest parent bone
        // to all bones in 'bones_to_consider'; note that the parent bone cannot be in 'bones_to_consider'.

        integer_32_bit  closest_parent_bone = *bones_to_consider.begin();
        for (; bones_to_consider.count(closest_parent_bone) != 0ULL; closest_parent_bone = parents.at(closest_parent_bone))
            ;

        angeo::coordinate_system  closest_parent_bone_frame;
        get_blackboard()->m_scene->get_frame_of_scene_node(
            get_blackboard()->m_bone_nids.at(closest_parent_bone),
            false,
            closest_parent_bone_frame
        );

        matrix44  W;
        angeo::to_base_matrix(closest_parent_bone_frame, W);

        target = transform_point(target, W);
    }

    angeo::bone_look_at_targets  look_at_targets;
    for (auto bone : m_current_intepolation_state.free_bones_look_at->end_effector_bones)
        look_at_targets.insert({ (integer_32_bit)bone, { vector3_unit_y(), target } });
    if (m_current_intepolation_state.free_bones_look_at != dst_look_at_bones)
        for (auto bone : dst_look_at_bones->end_effector_bones)
            look_at_targets.insert({ (integer_32_bit)bone, { vector3_unit_y(), target } });

    // Now execute the look-at algo for the prepared data.

    std::vector<angeo::coordinate_system>  target_frames;
    std::unordered_map<integer_32_bit, std::vector<natural_32_bit> >  bones_to_rotate;
    angeo::skeleton_look_at(
        target_frames,
        look_at_targets,
        get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
        parents,
        get_blackboard()->m_motion_templates.joints().data(),
        bones_to_consider,
        &bones_to_rotate);

    std::vector<angeo::coordinate_system>  frames;
    frames.resize(get_blackboard()->m_motion_templates.names().size());
    for (auto const& bone_and_anges : bones_to_rotate)
        get_blackboard()->m_scene->get_frame_of_scene_node(
            get_blackboard()->m_bone_nids.at(bone_and_anges.first),
            true,
            frames.at(bone_and_anges.first)
            );

    angeo::skeleton_rotate_bones_towards_target_pose(
        frames,
        target_frames,
        get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
        get_blackboard()->m_motion_templates.joints().data(),
        bones_to_rotate,
        time_step_in_seconds
        );

    // And write results to the vector 'm_current_intepolation_state.frames' of final frames.

    float_32_bit const  src_param = m_current_intepolation_state.free_bones_look_at->all_bones.empty() ? 0.0f : 1.0f;
    float_32_bit const  dst_param = dst_look_at_bones->all_bones.empty() ? 0.0f : 1.0f;
    float_32_bit const  param = src_param + interpolation_param * (dst_param - src_param);
    for (auto bone : bones_to_consider)
        angeo::interpolate_spherical(
            m_current_intepolation_state.frames.at(bone),
            frames.at(bone),
            param,
            m_current_intepolation_state.frames.at(bone)
            );
}


std::pair<skeletal_motion_templates::motion_template_cursor, float_32_bit>  action_controller::choose_transition() const
{
    TMPROF_BLOCK();

    std::vector<std::pair<skeletal_motion_templates::motion_template_cursor, float_32_bit> > successors;
    get_blackboard()->m_motion_templates.get_successor_keyframes(m_dst_cursor, successors);
    if (successors.size() == 1UL)
        return successors.front();
    std::vector<natural_32_bit>  satisfied_transitions;
    for (natural_32_bit  i = 0U; i != successors.size(); ++i)
    {
        skeletal_motion_templates::guarded_actions_ptr  satisfied_guarded_actions =
                detail::get_first_satisfied_motion_guarded_actions(
                        get_blackboard()->m_motion_templates.motions_map().at(successors.at(i).first.motion_name).actions
                                                                          .at(successors.at(i).first.keyframe_index),
                        get_blackboard()->m_collision_contacts,
                        m_motion_object_motion,
                        m_motion_desire_props,
                        m_gravity_acceleration
                        );
        if (satisfied_guarded_actions != nullptr)
            satisfied_transitions.push_back(i);
    }
    if (satisfied_transitions.size() == 1UL)
        return successors.at(satisfied_transitions.front());

    natural_32_bit  best_index;
    {
        best_index = std::numeric_limits<natural_32_bit>::max();
        float_32_bit  best_cost = std::numeric_limits<float_32_bit>::max();
        float_32_bit const  time_horizon = 0.2f;
        float_32_bit const  integration_time_step = 0.1f;
        for (natural_32_bit i = 0U; i != satisfied_transitions.size(); ++i)
        {
            detail::rigid_body_motion  motion = m_motion_object_motion;
            float_32_bit  consumed_time = 0.0f;
            {
                skeletal_motion_templates::motion_template_cursor  cursor = successors.at(satisfied_transitions.at(i)).first;
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
                                                // When all successors goes out from the initial animation, then take any => the first is ok.
                            });
                    if (integration_time > integration_time_step || consumed_time >= time_horizon)
                    {
                        detail::motion_action_persistent_data_map  discardable_motion_action_data;
                        std::vector<skeletal_motion_templates::action_ptr>  actions;
                        for (auto const&  disjunction : get_blackboard()->m_motion_templates.motions_map().at(cursor.motion_name).actions
                                                                                                          .at(cursor.keyframe_index))
                            for (auto const  action_ptr : disjunction->actions)
                                actions.push_back(action_ptr);
                        motion.acceleration.m_linear = motion.acceleration.m_angular = vector3_zero();
                        vector3  ideal_linear_velocity_in_world_space, ideal_angular_velocity_in_world_space;
                        ideal_velocity_buider.close(
                                motion.frame,
                                ideal_linear_velocity_in_world_space,
                                ideal_angular_velocity_in_world_space
                                );
                        ideal_velocity_buider.reset(cursor);
                        execute_satisfied_motion_guarded_actions(
                                actions,
                                integration_time,
                                ideal_linear_velocity_in_world_space,
                                ideal_angular_velocity_in_world_space,
                                vector3_zero(),
                                m_motion_desire_props,
                                m_motion_action_data,
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
                vector3 const&  current_origin = m_motion_object_motion.frame.origin();

                float_32_bit const  d_pos = length(current_origin + consumed_time * m_motion_desire_props.linear_speed * m_motion_desire_props.linear_velocity_unit_direction_in_world_space - motion.frame.origin());
                float_32_bit const  d_fwd = angle(m_motion_desire_props.forward_unit_vector_in_world_space, motion.forward) / PI();
                
                cost = 1.0f * d_pos + 2.0f * d_fwd;
            }

            if (cost < best_cost)
            {
                best_cost = cost;
                best_index = satisfied_transitions.at(i);
            }
        }
    }

    return  successors.at(best_index);
}


}
