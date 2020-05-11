#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <unordered_set>

namespace angeo { namespace detail {


inline vector3  project_vector_to_plane(vector3 const&  unit_plane_normal, vector3 const&  u, float_32_bit const  param_mult = 1.0f)
{
    return u - param_mult * project_to_unit_vector(u, unit_plane_normal);
}


inline vector3  vector_from_frame(vector3 const&  v, coordinate_system const&  frame)
{
    return quaternion_to_rotation_matrix(frame.orientation()) * v;
}


inline vector3  vector_to_frame(vector3 const&  v, coordinate_system const&  frame)
{
    return transpose33(quaternion_to_rotation_matrix(frame.orientation())) * v;
}


inline vector3  point_from_frame(vector3 const&  p, coordinate_system const&  frame)
{
    return frame.origin() + vector_from_frame(p, frame);
}


inline vector3  point_to_frame(vector3 const&  p, coordinate_system const&  frame)
{
    return vector_to_frame(p - frame.origin(), frame);
}


vector3  point_to_bone_frame(vector3 const&  p, matrix44 const&  to_parent_frame, coordinate_system const&  bone_frame)
{
    matrix44  B;
    to_base_matrix(bone_frame, B);
    return transform_point(p, B * to_parent_frame);
}


struct  skeleton_look_at_props
{
    std::vector<coordinate_system>*  frames;
    bone_look_at_targets const*  look_at_targets;
    std::vector<coordinate_system> const*  pose_frames;
    std::vector<integer_32_bit> const*  parents;
    std::vector<std::vector<joint_rotation_props> > const*  rotation_props;
};


}}

namespace angeo {


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


void  skeleton_look_at(
        std::vector<coordinate_system>&  output_frames,
        bone_look_at_targets const&  look_at_targets,
        std::vector<coordinate_system> const&  pose_frames,
        std::vector<integer_32_bit> const&  parents,
        std::vector<std::vector<joint_rotation_props> > const&  rotation_props,
        std::unordered_set<integer_32_bit> const&  bones_to_consider,
        std::unordered_map<integer_32_bit, std::vector<natural_32_bit> >* const  involved_rotations_of_bones,
        natural_32_bit const  max_iterations,
        float_32_bit const  angle_range_epsilon
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!pose_frames.empty() && pose_frames.size() == parents.size());
    ASSUMPTION(!look_at_targets.empty() && !bones_to_consider.empty() && max_iterations > 0U);
	ASSUMPTION(angle_range_epsilon > 0.0f);

    output_frames = pose_frames;

    detail::skeleton_look_at_props  props {
        &output_frames,
        &look_at_targets,
        &pose_frames,
        &parents,
        &rotation_props,
    };

    // Compute rotation angles for all inner bones, independently for each target.
    // (Note: later we compute averages to merge results for individual targets).
    std::unordered_map<integer_32_bit, std::vector<std::vector<float_32_bit> > > angles_for_inner_bones;
    for (auto const&  target_info : *props.look_at_targets)
    {
        std::unordered_map<integer_32_bit, std::vector<float_32_bit> >  angles;
        {
            for (integer_32_bit  bone = target_info.first; bones_to_consider.count(bone) != 0ULL; bone = props.parents->at(bone))
                angles[bone].resize(props.rotation_props->at(bone).size());

            std::vector<std::pair<vector2, vector2> >  angle_convergence_lines;
            {
                coordinate_system const&  frame = props.frames->at(target_info.first);

                vector3  to_target_vector;
                {
                    matrix44  from_world_to_parent_frame;
                    compute_to_bone_space_matrix(props.parents->at(target_info.first), *props.frames, *props.parents, bones_to_consider, from_world_to_parent_frame);
                    vector3 const  target_in_parent_bone = transform_point(target_info.second.second, from_world_to_parent_frame);
                    to_target_vector = target_in_parent_bone - frame.origin();
                }

                auto const&  rotations = props.rotation_props->at(target_info.first);
                for (natural_32_bit  i = 0U; i != rotations.size(); ++i)
                {
                    auto const& rot_props = rotations.at(i);
                    vector3 const  axis = rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, frame);
                    vector3 const  eye_vector = detail::vector_from_frame(target_info.second.first, frame);
                    float_32_bit const  angle = compute_rotation_angle(axis, eye_vector, to_target_vector);
                    float_32_bit const  angle_clipped = std::max(-0.5f * rot_props.m_max_angle, std::min(angle, 0.5f * rot_props.m_max_angle));
                    angle_convergence_lines.push_back({{ 0.0f, angle }, { angle_clipped, 0.0f }});
                }
            }

            for (natural_32_bit  iteration = 0U; iteration < max_iterations; ++iteration)
            {
                // 1. Copy candidate rotation angles of the target bone.
                {
                    auto const&  rotations = props.rotation_props->at(target_info.first);
                    auto&  target_angles = angles.at(target_info.first);
                    for (natural_32_bit  i = 0U; i != rotations.size(); ++i)
                        target_angles.at(i) = angle_convergence_lines.at(i).second(0);
                }

                // 2. Apply candidate rotation angles to the target bone.
                {
                    coordinate_system&  frame = props.frames->at(target_info.first);
                    frame = props.pose_frames->at(target_info.first);

                    auto const&  rotations = props.rotation_props->at(target_info.first);
                    auto const&  target_angles = angles.at(target_info.first);
                    for (natural_32_bit i = 0U; i != rotations.size(); ++i)
                    {
                        auto const&  rot_props = rotations.at(i);
                        vector3 const  axis = rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, frame);
            
                        rotate(frame, angle_axis_to_quaternion(target_angles.at(i), axis));
                    }
                }

                // 3. Compute rotation angles and apply them to frames of all inner bones (i.e. to all parents of the target bone).
                for (integer_32_bit  child_bone = target_info.first, bone = props.parents->at(target_info.first);
					 bones_to_consider.count(bone) != 0ULL;
                     child_bone = bone, bone = props.parents->at(bone))
                {
                    coordinate_system&  frame = props.frames->at(bone);
                    frame = props.pose_frames->at(bone);

                    auto const&  rotations = props.rotation_props->at(bone);
                    for (natural_32_bit i = 0U; i != rotations.size(); ++i)
                    {
                        auto const&  rot_props = rotations.at(i);

                        vector3 const  axis = rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, frame);

                        std::vector<float_32_bit> weights;
                        std::vector<float_32_bit> coefs;
                        {
                            float_32_bit  sum_weights = 0.0f;
                            auto const&  child_rotations = props.rotation_props->at(child_bone);
                            for (natural_32_bit j = 0U; j != child_rotations.size(); ++j)
                            {
                                auto const&  child_rot_props = child_rotations.at(j);
                                vector3 const  child_axis =
                                    child_rot_props.m_axis_in_parent_space ?
                                    child_rot_props.m_axis :
                                    detail::vector_from_frame(child_rot_props.m_axis, props.frames->at(child_bone))
                                    ;

                                float_32_bit const  weight = cos_angle(axis, detail::vector_from_frame(child_axis, frame));
                                weights.push_back(std::fabsf(weight));
                                sum_weights += weights.back();

                                float_32_bit  angle_ratio;
                                {
                                    angle_ratio = 2.0f * angles.at(child_bone).at(j) / child_rot_props.m_max_angle;
                                    if (weight < 0.0f)
                                        angle_ratio = -angle_ratio;
                                }

                                float_32_bit const  stiffness_exponent =
                                        std::max(0.0f, std::min(2.0f * (1.0f - child_rot_props.m_stiffness_with_parent_bone), 2.0f));
                                float_32_bit const  stiffness_coef = std::powf(std::fabsf(angle_ratio), stiffness_exponent);
                                coefs.push_back(stiffness_coef * angle_ratio);
                            }
                            if (sum_weights > 1e-5f)
                                for (natural_32_bit i = 0U; i != weights.size(); ++i)
                                    weights.at(i) /= sum_weights;
                        }

                        // Compute the rotation angle from all rotations of the child bone.
                        {
                            float_32_bit  angle = 0.0f;
                            for (natural_32_bit i = 0U; i != weights.size(); ++i)
                                angle += weights.at(i) * coefs.at(i) * (0.5f * rot_props.m_max_angle);
                            angle = std::max(-0.5f * rot_props.m_max_angle, std::min(angle, 0.5f * rot_props.m_max_angle));

                            angles.at(bone).at(i) = angle;
                        }

                        rotate(frame, angle_axis_to_quaternion(angles.at(bone).at(i), axis));
                    }
                }
            
                // 4. Compute y-coord of the second point in the angle_convergence_lines for each rotation axis of the target bone.
                {
                    coordinate_system const&  frame = props.frames->at(target_info.first);

                    vector3  to_target_vector;
                    {
                        matrix44  from_world_to_parent_frame;
                        compute_to_bone_space_matrix(props.parents->at(target_info.first), *props.frames, *props.parents, bones_to_consider, from_world_to_parent_frame);
                        vector3 const  target_in_parent_bone = transform_point(target_info.second.second, from_world_to_parent_frame);
                        to_target_vector = target_in_parent_bone - frame.origin();
                    }

                    bool  needs_next_iteration = false;

                    auto const&  rotations = props.rotation_props->at(target_info.first);
                    for (natural_32_bit  i = 0U; i != rotations.size(); ++i)
                    {
                        auto const&  rot_props = rotations.at(i);
                        vector3 const  axis = rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, frame);
                        vector3 const  eye_vector = detail::vector_from_frame(target_info.second.first, frame);
                        angle_convergence_lines.at(i).second(1) = compute_rotation_angle(axis, eye_vector, to_target_vector);

                        if (std::fabsf(angle_convergence_lines.at(i).second(1)) > angle_range_epsilon)
                        {
                            float_32_bit const  dist = length_2d(angle_convergence_lines.at(i).second - angle_convergence_lines.at(i).first);
                            if (std::fabsf(dist) > angle_range_epsilon)
                                needs_next_iteration = true;
                        }
                    }

                    if (needs_next_iteration == false)
                        break;
                }

                // 5. Find intersection of angle_convergence_lines with x axit to obtain new line for each rotation axis of the target bone.
                {
                    auto const&  rotations = props.rotation_props->at(target_info.first);
                    for (natural_32_bit i = 0U; i != rotations.size(); ++i)
                    {
                        vector2&  Q = angle_convergence_lines.at(i).second;
                        if (std::fabsf(Q(1)) > angle_range_epsilon)
                        {
                            vector2&  P = angle_convergence_lines.at(i).first;
                            float_32_bit const  angle =
                                    std::fabs(Q(1) - P(1)) > 1e-3f ? P(0) - P(1) * ((Q(0) - P(0)) / ((Q(1) - P(1))))
                                                                   : 0.5f * (P(0) + Q(0)) ;
                            auto const&  rot_props = rotations.at(i);
                            float_32_bit  angle_clipped = std::max(-0.5f * rot_props.m_max_angle, std::min(angle, 0.5f * rot_props.m_max_angle));
                            if (std::fabs(Q(0) - P(0)) < 1e-3f)
                            {
                                // We got to almost vertical convergence line; this situation is particularly bad
                                // at angle limits when we cannot recover from initial over-shooting towards the limit
                                // and now we cannot go back. So, we detect these situations and resolve them by reseting
                                // the new angle toward the center of the valid angle range of the rotation axis.
                                if (Q(0) > 0.5f * rot_props.m_max_angle - 1e-3f && Q(1) < -10.0f * angle_range_epsilon)
                                    angle_clipped = 0.25f * rot_props.m_max_angle;
                                else if (Q(0) < -0.5f * rot_props.m_max_angle + 1e-3f && Q(1) > 10.0f * angle_range_epsilon)
                                    angle_clipped = -0.25f * rot_props.m_max_angle;
                            }

                            P = Q;
                            Q(0) = angle_clipped;
                        }
                    }
                }
            }

            angles.erase(target_info.first);
        }
        for (auto const&  bone_and_angles : angles)
        {
            angles_for_inner_bones[bone_and_angles.first].resize(bone_and_angles.second.size());
            for (natural_32_bit i = 0U; i != bone_and_angles.second.size(); ++i)
                angles_for_inner_bones[bone_and_angles.first].at(i).push_back(bone_and_angles.second.at(i));
        }

        // And reset rotated frames.
        props.frames->at(target_info.first) = props.pose_frames->at(target_info.first);
        for (auto const& bone_and_angles : angles)
            props.frames->at(bone_and_angles.first) = props.pose_frames->at(bone_and_angles.first);
    }

    // Apply computed rotations of all inner bones. (Note: for each bone we compute average of angles computed for the bone for individual targets).
    for (auto const&  bone_and_angles : angles_for_inner_bones)
    {
        coordinate_system&  frame = props.frames->at(bone_and_angles.first);

        std::vector<joint_rotation_props> const&  rotations = props.rotation_props->at(bone_and_angles.first);

        for (natural_32_bit  rot_idx = 0U; rot_idx != rotations.size(); ++rot_idx)
        {
            joint_rotation_props const&  rot_props = rotations.at(rot_idx);

            vector3 const  axis = rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, frame);

            float_32_bit  angle;
            {
                angle = 0.0f;
                std::vector<float_32_bit> const&  angles = bone_and_angles.second.at(rot_idx);
                for (natural_32_bit i = 0U; i != angles.size(); ++i)
                    angle += angles.at(i);
                INVARIANT(!angles.empty());
                angle /= (float_32_bit)angles.size();
            }

            rotate(frame, angle_axis_to_quaternion(angle, axis));

            if (involved_rotations_of_bones != nullptr)
                (*involved_rotations_of_bones)[bone_and_angles.first].push_back(rot_idx);
        }
    }

    // Compute and apply rotations of all target bones (note: all inner bones are already rotated to best possible positions).
    for (auto target_it = props.look_at_targets->cbegin(); target_it != props.look_at_targets->cend(); ++target_it)
    {
        coordinate_system&  frame = props.frames->at(target_it->first);

        vector3  to_target_vector;
        {
            matrix44  from_world_to_parent_frame;
            compute_to_bone_space_matrix(props.parents->at(target_it->first), *props.frames, *props.parents, bones_to_consider, from_world_to_parent_frame);
            vector3 const  target_in_parent_bone = transform_point(target_it->second.second, from_world_to_parent_frame);
            to_target_vector = target_in_parent_bone - frame.origin();
        }

        auto const& rotations = props.rotation_props->at(target_it->first);

        for (natural_32_bit  i = 0U; i != rotations.size(); ++i)
        {
            auto const& rot_props = rotations.at(i);

            vector3 const  axis = rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, frame);

            float_32_bit  angle;
            {
                vector3 const  eye_vector = detail::vector_from_frame(target_it->second.first, frame);
                angle = compute_rotation_angle(axis, eye_vector, to_target_vector);
                angle = std::max(-0.5f * rot_props.m_max_angle, std::min(angle, 0.5f * rot_props.m_max_angle));
            }

            rotate(frame, angle_axis_to_quaternion(angle, axis));

            if (involved_rotations_of_bones != nullptr)
                (*involved_rotations_of_bones)[target_it->first].push_back(i);
        }
    }
}


bool  skeleton_rotate_bones_towards_target_pose(
        std::vector<coordinate_system>&  frames,
        std::vector<coordinate_system> const&  target_pose_frames,
        std::vector<coordinate_system> const&  pose_frames,
        std::vector<std::vector<joint_rotation_props> > const&  rotation_props,
        std::unordered_map<integer_32_bit, std::vector<natural_32_bit> > const&  bones_to_rotate,
        float_32_bit const  dt
        )
{
    TMPROF_BLOCK();

    bool  target_pose_reached = true;
    for (auto const&  bone_and_rotations : bones_to_rotate)
    {
        coordinate_system&  frame = frames.at(bone_and_rotations.first);
        coordinate_system const&  target_frame = target_pose_frames.at(bone_and_rotations.first);

        auto const&  rotations = rotation_props.at(bone_and_rotations.first);

        for (natural_32_bit  idx : bone_and_rotations.second)
        {
            joint_rotation_props const&  rot_props = rotations.at(idx);

            vector3 const  axis = rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, frame);

            float_32_bit  angle, current_angle;
            {
                current_angle =
                    compute_rotation_angle(
                            axis,
                            rot_props.m_zero_angle_direction,
                            detail::vector_from_frame(rot_props.m_direction, frame)
                            );
            
                float_32_bit const  target_angle =
                    compute_rotation_angle(
                            rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, target_frame),
                            rot_props.m_zero_angle_direction,
                            detail::vector_from_frame(rot_props.m_direction, target_frame)
                            );

                float_32_bit const  angle_delta = target_angle - current_angle;

                angle = angle_delta < 0.0f ? std::max(angle_delta, -rot_props.m_max_angular_speed * dt) :
                                             std::min(angle_delta,  rot_props.m_max_angular_speed * dt) ;

                if (target_pose_reached && std::fabs(angle_delta) > rot_props.m_max_angular_speed * dt)
                    target_pose_reached = false;
            }

            rotate(frame, angle_axis_to_quaternion(angle, axis));
        }
    }
    return target_pose_reached;
}


vector3  compute_common_look_at_target_for_multiple_eyes(
        std::vector<std::pair<vector3, vector3> > const&  vector_of_eye_pos_and_eye_unit_dir_pairs
        )
{
    TMPROF_BLOCK();

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
            float_32_bit const  d = 0.5f * (length(detail::project_vector_to_plane(w, P_i)) + length(detail::project_vector_to_plane(w, P_j)));
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


float_32_bit  compute_rotation_angle(vector3 const&  unit_axis, vector3 const&  current, vector3 const&  target)
{
    vector3 const  current_projected = detail::project_vector_to_plane(unit_axis, current);
    vector3 const  target_projected = detail::project_vector_to_plane(unit_axis, target);
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
    float_32_bit const  denominator = from_length * to_length;
    if (denominator < 1e-5f)
        return 0.0f;

    float_32_bit const  cos_angle = dot_product(from, to) / denominator;
    float_32_bit const  angle = std::fabsf(cos_angle >= 1.0f ? 0.0f : cos_angle <= -1.0f ? PI() : std::acosf(cos_angle));
    return dot_product(cross_product(unit_axis, from / from_length), to / to_length) >= 0.0f ? angle : -angle;
}


void  compute_to_bone_space_matrix(
        integer_32_bit  bone_index,
        std::vector<coordinate_system> const&  frames,
        std::vector<integer_32_bit> const&  parents,
        std::unordered_set<integer_32_bit> const&  bones_to_consider,
        matrix44&  W
        )
{
    W = matrix44_identity();
    for ( ; bones_to_consider.count(bone_index) != 0ULL; bone_index = parents.at(bone_index))
    {
        matrix44  B;
        to_base_matrix(frames.at(bone_index), B);
        W = W * B;
    }
}

void  compute_from_child_to_parent_bone_space_matrix(
        integer_32_bit  child_bone_index,
        integer_32_bit const  parent_bone_index,
        std::vector<coordinate_system> const&  frames,
        std::vector<integer_32_bit> const&  parents,
        matrix44&  W
        )
{
    W = matrix44_identity();
    for ( ; child_bone_index >= 0 && child_bone_index != parent_bone_index; child_bone_index = parents.at(child_bone_index))
    {
        matrix44  B;
        from_base_matrix(frames.at(child_bone_index), B);
        W = B * W;
    }
}


}
