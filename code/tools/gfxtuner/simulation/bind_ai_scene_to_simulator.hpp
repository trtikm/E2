#ifndef E2_TOOL_BIND_AI_SCENE_TO_SIMULATOR_HPP_INCLUDED
#   define E2_TOOL_BIND_AI_SCENE_TO_SIMULATOR_HPP_INCLUDED

#   include <ai/scene.hpp>
#   include <scene/scene_node_id.hpp>
#   include <scene/scene_record_id.hpp>
#   include <angeo/collision_scene.hpp>
#   include <unordered_map>
#   include <unordered_set>


struct  simulator;


struct bind_ai_scene_to_simulator : public ai::scene
{
    explicit  bind_ai_scene_to_simulator(simulator* const  simulator_ptr)
        : m_simulator_ptr(simulator_ptr)
        , m_collision_contacts_stream()
    {}

    ~bind_ai_scene_to_simulator()
    {
        m_collision_contacts_stream.clear();
        m_simulator_ptr = nullptr;
    }

    void  accept(request_ptr const  req, bool const  delay_processing_to_next_time_step);

    node_id  get_aux_root_node(ai::OBJECT_KIND const  kind, node_id const&  nid, std::string const&  aux_root_node_name) override;

    bool  has_scene_node(node_id const&  nid) const override;
    void  insert_scene_node(
            node_id const&  nid,
            angeo::coordinate_system const&  frame,
            bool const  frame_is_in_parent_space    // When false, then the frame is assumed in the world space
            ) override;
    void  get_frame_of_scene_node(
            node_id const&  nid,
            bool const  frame_in_parent_space,      // When false, then the frame will be in the world space
            angeo::coordinate_system&  frame
            ) const override;
    void  set_frame_of_scene_node(
            node_id const&  nid,
            bool const  frame_is_in_parent_space,   // When false, then the frame is assumed in the world space
            angeo::coordinate_system const&  frame
            ) override;
    vector3  get_origin_of_scene_node(
            node_id const&  nid,
            bool const  frame_in_parent_space       // When false, then the frame will be in the world space
            ) const override;
    void  erase_scene_node(node_id const&  nid) override;

    void  insert_collision_sphere_to_scene_node(
            node_id const&  nid,
            float_32_bit const  radius,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            angeo::COLLISION_CLASS const  collision_class,
            float_32_bit const  density_multiplier,
            bool const  as_dynamic
            ) override;
    void  insert_collision_capsule_to_scene_node(
            node_id const&  nid,
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            angeo::COLLISION_CLASS const  collision_class,
            float_32_bit const  density_multiplier,
            bool const  as_dynamic
            ) override;
    void  erase_collision_object_from_scene_node(node_id const&  nid) override;

    void  enable_colliding_colliders_of_scene_nodes(node_id const&  nid_1, node_id const&  nid_2, bool const  state) override;

    void  insert_rigid_body_to_scene_node(
            node_id const&  nid,
            vector3 const&  linear_velocity,
            vector3 const&  angular_velocity,
            vector3 const&  external_linear_acceleration,
            vector3 const&  external_angular_acceleration,
            float_32_bit const  mass_inverted,
            matrix33 const&  inertia_tensor_inverted
            ) override;
    vector3  get_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid) const override;
    void  set_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_velocity) override;
    vector3  get_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid) const override;
    void  set_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_velocity) override;
    vector3  get_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const override;
    void  set_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration) override;
    void  add_to_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration) override;
    vector3  get_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const override;
    void  set_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_acceleration) override;
    void  add_to_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_acceleration) override;
    float_32_bit  get_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid) const override;
    void  set_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid, float_32_bit const  inverted_mass) override;
    matrix33  get_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid) const override;
    void  set_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid, matrix33 const&  inverted_inertia_tensor) override;
    void  erase_rigid_body_from_scene_node(node_id const&  nid) override;

    node_id  get_scene_node_of_rigid_body_associated_with_collider(collision_object_id const  coid) const override;
    record_id  get_scene_record_of_rigid_body_associated_with_collider(collision_object_id const  coid) const override;
    node_id  get_scene_node_of_rigid_body_associated_with_collider_node(node_id const&  collider_node_id) const override;
    record_id  get_scene_record_of_rigid_body_associated_with_collider_node(node_id const&  collider_node_id) const override;

    vector3  get_initial_external_linear_acceleration_at_point(vector3 const&  position_in_world_space) const override;
    vector3  get_initial_external_angular_acceleration_at_point(vector3 const&  position_in_world_space) const override;

    vector3  get_external_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const override;
    vector3  get_external_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const override;

    vector3  get_linear_velocity_of_collider_at_point(collision_object_id const  coid, vector3 const&  point_in_world_space) const override;
    vector3  get_linear_velocity_of_rigid_body_at_point(node_id const&  rb_nid, vector3 const&  point_in_world_space) const override;

    void  register_to_collision_contacts_stream(
            node_id const&  collider_nid,   // A scene node with a collider whose collision contacts with other scene objects to capture.
            ai::object_id const&  oid       // Identifies an ai object which will receive the contancts of the collider to its blackboard.
            ) override;
    void  unregister_from_collision_contacts_stream(
            node_id const&  collider_nid,   // A scene node with a collider whose collision contacts with other scene objects to stop capturing.
            ai::object_id const&  oid       // Identifies an ai object which will stop receiving the contancts of the collider to its blackboard.
            ) override;
    bool  do_tracking_collision_contact_of_collision_object(angeo::collision_object_id const  coid) const;
    void  on_collision_contact(
            vector3 const&  contact_point_in_world_space,
            vector3 const&  unit_normal_in_world_space,
            angeo::collision_object_id const  coid,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            angeo::collision_object_id const  other_coid,
            angeo::COLLISION_MATERIAL_TYPE const  other_material
            ) const;


    angeo::collision_scene const&  get_collision_scene() const;
    void  get_coids_under_scene_node(scn::scene_node_ptr const  node_ptr, std::function<bool(collision_object_id)> const&  acceptor) const;
    void  get_coids_under_scene_node(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const;
    void  get_coids_under_scene_node_subtree(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const;

    custom_constraint_id  acquire_fresh_custom_constraint_id() override;
    void  release_generated_custom_constraint_id(custom_constraint_id const  ccid) override;
    void  insert_custom_constraint(
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
            float_32_bit const  initial_value_for_cache_miss
            ) override;
    void  insert_immediate_constraint(
            node_id const&  rb_nid_0,
            vector3 const&  linear_component_0,
            vector3 const&  angular_component_0,
            node_id const&  rb_nid_1,
            vector3 const&  linear_component_1,
            vector3 const&  angular_component_1,
            float_32_bit const  bias,
            float_32_bit const  variable_lower_bound,
            float_32_bit const  variable_upper_bound,
            float_32_bit const  initial_value
            ) override;

    record_id  __dbg_insert_sketch_box(node_id const& nid, std::string const& name, vector3 const&  half_sizes_along_axes, vector4 const&  colour) override;
    void  __dbg_erase_sketch_batch(record_id const&  rid) override;

private:

    using collision_contacts_stream_type =
            std::unordered_map<angeo::collision_object_id, std::pair<node_id, std::unordered_set<ai::object_id> > >;

    simulator*  m_simulator_ptr;
    collision_contacts_stream_type  m_collision_contacts_stream;
};


#endif
