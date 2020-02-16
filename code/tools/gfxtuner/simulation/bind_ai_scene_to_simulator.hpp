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

    void  accept(request_ptr const  req);

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
            ) override;
    void  set_frame_of_scene_node(
            node_id const&  nid,
            bool const  frame_is_in_parent_space,   // When false, then the frame is assumed in the world space
            angeo::coordinate_system const&  frame
            ) override;
    void  erase_scene_node(node_id const&  nid) override;

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

    vector3  get_initial_external_linear_acceleration_at_point(vector3 const&  position_in_world_space) const override;
    vector3  get_initial_external_angular_acceleration_at_point(vector3 const&  position_in_world_space) const override;

    vector3  get_external_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const override;
    vector3  get_external_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const override;

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
            angeo::collision_object_id const  coid,
            vector3 const&  contact_point_in_world_space,
            vector3 const&  unit_normal_in_world_space,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  normal_force_magnitude,
            angeo::collision_object_id const* const  other_coid_ptr
            ) const;

    angeo::collision_scene const&  get_collision_scene() const;
    void  get_coids_under_scene_node(scn::scene_node_ptr const  node_ptr, std::function<bool(collision_object_id)> const&  acceptor) const;
    void  get_coids_under_scene_node(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const;
    void  get_coids_under_scene_node_subtree(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const;

private:

    using collision_contacts_stream_type =
            std::unordered_map<angeo::collision_object_id, std::pair<node_id, std::unordered_set<ai::object_id> > >;

    void  on_collision_contact(
            collision_contacts_stream_type::const_iterator const  it,
            vector3 const&  contact_point_in_world_space,
            vector3 const&  unit_normal_in_world_space,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  normal_force_magnitude,
            collision_contacts_stream_type::const_iterator const  other_it
            ) const;

    simulator*  m_simulator_ptr;
    collision_contacts_stream_type  m_collision_contacts_stream;
};


#endif
