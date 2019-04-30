#ifndef E2_TOOL_BIND_AI_SCENE_TO_SIMULATOR_HPP_INCLUDED
#   define E2_TOOL_BIND_AI_SCENE_TO_SIMULATOR_HPP_INCLUDED

#   include <ai/scene.hpp>
#   include <scene/scene_node_id.hpp>
#   include <scene/scene_record_id.hpp>
#   include <angeo/collision_scene.hpp>
#   include <angeo/rigid_body_simulator.hpp>
#   include <unordered_map>


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

    node_id  get_aux_root_node_for_agent(node_id const&  agent_nid, std::string const&  aux_root_node_name) override;

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
    vector3  get_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid) override;
    void  set_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_velocity) override;
    vector3  get_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) override;
    void  set_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration) override;
    void  erase_rigid_body_from_scene_node(node_id const&  nid) override;

    vector3  get_gravity_acceleration_at_point(vector3 const&  position) const override; // Always in the world space.

    void  register_to_collision_contacts_stream(
            node_id const&  collider_nid,   // A scene node with a collider whose collision contacts with other scene objects to capture.
            ai::agent_id const  agent_id    // Identifies an agent which will receive the contancts of the collider to its blackboard.
            ) override;
    void  unregister_to_collision_contacts_stream(
            node_id const&  collider_nid,   // A scene node with a collider whose collision contacts with other scene objects to stop capturing.
            ai::agent_id const  agent_id    // Identifies an agent which will stop receiving the contancts of the collider to its blackboard.
            ) override;
    void  on_collision_contact(
            angeo::collision_object_id const  coid_1,
            angeo::collision_object_id const  coid_2,
            vector3 const&  contact_point,
            vector3 const&  unit_normal
            ) const;

private:

    simulator*  m_simulator_ptr;
    std::unordered_map<angeo::collision_object_id, std::pair<node_id, ai::agent_id> >  m_collision_contacts_stream;
};


#endif
