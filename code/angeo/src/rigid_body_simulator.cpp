#include <angeo/rigid_body_simulator.hpp>
#include <angeo/friction_coefficients.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace angeo { namespace detail {


vector3  compute_velocity_of_point_of_rigid_body(rigid_body const&  rb, vector3 const&  point)
{
    return rb.m_velocity.m_linear + cross_product(rb.m_velocity.m_angular, point - rb.m_position_of_mass_centre);
}


vector3  compute_relative_tangent_plane_velocity_of_point_of_rigid_bodies(
        rigid_body const&  rb_0,
        rigid_body const&  rb_1,
        vector3 const&  point,
        vector3 const&  unit_normal_vector
        )
{
    vector3 const  relative_velocity =
            compute_velocity_of_point_of_rigid_body(rb_1, point) - compute_velocity_of_point_of_rigid_body(rb_0, point);
    return relative_velocity - dot_product(relative_velocity, unit_normal_vector) * unit_normal_vector;
}                               


}}

namespace angeo {


rigid_body_id  rigid_body_simulator::insert_rigid_body(
        vector3 const&  position_of_mass_centre,
        quaternion const&  orientation,
        float_32_bit const  inverted_mass,
        matrix33 const&  inverted_inertia_tensor_in_local_space,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  external_linear_acceleration,
        vector3 const&  external_angular_acceleration
        )
{
    TMPROF_BLOCK();

    rigid_body_id  id;
    if (m_invalid_rigid_body_ids.empty())
    {
        id = (rigid_body_id)m_rigid_bodies.size();

        m_rigid_bodies.push_back({
                position_of_mass_centre,
                orientation,
                { linear_velocity, angular_velocity },
                { vector3_zero(), vector3_zero() },
                { external_linear_acceleration, external_angular_acceleration },
                inverted_mass,
                matrix33_identity()
                });
        m_inverted_inertia_tensors.push_back(inverted_inertia_tensor_in_local_space);
    }
    else
    {
        id = *m_invalid_rigid_body_ids.cbegin();
        m_invalid_rigid_body_ids.erase(id);

        m_rigid_bodies.at(id) = {
                position_of_mass_centre,
                orientation,
                { linear_velocity, angular_velocity },
                { vector3_zero(), vector3_zero() },
                { external_linear_acceleration, external_angular_acceleration },
                inverted_mass,
                matrix33_identity()
                };
        m_inverted_inertia_tensors.at(id) = inverted_inertia_tensor_in_local_space;
    }

    update_dependent_variables_of_rigid_body(id);

    return id;
}


void  rigid_body_simulator::erase_rigid_body(rigid_body_id const  id)
{
    m_invalid_rigid_body_ids.insert(id);
}


void  rigid_body_simulator::clear()
{
    TMPROF_BLOCK();

    m_constraint_system.clear();

    m_contact_cache.clear();
    m_invalidated_rigid_bodies_in_contact_cache.clear();
    m_from_constraints_to_contact_ids.clear();

    m_rigid_bodies.clear();
    m_inverted_inertia_tensors.clear();
    m_invalid_rigid_body_ids.clear();
}


void  rigid_body_simulator::set_orientation(rigid_body_id const  id, quaternion const&  orientation)
{
    m_rigid_bodies.at(id).m_orientation = orientation;
    update_dependent_variables_of_rigid_body(id);
    erase_from_contact_cache(id);
}


void  rigid_body_simulator::set_inverted_inertia_tensor_in_local_space(rigid_body_id const  id, matrix33 const&  inverted_inertia_tensor_in_local_space)
{
    m_inverted_inertia_tensors.at(id) = inverted_inertia_tensor_in_local_space;
    update_dependent_variables_of_rigid_body(id);
}


void  rigid_body_simulator::insert_contact_constraints(
        rigid_body_id const  rb_0,
        rigid_body_id const  rb_1,
        contact_id const&  cid,
        vector3 const&  contact_point,
        vector3 const&  unit_normal,
        contact_friction_constraints_info const* const  friction_info_ptr,
        float_32_bit const  penetration_depth,
        float_32_bit const  depenetration_coef,
        std::vector<motion_constraint_system::constraint_id>* const  output_constraint_ids_ptr
        )
{
    rigid_body const&  rb0 = m_rigid_bodies.at(rb_0);
    rigid_body const&  rb1 = m_rigid_bodies.at(rb_1);

    motion_constraint_system::constraint_id const  contact_constraint_id =
            get_constraint_system().insert_constraint(
                    rb_0,
                    unit_normal,
                    cross_product(contact_point - rb0.m_position_of_mass_centre, unit_normal),
                    rb_1,
                    -unit_normal,
                    -cross_product(contact_point - rb1.m_position_of_mass_centre, unit_normal),
                    -depenetration_coef * penetration_depth,
                    [](std::vector<float_32_bit> const&) { return 0.0f; },
                    [](std::vector<float_32_bit> const&) { return std::numeric_limits<float_32_bit>::max(); },
                    read_contact_cache({ rb_0, rb_1 }, cid, 0U, 0.0f)
                    );

    m_from_constraints_to_contact_ids.insert({ contact_constraint_id,{ cid, 0U } });

    if (output_constraint_ids_ptr != nullptr)
        output_constraint_ids_ptr->push_back(contact_constraint_id);

    if (friction_info_ptr == nullptr)
        return;

    ASSUMPTION(friction_info_ptr->m_unit_tangent_plane_vectors.size() > 1U);

    vector3 const  tangent_plane_velocity =
            detail::compute_relative_tangent_plane_velocity_of_point_of_rigid_bodies(rb0, rb1, contact_point, unit_normal);
    float_32_bit const  friction_coef =
            length(tangent_plane_velocity) > friction_info_ptr->m_max_tangent_relative_speed_for_static_friction ?
                    get_dynamic_friction_coefficient(friction_info_ptr->m_material_0, friction_info_ptr->m_material_1) :
                    get_static_friction_coefficient(friction_info_ptr->m_material_0, friction_info_ptr->m_material_1) ;

    for (natural_32_bit  i = 0U; i != (natural_32_bit)friction_info_ptr->m_unit_tangent_plane_vectors.size(); ++i)
    {
        vector3 const&  unit_tangent_plane_vector = friction_info_ptr->m_unit_tangent_plane_vectors.at(i);
        natural_32_bit const  unit_tangent_plane_vector_id = i + 1U;

        motion_constraint_system::constraint_id const  friction_constraint_id =
                get_constraint_system().insert_constraint(
                        rb_0,
                        unit_tangent_plane_vector,
                        cross_product(contact_point - rb0.m_position_of_mass_centre, unit_tangent_plane_vector),
                        rb_1,
                        -unit_tangent_plane_vector,
                        -cross_product(contact_point - rb1.m_position_of_mass_centre, unit_tangent_plane_vector),
                        0.0f,
                        friction_info_ptr->m_suppress_negative_directions ?
                                motion_constraint_system::variable_bound_getter(
                                    [](std::vector<float_32_bit> const&) { return 0.0f; }
                                    ) :
                                motion_constraint_system::variable_bound_getter(
                                    [contact_constraint_id, friction_coef](std::vector<float_32_bit> const& variables) {
                                            return -friction_coef * variables.at(contact_constraint_id);
                                        }
                                    ),
                        [contact_constraint_id, friction_coef](std::vector<float_32_bit> const& variables) {
                                return +friction_coef * variables.at(contact_constraint_id);
                            },
                        read_contact_cache({rb_0, rb_1}, cid, unit_tangent_plane_vector_id, 0.0f)
                        );

        m_from_constraints_to_contact_ids.insert({ friction_constraint_id, { cid, unit_tangent_plane_vector_id } });

        if (output_constraint_ids_ptr != nullptr)
            output_constraint_ids_ptr->push_back(friction_constraint_id);
    }
}


void  rigid_body_simulator::do_simulation_step(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  max_computation_time_in_seconds
        )
{
    TMPROF_BLOCK();

    get_constraint_system().solve(
            m_rigid_bodies,
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

    for (rigid_body_id  id = 0U; id != m_rigid_bodies.size(); ++id)
    {
        if (m_invalid_rigid_body_ids.count(id) != 0U)
            continue;

        rigid_body&  rb = m_rigid_bodies.at(id);

        rb.m_velocity.m_linear += time_step_in_seconds * (
                rb.m_acceleration_from_constraints.m_linear + rb.m_acceleration_from_external_forces.m_linear
                );
        rb.m_velocity.m_angular += time_step_in_seconds * (
                rb.m_acceleration_from_constraints.m_angular + rb.m_acceleration_from_external_forces.m_angular
                );

        quaternion const  orientation_derivative =
                scale(0.5f, make_quaternion(0.0f, rb.m_velocity.m_angular) * rb.m_orientation);

        rb.m_position_of_mass_centre += time_step_in_seconds * rb.m_velocity.m_linear;
        rb.m_orientation = normalised(rb.m_orientation + scale(time_step_in_seconds, orientation_derivative));

        update_dependent_variables_of_rigid_body(id);
    }

    update_contact_cache();

    get_constraint_system().clear();
}


void  rigid_body_simulator::update_dependent_variables_of_rigid_body(rigid_body_id const  id)
{
    rigid_body&  rb = m_rigid_bodies.at(id);

    matrix33 const  R = quaternion_to_rotation_matrix(rb.m_orientation);
    rb.m_inverted_inertia_tensor = R * m_inverted_inertia_tensors.at(id) * transpose33(R);

    rb.m_acceleration_from_constraints.m_linear = vector3_zero();
    rb.m_acceleration_from_constraints.m_angular = vector3_zero();
}


float_32_bit  rigid_body_simulator::read_contact_cache(
        pair_of_rigid_body_ids const&  rb_ids,
        contact_id const&  cid,
        natural_32_bit const  contact_vector_id,
        float_32_bit const  value_on_cache_miss
        ) const
{
    if (m_invalidated_rigid_bodies_in_contact_cache.count(rb_ids.first) != 0U ||
        m_invalidated_rigid_bodies_in_contact_cache.count(rb_ids.second) != 0U)
        return value_on_cache_miss;
    auto const  rbs_bucket_it = m_contact_cache.find(rb_ids);
    if (rbs_bucket_it == m_contact_cache.cend())
        return value_on_cache_miss;
    auto const  contact_and_lambdas_it = rbs_bucket_it->second.find(cid);
    if (contact_and_lambdas_it == rbs_bucket_it->second.cend())
        return value_on_cache_miss;
    auto const  lambda_it = contact_and_lambdas_it->second.find(contact_vector_id);
    return (lambda_it == contact_and_lambdas_it->second.cend()) ? value_on_cache_miss : lambda_it->second;
}


void  rigid_body_simulator::update_contact_cache()
{
    m_contact_cache.clear();
    for (auto const& constraint_and_contact_and_vector_ids : m_from_constraints_to_contact_ids)
        m_contact_cache
                [get_constraint_system().get_rigid_bodies_of_constraint(constraint_and_contact_and_vector_ids.first)]
                [constraint_and_contact_and_vector_ids.second.first]
                [constraint_and_contact_and_vector_ids.second.second]
            = get_constraint_system().get_solution_of_constraint(constraint_and_contact_and_vector_ids.first);
    m_invalidated_rigid_bodies_in_contact_cache.clear();
    m_from_constraints_to_contact_ids.clear();
}


}
