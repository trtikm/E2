#include <ai/action_controller_human.hpp>
#include <ai/skeleton_utils.hpp>
#include <scene/scene_node_id.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai { namespace detail {


struct  rigid_body_motion
{
    rigid_body_motion()
        : velocity({vector3_zero(),vector3_zero()})
        , acceleration({vector3(0.0f, 0.0f, -9.81f),vector3_zero()})
        , inverted_mass(1.0f / 60.0f)
        , inverted_inertia_tensor(matrix33_zero())
    {}

    rigid_body_motion(scene_ptr const  s, scene::node_id const&  motion_object_nid)
        : velocity({ s->get_linear_velocity_of_rigid_body_of_scene_node(motion_object_nid),
                     s->get_angular_velocity_of_rigid_body_of_scene_node(motion_object_nid) })
        , acceleration({ s->get_linear_acceleration_of_rigid_body_of_scene_node(motion_object_nid),
                         s->get_angular_acceleration_of_rigid_body_of_scene_node(motion_object_nid) })
        , inverted_mass(s->get_inverted_mass_of_rigid_body_of_scene_node(motion_object_nid))
        , inverted_inertia_tensor(s->get_inverted_inertia_tensor_of_rigid_body_of_scene_node(motion_object_nid))
    {}

    void restore(scene_ptr const  s, scene::node_id const&  motion_object_nid)
    {
        s->set_linear_velocity_of_rigid_body_of_scene_node(motion_object_nid, velocity.m_linear);
        s->set_angular_velocity_of_rigid_body_of_scene_node(motion_object_nid, velocity.m_angular);
        s->set_linear_acceleration_of_rigid_body_of_scene_node(motion_object_nid, acceleration.m_linear);
        s->set_angular_acceleration_of_rigid_body_of_scene_node(motion_object_nid, acceleration.m_angular);
        s->set_inverted_mass_of_rigid_body_of_scene_node(motion_object_nid, inverted_mass);
        s->set_inverted_inertia_tensor_of_rigid_body_of_scene_node(motion_object_nid, inverted_inertia_tensor);
    }

    angeo::linear_and_angular_vector  velocity;
    angeo::linear_and_angular_vector  acceleration;
    float_32_bit  inverted_mass;
    matrix33  inverted_inertia_tensor;
};


scene::node_id  get_motion_object_nid(scene_ptr const  s, scene::node_id const  agent_nid)
{
    return s->get_aux_root_node_for_agent(agent_nid, "motion_object");
}


void  create_collider_and_rigid_body_of_motion_scene_node(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        skeletal_motion_templates::keyframes::meta_data::record const&  collider_props,
        rigid_body_motion const&  rb_motion
        )
{
    if (collider_props.keyword == "capsule")
        s->insert_collision_capsule_to_scene_node(
                motion_object_nid,
                collider_props.arguments.at(0),
                collider_props.arguments.at(1),
                angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING,
                1.0f,
                true
                );
    else
    {
        NOT_IMPLEMENTED_YET();
    }
    s->insert_rigid_body_to_scene_node(
            motion_object_nid,
            rb_motion.velocity.m_linear,
            rb_motion.velocity.m_angular,
            rb_motion.acceleration.m_linear,
            rb_motion.acceleration.m_angular,
            rb_motion.inverted_mass,
            rb_motion.inverted_inertia_tensor
            );
}


void  destroy_collider_and_rigid_bofy_of_motion_scene_node(scene_ptr const  s, scene::node_id const&  motion_object_nid)
{
    s->erase_rigid_body_from_scene_node(motion_object_nid);
    s->erase_collision_object_from_scene_node(motion_object_nid);
}


scene::node_id  create_motion_scene_node(
        scene_ptr const  s,
        scene::node_id const  agent_nid,
        angeo::coordinate_system const&  frame_in_world_space,
        skeletal_motion_templates::keyframes::meta_data::record const&  collider_props,
        rigid_body_motion const&  rb_motion
        )
{
    scene::node_id const  motion_object_nid(get_motion_object_nid(s, agent_nid));
    s->insert_scene_node(motion_object_nid, frame_in_world_space, false);
    create_collider_and_rigid_body_of_motion_scene_node(s, motion_object_nid, collider_props, rb_motion);
    return motion_object_nid;
}


void  destroy_motion_scene_node(scene_ptr const  s, scene::node_id const&  motion_object_nid)
{
    destroy_collider_and_rigid_bofy_of_motion_scene_node(s, motion_object_nid);
    s->erase_scene_node(motion_object_nid);
}


}}

namespace ai {


action_controller_human::action_controller_human(blackboard_ptr const  blackboard_)
    : action_controller(blackboard_)
    , m_template_motion_info({
            {"", 0U},       // src_pose
            {"", 0U},       // dst_pose
            0.0f,           // total_interpolation_time_in_seconds
            0.0f            // consumed_time_in_seconds
            })
    , m_desired_linear_velocity_in_world_space(vector3_zero())
    , m_desired_angular_speed_in_world_space(0.0f)
    , m_motion_object_nid()
    , m_motion_object_collider_props()
{
}


action_controller_human::~action_controller_human()
{
    if (m_motion_object_nid.valid())
    {
        get_blackboard()->m_scene->unregister_to_collision_contacts_stream(m_motion_object_nid, get_blackboard()->m_agent_id);
        detail::destroy_motion_scene_node(get_blackboard()->m_scene, m_motion_object_nid);
    }
}


void  action_controller_human::next_round(float_32_bit  time_step_in_seconds)
{
    TMPROF_BLOCK();

    if (m_template_motion_info.src_pose.motion_name.empty())
    {
        m_template_motion_info.src_pose.motion_name = get_blackboard()->m_motion_templates->motions_map.begin()->first;
        m_template_motion_info.dst_pose.motion_name = get_blackboard()->m_motion_templates->motions_map.begin()->first;

        angeo::coordinate_system  agent_frame;
        {
            angeo::coordinate_system  tmp;
            get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_bone_nids.front(), false, tmp);
            agent_frame.set_origin(tmp.origin());
            get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, tmp);
            agent_frame.set_orientation(tmp.orientation());
        }
        skeletal_motion_templates::keyframes const&  src_animation =
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.src_pose.motion_name
                );
        m_motion_object_collider_props = src_animation.get_meta_data().m_motion_colliders.at(m_template_motion_info.src_pose.keyframe_index);
        m_motion_object_nid = detail::create_motion_scene_node(
                    get_blackboard()->m_scene,
                    get_blackboard()->m_agent_nid,
                    agent_frame,
                    m_motion_object_collider_props,
                    detail::rigid_body_motion()
                    );
        get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_nid, get_blackboard()->m_agent_id);
    }

    m_desired_angular_speed_in_world_space =
            get_blackboard()->m_cortex_cmd_turn_intensity * get_blackboard()->m_max_turn_speed_in_radians_per_second;

    angeo::coordinate_system  motion_object_frame;
    get_blackboard()->m_scene->get_frame_of_scene_node(m_motion_object_nid, false, motion_object_frame);
    vector3 const  gravity = get_blackboard()->m_scene->get_gravity_acceleration_at_point(motion_object_frame.origin());
    vector3 const  current_linear_velocity_in_world_space =
            get_blackboard()->m_scene->get_linear_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);

    bool  has_contact_with_ground = false;
    {
        auto const  begin_and_end = get_blackboard()->m_collision_contacts.equal_range(m_motion_object_nid);
        for (auto  it = begin_and_end.first; it != begin_and_end.second; ++it)
            if (angle(it->second.unit_normal, -gravity) < PI() / 4.0f)
            {
                has_contact_with_ground = true;
                break;
            }
    }

    vector3  agent_linear_acceleration = vector3_zero();
    if (has_contact_with_ground == true)
    {
        agent_linear_acceleration =
                (m_desired_linear_velocity_in_world_space - current_linear_velocity_in_world_space) / time_step_in_seconds;
        float_32_bit const  agent_linear_acceleration_magnitude = length(agent_linear_acceleration);
        float_32_bit constexpr  MAX_AGENT_LINEAR_ACCELERATION = 20.0f;
        if (agent_linear_acceleration_magnitude > MAX_AGENT_LINEAR_ACCELERATION)
            agent_linear_acceleration *= MAX_AGENT_LINEAR_ACCELERATION / agent_linear_acceleration_magnitude;
    }

    get_blackboard()->m_scene->set_linear_acceleration_of_rigid_body_of_scene_node(m_motion_object_nid, agent_linear_acceleration + gravity);

    if (m_template_motion_info.consumed_time_in_seconds + time_step_in_seconds > m_template_motion_info.total_interpolation_time_in_seconds)
    {
        float_32_bit const  time_till_dst_pose =
                m_template_motion_info.total_interpolation_time_in_seconds - m_template_motion_info.consumed_time_in_seconds;
        INVARIANT(time_till_dst_pose >= 0.0f);

        angeo::coordinate_system  agent_frame;
        {
            get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, agent_frame);
            //agent_frame.set_origin(agent_frame.origin() + time_till_dst_pose * m_desired_linear_velocity_in_world_space);
            agent_frame.set_origin(motion_object_frame.origin());
            {
                matrix33 const  reference_frame_rotation = quaternion_to_rotation_matrix(agent_frame.orientation());
                vector3  turn_axis;
                {
                    vector3  x, y;
                    rotation_matrix_to_basis(quaternion_to_rotation_matrix(motion_object_frame.orientation()), x, y, turn_axis);
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
                    dst_animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index).origin() -
                    dst_animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index - 1U).origin();
            desired_linear_velocity_in_animation_space += position_delta / std::max(time_delta, 0.0001f);
        }
        while (time_step_in_seconds > m_template_motion_info.total_interpolation_time_in_seconds);

        INVARIANT(m_template_motion_info.total_interpolation_time_in_seconds > 0.0f);

        m_desired_linear_velocity_in_world_space =
                quaternion_to_rotation_matrix(agent_frame.orientation())
                * desired_linear_velocity_in_animation_space;

        float_32_bit const  linear_acceleration_ratio =
                (length(current_linear_velocity_in_world_space) + 0.001f) / (length(m_desired_linear_velocity_in_world_space) + 0.001f);
        float_32_bit const  animation_speed_coef = std::max(0.5f, std::min(2.0f, linear_acceleration_ratio));
        m_template_motion_info.total_interpolation_time_in_seconds /= animation_speed_coef;

        time_step_in_seconds = std::min(time_step_in_seconds, m_template_motion_info.total_interpolation_time_in_seconds);
    }

    angeo::coordinate_system  agent_frame;
    {
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, agent_frame);
        //agent_frame.set_origin(agent_frame.origin() + time_step_in_seconds * m_desired_linear_velocity_in_world_space);
        agent_frame.set_origin(motion_object_frame.origin());
        {
            matrix33 const  reference_frame_rotation = quaternion_to_rotation_matrix(agent_frame.orientation());
            vector3  turn_axis;
            {
                vector3  x, y;
                rotation_matrix_to_basis(quaternion_to_rotation_matrix(motion_object_frame.orientation()), x, y, turn_axis);
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
            src_animation.get_meta_data().m_reference_frames.at(m_template_motion_info.src_pose.keyframe_index),
            dst_animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index),
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

    auto const&  src_meta_data = src_animation.get_meta_data().m_motion_colliders.at(m_template_motion_info.src_pose.keyframe_index);
    auto const&  dst_meta_data = dst_animation.get_meta_data().m_motion_colliders.at(m_template_motion_info.dst_pose.keyframe_index);

    float_32_bit const  src_weight = src_meta_data.arguments.back();
    float_32_bit const  dst_weight = dst_meta_data.arguments.back();

    float_32_bit const  motion_object_interpolation_param =
            (src_weight + dst_weight < 0.0001f) ? 0.5f : src_weight / (src_weight + dst_weight);

    if (motion_object_interpolation_param <= interpolation_param && m_motion_object_collider_props != dst_meta_data)
    {
        m_motion_object_collider_props = dst_meta_data;

        get_blackboard()->m_scene->unregister_to_collision_contacts_stream(m_motion_object_nid, get_blackboard()->m_agent_id);
        detail::rigid_body_motion const  rb_motion(get_blackboard()->m_scene, m_motion_object_nid);
        detail::destroy_collider_and_rigid_bofy_of_motion_scene_node(get_blackboard()->m_scene, m_motion_object_nid);
        detail::create_collider_and_rigid_body_of_motion_scene_node(
                get_blackboard()->m_scene,
                m_motion_object_nid,
                m_motion_object_collider_props,
                rb_motion);
        get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_nid, get_blackboard()->m_agent_id);
    }
}


}
