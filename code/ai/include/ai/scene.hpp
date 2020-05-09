#ifndef AI_SCENE_HPP_INCLUDED
#   define AI_SCENE_HPP_INCLUDED

#   include <ai/scene_basic_types_binding.hpp>
#   include <ai/object_id.hpp>
#   include <ai/property_map.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <angeo/rigid_body_simulator.hpp>
#   include <memory>
#   include <functional>

namespace ai {


struct  scene
{
    using  node_id = scene_node_id;
    using  record_id = scene_record_id;
    using  collision_object_id = scene_collision_object_id;
    using  custom_constraint_id = angeo::rigid_body_simulator::custom_constraint_id;

    struct  collicion_contant_info
    {
        collicion_contant_info(
                vector3 const&  contact_point_in_world_space_,
                vector3 const&  unit_normal_in_world_space_,
                node_id const&  self_collider_nid_,
                collision_object_id const  self_coid_,
                angeo::COLLISION_MATERIAL_TYPE const  self_material_,
                node_id const&  other_collider_nid_,
                collision_object_id const  other_coid_,
                angeo::COLLISION_MATERIAL_TYPE const  other_material_
                )
            : contact_point_in_world_space(contact_point_in_world_space_)
            , unit_normal_in_world_space(unit_normal_in_world_space_)
            , self_collider_nid(self_collider_nid_)
            , self_coid(self_coid_)
            , self_material(self_material_)
            , other_collider_nid(other_collider_nid_)
            , other_coid(other_coid_)
            , other_material(other_material_)
        {}
        vector3  contact_point_in_world_space;
        vector3  unit_normal_in_world_space;
        node_id  self_collider_nid;
        collision_object_id  self_coid;
        angeo::COLLISION_MATERIAL_TYPE  self_material;
        node_id  other_collider_nid;
        collision_object_id  other_coid;
        angeo::COLLISION_MATERIAL_TYPE  other_material;
    };
    using  collicion_contant_info_ptr = std::shared_ptr<collicion_contant_info const>;

    // A base class of all requests to the scene. A request defines any operation
    // changing the set of ai objects in the ai module.
    // INVARIANT: A request may only be resolved outside a time step of the ai module.
    //            It in particular means that a request may NOT be resolved directly inside
    //            the call to 'accept' method. The resolution must be postponed till the
    //            current time step of ai module completes.
    struct  request { virtual ~request() {} };
    using  request_ptr = std::shared_ptr<request const>;

    template<typename request_type, typename... arg_types>
    static inline request_ptr  create_request(arg_types... args_for_constructor_of_the_request)
    { return std::make_shared<request_type>(args_for_constructor_of_the_request...); }

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
        // The passed property map must be the one of the sensor action kind BEGIN_OF_LIFE.
        request_merge_scene(property_map const&  begin_of_life_props) : props(begin_of_life_props.clone()) {}
        property_map  props;
    };
    using  request_merge_scene_ptr = std::shared_ptr<request_merge_scene const>;

    // A request of removal of a sub-tree in the scene.
    struct  request_erase_nodes_tree : public request
    {
        // Pass a root node of scene sub-tree to be erased. The sub-tree MUST represent either agent, device, or a sensor.
        request_erase_nodes_tree(node_id const&  root_nid_) : root_nid(root_nid_) {}
        node_id  root_nid;
    };
    using  request_erase_nodes_tree_ptr = std::shared_ptr<request_erase_nodes_tree const>;

    struct  request_update_radial_force_field : public request
    {
        request_update_radial_force_field(
                record_id const&  affected_object_rid_,
                record_id const&  force_field_rid_,
                property_map const&  props_
                )
            : affected_object_rid(affected_object_rid_)
            , force_field_rid(force_field_rid_)
            , props(props_.clone())
        {}
        record_id  affected_object_rid;
        record_id  force_field_rid;
        property_map  props;
    };
    using  request_update_radial_force_field_ptr = std::shared_ptr<request_update_radial_force_field const>;

    struct  request_erase_radial_force_field : public request
    {
        request_erase_radial_force_field(record_id const&  affected_object_rid_, record_id const&  force_field_rid_)
            : affected_object_rid(affected_object_rid_)
            , force_field_rid(force_field_rid_)
        {}
        record_id  affected_object_rid;
        record_id  force_field_rid;
    };
    using  request_erase_radial_force_field_ptr = std::shared_ptr<request_erase_radial_force_field const>;

    struct  request_update_linear_force_field : public request
    {
        request_update_linear_force_field(
                record_id const&  affected_object_rid_,
                record_id const&  force_field_rid_,
                property_map const&  props_
                )
            : affected_object_rid(affected_object_rid_)
            , force_field_rid(force_field_rid_)
            , props(props_.clone())
        {}
        record_id  affected_object_rid;
        record_id  force_field_rid;
        property_map  props;
    };
    using  request_update_linear_force_field_ptr = std::shared_ptr<request_update_linear_force_field const>;

    struct  request_leave_force_field : public request
    {
        request_leave_force_field(record_id const&  affected_object_rid_, record_id const&  force_field_rid_)
            : affected_object_rid(affected_object_rid_)
            , force_field_rid(force_field_rid_)
        {}
        record_id  affected_object_rid;
        record_id  force_field_rid;
    };
    using  request_leave_force_field_ptr = std::shared_ptr<request_leave_force_field const>;

    struct  request_insert_rigid_body_constraint : public request
    {
        request_insert_rigid_body_constraint(
                node_id const&  self_rb_nid_,
                vector3 const&  self_linear_component_,
                vector3 const&  self_angular_component_,
                node_id const&  other_rb_nid_,
                vector3 const&  other_linear_component_,
                vector3 const&  other_angular_component_,
                float_32_bit const  bias_,
                float_32_bit const  variable_lower_bound_,
                float_32_bit const  variable_upper_bound_,
                float_32_bit const  variable_initial_value_
                )
            : self_rb_nid(self_rb_nid_)
            , self_linear_component(self_linear_component_)
            , self_angular_component(self_angular_component_)
            , other_rb_nid(other_rb_nid_)
            , other_linear_component(other_linear_component_)
            , other_angular_component(other_angular_component_)
            , bias(bias_)
            , variable_lower_bound(variable_lower_bound_)
            , variable_upper_bound(variable_upper_bound_)
            , variable_initial_value(variable_initial_value_)
        {}
        node_id  self_rb_nid;
        vector3  self_linear_component;
        vector3  self_angular_component;
        node_id  other_rb_nid;
        vector3  other_linear_component;
        vector3  other_angular_component;
        float_32_bit  bias;
        float_32_bit  variable_lower_bound;
        float_32_bit  variable_upper_bound;
        float_32_bit  variable_initial_value;
    };
    using  request_insert_rigid_body_constraint_ptr = std::shared_ptr<request_insert_rigid_body_constraint const>;

    virtual ~scene() = 0 {}

    // Accepts a passed request. However, the request may NOT be resolved during any time step of the ai module.
    // So, the implemtation must save the request and resolve it after the time step of the ai module completes.
    virtual void  accept(request_ptr, bool delay_processing_to_next_time_step) = 0;

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
            ) const = 0;
    virtual void  set_frame_of_scene_node(
            node_id const&  nid,
            bool const  frame_is_in_parent_space,   // When false, then the frame is assumed in the world space
            angeo::coordinate_system const&  frame
            ) = 0;
    virtual vector3  get_origin_of_scene_node(
            node_id const&  nid,
            bool const  frame_in_parent_space       // When false, then the frame will be in the world space
            ) const = 0;
    // ASSUMPTION: The erased node must have NO children and NO record (in any folder).
    virtual void  erase_scene_node(node_id const&  nid) = 0;

    // ASSUMPTION: A scene node may have at most one collider.
    virtual void  insert_collision_sphere_to_scene_node(
            node_id const&  nid,
            float_32_bit const  radius,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            angeo::COLLISION_CLASS const  collision_class,
            float_32_bit const  density_multiplier,
            bool const  as_dynamic
            ) = 0;
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

    virtual  void  enable_colliding_colliders_of_scene_nodes(node_id const& nid_1, node_id const& nid_2, bool const  state) = 0;

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

    virtual vector3  get_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid) const = 0;
    virtual void  set_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_velocity) = 0;
    virtual vector3  get_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid) const = 0;
    virtual void  set_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  algular_velocity) = 0;
    virtual vector3  get_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const = 0;
    virtual void  set_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration) = 0;
    virtual void  add_to_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration) = 0;
    virtual vector3  get_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const = 0;
    virtual void  set_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_acceleration) = 0;
    virtual void  add_to_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_acceleration) = 0;
    virtual float_32_bit  get_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid) const = 0;
    virtual void  set_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid, float_32_bit const  inverted_mass) = 0;
    virtual matrix33  get_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid) const = 0;
    virtual void  set_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid, matrix33 const&  inverted_inertia_tensor) = 0;
    virtual void  erase_rigid_body_from_scene_node(node_id const&  nid) = 0;

    virtual node_id  get_scene_node_of_rigid_body_associated_with_collider(collision_object_id const  coid) const = 0;
    virtual record_id  get_scene_record_of_rigid_body_associated_with_collider(collision_object_id const  coid) const = 0;
    virtual node_id  get_scene_node_of_rigid_body_associated_with_collider_node(node_id const&  collider_node_id) const = 0;
    virtual record_id  get_scene_record_of_rigid_body_associated_with_collider_node(node_id const&  collider_node_id) const = 0;

    virtual vector3  get_linear_velocity_of_collider_at_point(collision_object_id const  coid, vector3 const&  point_in_world_space) const = 0;
    virtual vector3  get_linear_velocity_of_rigid_body_at_point(node_id const&  rb_nid, vector3 const&  point_in_world_space) const = 0;

    virtual vector3  get_initial_external_linear_acceleration_at_point(vector3 const&  position_in_world_space) const = 0;
    virtual vector3  get_initial_external_angular_acceleration_at_point(vector3 const&  position_in_world_space) const = 0;

    virtual vector3  get_external_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const = 0;
    virtual vector3  get_external_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const = 0;

    virtual void  register_to_collision_contacts_stream(
            node_id const&  collider_nid,   // A scene node with a collider whose collision contacts with other scene objects to capture.
            object_id const&  oid           // Identifies an ai object which will receive the contancts of the collider to its blackboard.
            ) = 0;
    virtual void  unregister_from_collision_contacts_stream(
            node_id const&  collider_nid,   // A scene node with a collider whose collision contacts with other scene objects to stop capturing.
            object_id const&  oid           // Identifies an ai object which will stop receiving the contancts of the collider to its blackboard.
            ) = 0;

    virtual angeo::collision_scene const&  get_collision_scene() const = 0;
    virtual void  get_coids_under_scene_node(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const = 0;
    virtual void  get_coids_under_scene_node_subtree(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const = 0;

    virtual custom_constraint_id  acquire_fresh_custom_constraint_id() = 0;
    virtual void  release_generated_custom_constraint_id(custom_constraint_id const  ccid) = 0;
    virtual void  insert_custom_constraint(
            custom_constraint_id const  ccid,
            node_id const&  rb_nid_0,
            vector3 const&  linear_component_0,
            vector3 const&  angular_component_0,
            node_id const&  rb_nid_1,
            vector3 const&  linear_component_1,
            vector3 const&  angular_component_1,
            float_32_bit const  bias,
            float_32_bit const  variable_lower_bound,
            float_32_bit const  variable_upper_bound,
            float_32_bit const  initial_value_for_cache_miss = 0.0f
            ) = 0;
    virtual void  insert_immediate_constraint(
            node_id const&  rb_nid_0,
            vector3 const&  linear_component_0,
            vector3 const&  angular_component_0,
            node_id const&  rb_nid_1,
            vector3 const&  linear_component_1,
            vector3 const&  angular_component_1,
            float_32_bit const  bias,
            float_32_bit const  variable_lower_bound,
            float_32_bit const  variable_upper_bound,
            float_32_bit const  initial_value = 0.0f
            ) = 0;
};


using  scene_ptr = std::shared_ptr<scene>;


}

#endif
