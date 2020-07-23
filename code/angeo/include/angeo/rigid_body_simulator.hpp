#ifndef ANGEO_RIGID_BODY_SIMULATOR_HPP_INCLUDED
#   define ANGEO_RIGID_BODY_SIMULATOR_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/rigid_body.hpp>
#   include <angeo/custom_constraint_id.hpp>
#   include <angeo/motion_constraint_system.hpp>
#   include <angeo/contact_id.hpp>
#   include <angeo/collision_material.hpp>
#   include <com/object_guid.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <vector>
#   include <unordered_set>
#   include <unordered_map>

namespace angeo {


struct  rigid_body_simulator
{
    struct  computation_statistics
    {
        computation_statistics();
        natural_32_bit  m_num_rigid_bodies;
        natural_32_bit  m_contact_cache_size;
        natural_32_bit  m_num_contact_cache_hits;
        natural_32_bit  m_num_contact_cache_misses;
        natural_32_bit  m_custom_constraints_cache_size;
        natural_32_bit  m_num_custom_constraints_cache_hits;
        natural_32_bit  m_num_custom_constraints_cache_misses;
        natural_64_bit  m_performed_simulation_steps;
        float_64_bit  m_duration_of_rigid_body_update_in_seconds;
        float_64_bit  m_duration_of_contact_cache_update_in_seconds;
    };

    rigid_body_simulator();

    rigid_body_id  insert_rigid_body(
            vector3 const&  position_of_mass_centre,
            quaternion const&  orientation,
            float_32_bit const  inverted_mass,                          // The value 0.0f means the mass is infinite. 
            matrix33 const&  inverted_inertia_tensor_in_local_space,    // Zero matrix means an infinite inertia.
            vector3 const&  linear_velocity = vector3_zero(),
            vector3 const&  angular_velocity = vector3_zero(),
            vector3 const&  external_linear_acceleration = vector3_zero(),
            vector3 const&  external_angular_acceleration = vector3_zero()
            );

    void  erase_rigid_body(rigid_body_id const  id);

    bool  contains(rigid_body_id const  id) { return id < m_rigid_bodies.size() && m_invalid_rigid_body_ids.count(id) == 0UL; }

    void  clear();

    vector3 const&  get_position_of_mass_centre(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_position_of_mass_centre; }
    void  set_position_of_mass_centre(rigid_body_id const  id, vector3 const&  position)
    { m_rigid_bodies.at(id).m_position_of_mass_centre = position; erase_from_contact_cache(id); }

    quaternion const&  get_orientation(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_orientation; }
    void  set_orientation(rigid_body_id const  id, quaternion const&  orientation);

    vector3 const&  get_linear_velocity(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_velocity.m_linear; }
    vector3 const&  get_angular_velocity(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_velocity.m_angular; }
    void  set_linear_velocity(rigid_body_id const  id, vector3 const&  velocity) { m_rigid_bodies.at(id).m_velocity.m_linear = velocity; }
    void  set_angular_velocity(rigid_body_id const  id, vector3 const&  velocity) { m_rigid_bodies.at(id).m_velocity.m_angular = velocity; }

    vector3 const&  get_linear_acceleration(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_acceleration_from_external_forces.m_linear; }
    vector3 const&  get_angular_acceleration(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_acceleration_from_external_forces.m_angular; }
    void  set_linear_acceleration_from_source(rigid_body_id const  id, com::object_guid const  source_guid, vector3 const&  acceleration);
    void  set_angular_acceleration_from_source(rigid_body_id const  id, com::object_guid const  source_guid, vector3 const&  acceleration);
    void  remove_linear_acceleration_from_source(rigid_body_id const  id, com::object_guid const  source_guid);
    void  remove_angular_acceleration_from_source(rigid_body_id const  id, com::object_guid const  source_guid);
    void  remove_linear_accelerations_from_all_sources(rigid_body_id const  id);
    void  remove_angular_accelerations_from_all_sources(rigid_body_id const  id);

    // Deprecaded section - begin
        vector3 const&  get_external_linear_acceleration(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_acceleration_from_external_forces.m_linear; }
        vector3 const&  get_external_angular_acceleration(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_acceleration_from_external_forces.m_angular; }
        void  set_external_linear_acceleration(rigid_body_id const  id, vector3 const&  acceleration) { m_rigid_bodies.at(id).m_acceleration_from_external_forces.m_linear = acceleration; }
        void  set_external_angular_acceleration(rigid_body_id const  id, vector3 const&  acceleration) { m_rigid_bodies.at(id).m_acceleration_from_external_forces.m_angular = acceleration; }
    // Deprecaded section - end

    float_32_bit  get_inverted_mass(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_inverted_mass; }
    void  set_inverted_mass(rigid_body_id const  id, float_32_bit const  inverted_mass) { m_rigid_bodies.at(id).m_inverted_mass = inverted_mass; }

    matrix33 const&  get_inverted_inertia_tensor_in_world_space(rigid_body_id const  id) const { return m_rigid_bodies.at(id).m_inverted_inertia_tensor; }
    matrix33 const&  get_inverted_inertia_tensor_in_local_space(rigid_body_id const  id) const { return m_inverted_inertia_tensors.at(id); }
    void  set_inverted_inertia_tensor_in_local_space(rigid_body_id const  id, matrix33 const&  inverted_inertia_tensor_in_local_space);

    motion_constraint_system&  get_constraint_system() { return m_constraint_system; }
    motion_constraint_system const&  get_constraint_system() const { return m_constraint_system; }

    struct  contact_friction_constraints_info
    {
        std::vector<vector3>  m_unit_tangent_plane_vectors;
                                    // A vector of at least 2 linearly independent unit vectors, all lying the tangent
                                    // plane of a related contact point (the contact point is not specified here; it is
                                    // passed to function 'insert_contact_constraints' together with this instance.
                                    // Each vector3 identifies a direction in which the corresponding friction force
                                    // will act. A friction force can either acts in both positive and negative direction
                                    // of the vector, or only in the positive. You, can control that by the flag
                                    // 'm_suppress_negative_directions' (see below).
                                    // The most common case is that you specify only 2 vectors orthogonal to each other
                                    // and 'm_suppress_negative_directions' is set to 'false'.
        bool  m_suppress_negative_directions;
                                    // If true, then friction forces computed will act only in positive directions of the
                                    // vectors in 'm_unit_tangent_plane_vectors'. Otherwise, the forces can act in both
                                    // directions (positive and negative).
        float_32_bit  m_max_tangent_relative_speed_for_static_friction;
                                    // If the relative tnagent plane velocity of both rigid bodies at the contact point is
                                    // smaller that the passed value, then static friction coefficient will be used for this
                                    // constraint, else the dynamic friction coefficient will be used. Typically, the value
                                    // 0.001f (tangent speed 1mm per second) works well in most cases.
    };

    // Inserts contact constraints which will prevent penetration of collision objects of the specified
    // rigid bodies in the passed contact point and in the normal direction. If 'friction_info_ptr != nullptr',
    // then there are also generated constraints applying friction forces at the contact point.
    void  insert_contact_constraints(
            rigid_body_id const  rb_0,
            rigid_body_id const  rb_1,
            contact_id const&  cid,
            vector3 const&  contact_point,
            vector3 const&  unit_normal, // Must point towards the first rigid body (i.e. towards rb_0)
            COLLISION_MATERIAL_TYPE  rb_0_material, // The collision material of the rigid body 'rb_0' at the 'contact_point'.
            COLLISION_MATERIAL_TYPE  rb_1_material, // The collision material of the rigid body 'rb_1' at the 'contact_point'.
            contact_friction_constraints_info const* const  friction_info_ptr,
                                    // When passed 'nullptr', then no friction constraint will be generated.
            float_32_bit const  penetration_depth,
            float_32_bit const  depenetration_coef = 20.0f,
                                    // The greater the coeficient, the faster the resolution
                                    // of the penetration depth. However, for smooth convergence
                                    // of the constraint it is recommended to keep the value of
                                    // the coefficient smaller than the actual time-stepping
                                    // frequency of the physiscal system. The default value
                                    // assumes that the update frequency of the physical system
                                    // does not drop under 20Hz. We recommend to pass custom
                                    // values for the coeeficient (i.e. not to rely of the defaul one)
                                    // correlating with update frequency of the system.
            std::vector<motion_constraint_system::constraint_id>* const  output_constraint_ids_ptr = nullptr
                                    // If passed a valid pointer (i.e. not nullptr), then the referenced vector
                                    // is extended by all generated constraints. Namely, first will be added
                                    // the one preventing contact penetration, and then follow all friction
                                    // constraints (if 'friction_info_ptr != nullptr', of course; for each vector3
                                    // in 'friction_info_ptr->m_unit_tangent_plane_vectors' one friction constraint).
            );

    custom_constraint_id  gen_fresh_custom_constraint_id();
    void  release_generated_custom_constraint_id(custom_constraint_id const  id);

    void  insert_custom_constraint(
            custom_constraint_id const  id,
            rigid_body_id const  rb_0,
            vector3 const&  linear_component_0,
            vector3 const&  angular_component_0,
            rigid_body_id const  rb_1,
            vector3 const&  linear_component_1,
            vector3 const&  angular_component_1,
            float_32_bit const  bias,
            motion_constraint_system::variable_bound_getter const&  variable_lower_bound = variable_bound_getter_ZERO,
            motion_constraint_system::variable_bound_getter const&  variable_upper_bound = variable_bound_getter_MAX,
            float_32_bit const  initial_value_for_cache_miss = 0.0f
            );

    void  clear_cache_of_custom_constraint(custom_constraint_id const  id) { m_custom_constraints_cache.erase(id); }

    static float_32_bit  variable_bound_getter_MIN(std::vector<float_32_bit> const&) { return -std::numeric_limits<float_32_bit>::max(); }
    static float_32_bit  variable_bound_getter_ZERO(std::vector<float_32_bit> const&) { return 0.0f; }
    static float_32_bit  variable_bound_getter_MAX(std::vector<float_32_bit> const&) { return std::numeric_limits<float_32_bit>::max(); }

    void  solve_constraint_system(
            float_32_bit const  time_step_in_seconds,
            float_32_bit const  max_computation_time_in_seconds
            );
    void  integrate_motion_of_rigid_bodies(float_32_bit const  time_step_in_seconds);
    void  prepare_contact_cache_and_constraint_system_for_next_frame();

    void  do_simulation_step(
            float_32_bit const  time_step_in_seconds,
            float_32_bit const  max_computation_time_in_seconds
            );


    // The both statistics below are updated updated during an execution of the method 'do_simulation_step' above.
    // It means that values in the returned structures are vaild/actual only after each call to 'do_simulation_step'.
    computation_statistics const&  get_simulation_statistics() const { return m_statistics; }
    motion_constraint_system::computation_statistics const&  get_constraint_system_statistics() const
    { return m_constraint_system.get_statistics(); }

private:

    void  update_dependent_variables_of_rigid_body(rigid_body_id const  id);

    float_32_bit  read_contact_cache(
            pair_of_rigid_body_ids const&  rb_ids,
            contact_id const&  cid,
            natural_32_bit const  contact_vector_id,
            float_32_bit const  value_on_cache_miss
            ) const;
    void  erase_from_contact_cache(rigid_body_id const&  rb_id)
    { m_invalidated_rigid_bodies_in_contact_cache.insert(rb_id); }
    void  update_contact_cache();

    float_32_bit  read_custom_constraints_cache(
            custom_constraint_id const  id,
            pair_of_rigid_body_ids const&  rb_ids,
            float_32_bit const  value_on_cache_miss
            ) const;
    void  update_custom_constraints_cache();

    mutable computation_statistics  m_statistics;

    motion_constraint_system  m_constraint_system;

    std::unordered_map<pair_of_rigid_body_ids,
                       std::unordered_map<contact_id,
                                          std::unordered_map<natural_32_bit, float_32_bit> > >
            m_contact_cache;
    std::unordered_set<rigid_body_id>  m_invalidated_rigid_bodies_in_contact_cache;
    std::unordered_map<motion_constraint_system::constraint_id, std::pair<contact_id, natural_32_bit> >
            m_from_constraints_to_contact_ids;

    std::unordered_map<custom_constraint_id, std::pair<float_32_bit, pair_of_rigid_body_ids> >  m_custom_constraints_cache;
    std::unordered_map<motion_constraint_system::constraint_id, custom_constraint_id>  m_from_constraints_to_custom_constraint_ids;
    std::unordered_set<custom_constraint_id>  m_released_custom_constraint_ids;
    custom_constraint_id  m_max_generated_custom_constraint_id;

    // All the vectors below have the same size.

    std::vector<rigid_body>  m_rigid_bodies;
    std::vector<matrix33>  m_inverted_inertia_tensors;  // Always in the local space. Zero matrix means an infinite inertia.
    std::unordered_set<rigid_body_id>  m_invalid_rigid_body_ids;
    std::unordered_map<rigid_body_id, std::unordered_map<com::object_guid, vector3> >  m_linear_accelerations_from_sources;
    std::unordered_map<rigid_body_id, std::unordered_map<com::object_guid, vector3> >  m_angular_accelerations_from_sources;
};


vector3  compute_velocity_of_point_of_rigid_body(
        // All vectors must be in the same coord system, of course.
        vector3 const&  rigid_body_mass_centre_position,
        vector3 const&  rigid_body_linear_velocity,
        vector3 const&  rigid_body_angular_velocity,
        vector3 const&  point
        );


}

#endif
