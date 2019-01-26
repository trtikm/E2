#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace angeo { namespace detail {


inline vector3  vector_from_frame(vector3 const&  v, coordinate_system const&  space)
{
    return quaternion_to_rotation_matrix(space.orientation()) * v;
}


vector3  point_to_bone_frame(
        vector3 const&  p,
        integer_32_bit  bone_index,
        std::vector<coordinate_system> const&  frames,
        std::vector<integer_32_bit> const&  parents
        )
{
    matrix44  W = matrix44_identity();
    for ( ; bone_index >= 0; bone_index = parents.at(bone_index))
    {
        matrix44  B;
        to_base_matrix(frames.at(bone_index), B);
        W = W * B;
    }
    return transform_point(p, W);
}


inline vector3  get_rotation_axis(joint_rotation_props const&  props, coordinate_system const&  frame)
{
    return props.m_axis_in_parent_space ? props.m_axis : vector_from_frame(props.m_axis, frame);
}


float_32_bit  clip_rotation_to_allowed_limits(
        float_32_bit  rot_angle,
        float_32_bit const  current_angle,
        float_32_bit const  start_angle,
        float_32_bit const  max_angle,
        float_32_bit const  max_angular_speed,
        float_32_bit const  dt
        )
{
    if (current_angle + rot_angle - start_angle > max_angular_speed * dt)
        rot_angle -= current_angle + rot_angle - start_angle - max_angular_speed * dt;
    else if (current_angle + rot_angle - start_angle < -max_angular_speed * dt)
        rot_angle -= current_angle + rot_angle - start_angle + max_angular_speed * dt;

    if (current_angle + rot_angle > 0.5f * max_angle)
        rot_angle -= current_angle + rot_angle - 0.5f * max_angle;
    else if (current_angle + rot_angle < -0.5f * max_angle)
        rot_angle -= current_angle + rot_angle + 0.5f * max_angle;

    return rot_angle;
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
                            detail::get_rotation_axis(props, frames.at(i)),
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
            for (natural_32_bit i = 0U; i != rotation_props.at(bone_and_target.first).size(); ++i)
            {
                joint_rotation_props const&  props = rotation_props.at(bone_and_target.first).at(i);
                coordinate_system&  frame = frames.at(bone_and_target.first);
                vector3 const  axis = detail::get_rotation_axis(props, frame);

                float_32_bit  rot_angle =
                        compute_rotation_angle(
                                axis,
                                detail::vector_from_frame(bone_and_target.second.first, frame),
                                detail::vector_from_frame(
                                    detail::point_to_bone_frame(bone_and_target.second.second, bone_and_target.first, frames, parents),
                                    frame
                                    )
                                );

                rot_angle = detail::clip_rotation_to_allowed_limits(
                                rot_angle,
                                compute_rotation_angle(axis, props.m_zero_angle_direction, detail::vector_from_frame(props.m_direction, frame)),
                                start_angles.at(bone_and_target.first).at(i),
                                props.m_max_angle,
                                props.m_max_angular_speed,
                                dt
                                );

                rotate(frame, angle_axis_to_quaternion(rot_angle, axis));

                absolute_angle_change += std::fabs(rot_angle);
            }

        if (absolute_angle_change <= angle_change_epsilon)
            break;
if (iteration > 0U)
    continue;
    }
}


float_32_bit  compute_rotation_angle(vector3 const&  axis, vector3 const&  current, vector3 const&  target)
{
    vector3 const  current_projected = current - dot_product(current, axis) * axis;
    float_32_bit const  current_projected_len = length(current_projected);
    if (current_projected_len < 1e-3f)
        return 0.0f;

    vector3 const  target_projected = target - dot_product(target, axis) * axis;
    float_32_bit const  target_projected_len = length(target_projected);
    if (target_projected_len < 1e-3f)
        return 0.0f;

    float_32_bit const  cos_angle = dot_product(current_projected, target_projected) / (current_projected_len * target_projected_len);
    float_32_bit const  angle = std::fabsf(cos_angle >= 1.0f ? 0.0f : cos_angle <= -1.0f ? PI() : std::acosf(cos_angle));
    return dot_product(cross_product(axis, current_projected / current_projected_len), target_projected / target_projected_len) >= 0.0f ? angle : -angle;
}


}
