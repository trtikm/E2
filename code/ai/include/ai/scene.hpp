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
    using  record_id = scn::scene_record_id;
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
    // The imported scene must have exactly one root node (nodes strating with
    // @ are ignored) and it must represent either agent, device, or sensor.
    // The request MUST be resolved AFTER the current time step of the ai module and
    // before the next time step of the ai module.
    struct  request_merge_scene : public request
    {
        request_merge_scene(
                std::string const&  scene_id_,
                node_id const&  parent_nid_,
                node_id const&  frame_nid_,
                vector3 const&  linear_velocity_,
                vector3 const&  angular_velocity_,
                node_id const&  velocities_frame_nid_
                )
            : scene_id(scene_id_)
            , parent_nid(parent_nid_)
            , frame_nid(frame_nid_)
            , linear_velocity(linear_velocity_)
            , angular_velocity(angular_velocity_)
            , velocities_frame_nid(velocities_frame_nid_)
        {}

        std::string  scene_id;                  // A unique id of scene to be imported (merged) into the current scene.
                                                // Typically, an id is a scene directory (relative to the data root directory).
        node_id  parent_nid;                    // A scene node under which the scene will be merged. If the 'parent_nid' node
                                                // already contains a node of the same name as the root node of the imported
                                                // scene, then the name of the name of the impoted root node is extended by
                                                // a suffix making the resulting name unique under the 'parent_nid' node.
                                                //      For imported agent the 'parent_nid' must be empty.
                                                //      For imported sensor the 'parent_nid' must reference a node under
                                                //      nodes tree of an agent or a device.
                                                // NOTE: When 'parent_nid' is empty (i.e. not valid), then the root node of the
                                                //       impored scene will be put at the root level of the current scene.
        node_id  frame_nid;                     // The root node of the imported scene will be put at such location under
                                                // 'parent_nid' node, so that its world matrix will be the same as the one
                                                // of the 'frame_nid' node.
        vector3  linear_velocity;               // If the root node of the impored scene has a rigid body record, then its
                                                // its linear velocity is initialised to this vector.
        vector3  angular_velocity;              // If the root node of the impored scene has a rigid body record, then its
                                                // its angular velocity is initialised to this vector.

        node_id  velocities_frame_nid;          // When represents a valid scene node, then both linear/angular_velocity
                                                // will be transformed from the space of the referenced frame to the world space.
                                                // Otherwise, both vectors are used not-transformed.
    };
    using  request_merge_scene_ptr = std::shared_ptr<request_merge_scene const>;

    // A request of removal of a sub-tree in the scene.
    struct  request_erase_nodes_tree : public request
    {
        request_erase_nodes_tree(scene::node_id const&  root_nid_) : root_nid(root_nid_) {}
        scene::node_id  root_nid;               // A root node of scene sub-tree to be erased. The sub-tree MUST represent
                                                // either agent, device, or a sensor.
    };
    using  request_erase_nodes_tree_ptr = std::shared_ptr<request_erase_nodes_tree const>;

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
