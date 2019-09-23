#include <ai/detail/rigid_body_motion.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


rigid_body_motion::rigid_body_motion()
    : nid()
    , frame()
    , forward(vector3_zero())
    , up(vector3_zero())
    , velocity({vector3_zero(),vector3_zero()})
    , acceleration({vector3_zero(),vector3_zero()})
    , inverted_mass(0.0f)
    , inverted_inertia_tensor(matrix33_zero())
{}

rigid_body_motion::rigid_body_motion(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        skeletal_motion_templates::anim_space_directions const& directions
        )
    : nid(motion_object_nid)
    , frame()
    , forward()
    , up()
    , velocity({ s->get_linear_velocity_of_rigid_body_of_scene_node(motion_object_nid),
                    s->get_angular_velocity_of_rigid_body_of_scene_node(motion_object_nid) })
    , acceleration({ s->get_linear_acceleration_of_rigid_body_of_scene_node(motion_object_nid),
                        s->get_angular_acceleration_of_rigid_body_of_scene_node(motion_object_nid) })
    , inverted_mass(s->get_inverted_mass_of_rigid_body_of_scene_node(motion_object_nid))
    , inverted_inertia_tensor(s->get_inverted_inertia_tensor_of_rigid_body_of_scene_node(motion_object_nid))
{
    update_frame_with_forward_and_up_directions(s, directions);
}

rigid_body_motion::rigid_body_motion(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        skeletal_motion_templates::anim_space_directions const&  directions,
        skeletal_motion_templates::mass_distribution const&  mass_distribution
        )
    : nid(motion_object_nid)
    , frame()
    , forward()
    , up()
    , velocity({ s->get_linear_velocity_of_rigid_body_of_scene_node(motion_object_nid),
        s->get_angular_velocity_of_rigid_body_of_scene_node(motion_object_nid) })
    , acceleration({ s->get_linear_acceleration_of_rigid_body_of_scene_node(motion_object_nid),
        s->get_angular_acceleration_of_rigid_body_of_scene_node(motion_object_nid) })
    , inverted_mass()
    , inverted_inertia_tensor()
{
    update_frame_with_forward_and_up_directions(s, directions);
    set_inverted_mass(mass_distribution);
    set_inverted_inertia_tensor(mass_distribution);
}

void  rigid_body_motion::update_frame_with_forward_and_up_directions(
        scene_ptr const  s,
        skeletal_motion_templates::anim_space_directions const& directions)
{
    s->get_frame_of_scene_node(nid, false, frame);

    matrix44 W;
    angeo::from_base_matrix(frame, W);

    forward = transform_vector(directions.forward(), W);
    up = transform_vector(directions.up(), W);
}

void  rigid_body_motion::update_linear_velocity(scene_ptr const  s)
{
    velocity.m_linear = s->get_linear_velocity_of_rigid_body_of_scene_node(nid);
}

void  rigid_body_motion::update_angular_velocity(scene_ptr const  s)
{
    velocity.m_angular = s->get_angular_velocity_of_rigid_body_of_scene_node(nid);
}

void  rigid_body_motion::integrate(float_32_bit const  time_step_in_seconds)
{
    velocity.m_linear += time_step_in_seconds * acceleration.m_linear;
    velocity.m_angular += time_step_in_seconds * acceleration.m_angular;
    angeo::integrate(frame, time_step_in_seconds, velocity.m_linear, velocity.m_angular);
    float_32_bit const  angular_speed = length(velocity.m_angular);
    if (angular_speed > 0.001f)
    {
        vector3 const  rot_axis = (1.0f / angular_speed) * velocity.m_angular;
        quaternion const  rot_quaternion = angle_axis_to_quaternion(angular_speed * time_step_in_seconds, rot_axis);
        matrix33 const  rot_matrix = quaternion_to_rotation_matrix(rot_quaternion);
        forward = normalised(rot_matrix * forward);
        up = normalised(rot_matrix * up);
    }
}

void  rigid_body_motion::commit(scene_ptr const  s, scene::node_id const&  motion_object_nid)
{
    //commit_frame(s, motion_object_nid);

    s->set_linear_velocity_of_rigid_body_of_scene_node(motion_object_nid, velocity.m_linear);
    s->set_angular_velocity_of_rigid_body_of_scene_node(motion_object_nid, velocity.m_angular);

    commit_accelerations(s, motion_object_nid);

    s->set_inverted_mass_of_rigid_body_of_scene_node(motion_object_nid, inverted_mass);
    s->set_inverted_inertia_tensor_of_rigid_body_of_scene_node(motion_object_nid, inverted_inertia_tensor);
}

void  rigid_body_motion::commit_accelerations(scene_ptr const  s, scene::node_id const&  motion_object_nid)
{
    s->set_linear_acceleration_of_rigid_body_of_scene_node(motion_object_nid, acceleration.m_linear);
    s->set_angular_acceleration_of_rigid_body_of_scene_node(motion_object_nid, acceleration.m_angular);
}

void  rigid_body_motion::commit_frame(scene_ptr const  s, scene::node_id const&  motion_object_nid)
{
    s->set_frame_of_scene_node(motion_object_nid, false, frame);
}


}}

namespace ai { namespace detail {


void  create_collider_and_rigid_body_of_motion_scene_node(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        skeletal_motion_templates::collider_ptr const&  collider_props,
        rigid_body_motion const&  rb_motion
        )
{
    if (auto const  capsule_ptr = std::dynamic_pointer_cast<skeletal_motion_templates::collider_capsule const>(collider_props))
    {
        s->insert_collision_capsule_to_scene_node(
                motion_object_nid,
                capsule_ptr->length,
                capsule_ptr->radius,
                angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING,
                1.0f,
                true
                );
    }
    else
    {
        NOT_IMPLEMENTED_YET();
    }
    s->set_frame_of_scene_node(motion_object_nid, false, rb_motion.frame);
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


scene::node_id  get_motion_object_nid(scene_ptr const  s, scene::node_id const  agent_nid)
{
    return s->get_aux_root_node_for_agent(agent_nid, "motion_object");
}


scene::node_id  create_motion_scene_node(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        angeo::coordinate_system const&  frame_in_world_space,
        skeletal_motion_templates::collider_ptr const&  collider_props,
        skeletal_motion_templates::mass_distribution const&  mass_distribution
        )
{
    s->insert_scene_node(motion_object_nid, frame_in_world_space, false);
    rigid_body_motion  rb_motion;
    rb_motion.set_linear_acceleration(s->get_gravity_acceleration_at_point(frame_in_world_space.origin()));
    rb_motion.set_inverted_mass(mass_distribution);
    rb_motion.set_inverted_inertia_tensor(mass_distribution);
    create_collider_and_rigid_body_of_motion_scene_node(s, motion_object_nid, collider_props, rb_motion);
    return motion_object_nid;
}


void  destroy_motion_scene_node(scene_ptr const  s, scene::node_id const&  motion_object_nid)
{
    destroy_collider_and_rigid_bofy_of_motion_scene_node(s, motion_object_nid);
    s->erase_scene_node(motion_object_nid);
}


}}
