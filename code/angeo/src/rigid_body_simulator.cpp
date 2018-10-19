#include <angeo/rigid_body_simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace angeo {


rigid_body_id  rigid_body_simulator::insert_rigid_body(
        vector3 const&  position_of_mass_centre,
        quaternion const&  orientation,
        float_32_bit const  inverted_mass,
        matrix33 const&  inverted_inertia_tensor_in_local_space,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  external_force,
        vector3 const&  external_torque
        )
{
    TMPROF_BLOCK();

    rigid_body_id  id;
    if (m_invalid_rigid_body_ids.empty())
    {
        id = (rigid_body_id)m_rigid_bosies.size();

        m_rigid_bosies.push_back({
                position_of_mass_centre,
                orientation,
                { linear_velocity, angular_velocity },
                { vector3_zero(), vector3_zero() },
                { inverted_mass * external_force, vector3_zero() },
                inverted_mass,
                matrix33_identity()
                });
        m_inverted_inertia_tensors.push_back(inverted_inertia_tensor_in_local_space);
        m_external_torques.push_back(external_torque);

    }
    else
    {
        rigid_body_id const  id = *m_invalid_rigid_body_ids.cbegin();
        m_invalid_rigid_body_ids.erase(id);

        m_rigid_bosies.at(id) = {
                position_of_mass_centre,
                orientation,
                { linear_velocity, angular_velocity },
                { vector3_zero(), vector3_zero() },
                { inverted_mass * external_force, vector3_zero() },
                inverted_mass,
                matrix33_identity()
                };
        m_inverted_inertia_tensors.at(id) = inverted_inertia_tensor_in_local_space;
        m_external_torques.at(id) = external_torque;

        return id;
    }

    update_dependent_variables_of_rigid_body(id);

    return id;
}


void  rigid_body_simulator::erase_rigid_body(rigid_body_id const  id)
{
    m_invalid_rigid_body_ids.insert(id);
}


void  rigid_body_simulator::set_orientation(rigid_body_id const  id, quaternion const&  orientation)
{
    m_rigid_bosies.at(id).m_orientation = orientation;
    update_dependent_variables_of_rigid_body(id);
}


vector3  rigid_body_simulator::get_external_force(rigid_body_id const  id) const
{
    rigid_body const&  rb = m_rigid_bosies.at(id);
    return rb.m_inverted_mass < 1e-5f ? vector3_zero() : rb.m_acceleration_from_external_forces.m_linear / rb.m_inverted_mass;
}


void  rigid_body_simulator::set_external_torque(rigid_body_id const  id, vector3 const&  external_torque)
{
    m_external_torques.at(id) = external_torque;
    m_rigid_bosies.at(id).m_acceleration_from_external_forces.m_angular =
            m_rigid_bosies.at(id).m_inverted_inertia_tensor * m_external_torques.at(id);
}


void  rigid_body_simulator::set_inverted_mass(rigid_body_id const  id, float_32_bit const  inverted_mass)
{
    vector3  external_force = get_external_force(id);
    rigid_body&  rb = m_rigid_bosies.at(id);
    rb.m_inverted_mass = inverted_mass;
    rb.m_acceleration_from_external_forces.m_linear = rb.m_inverted_inertia_tensor * external_force;
}


void  rigid_body_simulator::set_inverted_inertia_tensor_in_local_space(rigid_body_id const  id, matrix33 const&  inverted_inertia_tensor_in_local_space)
{
    m_inverted_inertia_tensors.at(id) = inverted_inertia_tensor_in_local_space;
    update_dependent_variables_of_rigid_body(id);
}


void  rigid_body_simulator::do_simulation_step(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  max_computation_time_in_seconds
        )
{
    TMPROF_BLOCK();

    get_constraint_system().solve(
            m_rigid_bosies,
            std::bind(
                    &motion_constraint_system::default_computation_terminator,
                    get_constraint_system().get_num_constraints(),
                    1e-5f,
                    natural_32_bit(0.95f * max_computation_time_in_seconds * 1000000.0f),
                    std::placeholders::_1
                    ),
            time_step_in_seconds,
            nullptr
            );

    for (rigid_body_id  id = 0U; id != m_rigid_bosies.size(); ++id)
    {
        if (m_invalid_rigid_body_ids.count(id) != 0U)
            continue;

        rigid_body&  rb = m_rigid_bosies.at(id);

        rb.m_velocity.m_linear += time_step_in_seconds * (
                rb.m_acceleration_from_constraints.m_linear + rb.m_acceleration_from_external_forces.m_linear
                );
        rb.m_velocity.m_angular += time_step_in_seconds * (
                rb.m_acceleration_from_constraints.m_angular + rb.m_acceleration_from_external_forces.m_angular
                );

        quaternion const  orientation_derivative =
                scale(0.5f, make_quaternion(0.0f, rb.m_velocity.m_angular) * rb.m_orientation);

        rb.m_position_of_mass_centre += time_step_in_seconds * rb.m_velocity.m_linear;
        rb.m_orientation = normalised(scale(time_step_in_seconds, orientation_derivative) * rb.m_orientation);

        update_dependent_variables_of_rigid_body(id);
    }

    get_constraint_system().clear();
}


void  rigid_body_simulator::update_dependent_variables_of_rigid_body(rigid_body_id const  id)
{
    rigid_body&  rb = m_rigid_bosies.at(id);

    matrix33 const  R = quaternion_to_rotation_matrix(rb.m_orientation);
    rb.m_inverted_inertia_tensor = R * m_inverted_inertia_tensors.at(id) * transpose33(R);

    rb.m_acceleration_from_constraints.m_linear = vector3_zero();
    rb.m_acceleration_from_constraints.m_angular = vector3_zero();

    rb.m_acceleration_from_external_forces.m_angular = rb.m_inverted_inertia_tensor * m_external_torques.at(id);
}


}
