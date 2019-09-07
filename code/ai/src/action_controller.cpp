#include <ai/action_controller.hpp>
#include <ai/skeleton_utils.hpp>
#include <ai/detail/rigid_body_motion.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <queue>
#include <functional>

namespace ai { namespace detail { namespace {


//struct  skeletal_motion_template_walk_info
//{
//    float_32_bit  cost;
//    skeletal_motion_templates::motion_template_cursor  cursor;
//    rigid_body_motion  m_motion;
//    float_32_bit  consumed_time;
//
//    natural_32_bit  data_id;
//};


float_32_bit  distance_of_object_motion_to_desired_motion(
        rigid_body_motion const&  motion,
        vector3 const&  current_origin,
        float_32_bit const  time_step,
        motion_desire_props const&  desire
        )
{
    float_32_bit const  d_pos  = length(current_origin + time_step * desire.linear_speed * desire.linear_velocity_unit_direction_in_world_space - motion.frame.origin());
    float_32_bit const  d_fwd  = angle(desire.forward_unit_vector_in_world_space, motion.forward) / PI();
    //float_32_bit const  d_vlin = length(desire.linear_speed * desire.linear_velocity_unit_direction_in_world_space - motion.velocity.m_linear);
    //float_32_bit const  d_vang = length(desire.angular_speed * desire.angular_velocity_unit_axis_in_world_space - motion.velocity.m_angular);
    float_32_bit const  total_cost = 1.0f * d_pos + 2.0f * d_fwd;// +1.0f * d_vlin + 1.0f * d_vang;
//std::cout << "d_pos: " << d_pos << ", d_fwd: " << d_fwd  << ", total: "<< total_cost << std::endl; std::cout.flush();
    return total_cost;
}


void  extend_cursor_path(
        std::vector<skeletal_motion_templates::motion_template_cursor>&  cursor_path,
        skeletal_motion_templates::motion_template_cursor const&  cursor,
        float_32_bit const  transition_time
        )
{

    if ((cursor_path.size() & 1UL) == 0UL)
    {
        // Even number of cursors.

        if (cursor.motion_name == cursor_path.back().motion_name && cursor.keyframe_index > cursor_path.back().keyframe_index)
            cursor_path.back() = cursor;
        else
            cursor_path.push_back(cursor);
    }
    else
    {
        // Odd number of cursors.

        if (cursor.motion_name == cursor_path.back().motion_name && cursor.keyframe_index > cursor_path.back().keyframe_index)
            cursor_path.push_back(cursor);
        else if (transition_time < 0.001f)
            cursor_path.back() = cursor;
        else
        {
            if (cursor_path.back().keyframe_index > 0U)
                --cursor_path.back().keyframe_index;
            cursor_path.push_back(cursor_path.back());
            ++cursor_path.back().keyframe_index;
            cursor_path.push_back(cursor);
        }
    }
}


void  close_cursor_path(
        std::vector<skeletal_motion_templates::motion_template_cursor>&  cursor_path,
        skeletal_motion_templates const&  motion_templates)
{
    if ((cursor_path.size() & 1UL) != 0UL)
    {
        // Odd number of cursors.

        if (cursor_path.back().keyframe_index + 1U >= motion_templates.at(cursor_path.back().motion_name).keyframes.num_keyframes())
            --cursor_path.back().keyframe_index;
        cursor_path.push_back(cursor_path.back());
        ++cursor_path.back().keyframe_index;
    }
}


void  compute_ideal_linear_and_angular_velocity(
        std::vector<skeletal_motion_templates::motion_template_cursor> const&  cursor_path,
        skeletal_motion_templates const  motion_templates,
        angeo::coordinate_system const&  motion_object_frame,
        float_32_bit const  consumed_time,
        vector3&  ideal_linear_velocity_in_world_space,
        vector3&  ideal_angular_velocity_in_world_space
        )
{
    ASSUMPTION((cursor_path.size() & 1UL) == 0UL && consumed_time > 0.0001f);

    matrix44  A, M = matrix44_identity();
    for (auto rit = cursor_path.crbegin(); rit != cursor_path.crend(); ++rit)
    {
        angeo::from_base_matrix(motion_templates.at(rit->motion_name).reference_frames.at(rit->keyframe_index), A);
        M = A * M;
        ++rit;
        angeo::to_base_matrix(motion_templates.at(rit->motion_name).reference_frames.at(rit->keyframe_index), A);
        M = A * M;
    }
    vector3  pos;
    matrix33  rot;
    decompose_matrix44(M, pos, rot);

    angeo::from_base_matrix(motion_object_frame, A);

    ideal_linear_velocity_in_world_space = transform_vector(pos, A) / consumed_time;

    vector3  axis;
    float_32_bit const angle = quaternion_to_angle_axis(rotation_matrix_to_quaternion(rot), axis);
    ideal_angular_velocity_in_world_space = (angle / consumed_time) * transform_vector(axis, A);
}


}}}

namespace ai {


action_controller::intepolation_state::intepolation_state()
    : frames()
    , free_bones_look_at(nullptr)
    , collider(nullptr)
    , mass_distribution(nullptr)
    , disjunction_of_guarded_actions()
{}


action_controller::action_controller(blackboard_ptr const  blackboard_)
    : m_motion_desire_props()
    , m_motion_object_motion()
    , m_gravity_acceleration(vector3_zero())

    , m_blackboard(blackboard_)

    , m_total_interpolation_time_in_seconds(0.0f)
    , m_consumed_time_in_seconds(0.0f)

    , m_src_intepolation_state()
    , m_current_intepolation_state()

    , m_dst_cursor{ get_blackboard()->m_motion_templates.motions_map().begin()->first, 0U }
    , m_ideal_linear_velocity_in_world_space(vector3_zero())
    , m_ideal_angular_velocity_in_world_space(vector3_zero())

    , m_motion_action_data()
{
    TMPROF_BLOCK();

    auto const  collider = get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name).colliders
                                                                             .at(m_dst_cursor.keyframe_index);
    auto const  mass_distribution =
        get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name).mass_distributions
                                                          .at(m_dst_cursor.keyframe_index);

    angeo::coordinate_system  agent_frame;
    {
        angeo::coordinate_system  tmp;
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_bone_nids.front(), false, tmp);
        agent_frame.set_origin(tmp.origin());
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, tmp);
        agent_frame.set_orientation(tmp.orientation());
    }
    scene::node_id  nid(detail::get_motion_object_nid(blackboard_->m_scene, blackboard_->m_agent_nid));
    if (!get_blackboard()->m_scene->has_scene_node(nid))
        nid = detail::create_motion_scene_node(get_blackboard()->m_scene, nid, agent_frame, collider, *mass_distribution);

    m_motion_object_motion = detail::rigid_body_motion(get_blackboard()->m_scene, nid, get_blackboard()->m_motion_templates.directions());
    m_gravity_acceleration = get_blackboard()->m_scene->get_gravity_acceleration_at_point(m_motion_object_motion.frame.origin());

    get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_agent_id);

    angeo::coordinate_system  motion_object_frame;
    get_blackboard()->m_scene->get_frame_of_scene_node(m_motion_object_motion.nid, false, motion_object_frame);

    m_current_intepolation_state.frames =
        get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name).keyframes.get_keyframes()
                                                          .at(m_dst_cursor.keyframe_index).get_coord_systems();
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

        std::vector<skeletal_motion_templates::motion_template_cursor>  cursor_path{ m_dst_cursor };
        float_32_bit  consumed_time = 0.0f;
        while (m_consumed_time_in_seconds + interpolation_time_step_in_seconds >= m_total_interpolation_time_in_seconds)
        {
            std::pair<skeletal_motion_templates::motion_template_cursor, float_32_bit> const  best_transition = choose_transition();
            m_dst_cursor = best_transition.first;
            m_total_interpolation_time_in_seconds += best_transition.second;
            detail::extend_cursor_path(cursor_path, best_transition.first, best_transition.second);
            consumed_time += best_transition.second;
        }
        detail::close_cursor_path(cursor_path, get_blackboard()->m_motion_templates);
        detail::compute_ideal_linear_and_angular_velocity(
                cursor_path,
                get_blackboard()->m_motion_templates,
                m_motion_object_motion.frame,
                consumed_time,
                m_ideal_linear_velocity_in_world_space,
                m_ideal_angular_velocity_in_world_space
                );
    }

    m_consumed_time_in_seconds += interpolation_time_step_in_seconds;
    INVARIANT(m_consumed_time_in_seconds <= m_total_interpolation_time_in_seconds);

    float_32_bit const  interpolation_param = m_consumed_time_in_seconds / m_total_interpolation_time_in_seconds;

    interpolate(interpolation_param);
    //look_at_target(time_step_in_seconds, interpolation_param);

    // Next we update frames of scene nodes corresponding to the interpolated frames of bones.
    for (natural_32_bit bone = 0; bone != m_current_intepolation_state.frames.size(); ++bone)
    {
        auto const&  frame = m_current_intepolation_state.frames.at(bone);
        get_blackboard()->m_scene->set_frame_of_scene_node(
                get_blackboard()->m_bone_nids.at(bone),
                true,
                //m_current_intepolation_state.frames.at(bone)
                // We always have to add origins of pose bones to corresponding keyframe bones,
                // because keyframe bone positions are relative to the pose bone positions.
                // NOTE: Rotations are absolute (in the space of parent bone), so nothing to do for rotations.
                { frame.origin() + get_blackboard()->m_motion_templates.pose_frames().at(bone).origin(), frame.orientation() }
                );
    }

    // And finally, we update dynamics of agent's motion object (forces and torques).
    skeletal_motion_templates::guarded_actions_ptr const  satisfied_guarded_actions =
        detail::get_first_satisfied_motion_guarded_actions(
                m_current_intepolation_state.disjunction_of_guarded_actions,
                get_blackboard()->m_collision_contacts,
                m_motion_object_motion,
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
    // TODO: include also angular speed to the computation.

    float_32_bit const  real_linear_speed = length(m_motion_object_motion.velocity.m_linear);
    float_32_bit const  ideal_linear_speed = length(m_ideal_linear_velocity_in_world_space);
    //float_32_bit const  real_angular_speed = length(m_motion_object_motion.velocity.m_angular);
    //float_32_bit const  ideal_angular_speed = length(m_ideal_angular_velocity_in_world_space);

    float_32_bit const  linear_interpolantion_speed =
            ideal_linear_speed < 0.0001f ? 1.0f + real_linear_speed : real_linear_speed / ideal_linear_speed
            ;
    float_32_bit const  angular_interpolantion_speed =
            1.0f
            //ideal_angular_speed < 0.0001f ? 1.0f + real_angular_speed : real_angular_speed / ideal_angular_speed
            ;

    float_32_bit const  raw_interpolantion_speed =
            std::fabs(linear_interpolantion_speed - 1.0f) >= std::fabs(angular_interpolantion_speed - 1.0f) ?
                    linear_interpolantion_speed :
                    angular_interpolantion_speed
                    ;

    return std::max(0.5f, std::min(raw_interpolantion_speed, 1.5f));
}


void  action_controller::interpolate(float_32_bit const  interpolation_param)
{
    TMPROF_BLOCK();

    skeletal_motion_templates::motion_template const&  dst_motion_template =
            get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name);

    interpolate_keyframes_spherical(
            m_src_intepolation_state.frames,
            dst_motion_template.keyframes.keyframe_at(m_dst_cursor.keyframe_index).get_coord_systems(),
            interpolation_param,
            m_current_intepolation_state.frames
            );

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
    if (*interpolated_collider != *m_current_intepolation_state.collider || interpolated_mass_distribution != m_current_intepolation_state.mass_distribution)
    {
        get_blackboard()->m_scene->unregister_to_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_agent_id);
        detail::destroy_collider_and_rigid_bofy_of_motion_scene_node(get_blackboard()->m_scene, m_motion_object_motion.nid);
        detail::create_collider_and_rigid_body_of_motion_scene_node(
                get_blackboard()->m_scene,
                m_motion_object_motion.nid,
                m_src_intepolation_state.collider,
                m_motion_object_motion
                );
        get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_agent_id);
    }

    m_current_intepolation_state.collider = interpolated_collider;
    m_current_intepolation_state.mass_distribution = interpolated_mass_distribution;

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
        get_blackboard()->m_motion_templates.joints().data(),
        bones_to_rotate,
        time_step_in_seconds
    );

    // And write results to the vector 'm_current_intepolation_state.frames' of final frames.

    float_32_bit const  src_param = m_current_intepolation_state.free_bones_look_at->all_bones.empty() ? 0.0f : 1.0f;
    float_32_bit const  dst_param = dst_look_at_bones->all_bones.empty() ? 0.0f : 1.0f;
    float_32_bit const  param = src_param + interpolation_param * (dst_param - src_param);
    for (auto bone : bones_to_consider)
    {
        auto const&  frame = frames.at(bone);
        angeo::interpolate_spherical(
                m_current_intepolation_state.frames.at(bone),
                // We have to subtract origins of pose bones from look-at frames in order to get them
                // to the same coordinates as frames in 'm_current_intepolation_state'.
                { frame.origin() - get_blackboard()->m_motion_templates.pose_frames().at(bone).origin(), frame.orientation() },
                param,
                m_current_intepolation_state.frames.at(bone)
                );
    }
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
                        m_gravity_acceleration
                        );
        if (satisfied_guarded_actions != nullptr)
            satisfied_transitions.push_back(i);
    }
    if (satisfied_transitions.size() == 1UL)
        return successors.at(satisfied_transitions.front());

//if (m_motion_desire_props.linear_speed > 0.1f)
//{
//    int iii = 0;
//}

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
                std::vector<skeletal_motion_templates::motion_template_cursor>  cursor_path{ cursor };
                float_32_bit  integration_time = 0.0f;
                while (consumed_time < time_horizon)
                {
                    get_blackboard()->m_motion_templates.for_each_successor_keyframe(
                            cursor,
                            [&cursor, &cursor_path, &consumed_time, &integration_time]
                            (skeletal_motion_templates::motion_template_cursor const&  target_cursor, float_32_bit const  transition_time)
                            {
                                cursor = target_cursor;
                                consumed_time += transition_time;
                                integration_time += transition_time;
                                detail::extend_cursor_path(cursor_path, target_cursor, transition_time);
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
                        detail::close_cursor_path(cursor_path, get_blackboard()->m_motion_templates);
                        detail::compute_ideal_linear_and_angular_velocity(
                                cursor_path,
                                get_blackboard()->m_motion_templates,
                                motion.frame,
                                integration_time,
                                ideal_linear_velocity_in_world_space,
                                ideal_angular_velocity_in_world_space
                                );
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
                        cursor_path.clear();
                        cursor_path.push_back(cursor);
                    }
                }
            }

//std::cout << successors.at(satisfied_transitions.at(i)).first.motion_name << ": ";

            float_32_bit const  cost =
                    detail::distance_of_object_motion_to_desired_motion(
                            motion,
                            m_motion_object_motion.frame.origin(),
                            time_horizon,
                            m_motion_desire_props);

//std::cout << "    " << " origin: " << motion.frame.origin()(0) << ", "
//                                   << motion.frame.origin()(1) << ", "
//                                   << motion.frame.origin()(2) << std::endl; std::cout.flush();
//std::cout << "    " << " forward: " << motion.forward(0) << ", "
//                                    << motion.forward(1) << ", "
//                                    << motion.forward(2) << std::endl; std::cout.flush();

            if (cost < best_cost)
            {
                best_cost = cost;
                best_index = satisfied_transitions.at(i);
            }
        }

//std::cout << "current origin: " << m_motion_object_motion.frame.origin()(0) << ", "
//                                << m_motion_object_motion.frame.origin()(1) << ", "
//                                << m_motion_object_motion.frame.origin()(2) << std::endl; std::cout.flush();
//vector3  dst_pos = m_motion_object_motion.frame.origin() + time_horizon * m_motion_desire_props.linear_speed * m_motion_desire_props.linear_velocity_unit_direction_in_world_space;
//std::cout << "target origin: " << dst_pos(0) << ", "
//                               << dst_pos(1) << ", "
//                               << dst_pos(2) << std::endl; std::cout.flush();
//std::cout << "target forward: " << m_motion_desire_props.forward_unit_vector_in_world_space(0) << ", "
//                                << m_motion_desire_props.forward_unit_vector_in_world_space(1) << ", "
//                                << m_motion_desire_props.forward_unit_vector_in_world_space(2) << std::endl; std::cout.flush();
//std::cout << "------------------------------------" << std::endl; std::cout.flush();
//if (successors.at(best_index).first.motion_name != "stand")
//{
//    int iii = 0;
//}
//if (successors.at(best_index).first.motion_name == "jump_onto")
//{
//    int iii = 0;
//}

    }


    return  successors.at(best_index);

    //natural_32_bit  best_data_id;
    //{
    //    best_data_id = std::numeric_limits<natural_32_bit>::max();
    //    float_32_bit  best_cost = std::numeric_limits<float_32_bit>::max();

    //    float_32_bit const  time_horizon = 1.0f;
    //    float_32_bit const  integration_time_step = 0.1f;

    //    std::priority_queue<
    //            detail::skeletal_motion_template_walk_info,
    //            std::vector<detail::skeletal_motion_template_walk_info>,
    //            std::function<bool(detail::skeletal_motion_template_walk_info const&, detail::skeletal_motion_template_walk_info const&)>
    //            >
    //        queue([](detail::skeletal_motion_template_walk_info const&  left, detail::skeletal_motion_template_walk_info const&  right) {
    //                // We need inverse order: obtain lower costs first.
    //                return left.cost > right.cost;
    //                });
    //    detail::rigid_body_motion  initial_motion = m_motion_object_motion;
    //    initial_motion.acceleration.m_linear = initial_motion.acceleration.m_angular = vector3_zero();
    //    float_32_bit const  initial_cost =
    //            detail::distance_of_object_motion_to_desired_motion(
    //                    initial_motion,
    //                    m_motion_object_motion.frame.origin(),
    //                    m_motion_desire_props);
    //    for (auto const i : satisfied_transitions)
    //        queue.push({ initial_cost, successors.at(i).first, initial_motion, 0.0f, i });
    //    do
    //    {
    //        detail::skeletal_motion_template_walk_info const  walk_info = queue.top();
    //        queue.pop();

    //        if (walk_info.consumed_time >= time_horizon)
    //        {
    //            if (walk_info.cost < best_cost)
    //            {
    //                best_cost = walk_info.cost;
    //                best_data_id = walk_info.data_id;
    //            }
    //            continue;
    //        }

    //        std::vector<std::pair<skeletal_motion_templates::motion_template_cursor, float_32_bit> > walk_successors;
    //        float_32_bit  consumed_time = 0.0f;
    //        {
    //            skeletal_motion_templates::motion_template_cursor  cursor = walk_info.cursor;
    //            while (true)
    //            {
    //                walk_successors.clear();
    //                get_blackboard()->m_motion_templates.get_successor_keyframes(cursor, walk_successors);
    //                if (walk_successors.size() > 1UL ||
    //                    consumed_time + walk_successors.front().second > integration_time_step ||
    //                    walk_info.consumed_time + consumed_time + walk_successors.front().second > time_horizon)
    //                    break;
    //                cursor = walk_successors.front().first;
    //                consumed_time += walk_successors.front().second;
    //            }
    //        }
    //        for (auto const&  cursor_and_time : walk_successors)
    //        {
    //            detail::motion_action_persistent_data_map  discardable_motion_action_data;
    //            std::vector<skeletal_motion_templates::action_ptr>  actions;
    //            for (auto const&  disjunction : get_blackboard()->m_motion_templates.motions_map().at(cursor_and_time.first.motion_name)
    //                                                                                              .actions
    //                                                                                              .at(cursor_and_time.first.keyframe_index))
    //                for (auto const  action_ptr : disjunction->actions)
    //                    actions.push_back(action_ptr);
    //            detail::skeletal_motion_template_walk_info  walk_succ_info = walk_info;
    //            walk_succ_info.consumed_time += consumed_time + cursor_and_time.second;
    //            walk_succ_info.cursor = cursor_and_time.first;
    //            execute_satisfied_motion_guarded_actions(
    //                    actions,
    //                    consumed_time + cursor_and_time.second,
    //                    m_current_intepolation_state.linear_velocity_in_world_space,
    //                    m_current_intepolation_state.angular_velocity_in_world_space,
    //                    vector3_zero(),
    //                    m_motion_desire_props,
    //                    m_motion_action_data,
    //                    walk_succ_info.m_motion,
    //                    discardable_motion_action_data
    //                    );
    //            walk_succ_info.m_motion.integrate(consumed_time + cursor_and_time.second);
    //            walk_succ_info.cost =
    //                    detail::distance_of_object_motion_to_desired_motion(
    //                            walk_succ_info.m_motion,
    //                            m_motion_object_motion.frame.origin(),
    //                            m_motion_desire_props);
    //            queue.push(walk_succ_info);
    //        }
    //    }
    //    while (!queue.empty());
    //}
    //return successors.at(best_data_id);
}


}
