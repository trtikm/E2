#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

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


}}

namespace angeo {


void  skeleton_bones_move_towards_targets(
        std::vector<coordinate_system>&  frames,
        std::vector<integer_32_bit> const&  parents,
        std::vector<std::vector<joint_rotation_props> > const&  rotation_props,
        bone_look_at_targets const&  look_at_targets,
        float_32_bit const  dt
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!frames.empty() && frames.size() == parents.size() && frames.size() == rotation_props.size());
    ASSUMPTION(!look_at_targets.empty());
    ASSUMPTION(dt > 0.0f);

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

    natural_32_bit const  max_num_iterations = 10U;
    float_32_bit const  angle_change_epsilon = (PI() / 180.0f);
    for (natural_32_bit  iteration = 0U; iteration < max_num_iterations; ++iteration)
    {
        float_32_bit  absolute_angle_change = 0.0f;

        for (auto const&  bone_and_target : look_at_targets)
        {
            coordinate_system&  current_frame = frames.at(bone_and_target.first);
            coordinate_system  target_frame = current_frame;
            matrix44  to_parent_frame;
            detail::compute_to_bone_space_matrix(parents.at(bone_and_target.first), frames, parents, to_parent_frame);
            for (auto const& props : rotation_props.at(bone_and_target.first))
            {
                vector3 const  axis = props.m_axis_in_parent_space ? props.m_axis : detail::vector_from_frame(props.m_axis, target_frame);
                vector3 const  target = detail::point_to_bone_frame(bone_and_target.second.second, to_parent_frame, target_frame);
                float_32_bit const  rot_angle =
                        compute_rotation_angle(
                                axis,
                                detail::vector_from_frame(bone_and_target.second.first, target_frame),
                                detail::vector_from_frame(target, target_frame)
                                );
                rotate(target_frame, angle_axis_to_quaternion(rot_angle, axis));
            }
            for (natural_32_bit i = 0U; i != rotation_props.at(bone_and_target.first).size(); ++i)
            {
                joint_rotation_props const&  props = rotation_props.at(bone_and_target.first).at(i);
                float_32_bit const  target_rot_angle =
                        compute_rotation_angle(
                                props.m_axis_in_parent_space ? props.m_axis : detail::vector_from_frame(props.m_axis, target_frame),
                                props.m_zero_angle_direction,
                                detail::vector_from_frame(props.m_direction, target_frame)
                                );
                vector3 const  current_axis = props.m_axis_in_parent_space ? props.m_axis : detail::vector_from_frame(props.m_axis, current_frame);
                float_32_bit const  current_rot_angle =
                        compute_rotation_angle(
                                current_axis,
                                props.m_zero_angle_direction,
                                detail::vector_from_frame(props.m_direction, current_frame)
                                );
                float_32_bit const  rot_angle =
                        detail::clip_rotation_to_allowed_limits(
                                target_rot_angle,
                                current_rot_angle,
                                start_angles.at(bone_and_target.first).at(i),
                                props.m_max_angle,
                                props.m_max_angular_speed,
                                dt
                                );
                rotate(current_frame, angle_axis_to_quaternion(rot_angle, current_axis));

                absolute_angle_change += std::fabs(rot_angle);
            }
        }
        if (absolute_angle_change <= angle_change_epsilon)
            break;
if (iteration > 0U)
    continue;
    }
}


float_32_bit  compute_rotation_angle(vector3 const&  axis, vector3 const&  current, vector3 const&  target)
{
    vector3 const  current_projected = detail::project_vector_to_plane(axis, current);
    float_32_bit const  current_projected_len = length(current_projected);
    if (current_projected_len < 1e-3f)
        return 0.0f;

    vector3 const  target_projected = detail::project_vector_to_plane(axis, target);
    float_32_bit const  target_projected_len = length(target_projected);
    if (target_projected_len < 1e-3f)
        return 0.0f;

    float_32_bit const  cos_angle = dot_product(current_projected, target_projected) / (current_projected_len * target_projected_len);
    float_32_bit const  angle = std::fabsf(cos_angle >= 1.0f ? 0.0f : cos_angle <= -1.0f ? PI() : std::acosf(cos_angle));
    return dot_product(cross_product(axis, current_projected / current_projected_len), target_projected / target_projected_len) >= 0.0f ? angle : -angle;
}


}
