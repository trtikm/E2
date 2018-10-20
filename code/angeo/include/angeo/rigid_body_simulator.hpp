#ifndef ANGEO_RIGID_BODY_SIMULATOR_HPP_INCLUDED
#   define ANGEO_RIGID_BODY_SIMULATOR_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/rigid_body.hpp>
#   include <angeo/motion_constraint_system.hpp>
#   include <angeo/contact_id.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <vector>
#   include <unordered_set>
#   include <unordered_map>

namespace angeo {


struct  rigid_body_simulator
{
    rigid_body_id  insert_rigid_body(
            vector3 const&  position_of_mass_centre,
            quaternion const&  orientation,
            float_32_bit const  inverted_mass,                          // The value 0.0f means the mass is infinite. 
            matrix33 const&  inverted_inertia_tensor_in_local_space,    // Zero matrix means an infinite inertia.
            vector3 const&  linear_velocity = vector3_zero(),
            vector3 const&  angular_velocity = vector3_zero(),
            vector3 const&  external_force = vector3_zero(),
            vector3 const&  external_torque = vector3_zero()
            );

    void  erase_rigid_body(rigid_body_id const  id);

    vector3 const&  get_position_of_mass_centre(rigid_body_id const  id) const { return m_rigid_bosies.at(id).m_position_of_mass_centre; }
    void  set_position_of_mass_centre(rigid_body_id const  id, vector3 const&  position)
    { m_rigid_bosies.at(id).m_position_of_mass_centre = position; erase_from_contact_cache(id); }

    quaternion const&  get_orientation(rigid_body_id const  id) const { return m_rigid_bosies.at(id).m_orientation; }
    void  set_orientation(rigid_body_id const  id, quaternion const&  orientation);

    vector3 const&  get_linear_velocity(rigid_body_id const  id) const { return m_rigid_bosies.at(id).m_velocity.m_linear; }
    vector3 const&  get_anguar_velocity(rigid_body_id const  id) const { return m_rigid_bosies.at(id).m_velocity.m_angular; }
    void  set_linear_velocity(rigid_body_id const  id, vector3 const&  velocity) { m_rigid_bosies.at(id).m_velocity.m_linear = velocity; }
    void  set_angular_velocity(rigid_body_id const  id, vector3 const&  velocity) { m_rigid_bosies.at(id).m_velocity.m_angular = velocity; }

    vector3  get_external_force(rigid_body_id const  id) const;
    vector3  get_external_torque(rigid_body_id const  id) const { return m_external_torques.at(id); }
    void  set_external_force(rigid_body_id const  id, vector3 const&  external_force) { m_external_torques.at(id) = external_force; }
    void  set_external_torque(rigid_body_id const  id, vector3 const&  external_torque);

    float_32_bit  get_inverted_mass(rigid_body_id const  id) const { return m_rigid_bosies.at(id).m_inverted_mass; }
    void  set_inverted_mass(rigid_body_id const  id, float_32_bit const  inverted_mass);

    matrix33 const&  get_inverted_inertia_tensor_in_world_space(rigid_body_id const  id) const { return m_rigid_bosies.at(id).m_inverted_inertia_tensor; }
    void  set_inverted_inertia_tensor_in_local_space(rigid_body_id const  id, matrix33 const&  inverted_inertia_tensor_in_local_space);

    motion_constraint_system&  get_constraint_system() { return m_constraint_system; }
    motion_constraint_system const&  get_constraint_system() const { return m_constraint_system; }

    motion_constraint_system::constraint_id  insert_contact_constraint(
            rigid_body_id const  rb_0,
            rigid_body_id const  rb_1,
            contact_id const& cid,
            vector3 const& contact_point,
            vector3 const& unit_normal, // Must point towards the first rigid body (i.e. towards rb_0)
            float_32_bit const  penetration_depth,
            float_32_bit const  depenetration_coef = 20.0f  // The greater the coeficient, the faster the resolution
                                                            // of the penetration depth. However, for smooth convergence
                                                            // of the constraint it is recommended to keep the value of
                                                            // the coefficient smaller than the actual time-stepping
                                                            // frequency of the physiscal system. The default value
                                                            // assumes that the update frequency of the physical system
                                                            // does not drop under 20Hz. We recommend to pass custom
                                                            // values for the coeeficient (i.e. not to rely of the defaul one)
                                                            // correlating with update frequency of the system.
            );

    void  do_simulation_step(
            float_32_bit const  time_step_in_seconds,
            float_32_bit const  max_computation_time_in_seconds
            );

private:

    void  update_dependent_variables_of_rigid_body(rigid_body_id const  id);

    float_32_bit  read_contact_cache(
            pair_of_rigid_body_ids const&  rb_ids,
            contact_id const&  cid,
            float_32_bit const  value_on_cache_miss
            ) const;
    void  erase_from_contact_cache(rigid_body_id const&  rb_id)
    { m_invalidated_rigid_bodies_in_contact_cache.insert(rb_id); }
    void  update_contact_cache();

    motion_constraint_system  m_constraint_system;

    std::unordered_map<pair_of_rigid_body_ids, std::unordered_map<contact_id, float_32_bit> >  m_contact_cache;
    std::unordered_set<rigid_body_id>  m_invalidated_rigid_bodies_in_contact_cache;
    std::unordered_map<motion_constraint_system::constraint_id, contact_id>  m_from_constraints_to_contact_ids;

    // All the vectors below have the same size.

    std::vector<rigid_body>  m_rigid_bosies;
    std::vector<matrix33>  m_inverted_inertia_tensors;  // Always in the local space. Zero matrix means an infinite inertia.
    std::vector<vector3>  m_external_torques;           // In the world space.
    std::unordered_set<rigid_body_id>  m_invalid_rigid_body_ids;
};


}

#endif
