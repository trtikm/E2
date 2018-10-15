#ifndef ANGEO_RIGID_BODY_SIMULATOR_HPP_INCLUDED
#   define ANGEO_RIGID_BODY_SIMULATOR_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/rigid_body.hpp>
#   include <angeo/motion_constraint_system.hpp>
#   include <vector>
#   include <unordered_set>

namespace angeo {


struct  rigid_body_simulator
{
    rigid_body_matter_props_id  insert_matter_props(
            float_32_bit const  inverted_mass,                          // The value 0.0f means the mass is infinite. 
            matrix33 const&  inverted_inertia_tensor_in_local_space     // Zero matrix means an infinite inertia.
            );

    // Only matter props which are not used by any rigid body in the simulator can be erased.
    void  erase_matter_props(rigid_body_matter_props_id const  id);

    rigid_body_id  insert_rigid_body(
            rigid_body_matter_props_id const  matter_props_id,
            coordinate_system_ptr const  coord_system_ptr, // The centre of mass is assumed to be in the origin of that coordinate system!!
            vector3 const&  linear_velocity = vector3_zero(),
            vector3 const&  angular_velocity = vector3_zero(),
            vector3 const&  external_force = vector3_zero(),
            vector3 const&  external_torque = vector3_zero()
            );

    void  erase_rigid_body(rigid_body_id const  id);

    void  set_velocity(rigid_body_id const  id, rigid_body_velocity const&  velocity);

    void  set_external_force_and_torque(
            rigid_body_id const  id,
            vector3 const&  external_force = vector3_zero(),
            vector3 const&  external_torque = vector3_zero()
            );

    motion_constraint_system&  get_constraint_system() { return m_constraint_system; }
    motion_constraint_system const&  get_constraint_system() const { return m_constraint_system; }

    void  do_simulation_step(
            float_32_bit const  time_step_in_seconds,
            float_32_bit const  max_computation_time_in_seconds
            );

private:

    // All the vectors below have the same size.
    std::vector<rigid_body>  m_rigid_bosies;
    std::vector<natural_32_bit>  m_matter_props_indices;
    std::vector<matrix33>  m_inverted_inertia_tensors_in_world_space;
    std::vector<linear_and_angular_vector>  m_accelerations_from_constraints;
    std::vector<linear_and_angular_vector>  m_accelerations_from_external_forces;

    // The following 2 vectors have the same size (which is in in general different to size of vectors above).
    std::vector<rigid_body_matter_props>  m_matter_props;
    std::vector<natural_32_bit>  m_matter_props_ref_counts;

    motion_constraint_system  m_constraint_system;

    // We store IDs of deleted rigid bodies and matters so that they can be later reused.
    std::unordered_set<rigid_body_id>  m_invalid_rigid_body_ids;
    std::unordered_set<rigid_body_matter_props_id>  m_invalid_matter_props_ids;
};


}

#endif
