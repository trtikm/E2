#ifndef AI_SCENE_HPP_INCLUDED
#   define AI_SCENE_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <scene/scene_node_id.hpp>
#   include <scene/scene_record_id.hpp>
#   include <angeo/collision_scene.hpp>
#   include <angeo/rigid_body_simulator.hpp>
#   include <memory>

namespace ai {


struct  scene
{
    using  node_id = scn::scene_node_id;

    virtual ~scene() = 0 {}

    // When an agent wants to create auxiliary scene nodes outside the subtree under its agent node,
    // then he should use this method to obtain a 'standard' auxiliary root node of the passed name
    // for the agent.
    virtual node_id  get_aux_root_node_for_agent(node_id const&  agent_nid, std::string const&  aux_root_node_name) = 0;

    virtual void  insert_scene_node(
            node_id const&  nid,
            angeo::coordinate_system const&  frame,
            bool const  frame_is_in_parent_space    // When false, then the frame is assumed in the world space
            ) = 0;
    virtual void  get_frame_of_scene_node(
            node_id const&  nid,
            bool const  frame_in_parent_space,      // When false, then the frame will be in the world space
            angeo::coordinate_system&  frame
            ) = 0;
    virtual void  set_frame_of_scene_node(
            node_id const&  nid,
            bool const  frame_is_in_parent_space,   // When false, then the frame is assumed in the world space
            angeo::coordinate_system const&  frame
            ) = 0;
    // ASSUMPTION: The erased node must have NO children and NO record (in any folder).
    virtual void  erase_scene_node(node_id const&  nid) = 0;

    // ASSUMPTION: A scene node may have at most one collider.
    virtual void  insert_collision_capsule_to_scene_node(
            node_id const&  nid,
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  density_multiplier,
            bool const  as_dynamic
            ) = 0;
    virtual void  erase_collision_object_from_scene_node(node_id const&  nid) = 0;

    // ASSUMPTION: Each scene node may have at most one rigid body in its subtree (where the node is the root node of the subtree).
    virtual void  insert_rigid_body_to_scene_node(
            node_id const&  nid,
            vector3 const&  linear_velocity,
            vector3 const&  angular_velocity,
            vector3 const&  external_linear_acceleration,
            vector3 const&  external_angular_acceleration,
            float_32_bit const  mass_inverted,
            matrix33 const&  inertia_tensor_inverted
            ) = 0;
    virtual vector3  get_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid) = 0;
    virtual void  set_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_velocity) = 0;
    virtual void  erase_rigid_body_from_scene_node(node_id const&  nid) = 0;
};


using  scene_ptr = std::shared_ptr<scene>;


}

#endif