#ifndef AI_SCENE_HPP_INCLUDED
#   define AI_SCENE_HPP_INCLUDED

#   include <ai/object_id.hpp>
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
    using  collision_object_id = angeo::collision_object_id;

    struct  collicion_contant_info
    {
        collicion_contant_info(
                vector3 const&  contact_point_in_world_space_,
                vector3 const&  unit_normal_in_world_space_,
                angeo::COLLISION_MATERIAL_TYPE const  material_,
                float_32_bit const  normal_force_magnitude_
                )
            : contact_point_in_world_space(contact_point_in_world_space_)
            , unit_normal_in_world_space(unit_normal_in_world_space_)
            , material(material_)
            , normal_force_magnitude(normal_force_magnitude_)
        {}
        vector3  contact_point_in_world_space;
        vector3  unit_normal_in_world_space;
        angeo::COLLISION_MATERIAL_TYPE  material;
        float_32_bit  normal_force_magnitude;
    };

    // A base class of all requests to the scene. A request defines any operation
    // changing the set of ai objects in the ai module.
    // INVARIANT: A request may only be resolved outside a time step of the ai module.
    //            It in particular means that a request may NOT be resolved directly inside
    //            the call to 'accept' method. The resolution must be postponed till the
    //            current time step of ai module completes.
    struct  request { virtual ~request() {} };
    using  request_ptr = std::shared_ptr<request const>;
    template<typename T>
    static inline std::shared_ptr<T const>  cast(request_ptr const  req)
    { return std::dynamic_pointer_cast<T const>(req); }

    // A request of importing (merging) a template scene into the current one.
    // The request MUST be resolved AFTER the current time step of the ai module and
    // before the next time step of the ai module.
    struct  request_merge_scene : public request
    {
        request_merge_scene(
                std::string const& template_pathname_,
                scn::scene_node_id const& parent_nid_,
                angeo::coordinate_system const& frame_,
                bool const  frame_is_in_parent_space_
                )
            : template_pathname(template_pathname_)
            , parent_nid(parent_nid_)
            , frame(frame_)
            , frame_is_in_parent_space(frame_is_in_parent_space_)
        {}

        std::string  template_pathname;         // Path-name of scene to be imported (merged) into the current scene.
                                                //      The imported scene must have exactly one root node (besides @pivot)
                                                //      and it must represent either agent, device, or sensor.
        node_id  parent_nid;                    // A scene node under which the scene will be merged.
                                                //      For imported agent the parent_nid must be empty.
                                                //      For imported sensor the parent_nid must reference a node under
                                                //      nodes tree of an agent or a device.
        angeo::coordinate_system  frame;        // A desired location of the root node of the merged scene.
        bool  frame_is_in_parent_space;         // When false, then the frame is assumed in the world space
    };
    using  request_merge_scene_ptr = std::shared_ptr<request_merge_scene const>;
    static inline request_merge_scene_ptr  create_request_merge_scene(
            std::string const& template_pathname_,
            scn::scene_node_id const& parent_nid_,
            angeo::coordinate_system const& frame_,
            bool const  frame_is_in_parent_space_
            )
    { return std::make_shared<request_merge_scene const>(template_pathname_, parent_nid_, frame_, frame_is_in_parent_space_); }

    // A request of removal of a sub-tree in the scene.
    struct  request_erase_nodes_tree : public request
    {
        request_erase_nodes_tree(scene::node_id const&  root_nid_) : root_nid(root_nid_) {}
        scene::node_id  root_nid;               // A root node of scene sub-tree to be erased. The sub-tree MUST represent
                                                // either agent, device, or a sensor.
    };
    using  request_erase_nodes_tree_ptr = std::shared_ptr<request_erase_nodes_tree const>;
    static inline request_erase_nodes_tree_ptr  create_request_erase_nodes_tree(scene::node_id const& root_nid_)
    { return std::make_shared<request_erase_nodes_tree const>(root_nid_); }

    virtual ~scene() = 0 {}

    // Accepts a passed request. However, the request may NOT be resolved during any time step of the ai module.
    // So, the implemtation must save the request and resolve it after the time step of the ai module completes.
    virtual void  accept(request_ptr) = 0;

    // When an agent wants to create auxiliary scene nodes outside the subtree under its agent node,
    // then he should use this method to obtain a 'standard' auxiliary root node of the passed name
    // for the agent.
    virtual node_id  get_aux_root_node(OBJECT_KIND const  kind, node_id const&  nid, std::string const&  aux_root_node_name) = 0;

    virtual bool  has_scene_node(node_id const&  nid) const = 0;
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
            angeo::COLLISION_CLASS const  collision_class,
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
    virtual vector3  get_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid) = 0;
    virtual void  set_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  algular_velocity) = 0;
    virtual vector3  get_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) = 0;
    virtual void  set_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration) = 0;
    virtual void  add_to_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration) = 0;
    virtual vector3  get_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) = 0;
    virtual void  set_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_acceleration) = 0;
    virtual void  add_to_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_acceleration) = 0;
    virtual float_32_bit  get_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid) = 0;
    virtual void  set_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid, float_32_bit const  inverted_mass) = 0;
    virtual matrix33  get_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid) = 0;
    virtual void  set_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid, matrix33 const&  inverted_inertia_tensor) = 0;
    virtual void  erase_rigid_body_from_scene_node(node_id const&  nid) = 0;

    virtual vector3  get_gravity_acceleration_at_point(vector3 const&  position) const = 0; // Always in the world space.

    virtual void  register_to_collision_contacts_stream(
            node_id const&  collider_nid,   // A scene node with a collider whose collision contacts with other scene objects to capture.
            object_id const&  oid           // Identifies an ai object which will receive the contancts of the collider to its blackboard.
            ) = 0;
    virtual void  unregister_to_collision_contacts_stream(
            node_id const&  collider_nid,   // A scene node with a collider whose collision contacts with other scene objects to stop capturing.
            object_id const&  oid           // Identifies an ai object which will stop receiving the contancts of the collider to its blackboard.
            ) = 0;

    virtual angeo::collision_scene const&  get_collision_scene() const = 0;
    virtual void  get_coids_under_scene_node(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const = 0;
    virtual void  get_coids_under_scene_node_subtree(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const = 0;
};


using  scene_ptr = std::shared_ptr<scene>;


}

#endif
