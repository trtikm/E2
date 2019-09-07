#ifndef AI_DETAIL_RIGID_BODY_MOTION_HPP_INCLUDED
#   define AI_DETAIL_RIGID_BODY_MOTION_HPP_INCLUDED

#   include <ai/scene.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/rigid_body.hpp>

namespace ai { namespace detail {


struct  rigid_body_motion
{
    rigid_body_motion();
    rigid_body_motion(
            scene_ptr const  s,
            scene::node_id const&  motion_object_nid,
            skeletal_motion_templates::anim_space_directions const&  directions
            );

    rigid_body_motion(
            scene_ptr const  s,
            scene::node_id const&  motion_object_nid,
            skeletal_motion_templates::anim_space_directions const&  directions,
            skeletal_motion_templates::mass_distribution const&  mass_distribution
            );

    void  set_linear_acceleration(vector3 const& linear_acceleration)
    {
        acceleration.m_linear = linear_acceleration;
    }

    void  set_angular_acceleration(vector3 const& angular_acceleration)
    {
        acceleration.m_angular = angular_acceleration;
    }

    void  set_inverted_mass(skeletal_motion_templates::mass_distribution const&  mass_distribution)
    {
        inverted_mass = mass_distribution.mass_inverted;
    }

    void  set_inverted_inertia_tensor(skeletal_motion_templates::mass_distribution const&  mass_distribution)
    {
        inverted_inertia_tensor = mass_distribution.inertia_tensor_inverted;
    }

    void  update_frame_with_forward_and_up_directions(scene_ptr const  s, skeletal_motion_templates::anim_space_directions const& directions);
    void  update_linear_velocity(scene_ptr const  s);
    void  update_angular_velocity(scene_ptr const  s);

    void  integrate(float_32_bit const  time_step_in_seconds);

    void  commit(scene_ptr const  s, scene::node_id const&  motion_object_nid);
    void  commit_accelerations(scene_ptr const  s, scene::node_id const&  motion_object_nid);
    void  commit_frame(scene_ptr const  s, scene::node_id const&  motion_object_nid);

    scene::node_id  nid;
    angeo::coordinate_system  frame;
    vector3  forward;
    vector3  up;
    angeo::linear_and_angular_vector  velocity;
    angeo::linear_and_angular_vector  acceleration;
    float_32_bit  inverted_mass;
    matrix33  inverted_inertia_tensor;
};


}}

namespace ai { namespace detail {


void  create_collider_and_rigid_body_of_motion_scene_node(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        skeletal_motion_templates::collider_ptr const&  collider_props,
        rigid_body_motion const&  rb_motion
        );


void  destroy_collider_and_rigid_bofy_of_motion_scene_node(scene_ptr const  s, scene::node_id const&  motion_object_nid);


scene::node_id  get_motion_object_nid(scene_ptr const  s, scene::node_id const  agent_nid);


scene::node_id  create_motion_scene_node(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        angeo::coordinate_system const&  frame_in_world_space,
        skeletal_motion_templates::collider_ptr const&  collider_props,
        skeletal_motion_templates::mass_distribution const&  mass_distribution
        );

void  destroy_motion_scene_node(scene_ptr const  s, scene::node_id const&  motion_object_nid);


}}


#endif
