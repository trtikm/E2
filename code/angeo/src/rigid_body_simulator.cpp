#include <angeo/rigid_body_simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace angeo {


rigid_body_matter_props_id  rigid_body_simulator::insert_matter_props(
        float_32_bit const  inverted_mass,
        matrix33 const&  inverted_inertia_tensor_in_local_space
        )
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


void  rigid_body_simulator::erase_matter_props(rigid_body_matter_props_id const  id)
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


rigid_body_id  rigid_body_simulator::insert_rigid_body(
        rigid_body_matter_props_id const  matter_props_id,
        coordinate_system_ptr const  coord_system_ptr,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  external_force,
        vector3 const&  external_torque
        )
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


void  rigid_body_simulator::erase_rigid_body(rigid_body_id const  id)
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


void  rigid_body_simulator::set_velocity(rigid_body_id const  id, rigid_body_velocity const&  velocity)
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


void  rigid_body_simulator::set_external_force_and_torque(
        rigid_body_id const  id,
        vector3 const&  external_force,
        vector3 const&  external_torque
        )
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


void  rigid_body_simulator::do_simulation_step(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  max_computation_time_in_seconds
        )
{
    TMPROF_BLOCK();

    get_constraint_system().solve(
            m_rigid_bosies,
            m_accelerations_from_external_forces,
            std::bind(
                    &motion_constraint_system::default_computation_terminator,
                    get_constraint_system().get_num_constraints(),
                    1e-5f,
                    natural_32_bit(0.95f * max_computation_time_in_seconds * 1000000.0f),
                    std::placeholders::_1
                    ),
            time_step_in_seconds,
            m_accelerations_from_constraints,
            nullptr
            );

    for (natural_32_bit  id = 0U; id != m_rigid_bosies.size(); ++id)
    {
        coordinate_system&  coord_system = *m_rigid_bosies.at(id).m_coord_system;
        rigid_body_velocity&  velocity = m_rigid_bosies.at(id).m_velocity;
        linear_and_angular_vector&  constraints_accel = m_accelerations_from_constraints.at(id);
        linear_and_angular_vector&  external_accel = m_accelerations_from_external_forces.at(id);

        velocity.m_linear += time_step_in_seconds * (constraints_accel.m_linear + external_accel.m_linear);
        velocity.m_angular += time_step_in_seconds * (constraints_accel.m_angular + external_accel.m_angular);

        quaternion const  orientation_derivative =
                scale(0.5f, make_quaternion(0.0f, velocity.m_angular) * coord_system.orientation());

        translate(coord_system, time_step_in_seconds * velocity.m_linear);
        rotate(coord_system, scale(time_step_in_seconds, orientation_derivative));

        constraints_accel.m_linear = vector3_zero();
        constraints_accel.m_angular = vector3_zero();
    }

    get_constraint_system().clear();
}


}
