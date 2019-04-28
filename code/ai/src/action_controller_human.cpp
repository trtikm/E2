#include <ai/action_controller_human.hpp>
#include <ai/skeleton_utils.hpp>
#include <scene/scene_node_id.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai { namespace detail {


scene::node_id  get_motion_capsule_nid(scene_ptr const  s, scene::node_id const  agent_nid)
{
    return s->get_aux_root_node_for_agent(agent_nid, "motion_capsule");
}


void  create_motion_capsule(scene_ptr const  s, scene::node_id const  agent_nid, angeo::coordinate_system const&  frame_in_world_space)
{
    scene::node_id const  motion_capsule_nid(get_motion_capsule_nid(s, agent_nid));
    s->insert_scene_node(motion_capsule_nid, frame_in_world_space, false);
    s->insert_collision_capsule_to_scene_node(
            motion_capsule_nid,
            0.65f,
            0.2f,
            angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING,
            1.0f,
            true
            );
    s->insert_rigid_body_to_scene_node(
            motion_capsule_nid,
            vector3_zero(),
            vector3_zero(),
            vector3(0.0f, 0.0f, -9.81f),
            vector3_zero(),
            1.0f / 60.0f,
            matrix33_zero()
            );
}


void  destroy_motion_capsule(scene_ptr const  s, scene::node_id const  agent_nid)
{
    scene::node_id const  motion_capsule_nid(get_motion_capsule_nid(s, agent_nid));
    s->erase_rigid_body_from_scene_node(motion_capsule_nid);
    s->erase_collision_object_from_scene_node(motion_capsule_nid);
    s->erase_scene_node(motion_capsule_nid);
}


}}

namespace ai {


action_controller_human::action_controller_human(
            blackboard_ptr const  blackboard_,
            skeletal_motion_templates::motion_template_cursor const&  start_pose
            )
    : action_controller(blackboard_)
    , m_template_motion_info({
            start_pose,     // src_pose
            start_pose,     // dst_pose
            0.0f,           // total_interpolation_time_in_seconds
            0.0f            // consumed_time_in_seconds
            })
    , m_desired_linear_velocity_in_world_space(vector3_zero())
    , m_desired_angular_speed_in_world_space(0.0f)
{
    angeo::coordinate_system  agent_frame;
    {
        angeo::coordinate_system  tmp;
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_bone_nids.front(), false, tmp);
        agent_frame.set_origin(tmp.origin());
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, tmp);
        agent_frame.set_orientation(tmp.orientation());
    }
    detail::create_motion_capsule(get_blackboard()->m_scene, get_blackboard()->m_agent_nid, agent_frame);
}


action_controller_human::~action_controller_human()
{
    detail::destroy_motion_capsule(get_blackboard()->m_scene, get_blackboard()->m_agent_nid);
}


void  action_controller_human::next_round(float_32_bit  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_desired_angular_speed_in_world_space =
            get_blackboard()->m_cortex_cmd_turn_intensity * get_blackboard()->m_max_turn_speed_in_radians_per_second;

    if (m_template_motion_info.consumed_time_in_seconds + time_step_in_seconds > m_template_motion_info.total_interpolation_time_in_seconds)
    {
        float_32_bit const  time_till_dst_pose =
                m_template_motion_info.total_interpolation_time_in_seconds - m_template_motion_info.consumed_time_in_seconds;
        INVARIANT(time_till_dst_pose >= 0.0f);

        angeo::coordinate_system  agent_frame;
        {
            get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, agent_frame);
            agent_frame.set_origin(agent_frame.origin() + time_till_dst_pose * m_desired_linear_velocity_in_world_space);
            {
                matrix33 const  reference_frame_rotation = quaternion_to_rotation_matrix(agent_frame.orientation());
                vector3  turn_axis;
                {
                    vector3  x, y;
                    rotation_matrix_to_basis(reference_frame_rotation, x, y, turn_axis);
                }
                agent_frame.set_orientation(
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
            get_blackboard()->m_scene->set_frame_of_scene_node(get_blackboard()->m_agent_nid, false, agent_frame);
        }

        time_step_in_seconds -= time_till_dst_pose;
        INVARIANT(time_step_in_seconds >= 0.0f);

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
            INVARIANT(time_delta > 0.0f);
            m_template_motion_info.total_interpolation_time_in_seconds += time_delta;

            vector3 const  position_delta =
                    dst_animation.get_meta_reference_frames().at(m_template_motion_info.dst_pose.keyframe_index).origin() -
                    dst_animation.get_meta_reference_frames().at(m_template_motion_info.dst_pose.keyframe_index - 1U).origin();
            desired_linear_velocity_in_animation_space += position_delta / std::max(time_delta, 0.0001f);
        }
        while (time_step_in_seconds > m_template_motion_info.total_interpolation_time_in_seconds);

        INVARIANT(m_template_motion_info.total_interpolation_time_in_seconds > 0.0f);

        m_desired_linear_velocity_in_world_space =
                quaternion_to_rotation_matrix(agent_frame.orientation())
                * desired_linear_velocity_in_animation_space;
    }

    angeo::coordinate_system  agent_frame;
    {
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, agent_frame);
        agent_frame.set_origin(agent_frame.origin() + time_step_in_seconds * m_desired_linear_velocity_in_world_space);
        {
            matrix33 const  reference_frame_rotation = quaternion_to_rotation_matrix(agent_frame.orientation());
            vector3  turn_axis;
            {
                vector3  x, y;
                rotation_matrix_to_basis(reference_frame_rotation, x, y, turn_axis);
            }
            agent_frame.set_orientation(
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
        get_blackboard()->m_scene->set_frame_of_scene_node(get_blackboard()->m_agent_nid, false, agent_frame);
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
        angeo::from_base_matrix(agent_frame, W);
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

    for (natural_32_bit  bone = 0; bone != interpolated_frames_in_world_space.size(); ++bone)
        get_blackboard()->m_scene->set_frame_of_scene_node(
                get_blackboard()->m_bone_nids.at(bone),
                false,
                interpolated_frames_in_world_space.at(bone)
                );
}


}
