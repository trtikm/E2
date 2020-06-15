#include <angeo/skeleton_kinematics.hpp>
#include <angeo/collide.hpp>
#include <angeo/utility.hpp>
#include <angeo/tensor_hash.hpp>
#include <angeo/tensor_equal_to.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <unordered_set>

namespace angeo {


void  skeleton_setup_joint_states_from_default_pose_frame(
        joint_rotation_state_vector&  joint_states,
        joint_rotation_props_vector const&  joint_defeintion,
        coordinate_system const&  pose_frame
        )
{
    TMPROF_BLOCK();

    joint_states.clear();

    coordinate_system const  identity_frame = { vector3_zero(), quaternion_identity() };
    joint_rotation_state  state;
    state.from_bone_to_world_space_matrix = matrix44_identity();
    for (natural_32_bit  i = 0U, n = (natural_32_bit)joint_defeintion.size(); i < n; ++i)
    {
        joint_rotation_props const&  props = joint_defeintion.at(i);
        state.frame = i == 0 ? pose_frame : identity_frame;
        state.current_angle = compute_rotation_angle(
                props.axis,
                props.zero_angle_direction,                    
                i == 0 ? vector3_from_coordinate_system(props.direction, state.frame) : props.direction
                );
        joint_states.push_back(state);
    }
}


void  skeleton_commit_joint_states_to_frame(coordinate_system& output_frame, joint_rotation_state_vector const& joint_states)
{
    TMPROF_BLOCK();

    from_base_matrix(joint_states.front().frame, joint_states.front().from_bone_to_world_space_matrix);
    for (natural_32_bit i = 1U, n = (natural_32_bit)joint_states.size(); i != n; ++i)
    {
        matrix44  F;
        from_base_matrix(joint_states.at(i).frame, F);
        joint_states.at(i).from_bone_to_world_space_matrix = joint_states.at(i - 1U).from_bone_to_world_space_matrix * F;
    }
    vector3  S;
    matrix33  R;
    decompose_matrix44(joint_states.back().from_bone_to_world_space_matrix, S, R);
    output_frame.set_origin(S);
    output_frame.set_orientation(rotation_matrix_to_quaternion(R));
}


void  skeleton_commit_joint_states_to_frames(
        std::unordered_map<natural_32_bit, coordinate_system*>&  output_frames,
        joint_rotation_states_of_bones const&  joint_states
        )
{
    TMPROF_BLOCK();

    for (auto const&  bone_and_states : joint_states)
    {
        auto const  it = output_frames.find(bone_and_states.first);
        if (it != output_frames.end())
            skeleton_commit_joint_states_to_frame(*it->second, bone_and_states.second);
    }
}


void  skeleton_add_joint_angle_deltas(joint_angle_deltas_vector&  angle_deltas, joint_angle_deltas_vector const&  added_angle_deltas)
{
    for (natural_32_bit i = 0U, n = (natural_32_bit)angle_deltas.size(); i != n; ++i)
        angle_deltas.at(i) += added_angle_deltas.at(i);
}


void  skeleton_add_joint_angle_deltas(
        joint_angle_deltas_of_bones&  angle_deltas,
        joint_angle_deltas_of_bones const&  added_angle_deltas
        )
{
    for (auto const& bone_and_deltas : added_angle_deltas)
    {
        auto const  it = angle_deltas.find(bone_and_deltas.first);
        if (it == angle_deltas.end())
            angle_deltas[bone_and_deltas.first] = bone_and_deltas.second;
        else
            skeleton_add_joint_angle_deltas(it->second, bone_and_deltas.second);
    }
}


void  skeleton_scale_angle_deltas(joint_angle_deltas_vector&  angle_deltas, float_32_bit const  scale)
{
    for (float_32_bit&  angle_delta : angle_deltas)
        angle_delta *= scale;
}


void  skeleton_scale_joint_angle_deltas(joint_angle_deltas_of_bones&  angle_deltas, float_32_bit const  scale)
{
    for (auto&  bone_and_deltas : angle_deltas)
        skeleton_scale_angle_deltas(bone_and_deltas.second, scale);
}


void  skeleton_increment_bone_count(std::unordered_map<natural_32_bit, natural_32_bit>&  bone_counts, natural_32_bit const  bone)
{
    auto const  it = bone_counts.find(bone);
    if (it == bone_counts.end())
        bone_counts.insert({ bone, 1U });
    else
        ++it->second;
}


void  skeleton_average_joint_angle_deltas(
        joint_angle_deltas_of_bones&  averaged_angle_deltas,
        std::vector<joint_angle_deltas_of_bones> const&  angle_deltas,
        std::unordered_map<natural_32_bit, natural_32_bit> const* const  bone_counts
        )
{
    TMPROF_BLOCK();

    if (angle_deltas.empty())
        return;

    averaged_angle_deltas = angle_deltas.front();

    if (angle_deltas.size() == 1UL)
        return;

    for (auto it = std::next(angle_deltas.cbegin()); it != angle_deltas.cend(); ++it)
        skeleton_add_joint_angle_deltas(averaged_angle_deltas, *it);
    if (bone_counts == nullptr)
        skeleton_scale_joint_angle_deltas(averaged_angle_deltas, (float_32_bit)angle_deltas.size());
    else
        for (auto const&  bone_and_count : *bone_counts)
            if (bone_and_count.second > 1U)
                skeleton_scale_angle_deltas(averaged_angle_deltas.at(bone_and_count.first), 1.0f / (float_32_bit)bone_and_count.second);
}


void  skeleton_interpolate_joint_rotation_states(
        joint_angle_deltas_vector&  angle_deltas,
        joint_rotation_state_vector const&  src_states,
        joint_rotation_state_vector const&  dst_states,
        joint_rotation_props_vector const&  joint_definitions,
        float_32_bit const  time_step_in_seconds
        )
{
    INVARIANT(src_states.size() == dst_states.size() && src_states.size() == joint_definitions.size() && time_step_in_seconds >= 0.0f);
    angle_deltas.resize(joint_definitions.size());
    for (natural_32_bit i = 0U, n = (natural_32_bit)joint_definitions.size(); i != n; ++i)
    {
        float_32_bit const  max_angle_delta = joint_definitions.at(i).max_angular_speed * time_step_in_seconds;
        float_32_bit const  angle_delta = dst_states.at(i).current_angle - src_states.at(i).current_angle;
        if (angle_delta < 0.0f)
            angle_deltas[i] = std::max(-max_angle_delta, angle_delta);
        else
            angle_deltas[i] = std::min( max_angle_delta, angle_delta);
    }
}


void  skeleton_interpolate_joint_rotation_states(
        joint_angle_deltas_of_bones&  angle_deltas,
        joint_rotation_states_of_bones const&  src_states,
        joint_rotation_states_of_bones const&  dst_states,
        joint_rotation_props_of_bones const&  joint_definitions,
        float_32_bit const  time_step_in_seconds
        )
{
    for (auto&  bone_and_state : src_states)
    {
        auto const  dst_it = dst_states.find(bone_and_state.first);
        if (dst_it != dst_states.end())
            skeleton_interpolate_joint_rotation_states(
                    angle_deltas[bone_and_state.first],
                    bone_and_state.second,
                    dst_it->second,
                    joint_definitions.at(bone_and_state.first),
                    time_step_in_seconds
                    );
    }
}


void  skeleton_apply_angle_deltas(
        joint_rotation_state_vector&  joint_states,
        joint_angle_deltas_vector const&  angle_deltas,
        joint_rotation_props_vector const&  joint_definitions
        )
{
    INVARIANT(joint_states.size() == angle_deltas.size() && angle_deltas.size() == joint_definitions.size());
    for (natural_32_bit i = 0U, n = (natural_32_bit)joint_definitions.size(); i != n; ++i)
    {
        joint_rotation_state&  state = joint_states.at(i);
        state.current_angle += angle_deltas.at(i);
        rotate(state.frame, angle_axis_to_quaternion(angle_deltas.at(i), joint_definitions.at(i).axis));
    }
}


void  skeleton_apply_angle_deltas(
        joint_rotation_states_of_bones&  joint_states,
        joint_angle_deltas_of_bones const&  angle_deltas,
        joint_rotation_props_of_bones const&  joint_definitions
        )
{
    TMPROF_BLOCK();

    for (auto&  bone_and_state : joint_states)
    {
        auto const  deltas_it = angle_deltas.find(bone_and_state.first);
        if (deltas_it != angle_deltas.end())
            skeleton_apply_angle_deltas(
                    bone_and_state.second,
                    deltas_it->second,
                    joint_definitions.at(bone_and_state.first)
                    );
    }
}


skeleton_kinematics_of_chain_of_bones::skeleton_kinematics_of_chain_of_bones(
        std::unordered_map<natural_32_bit, coordinate_system const*> const&  pose_frames,
        std::unordered_set<natural_32_bit> const&  bones_of_the_chain,
        natural_32_bit const  end_effector_bone_,
        joint_rotation_props_of_bones const&  rotation_props, 
        std::vector<integer_32_bit> const&  parent_bones_,
        std::vector<float_32_bit> const&  bone_lengths_
        )
    : end_effector_bone(end_effector_bone_)
    , root_bone()
    , parent_bones()
    , bone_lengths()
    , child_bones()
    , joint_definitions()
    , joint_states()
{
    TMPROF_BLOCK();

    for (natural_32_bit  prev_bone = std::numeric_limits<natural_32_bit>::max(), bone = end_effector_bone;
         bones_of_the_chain.count(bone) != 0ULL;
         prev_bone = bone, bone = (natural_32_bit)parent_bones_.at(bone))
    {
        root_bone = bone;
        parent_bones.insert({ bone, (natural_32_bit)parent_bones_.at(bone) });
        bone_lengths.insert({ bone, bone_lengths_.at(bone) });
        child_bones.insert({ bone, prev_bone });
        joint_rotation_props_vector const&  joint_props = joint_definitions.insert({ bone, rotation_props.at(bone) }).first->second;
        ASSUMPTION(!joint_props.empty());
        skeleton_setup_joint_states_from_default_pose_frame(joint_states[bone], joint_props, *pose_frames.at(bone));
    }
    INVARIANT(!child_bones.empty());
}


void  skeleton_kinematics_of_chain_of_bones::update_world_matrices_of_all_joint_current_states() const
{
    TMPROF_BLOCK();

    matrix44 const*  last_computed = nullptr;
    for (auto  it = child_bones.find(root_bone); it != child_bones.end(); it = child_bones.find(it->second))
        for (joint_rotation_state const&  state : joint_states.at(it->first))
        {
            from_base_matrix(state.frame, state.from_bone_to_world_space_matrix);
            if (last_computed != nullptr)
                state.from_bone_to_world_space_matrix = (*last_computed) * state.from_bone_to_world_space_matrix;
            last_computed = &state.from_bone_to_world_space_matrix;
        }
}


joint_rotation_state const*  skeleton_kinematics_of_chain_of_bones::find_predecessor_joint_state(
        natural_32_bit const  bone,
        natural_32_bit const  state_index) const
{
    if (state_index == 0U)
    {
        auto const  it = joint_states.find(parent_bones.at(bone));
        return it == joint_states.end() ? nullptr : &it->second.back();
    }
    else
        return &joint_states.at(bone).at(state_index - 1U);
}


matrix44 const&  skeleton_kinematics_of_chain_of_bones::get_world_matrix_of_predecessor_joint_state(
        natural_32_bit const  bone,
        natural_32_bit const  state_index) const
{
    static matrix44 const  default_world_matrix = matrix44_identity();
    joint_rotation_state const* const  predecessor_state_ptr = find_predecessor_joint_state(bone, state_index);
    return predecessor_state_ptr == nullptr ? default_world_matrix : predecessor_state_ptr->from_bone_to_world_space_matrix;
}


struct  skeleton_ik_constraint_system_look_at
{
    struct  joint_ik_info
    {
        vector3  coefs;
        float_32_bit  min_angle_delta;
        float_32_bit  max_angle_delta;
    };

    using  left_hand_side = std::unordered_map<
            // Index of a bone in the chain.
            natural_32_bit,
            // For each rotational degree of freedom of the joint between the bone and its direct parent bone
            // there is one record in the vector.
            std::vector<joint_ik_info>
            >;

    float_32_bit  rhs;
    left_hand_side  lhs;
};


void  skeleton_setup_ik_system_look_at(
        skeleton_ik_constraint_system_look_at&  ik_system,
        skeleton_kinematics_of_chain_of_bones const&  kinematics,
        look_at_target const&  look_target
        )
{
    TMPROF_BLOCK();

    kinematics.update_world_matrices_of_all_joint_current_states();

    matrix44 const&  W = kinematics.joint_states.at(kinematics.end_effector_bone).back().from_bone_to_world_space_matrix;

    vector3 const  current_dir = transform_vector(look_target.direction, W);
    vector3 const  target_dir = look_target.target - transform_point(vector3_zero(), W);
    vector3 const  axis = cross_product(current_dir, target_dir);
    float_32_bit const  axis_length = length(axis);
    float_32_bit const  angle_to_reduce = angle(current_dir, target_dir);

    vector3  direction;
    vector3  tangent;
    vector3  bitangent;
    if (axis_length < 0.001f || angle_to_reduce < 0.001f)
    {
        ik_system.rhs = 0.0f;
        direction = tangent = bitangent = vector3_zero();
    }
    else
    {
        ik_system.rhs = angle_to_reduce;
        direction = (1.0f / axis_length) * axis;
        compute_tangent_space_of_unit_vector(
                direction,
                tangent,
                bitangent
                );
    }

    for (auto  it = kinematics.parent_bones.find(kinematics.end_effector_bone);
            it != kinematics.parent_bones.end();
            it = kinematics.parent_bones.find(it->second))
    {
        std::vector<joint_rotation_props> const&  joint_definitions = kinematics.joint_definitions.at(it->first);

        std::vector<skeleton_ik_constraint_system_look_at::joint_ik_info>&  joint_ik_infos = ik_system.lhs[it->first];
        joint_ik_infos.clear();
        joint_ik_infos.reserve(joint_definitions.size());
        for (natural_32_bit i = 0U, n = (natural_32_bit)joint_definitions.size(); i != n; ++i)
        {
            joint_rotation_props const&  joint_definition = joint_definitions.at(i);
            joint_rotation_state const&  joint_state = kinematics.joint_states.at(it->first).at(i);

            skeleton_ik_constraint_system_look_at::joint_ik_info  jiki;
            vector3 const  velocity = transform_vector(joint_definition.axis,
                kinematics.get_world_matrix_of_predecessor_joint_state(it->first, i));
            jiki.coefs = {
                dot_product(direction, velocity),
                dot_product(tangent, velocity),
                dot_product(bitangent, velocity)
            };
            jiki.min_angle_delta = -0.5f * joint_definition.max_angle - joint_state.current_angle;
            jiki.max_angle_delta =  0.5f * joint_definition.max_angle - joint_state.current_angle;

            joint_ik_infos.push_back(jiki);
        }
    }
}


void  skeleton_solve_ik_system_look_at(
        joint_angle_deltas_of_bones&  solution,
        skeleton_ik_constraint_system_look_at const&  ik_system,
        natural_32_bit const  max_iterations,
        std::unordered_set<natural_32_bit> const&  locked_bones
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(solution.empty() && max_iterations > 0U);
    vector3 const  rhs_goal = ik_system.rhs * vector3_unit_x();
    vector3  rhs = vector3_zero();
    float_32_bit const  alpha = 1.0f / ((float_32_bit)max_iterations);
    for (natural_32_bit  iteration_index = 0U; iteration_index != max_iterations; ++iteration_index)
        for (auto&  bone_and_ik_infos : ik_system.lhs)
        {
            joint_angle_deltas_vector&  joint_angle_deltas = solution[bone_and_ik_infos.first];
            joint_angle_deltas.resize(bone_and_ik_infos.second.size(), 0.0f);
            if (locked_bones.count(bone_and_ik_infos.first) != 0UL)
                continue;
            for (natural_32_bit  i = 0U, n = (natural_32_bit)bone_and_ik_infos.second.size(); i != n; ++i)
            {
                skeleton_ik_constraint_system_look_at::joint_ik_info const&  joint_ik_info = bone_and_ik_infos.second.at(i);
                float_32_bit&  joint_angle_delta = joint_angle_deltas.at(i);

                float_32_bit const  angle_delta_scaled = alpha * closest_point_on_ray_to_point(
                        rhs,
                        joint_ik_info.coefs,
                        rhs_goal,
                        joint_ik_info.min_angle_delta - joint_angle_delta,
                        joint_ik_info.max_angle_delta - joint_angle_delta,
                        nullptr
                        );
                joint_angle_delta += angle_delta_scaled;
                rhs += angle_delta_scaled * joint_ik_info.coefs;
            }
        }
}


void  skeleton_look_at_iteration(
        std::vector<skeleton_kinematics_of_chain_of_bones>&  kinematics,
        std::vector<look_at_target> const&  look_at_targets,
        natural_32_bit const  max_ik_solver_iterations,
        std::unordered_set<natural_32_bit> const&  locked_bones,
        std::unordered_map<natural_32_bit, natural_32_bit> const* const  bone_counts
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(kinematics.size() == look_at_targets.size());
    std::vector<joint_angle_deltas_of_bones>  angle_deltas_per_target;
    for (natural_32_bit i = 0U, n = (natural_32_bit)kinematics.size(); i != n; ++i)
    {
        skeleton_kinematics_of_chain_of_bones&  kinematic = kinematics.at(i);
        look_at_target const&  target = look_at_targets.at(i);

        skeleton_ik_constraint_system_look_at  ik_system;
        skeleton_setup_ik_system_look_at(ik_system, kinematic, target);

        angle_deltas_per_target.push_back({});
        skeleton_solve_ik_system_look_at(angle_deltas_per_target.back(), ik_system, max_ik_solver_iterations, locked_bones);
    }

    joint_angle_deltas_of_bones  angle_deltas;
    skeleton_average_joint_angle_deltas(angle_deltas, angle_deltas_per_target, bone_counts);

    for (skeleton_kinematics_of_chain_of_bones& kinematic : kinematics)
        kinematic.apply_angle_deltas(angle_deltas);
}


void  skeleton_look_at(
        std::vector<skeleton_kinematics_of_chain_of_bones>&  kinematics,
        std::vector< look_at_target> const&  look_at_targets,
        natural_32_bit const  max_iterations,
        natural_32_bit const  max_ik_solver_iterations
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(max_iterations > 0U);

    std::unordered_map<natural_32_bit, natural_32_bit>  bone_counts;
    for (skeleton_kinematics_of_chain_of_bones const&  kinematic : kinematics)
        skeleton_add_bone_counts(bone_counts, kinematic.joint_states);

    std::unordered_set<natural_32_bit> locked_bones;

    for (natural_32_bit  i = 0U; i != max_iterations; ++i)
        skeleton_look_at_iteration(kinematics, look_at_targets, max_ik_solver_iterations, locked_bones, &bone_counts);

    for (auto const&  bone_and_count : bone_counts)
        if (bone_and_count.second > 1U)
            locked_bones.insert(bone_and_count.first);
    skeleton_look_at_iteration(kinematics, look_at_targets, max_ik_solver_iterations, locked_bones, &bone_counts);
}


struct  skeleton_ik_constraint_system_aim_at
{
    struct  joint_ik_info
    {
        vector3  coefs;
        float_32_bit  min_angle_delta;
        float_32_bit  max_angle_delta;
    };

    using  left_hand_side = std::vector<std::pair<natural_32_bit, std::vector<joint_ik_info> > >;

    vector3  rhs;
    left_hand_side  lhs;
};


void  skeleton_setup_ik_system_aim_at(
        skeleton_ik_constraint_system_aim_at&  ik_system,
        skeleton_kinematics_of_chain_of_bones const&  kinematics,
        aim_at_target const&  aim_target
        )
{
    TMPROF_BLOCK();

    kinematics.update_world_matrices_of_all_joint_current_states();

    float_32_bit  chain_length = length(aim_target.source);
    for (auto const&  bone_and_length : kinematics.bone_lengths)
        if (bone_and_length.first != kinematics.end_effector_bone)
            chain_length += bone_and_length.second;

    vector3 const  chain_root_origin = kinematics.joint_states.at(kinematics.root_bone).front().frame.origin();
    vector3  target_dir = aim_target.target - chain_root_origin;
    float_32_bit const  target_dir_len = length(target_dir);

    vector3 const  target = target_dir_len > chain_length ? 
            chain_root_origin + (chain_length / target_dir_len) * target_dir :
            aim_target.target
            ;
    vector3 const  source = transform_point(
            aim_target.source,
            kinematics.joint_states.at(kinematics.end_effector_bone).back().from_bone_to_world_space_matrix
            );
    vector3  direction = target - source;
    float_32_bit const  distance = length(direction);
    vector3  tangent, bitangent;
    if (distance < 0.001f)
        ik_system.rhs = direction = tangent = bitangent = vector3_zero();
    else
    {
        ik_system.rhs = vector3{ distance, 0.0f, 0.0f };
        direction /= distance;
        compute_tangent_space_of_unit_vector(direction, tangent, bitangent);
    }

    std::unordered_map<natural_32_bit, vector3>  bone_origins_in_world_space{
        { kinematics.root_bone, chain_root_origin }
    };
    std::unordered_map<natural_32_bit, vector3>  bone_tails_in_world_space{
        { kinematics.end_effector_bone, source }
    };
    for (natural_32_bit  prev_bone = kinematics.root_bone, bone = kinematics.child_bones.find(prev_bone)->second;
         prev_bone != kinematics.end_effector_bone;
         prev_bone = bone, bone = kinematics.child_bones.find(bone)->second)
    {
        matrix44 const&  W = kinematics.joint_states.at(bone).front().from_bone_to_world_space_matrix;
        vector3 const  pos = transform_point(vector3_zero(), W);
        bone_origins_in_world_space.insert({ bone, pos });
        bone_tails_in_world_space.insert({ prev_bone, pos });
    }
    
    ik_system.lhs.clear();
    for (auto  it = kinematics.parent_bones.find(kinematics.end_effector_bone);
         it != kinematics.parent_bones.end();
         it = kinematics.parent_bones.find(it->second))
    {
        ik_system.lhs.push_back({ it->first, {} });
        std::vector<skeleton_ik_constraint_system_aim_at::joint_ik_info>&  joint_ik_infos = ik_system.lhs.back().second;

        std::vector<joint_rotation_props> const&  joint_definitions = kinematics.joint_definitions.at(it->first);

        vector3 const  radius_vector = bone_tails_in_world_space.at(it->first) - bone_origins_in_world_space.at(it->first);

        for (natural_32_bit  i = 0U, n = (natural_32_bit)joint_definitions.size(); i != n; ++i)
        {
            joint_rotation_props const&  joint_definition = joint_definitions.at(i);
            joint_rotation_state const&  joint_state = kinematics.joint_states.at(it->first).at(i);

            matrix44 const&  from_parent_bone_to_world = kinematics.get_world_matrix_of_predecessor_joint_state(it->first, i);
            vector3 const  axis_in_world_space = transform_vector(joint_definition.axis, from_parent_bone_to_world);

            skeleton_ik_constraint_system_aim_at::joint_ik_info  jiki;
            vector3 const  velocity = cross_product(axis_in_world_space, radius_vector);
            jiki.coefs = { dot_product(direction, velocity), dot_product(tangent, velocity), dot_product(bitangent, velocity) };
            float_32_bit const  MAX_ANGLE_DELTA = PI() / 10.0f;
            jiki.min_angle_delta = std::max(-MAX_ANGLE_DELTA, -0.5f * joint_definition.max_angle - joint_state.current_angle);
            jiki.max_angle_delta = std::min( MAX_ANGLE_DELTA, 0.5f * joint_definition.max_angle - joint_state.current_angle);

            joint_ik_infos.push_back(jiki);
        }
    }
}


void  skeleton_solve_ik_system_aim_at(
        joint_angle_deltas_of_bones&  solution,
        skeleton_ik_constraint_system_aim_at&  ik_system,
        natural_32_bit const  max_ik_solver_iterations
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(solution.empty() && max_ik_solver_iterations > 0U);
    vector3  rhs = vector3_zero();
    float_32_bit const  alpha = 1.0f / ((float_32_bit)max_ik_solver_iterations);
    for (natural_32_bit  iteration_index = 0U; iteration_index != max_ik_solver_iterations; ++iteration_index)
        for (natural_32_bit  k = 0U, m = (natural_32_bit)ik_system.lhs.size(); k != m; ++k)
        {
            auto&  bone_and_ik_infos = ik_system.lhs.at(k);
            joint_angle_deltas_vector&  joint_angle_deltas = solution[bone_and_ik_infos.first];
            joint_angle_deltas.resize(bone_and_ik_infos.second.size(), 0.0f);
            for (natural_32_bit  i = 0U, n = (natural_32_bit)bone_and_ik_infos.second.size(); i != n; ++i)
            {
                skeleton_ik_constraint_system_aim_at::joint_ik_info const&  joint_ik_info = bone_and_ik_infos.second.at(i);
                float_32_bit&  joint_angle_delta = joint_angle_deltas.at(i);

                float_32_bit const  angle_delta_scaled = alpha * closest_point_on_ray_to_point(
                        rhs,
                        joint_ik_info.coefs,
                        ik_system.rhs,
                        joint_ik_info.min_angle_delta - joint_angle_delta,
                        joint_ik_info.max_angle_delta - joint_angle_delta,
                        nullptr
                        );
                joint_angle_delta += angle_delta_scaled;
                rhs += angle_delta_scaled * joint_ik_info.coefs;
            }
        }
}


void  skeleton_aim_at_iteration(
        skeleton_kinematics_of_chain_of_bones&  kinematics,
        bone_aim_at_targets const&  aim_at_targets,
        natural_32_bit const  max_ik_solver_iterations
        )
{
    TMPROF_BLOCK();

    std::vector<joint_angle_deltas_of_bones>  angle_deltas_per_target;
    for (aim_at_target const&  target : aim_at_targets)
    {
        angle_deltas_per_target.push_back({});
        skeleton_ik_constraint_system_aim_at  ik_system;
        skeleton_setup_ik_system_aim_at(ik_system, kinematics, target);
        skeleton_solve_ik_system_aim_at(angle_deltas_per_target.back(), ik_system, max_ik_solver_iterations);
    }

    joint_angle_deltas_of_bones  angle_deltas;
    skeleton_average_joint_angle_deltas(angle_deltas, angle_deltas_per_target);

    kinematics.apply_angle_deltas(angle_deltas);
}


void  skeleton_aim_at(
        skeleton_kinematics_of_chain_of_bones&  kinematics,
        bone_aim_at_targets const&  aim_at_targets,
        natural_32_bit const  max_iterations,
        natural_32_bit const  max_ik_solver_iterations
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(max_iterations > 0U);
    for (natural_32_bit i = 0U; i != max_iterations; ++i)
        skeleton_aim_at_iteration(kinematics, aim_at_targets, max_ik_solver_iterations);
}


void  skeleton_compute_child_bones(std::vector<integer_32_bit> const&  parents, std::vector<std::vector<integer_32_bit> >&  children)
{
    children.clear();
    children.resize(parents.size());
    for (natural_32_bit i = 0U; i != parents.size(); ++i)
    {
        ASSUMPTION(parents.at(i) < (integer_32_bit)i);
        if (parents.at(i) >= 0)
            children.at(parents.at(i)).push_back(i);
    }
}


float_32_bit  compute_rotation_angle(vector3 const&  unit_axis, vector3 const&  current, vector3 const&  target)
{
    vector3 const  current_projected = subtract_projection_to_unit_vector(current, unit_axis);
    vector3 const  target_projected = subtract_projection_to_unit_vector(target, unit_axis);
    return compute_rotation_angle_of_projected_vector(
                    current_projected,
                    target_projected,
                    length(current_projected),
                    length(target_projected),
                    unit_axis
                    );
}


float_32_bit  compute_rotation_angle_of_projected_vector(
        vector3 const&  from,
        vector3 const&  to,
        float_32_bit const from_length,
        float_32_bit const to_length,
        vector3 const&  unit_axis
        )
{
    if (from_length < 1e-4f || to_length < 1e-4f)
        return 0.0f;
    float_32_bit const  denominator = from_length * to_length;
    float_32_bit const  cos_angle = dot_product(from, to) / denominator;
    float_32_bit const  angle = std::fabsf(cos_angle >= 1.0f ? 0.0f : cos_angle <= -1.0f ? PI() : std::acosf(cos_angle));
    return dot_product(cross_product(unit_axis, from / from_length), to / to_length) >= 0.0f ? angle : -angle;
}


float_32_bit  compute_rotation_axis_and_angle(vector3&  output_unit_axis, vector3 const&  current, vector3 const&  target)
{
    output_unit_axis = cross_product(current, target);
    float_32_bit  axis_len = length(output_unit_axis);
    if (axis_len < 0.0001f)
    {
        float_32_bit const  curr_len = length(current);
        if (curr_len < 0.0001f || dot_product(current, target) > -0.00001f)
        {
            output_unit_axis = vector3_unit_z();
            return 0.0f;
        }
        vector3  tmp;
        compute_tangent_space_of_unit_vector((1.0f / curr_len) * current, output_unit_axis, tmp);
        return PI();
    }
    output_unit_axis /= axis_len;
    return angle(current, target);
}


vector3  compute_common_look_at_target_for_multiple_eyes(
        std::vector<std::pair<vector3, vector3> > const&  vector_of_eye_pos_and_eye_unit_dir_pairs
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(vector_of_eye_pos_and_eye_unit_dir_pairs.size() > 1UL);

    natural_32_bit  num_candidates = 0U;
    vector3  target = vector3_zero();
    for (natural_32_bit  i = 0U; i < vector_of_eye_pos_and_eye_unit_dir_pairs.size(); ++i)
    {
        std::pair<vector3, vector3> const&  pos_and_dir_i = vector_of_eye_pos_and_eye_unit_dir_pairs.at(i);
        vector3 const&  P_i = pos_and_dir_i.first;
        vector3 const&  d_i = pos_and_dir_i.second;
        for (natural_32_bit j = i + 1U; j < vector_of_eye_pos_and_eye_unit_dir_pairs.size(); ++j)
        {
            std::pair<vector3, vector3> const& pos_and_dir_j = vector_of_eye_pos_and_eye_unit_dir_pairs.at(j);
            vector3 const& P_j = pos_and_dir_j.first;
            vector3 const& d_j = pos_and_dir_j.second;

            vector3 const  S = 0.5f * (P_i + P_j);
            vector3 const  w = 0.5f * (d_i + d_j);
            float_32_bit const  d = 0.5f * (length(subtract_projection_to_unit_vector(P_i, w)) +
                                            length(subtract_projection_to_unit_vector(P_j, w)));
            float_32_bit const  a = 0.5f * (angle(d_i, w) + angle(d_j, w));
            float_32_bit const  t = std::fabsf(a) < 0.001f || std::fabsf(a) > PI() / 2.0 - 0.001f ? 1.0f : d / std::tanf(a);

            target += S + t * w;
            ++num_candidates;
        }
    }
    if (num_candidates > 1U)
        target = (1.0f / (float_32_bit)num_candidates) * target;
    return target;
}


}
