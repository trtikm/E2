#include <ai/action_controller_human.hpp>
#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


action_controller_human::action_controller_human(
            blackboard_ptr const  blackboard_,
            angeo::coordinate_system const&  start_reference_frame_in_world_space,
            skeletal_motion_templates::motion_template_cursor const&  start_pose
            )
    : action_controller(blackboard_)
    , m_template_motion_info({
            start_pose,     // src_pose
            start_pose,     // dst_pose
            0.0f,           // total_interpolation_time_in_seconds
            0.0f            // consumed_time_in_seconds
            })
    , m_reference_frame_in_world_space(start_reference_frame_in_world_space)
    , m_desired_linear_velocity_in_world_space(vector3_zero())
    , m_desired_angular_speed_in_world_space(0.0f)
{}


void  action_controller_human::next_round(float_32_bit  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_desired_angular_speed_in_world_space =
            get_blackboard()->m_cortex_cmd_turn_intensity * get_blackboard()->m_max_turn_speed_in_radians_per_second;

    if (m_template_motion_info.consumed_time_in_seconds + time_step_in_seconds > m_template_motion_info.total_interpolation_time_in_seconds)
    {
        float_32_bit const  time_till_dst_pose =
                m_template_motion_info.total_interpolation_time_in_seconds - m_template_motion_info.consumed_time_in_seconds;
        time_step_in_seconds -= time_till_dst_pose;

        m_reference_frame_in_world_space.set_origin(
                m_reference_frame_in_world_space.origin() + time_till_dst_pose * m_desired_linear_velocity_in_world_space
                );
        {
            matrix33 const  reference_frame_rotation = quaternion_to_rotation_matrix(m_reference_frame_in_world_space.orientation());
            vector3  turn_axis;
            {
                vector3  x, y;
                rotation_matrix_to_basis(reference_frame_rotation, x, y, turn_axis);
            }
            m_reference_frame_in_world_space.set_orientation(
                    normalised(
                            rotation_matrix_to_quaternion(
                                    quaternion_to_rotation_matrix(
                                            angle_axis_to_quaternion(time_till_dst_pose * m_desired_angular_speed_in_world_space, turn_axis)
                                            )
                                    * reference_frame_rotation
                                    )
                            )
                    );
        }

        m_template_motion_info.src_pose = m_template_motion_info.dst_pose;
        m_template_motion_info.consumed_time_in_seconds = 0.0f;
        m_template_motion_info.total_interpolation_time_in_seconds = 0.0f;

        m_desired_linear_velocity_in_world_space = vector3_zero();
        m_desired_angular_speed_in_world_space = 0.0f;
        vector3  desired_linear_velocity_in_animation_space = vector3_zero();
        float_32_bit  desired_angular_speed_in_animation_space = 0.0f;

        skeletal_motion_templates::keyframes const&  src_animation =
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.src_pose.motion_name
                );
        skeletal_motion_templates::keyframes const&  dst_animation =
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.dst_pose.motion_name
                );
        do
        {
            if (m_template_motion_info.dst_pose.keyframe_index + 1U == dst_animation.get_keyframes().size())
                if (true) // do repeat?
                    m_template_motion_info.dst_pose.keyframe_index = 0U;
                else
                {
                    time_step_in_seconds = m_template_motion_info.total_interpolation_time_in_seconds;
                    break;
                }
            ++m_template_motion_info.dst_pose.keyframe_index;

            float_32_bit const  time_delta =
                    dst_animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index).get_time_point() -
                    dst_animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index - 1U).get_time_point();
            m_template_motion_info.total_interpolation_time_in_seconds += time_delta;

            vector3 const  position_delta =
                    dst_animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index).get_coord_systems().at(0).origin() -
                    dst_animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index - 1U).get_coord_systems().at(0).origin();
            desired_linear_velocity_in_animation_space += position_delta / std::max(time_delta, 0.0001f);
        }
        while (time_step_in_seconds > m_template_motion_info.total_interpolation_time_in_seconds);

        m_desired_linear_velocity_in_world_space =
                quaternion_to_rotation_matrix(m_reference_frame_in_world_space.orientation())
                * desired_linear_velocity_in_animation_space;
    }

    m_reference_frame_in_world_space.set_origin(
            m_reference_frame_in_world_space.origin() + time_step_in_seconds * m_desired_linear_velocity_in_world_space
            );
    {
        matrix33 const  reference_frame_rotation = quaternion_to_rotation_matrix(m_reference_frame_in_world_space.orientation());
        vector3  turn_axis;
        {
            vector3  x, y;
            rotation_matrix_to_basis(reference_frame_rotation, x, y, turn_axis);
        }
        m_reference_frame_in_world_space.set_orientation(
                normalised(
                        rotation_matrix_to_quaternion(
                                quaternion_to_rotation_matrix(
                                        angle_axis_to_quaternion(time_step_in_seconds * m_desired_angular_speed_in_world_space, turn_axis)
                                        )
                                * reference_frame_rotation
                                )
                        )
                );
    }

    m_template_motion_info.consumed_time_in_seconds += time_step_in_seconds;
    INVARIANT(m_template_motion_info.consumed_time_in_seconds <= m_template_motion_info.total_interpolation_time_in_seconds);

    float_32_bit const  interpolation_param =
            m_template_motion_info.consumed_time_in_seconds / m_template_motion_info.total_interpolation_time_in_seconds;

    skeletal_motion_templates::keyframes const&  src_animation =
            get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.src_pose.motion_name
            );
    skeletal_motion_templates::keyframes const&  dst_animation =
            get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.dst_pose.motion_name
            );

    std::vector<angeo::coordinate_system>  interpolated_frames_in_animation_space;
    interpolate_keyframes_spherical(
            src_animation.keyframe_at(m_template_motion_info.src_pose.keyframe_index).get_coord_systems(),
            dst_animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index).get_coord_systems(),
            interpolation_param,
            interpolated_frames_in_animation_space
            );

    angeo::coordinate_system  reference_frame_in_animation_space;
    angeo::interpolate_spherical(
            src_animation.get_meta_reference_frames().at(m_template_motion_info.src_pose.keyframe_index),
            dst_animation.get_meta_reference_frames().at(m_template_motion_info.dst_pose.keyframe_index),
            interpolation_param,
            reference_frame_in_animation_space
            );

    std::vector<angeo::coordinate_system>  interpolated_frames_in_world_space;
    {
        interpolated_frames_in_world_space.reserve(interpolated_frames_in_animation_space.size());

        matrix44  W, Ainv, M;
        angeo::from_base_matrix(m_reference_frame_in_world_space, W);
        angeo::to_base_matrix(reference_frame_in_animation_space, Ainv);
        M = W * Ainv;
        for (angeo::coordinate_system const&  frame : interpolated_frames_in_animation_space)
        {
            vector3  u;
            matrix33  R;
            {
                matrix44  N;
                angeo::from_base_matrix(frame, N);
                decompose_matrix44(M * N, u, R);
            }
            interpolated_frames_in_world_space.push_back({ u, normalised(rotation_matrix_to_quaternion(R)) });
        }
    }

    transform_skeleton_coord_systems_from_world_to_local_space(
            interpolated_frames_in_world_space,
            get_blackboard()->m_skeleton_composition->parents,
            get_blackboard()->m_frames
            );
}


}
