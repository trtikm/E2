#include <angeo/rigid_body_simulator.hpp>
#include <angeo/friction_coefficients.hpp>
#include <angeo/bouncing_coefficients.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace angeo { namespace detail {


vector3  compute_velocity_of_point_of_rigid_body(rigid_body const&  rb, vector3 const&  point)
{
    return angeo::compute_velocity_of_point_of_rigid_body(
            rb.m_position_of_mass_centre,
            rb.m_velocity.m_linear,
            rb.m_velocity.m_angular,
            point
            );
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


rigid_body_simulator::computation_statistics::computation_statistics()
    : m_num_rigid_bodies(0U)
    , m_contact_cache_size(0U)
    , m_num_contact_cache_hits(0U)
    , m_num_contact_cache_misses(0U)
    , m_custom_constraints_cache_size(0U)
    , m_num_custom_constraints_cache_hits(0U)
    , m_num_custom_constraints_cache_misses(0U)
    , m_performed_simulation_steps(0UL)
    , m_duration_of_rigid_body_update_in_seconds(0.0)
    , m_duration_of_contact_cache_update_in_seconds(0.0)
{}


rigid_body_simulator::rigid_body_simulator()
    : m_statistics()

    , m_constraint_system()

    , m_contact_cache()
    , m_invalidated_rigid_bodies_in_contact_cache()
    , m_from_constraints_to_contact_ids()

    , m_custom_constraints_cache()
    , m_from_constraints_to_custom_constraint_ids()
    , m_released_custom_constraint_ids()
    , m_max_generated_custom_constraint_id(0U)

    , m_rigid_bodies()
    , m_inverted_inertia_tensors() 
    , m_invalid_rigid_body_ids()
    , m_linear_accelerations_from_sources()
    , m_angular_accelerations_from_sources()
{}


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

    ++m_statistics.m_num_rigid_bodies;

    return id;
}


void  rigid_body_simulator::erase_rigid_body(rigid_body_id const  id)
{
    remove_linear_accelerations_from_all_sources(id);
    remove_angular_accelerations_from_all_sources(id);

    m_invalid_rigid_body_ids.insert(id);

    --m_statistics.m_num_rigid_bodies;
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

    m_statistics = computation_statistics();
}


void  rigid_body_simulator::set_orientation(rigid_body_id const  id, quaternion const&  orientation)
{
    m_rigid_bodies.at(id).m_orientation = orientation;
    update_dependent_variables_of_rigid_body(id);
    erase_from_contact_cache(id);
}


void  rigid_body_simulator::set_linear_acceleration_from_source(
        rigid_body_id const  id, com::object_guid const  source_guid, vector3 const&  acceleration
        )
{
    rigid_body&  rb = m_rigid_bodies.at(id);
    auto&  accels = m_linear_accelerations_from_sources[id];

    auto  it = accels.find(source_guid);
    if (it == accels.end())
        it = accels.insert({ source_guid, acceleration }).first;
    else
    {
        rb.m_acceleration_from_external_forces.m_linear -= it->second;
        it->second = acceleration;
    }

    rb.m_acceleration_from_external_forces.m_linear += it->second;
}


void  rigid_body_simulator::set_angular_acceleration_from_source(
        rigid_body_id const  id, com::object_guid const  source_guid, vector3 const&  acceleration
        )
{
    rigid_body&  rb = m_rigid_bodies.at(id);
    auto&  accels = m_angular_accelerations_from_sources[id];

    auto  it = accels.find(source_guid);
    if (it == accels.end())
        it = accels.insert({ source_guid, acceleration }).first;
    else
    {
        rb.m_acceleration_from_external_forces.m_angular -= it->second;
        it->second = acceleration;
    }

    rb.m_acceleration_from_external_forces.m_angular += it->second;
}


void  rigid_body_simulator::remove_linear_acceleration_from_source(rigid_body_id const  id, com::object_guid const  source_guid)
{
    auto const  accels_it = m_linear_accelerations_from_sources.find(id);
    if (accels_it == m_linear_accelerations_from_sources.end())
        return;

    auto  it = accels_it->second.find(source_guid);
    if (it == accels_it->second.end())
        return;

    m_rigid_bodies.at(id).m_acceleration_from_external_forces.m_linear -= it->second;

    accels_it->second.erase(it);

    if (accels_it->second.empty())
        m_linear_accelerations_from_sources.erase(accels_it);
}


void  rigid_body_simulator::remove_angular_acceleration_from_source(rigid_body_id const  id, com::object_guid const  source_guid)
{
    auto const  accels_it = m_angular_accelerations_from_sources.find(id);
    if (accels_it == m_angular_accelerations_from_sources.end())
        return;

    auto  it = accels_it->second.find(source_guid);
    if (it == accels_it->second.end())
        return;

    m_rigid_bodies.at(id).m_acceleration_from_external_forces.m_angular -= it->second;

    accels_it->second.erase(it);

    if (accels_it->second.empty())
        m_angular_accelerations_from_sources.erase(accels_it);
}


void  rigid_body_simulator::remove_linear_accelerations_from_all_sources(rigid_body_id const  id)
{
    auto const  accels_it = m_linear_accelerations_from_sources.find(id);
    if (accels_it == m_linear_accelerations_from_sources.end())
        return;

    rigid_body&  rb = m_rigid_bodies.at(id);
    for (auto const&  guid_and_accel : accels_it->second)
        rb.m_acceleration_from_external_forces.m_linear -= guid_and_accel.second;

    m_linear_accelerations_from_sources.erase(accels_it);
}


void  rigid_body_simulator::remove_angular_accelerations_from_all_sources(rigid_body_id const  id)
{
    auto const  accels_it = m_angular_accelerations_from_sources.find(id);
    if (accels_it == m_angular_accelerations_from_sources.end())
        return;

    rigid_body&  rb = m_rigid_bodies.at(id);
    for (auto const&  guid_and_accel : accels_it->second)
        rb.m_acceleration_from_external_forces.m_angular -= guid_and_accel.second;

    m_angular_accelerations_from_sources.erase(accels_it);
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
        COLLISION_MATERIAL_TYPE  rb_0_material,
        COLLISION_MATERIAL_TYPE  rb_1_material,
        contact_friction_constraints_info const* const  friction_info_ptr,
        float_32_bit const  penetration_depth,
        float_32_bit const  depenetration_coef,
        std::vector<motion_constraint_system::constraint_id>* const  output_constraint_ids_ptr
        )
{
    TMPROF_BLOCK();

    rigid_body const&  rb0 = m_rigid_bodies.at(rb_0);
    rigid_body const&  rb1 = m_rigid_bodies.at(rb_1);

    float_32_bit const  relative_normal_speed = dot_product(
            unit_normal,
            detail::compute_velocity_of_point_of_rigid_body(rb0, contact_point)
                - detail::compute_velocity_of_point_of_rigid_body(rb1, contact_point)
            );

    float_32_bit const  bouncer = std::max(0.0f, get_bouncing_coefficient(rb_0_material, rb_1_material) * -relative_normal_speed);

    motion_constraint_system::constraint_id const  contact_constraint_id =
            get_constraint_system().insert_constraint(
                    rb_0,
                    unit_normal,
                    cross_product(contact_point - rb0.m_position_of_mass_centre, unit_normal),
                    rb_1,
                    -unit_normal,
                    -cross_product(contact_point - rb1.m_position_of_mass_centre, unit_normal),
                    std::max(penetration_depth, bouncer) < 0.001f ? 0.0f : std::max(depenetration_coef * penetration_depth, bouncer),
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
                    get_dynamic_friction_coefficient(rb_0_material, rb_1_material) :
                    get_static_friction_coefficient(rb_0_material, rb_1_material)  ;

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


custom_constraint_id  rigid_body_simulator::gen_fresh_custom_constraint_id()
{
    if (m_released_custom_constraint_ids.empty())
        return ++m_max_generated_custom_constraint_id;
    auto const  it = m_released_custom_constraint_ids.begin();
    custom_constraint_id const  id = *it;
    m_released_custom_constraint_ids.erase(it);
    return id;
}


void  rigid_body_simulator::release_generated_custom_constraint_id(custom_constraint_id const  constraint_id)
{
    ASSUMPTION(constraint_id > 0U && constraint_id <= m_max_generated_custom_constraint_id);
    m_released_custom_constraint_ids.insert(constraint_id);
}


void  rigid_body_simulator::insert_custom_constraint(
        custom_constraint_id const  id,
        rigid_body_id const  rb_0,
        vector3 const&  linear_component_0,
        vector3 const&  angular_component_0,
        rigid_body_id const  rb_1,
        vector3 const&  linear_component_1,
        vector3 const&  angular_component_1,
        float_32_bit const  bias,
        motion_constraint_system::variable_bound_getter const&  variable_lower_bound,
        motion_constraint_system::variable_bound_getter const&  variable_upper_bound,
        float_32_bit const  initial_value_for_cache_miss
        )
{
    TMPROF_BLOCK();

    rigid_body const&  rb0 = m_rigid_bodies.at(rb_0);
    rigid_body const&  rb1 = m_rigid_bodies.at(rb_1);
    motion_constraint_system::constraint_id const  cid =
            get_constraint_system().insert_constraint(
                    rb_0,
                    linear_component_0,
                    angular_component_0,
                    rb_1,
                    linear_component_1,
                    angular_component_1,
                    bias,
                    variable_lower_bound,
                    variable_upper_bound,
                    read_custom_constraints_cache(id, { rb_0, rb_1 }, initial_value_for_cache_miss)
                    );
    m_from_constraints_to_custom_constraint_ids.insert({ cid, id });
}


void  rigid_body_simulator::solve_constraint_system(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  max_computation_time_in_seconds
        )
{
    TMPROF_BLOCK();

    get_constraint_system().solve(
            m_rigid_bodies,
            std::bind(
                    &motion_constraint_system::default_computation_terminator,
                    std::placeholders::_1,
                    0.95f * max_computation_time_in_seconds,
                    0.001f,
                    0.0025f
                    ),
            time_step_in_seconds
            );
}


void  rigid_body_simulator::integrate_motion_of_rigid_bodies(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    std::chrono::high_resolution_clock::time_point const  rb_update_start_time_point =
            std::chrono::high_resolution_clock::now();

    for (rigid_body_id  id = 0U; id != m_rigid_bodies.size(); ++id)
    {
        TMPROF_BLOCK();

        if (m_invalid_rigid_body_ids.count(id) != 0U)
            continue;

        rigid_body&  rb = m_rigid_bodies.at(id);

        //if (rb.m_inverted_mass < 0.0001f)
        //    continue;

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

    std::chrono::high_resolution_clock::time_point const  end_time_point = std::chrono::high_resolution_clock::now();

    m_statistics.m_duration_of_rigid_body_update_in_seconds =
            std::chrono::duration<float_64_bit>(end_time_point - rb_update_start_time_point).count();
}


void  rigid_body_simulator::prepare_contact_cache_and_constraint_system_for_next_frame()
{
    std::chrono::high_resolution_clock::time_point const  contact_cache_start_time_point = std::chrono::high_resolution_clock::now();

    update_contact_cache();
    update_custom_constraints_cache();
    get_constraint_system().clear();

    std::chrono::high_resolution_clock::time_point const  end_time_point = std::chrono::high_resolution_clock::now();

    m_statistics.m_duration_of_contact_cache_update_in_seconds =
        std::chrono::duration<float_64_bit>(end_time_point - contact_cache_start_time_point).count();
    ++m_statistics.m_performed_simulation_steps;
}


void  rigid_body_simulator::do_simulation_step(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  max_computation_time_in_seconds
        )
{
    solve_constraint_system(time_step_in_seconds, max_computation_time_in_seconds);
    integrate_motion_of_rigid_bodies(time_step_in_seconds);
    prepare_contact_cache_and_constraint_system_for_next_frame();
}


void  rigid_body_simulator::update_dependent_variables_of_rigid_body(rigid_body_id const  id)
{
    TMPROF_BLOCK();

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
    TMPROF_BLOCK();

    if (m_invalidated_rigid_bodies_in_contact_cache.count(rb_ids.first) != 0U ||
        m_invalidated_rigid_bodies_in_contact_cache.count(rb_ids.second) != 0U)
    {
        ++m_statistics.m_num_contact_cache_misses;
        return value_on_cache_miss;
    }
    auto const  rbs_bucket_it = m_contact_cache.find(rb_ids);
    if (rbs_bucket_it == m_contact_cache.cend())
    {
        ++m_statistics.m_num_contact_cache_misses;
        return value_on_cache_miss;
    }
    auto const  contact_and_lambdas_it = rbs_bucket_it->second.find(cid);
    if (contact_and_lambdas_it == rbs_bucket_it->second.cend())
    {
        ++m_statistics.m_num_contact_cache_misses;
        return value_on_cache_miss;
    }
    auto const  lambda_it = contact_and_lambdas_it->second.find(contact_vector_id);
    if (lambda_it == contact_and_lambdas_it->second.cend())
    {
        ++m_statistics.m_num_contact_cache_misses;
        return value_on_cache_miss;
    }
    ++m_statistics.m_num_contact_cache_hits;
    return lambda_it->second;
}


void  rigid_body_simulator::update_contact_cache()
{
    TMPROF_BLOCK();

    m_contact_cache.clear();
    for (auto const& constraint_and_contact_and_vector_ids : m_from_constraints_to_contact_ids)
        m_contact_cache
                [get_constraint_system().get_rigid_bodies_of_constraint(constraint_and_contact_and_vector_ids.first)]
                [constraint_and_contact_and_vector_ids.second.first]
                [constraint_and_contact_and_vector_ids.second.second]
            = get_constraint_system().get_solution_of_constraint(constraint_and_contact_and_vector_ids.first);
    m_invalidated_rigid_bodies_in_contact_cache.clear();
    m_from_constraints_to_contact_ids.clear();

    m_statistics.m_contact_cache_size = (natural_32_bit)m_contact_cache.size();
}


float_32_bit  rigid_body_simulator::read_custom_constraints_cache(
        custom_constraint_id const  id,
        pair_of_rigid_body_ids const&  rb_ids,
        float_32_bit const  value_on_cache_miss
        ) const
{
    TMPROF_BLOCK();

    auto const  it = m_custom_constraints_cache.find(id);
    if (it == m_custom_constraints_cache.cend() || it->second.second != rb_ids)
    {
        ++m_statistics.m_num_contact_cache_misses;
        return value_on_cache_miss;
    }
    ++m_statistics.m_num_custom_constraints_cache_hits;
    return it->second.first;
}


void  rigid_body_simulator::update_custom_constraints_cache()
{
    TMPROF_BLOCK();

    m_custom_constraints_cache.clear();
    for (auto const& cid_and_id : m_from_constraints_to_custom_constraint_ids)
        m_custom_constraints_cache.insert({
                cid_and_id.second,
                {
                    get_constraint_system().get_solution_of_constraint(cid_and_id.first),
                    get_constraint_system().get_rigid_bodies_of_constraint(cid_and_id.first)
                }
                });
    m_from_constraints_to_custom_constraint_ids.clear();

    m_statistics.m_custom_constraints_cache_size = (natural_32_bit)m_custom_constraints_cache.size();
}


}

namespace angeo {


vector3  compute_velocity_of_point_of_rigid_body(
        vector3 const&  rigid_body_mass_centre_position,
        vector3 const&  rigid_body_linear_velocity,
        vector3 const&  rigid_body_angular_velocity,
        vector3 const&  point
        )
{
    return rigid_body_linear_velocity + cross_product(rigid_body_angular_velocity, point - rigid_body_mass_centre_position);
}


}
