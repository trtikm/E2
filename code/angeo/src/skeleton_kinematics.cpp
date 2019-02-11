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
    return u - param_mult * dot_product(u, unit_plane_normal) * unit_plane_normal;
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


float_32_bit  clip_rotation_to_allowed_limits(
        float_32_bit  target_angle,
        float_32_bit const  current_angle,
        float_32_bit const  start_angle,
        float_32_bit const  max_angle,
        float_32_bit const  max_angular_speed,
        float_32_bit const  dt
        )
{
    float_32_bit  angle_delta = target_angle - current_angle;
    if (angle_delta >= 0.0f)
    {
        float_32_bit const  limit_angle = std::min(start_angle + max_angular_speed * dt, 0.5f * max_angle);
        if (target_angle > limit_angle)
            angle_delta = std::max(0.0f, limit_angle - current_angle);
    }
    else
    {
        float_32_bit const  limit_angle = std::max(start_angle - max_angular_speed * dt, -0.5f * max_angle);
        if (target_angle < limit_angle)
            angle_delta = std::min(0.0f, limit_angle - current_angle);
    }
    return angle_delta;
}


struct  skeleton_look_at_props
{
    std::vector<coordinate_system>*  frames;
    std::vector<integer_32_bit> const*  parents;
    std::vector<std::vector<integer_32_bit> > const*  children;
    std::vector<std::vector<joint_rotation_props> > const*  rotation_props;
    bone_look_at_targets const*  look_at_targets;
    std::unordered_set<integer_32_bit> const*  look_at_bases;
    std::vector<std::vector<float_32_bit> > const*  start_angles;
    float_32_bit const  dt;
};


float_32_bit  compute_look_at_rotation_angle_of_inner_bone( // I.e. for bone which is NOT end-effector (for end-effector we have faster algo).
        vector3 const&  bone_origin,    // The origin of the bone's coord. system => in the parent bone space of rotated bone.
        vector3 const&  axis,           // Unit vector; rotation axis; in the parent bone space of rotated bone. (the rotation angle is computed).
        vector3 const&  target,         // The look at target point; in the parent bone space of rotated bone.
        vector3 const&  eye_vector,     // The direction of vector of the end-effector bone; in the parent bone space of rotated bone.
        vector3 const&  eye_center      // The origin point of the end-effector bone; in the parent bone space of rotated bone.
        )
{
    vector3 const  to_target_projected = detail::project_vector_to_plane(axis, target - bone_origin);
    vector3 const  to_eye_center_projected = detail::project_vector_to_plane(axis, eye_center - bone_origin);
    vector3 const  eye_vector_projected = detail::project_vector_to_plane(axis, eye_vector);

    float_32_bit  q;
    {
        q = length(to_target_projected);
        if (q < 1e-3f)
            return 0.0f;
    }

    float_32_bit  R;
    {
        R = length(to_eye_center_projected);
        if (R < 1e-3f || q < R + 1e-3f)
            return 0.0f;
    }

    float_32_bit const  beta =
        compute_rotation_angle_of_projected_vector(to_eye_center_projected, eye_vector_projected, R, length(eye_vector_projected), axis);

    float_32_bit const  gamma =
            compute_rotation_angle_of_projected_vector(to_eye_center_projected, to_target_projected, R, q, axis);

    float_32_bit  alpha;
    {
        float_32_bit const  sin_beta = std::sinf(std::fabsf(beta));
        float_32_bit const  cos_beta = std::cosf(std::fabsf(beta));

        float_32_bit const  D = q*q - R*R * sin_beta*sin_beta;
        //INVARIANT(D >= 0.0f);
        float_32_bit const  sin_alpha = (sin_beta / q) * (std::sqrt(D) - R*cos_beta);
        //INVARIANT(sin_alpha >= 0.0f);
        alpha = sin_alpha >= 1.0f ? PI() * 0.5f : std::asinf(sin_alpha);
        //INVARIANT(0.0f <= alpha && alpha <= PI() * 0.5f);

        float_32_bit const  cos_alpha = std::cosf(alpha);

        auto  compute_beta_from_cos_alpha = [q, R, sin_alpha](float_32_bit const  cos_alpha) -> float_32_bit {
            vector2 const  E{ R * cos_alpha, R * sin_alpha };
            float_32_bit const  cos_beta = cos_angle_2d(vector2{ q, 0.0f } - E, E);
            float_32_bit const  beta = std::acosf(cos_beta);
            //INVARIANT(0.0f <= beta && beta <= PI());
            return beta;
        };

        float_32_bit const  beta_from_alpha = compute_beta_from_cos_alpha(cos_alpha);
        if (std::fabs(beta_from_alpha - std::fabsf(beta)) > PI() / 180.0f)
        {
            //INVARIANT(std::fabs(compute_beta_from_cos_alpha(-cos_alpha) - std::fabsf(beta)) <= PI() / 180.0f);
            alpha = PI() - alpha;
        }
        if (beta > 0.0f)
            alpha = -alpha;
    }

    return alpha + gamma;
}


void  bone_look_at_target(
        skeleton_look_at_props&  props,
        integer_32_bit const  bone,
        std::unordered_set<integer_32_bit>&  output_targets,
        std::unordered_set<integer_32_bit>&  visited,
        float_32_bit const* const  convergence_coef_override
        )
{
    if (visited.count(bone) != 0UL)
        return;
    visited.insert(bone);

    bool const  is_look_at_target_bone = props.look_at_targets->count(bone) != 0UL;
    std::unordered_set<integer_32_bit>  targets;
    {
        if (is_look_at_target_bone)
            targets.insert(bone);
        for (integer_32_bit  child_bone : props.children->at(bone))
            bone_look_at_target(props, child_bone, targets, visited, convergence_coef_override);
    }
    if (targets.empty())
        return;
    output_targets.insert(targets.cbegin(), targets.cend());

    matrix44  from_world_to_parent_frame;
    compute_to_bone_space_matrix(props.parents->at(bone), *props.frames, *props.parents, from_world_to_parent_frame);

    coordinate_system  frame = props.frames->at(bone);

    for (auto const& rot_props : props.rotation_props->at(bone))
    {
        vector3 const  axis = rot_props.m_axis_in_parent_space ? rot_props.m_axis : detail::vector_from_frame(rot_props.m_axis, frame);

        float_32_bit  rot_angle;
        {
            float_32_bit  min_alpha = std::numeric_limits<float_32_bit>::max();
            float_32_bit  max_alpha = std::numeric_limits<float_32_bit>::min();
            for (integer_32_bit  target_bone : targets)
            {
                vector3 const  target_in_parent_bone = transform_point(props.look_at_targets->at(target_bone).second, from_world_to_parent_frame);
            
                matrix44  from_target_to_parent_frame;
                compute_from_child_to_parent_bone_space_matrix(target_bone, props.parents->at(bone), *props.frames, *props.parents, from_target_to_parent_frame);
                vector3 const  eye_vector_in_parent_bone = transform_vector(props.look_at_targets->at(target_bone).first, from_target_to_parent_frame);
                vector3 const  eye_center_in_parent_bone = transform_point(vector3_zero(), from_target_to_parent_frame);
                float_32_bit const  alpha =
                        is_look_at_target_bone
                            ? compute_rotation_angle(axis, eye_vector_in_parent_bone, rot_props.m_zero_angle_direction)
                                + compute_rotation_angle(axis, rot_props.m_zero_angle_direction, target_in_parent_bone - eye_center_in_parent_bone)
                            : detail::compute_look_at_rotation_angle_of_inner_bone(
                                        frame.origin(),
                                        axis,
                                        target_in_parent_bone,
                                        eye_vector_in_parent_bone,
                                        eye_center_in_parent_bone
                                        );

                min_alpha = std::min(alpha, min_alpha);
                max_alpha = std::max(alpha, min_alpha);
            }

            if (min_alpha >= 0.0f)
                rot_angle = max_alpha;
            else if (max_alpha <= 0.0f)
                rot_angle = min_alpha;
            else
                rot_angle = 0.5f * (min_alpha + max_alpha);
        }

        float_32_bit const  convergence_coef = convergence_coef_override != nullptr ? *convergence_coef_override : rot_props.m_convergence_coef;
        rotate(frame, angle_axis_to_quaternion(convergence_coef * rot_angle, axis));
    }

    clip_all_joint_rotations_to_allowed_limits(
            props.frames->at(bone),
            props.rotation_props->at(bone),
            props.start_angles->at(bone),
            props.dt,
            frame
            );
}


void  look_at_targets(
        skeleton_look_at_props&  props,
        bool const override_convergence_coef = false
        )
{
    float_32_bit const  convergence_coef_override = 1.0f;
    std::unordered_set<integer_32_bit>  targets;
    std::unordered_set<integer_32_bit>  visited;
    for (integer_32_bit  base_bone : *props.look_at_bases)
        bone_look_at_target(props, base_bone, targets, visited, override_convergence_coef ? &convergence_coef_override : nullptr);
}


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


void  skeleton_bones_move_towards_targets(
        std::vector<coordinate_system>&  frames,
        std::vector<integer_32_bit> const&  parents,
        std::vector<std::vector<integer_32_bit> > const&  children,
        std::vector<std::vector<joint_rotation_props> > const&  rotation_props,
        bone_look_at_targets const&  look_at_targets,
        float_32_bit const  dt,
        natural_32_bit const  num_sub_iterations
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!frames.empty() && frames.size() == parents.size() && frames.size() == rotation_props.size());
    ASSUMPTION(!look_at_targets.empty());
    ASSUMPTION(dt > 0.0f);

    std::unordered_set<integer_32_bit>  look_at_bases;
    for (auto const& bone_and_target : look_at_targets)
    {
        integer_32_bit bone_idx = bone_and_target.first;
        while (true)
        {
            integer_32_bit const  parent_bone_idx = parents.at(bone_idx);
            if (parent_bone_idx < 0)
                break;
            bone_idx = parent_bone_idx;
        }
        look_at_bases.insert(bone_idx);
    }

    std::vector<std::vector<float_32_bit> >  start_angles(rotation_props.size());
    for (natural_32_bit i = 0U; i != rotation_props.size(); ++i)
        for (auto const& props : rotation_props.at(i))
            start_angles.at(i).push_back(
                    compute_rotation_angle(
                            props.m_axis_in_parent_space ? props.m_axis : detail::vector_from_frame(props.m_axis, frames.at(i)),
                            props.m_zero_angle_direction,
                            detail::vector_from_frame(props.m_direction, frames.at(i))
                            )
                    );

    std::vector<coordinate_system>  computed_frames = frames;

    detail::skeleton_look_at_props  look_at_props {
        &frames,
        &parents,
        &children,
        &rotation_props,
        &look_at_targets,
        &look_at_bases,
        &start_angles,
        dt
    };

    detail::look_at_targets(look_at_props);
    for (natural_32_bit  iteration = 1U; iteration < num_sub_iterations; ++iteration)
        detail::look_at_targets(look_at_props);
}


float_32_bit  clip_all_joint_rotations_to_allowed_limits(
        coordinate_system&  frame,
        std::vector<joint_rotation_props> const&  rotation_props,
        std::vector<float_32_bit> const&  start_angles,
        float_32_bit const  dt,
        coordinate_system const&  target_frame
        )
{
    float_32_bit  absolute_angle_change = 0.0f;
    for (natural_32_bit i = 0U; i != rotation_props.size(); ++i)
    {
        float_32_bit const  rot_angle =
                clip_joint_rotation_to_allowed_limits(frame, rotation_props.at(i), start_angles.at(i), dt, target_frame);
        absolute_angle_change += std::fabs(rot_angle);
    }
    return absolute_angle_change;
}


float_32_bit  clip_joint_rotation_to_allowed_limits(
        coordinate_system&  frame,
        joint_rotation_props const&  props,
        float_32_bit const  start_angle,
        float_32_bit const  dt,
        coordinate_system const&  target_frame
        )
{
    float_32_bit const  target_rot_angle =
            compute_rotation_angle(
                    props.m_axis_in_parent_space ? props.m_axis : detail::vector_from_frame(props.m_axis, target_frame),
                    props.m_zero_angle_direction,
                    detail::vector_from_frame(props.m_direction, target_frame)
                    );
    vector3 const  axis = props.m_axis_in_parent_space ? props.m_axis : detail::vector_from_frame(props.m_axis, frame);
    float_32_bit const  current_rot_angle =
            compute_rotation_angle(
                    axis,
                    props.m_zero_angle_direction,
                    detail::vector_from_frame(props.m_direction, frame)
                    );
    float_32_bit const  rot_angle =
            detail::clip_rotation_to_allowed_limits(
                    target_rot_angle,
                    current_rot_angle,
                    start_angle,
                    props.m_max_angle,
                    props.m_max_angular_speed,
                    dt
                    );
    rotate(frame, angle_axis_to_quaternion(rot_angle, axis));

    return rot_angle;
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
        matrix44&  W
        )
{
    W = matrix44_identity();
    for ( ; bone_index >= 0; bone_index = parents.at(bone_index))
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
