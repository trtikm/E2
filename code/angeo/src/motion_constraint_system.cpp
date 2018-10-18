#include <angeo/motion_constraint_system.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace angeo {


motion_constraint_system::constraint_id  motion_constraint_system::insert_constraint(
        rigid_body_id const  rb_0,
        vector3 const&  linear_component_0,
        vector3 const&  angular_component_0,
        rigid_body_id const  rb_1,
        vector3 const&  linear_component_1,
        vector3 const&  angular_component_1,
        float_32_bit const  bias,
        VARIABLE_BOUND_TYPE const  variable_lower_bound_type,
        variable_bound const&  variable_lower_bound,
        VARIABLE_BOUND_TYPE const  variable_upper_bound_type,
        variable_bound const&  variable_upper_bound,
        float_32_bit const  variable_initial_value
        )
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


void  motion_constraint_system::clear()
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


bool  motion_constraint_system::default_computation_terminator(
        natural_32_bit const  max_num_iterations,
        float_32_bit const  min_change_of_variables,
        natural_32_bit const  max_computation_time_in_micro_seconds,
        computation_statistics const&  statistics
        )
{
    return  max_num_iterations <= statistics.m_num_performed_iterations ||
            min_change_of_variables >= statistics.m_max_change_of_variables ||
            max_computation_time_in_micro_seconds <= statistics.m_total_time_of_all_performed_iterations_in_micro_seconds
            ;
}


std::vector<float_32_bit> const&  motion_constraint_system::solve(
        std::vector<rigid_body> const&  rigid_bodies,
        std::vector<linear_and_angular_vector> const&  accelerations_from_external_forces,
        std::function<bool(computation_statistics const&)> const&  terminate_comutation,
        float_32_bit const  time_step_in_seconds,
        std::vector<linear_and_angular_vector>& output_accelerations_from_constraints,
        computation_statistics*  output_statistics_ptr
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(accelerations_from_external_forces.size() == rigid_bodies.size());

    {
        TMPROF_BLOCK();

        // We fill-in the matrix 'm_inverted_mass_matrix_times_jacobian_transposed'.
        for (natural_32_bit  i = 0U; i != get_num_constraints(); ++i)
        {
            index_element const&  rb_ids = m_index.at(i);
            rigid_body const&  rb_first = rigid_bodies.at(rb_ids.first);
            rigid_body const&  rb_second = rigid_bodies.at(rb_ids.second);
            matrix_element const&  jacobian_elem = m_jacobian.at(i);

            matrix_element&  result_elem_ref = m_inverted_mass_matrix_times_jacobian_transposed.at(i);

            result_elem_ref.first.m_linear = rb_first.m_inverted_mass * jacobian_elem.first.m_linear;
            result_elem_ref.first.m_angular = rb_first.m_inverted_inertia_tensor * jacobian_elem.first.m_angular;

            result_elem_ref.second.m_linear = rb_second.m_inverted_mass * jacobian_elem.second.m_linear;
            result_elem_ref.second.m_angular = rb_second.m_inverted_inertia_tensor * jacobian_elem.second.m_angular;
        }
    }

    float_32_bit const  dt_inverted = 1.0f / time_step_in_seconds;

    {
        TMPROF_BLOCK();

        // We fill-in the vector 'm_rhs_vector'.
        for (natural_32_bit i = 0U; i != get_num_constraints(); ++i)
        {
            index_element const&  rb_ids = m_index.at(i);
            rigid_body const&  rb_first = rigid_bodies.at(rb_ids.first);
            rigid_body const&  rb_second = rigid_bodies.at(rb_ids.second);
            linear_and_angular_vector const&  external_accel_first = accelerations_from_external_forces.at(rb_ids.first);
            linear_and_angular_vector const&  external_accel_second = accelerations_from_external_forces.at(rb_ids.second);
            matrix_element const&  jacobian_elem = m_jacobian.at(i);

            m_rhs_vector.at(i) =
                    dt_inverted * m_rhs_vector.at(i) - (
                        dot_product(jacobian_elem.first.m_linear,
                                    dt_inverted * rb_first.m_velocity.m_linear - external_accel_first.m_linear) +
                        dot_product(jacobian_elem.first.m_angular,
                                    dt_inverted * rb_first.m_velocity.m_angular - external_accel_first.m_angular) +
                        dot_product(jacobian_elem.second.m_linear,
                                    dt_inverted * rb_second.m_velocity.m_linear - external_accel_second.m_linear) +
                        dot_product(jacobian_elem.second.m_angular,
                                    dt_inverted * rb_second.m_velocity.m_angular - external_accel_second.m_angular)
                        );
        }
    }

    {
        TMPROF_BLOCK();

        // Compute initial values of 'output_accelerations_from_constraints'.
        output_accelerations_from_constraints.clear();
        output_accelerations_from_constraints.resize(rigid_bodies.size(), { vector3_zero(), vector3_zero() });
        for (natural_32_bit i = 0U; i != get_num_constraints(); ++i)
        {
            matrix_element const&  matrix_elem = m_inverted_mass_matrix_times_jacobian_transposed.at(i);
            index_element const&  rb_ids = m_index.at(i);
            linear_and_angular_vector&  rb_first_accel_ref = output_accelerations_from_constraints.at(rb_ids.first);
            linear_and_angular_vector&  rb_second_accel_ref = output_accelerations_from_constraints.at(rb_ids.second);

            rb_first_accel_ref.m_linear += m_lambdas.at(i) * matrix_elem.first.m_linear;
            rb_first_accel_ref.m_angular += m_lambdas.at(i) * matrix_elem.first.m_angular;

            rb_second_accel_ref.m_linear += m_lambdas.at(i) * matrix_elem.second.m_linear;
            rb_second_accel_ref.m_angular += m_lambdas.at(i) * matrix_elem.second.m_angular;
        }
    }

    std::vector<float_32_bit>  diagonal_elements;

    {
        TMPROF_BLOCK();

        // Compute diagonal elements.
        diagonal_elements.resize(get_num_constraints());
        for (natural_32_bit i = 0U; i != get_num_constraints(); ++i)
        {
            matrix_element const&  jacobian_elem = m_jacobian.at(i);
            matrix_element const&  matrix_elem = m_inverted_mass_matrix_times_jacobian_transposed.at(i);

            diagonal_elements.at(i) =
                dot_product(jacobian_elem.first.m_linear, matrix_elem.first.m_linear) +
                dot_product(jacobian_elem.first.m_angular, matrix_elem.first.m_angular) +

                dot_product(jacobian_elem.second.m_linear, matrix_elem.second.m_linear) +
                dot_product(jacobian_elem.second.m_angular, matrix_elem.second.m_angular)
                ;
        }
    }

    {
        TMPROF_BLOCK();

        // And we iteratively improve unknowns 'm_lambdas' towards a solution of the system.

        computation_statistics  statistics;
        if (output_statistics_ptr == nullptr)
            output_statistics_ptr = &statistics;
        *output_statistics_ptr = {
                get_num_constraints(),      // m_num_constraints_in_system
                0U,                         // m_num_performed_iterations
                0.0f,                       // m_max_change_of_variables
                0U,                         // m_time_of_last_iteration_in_micro_seconds
                0U                          // m_total_time_of_all_performed_iterations_in_micro_seconds
        };
        std::chrono::high_resolution_clock::time_point const  start_time_point = std::chrono::high_resolution_clock::now();
        do
        {
            std::chrono::high_resolution_clock::time_point const  iteration_start_time_point =
                    std::chrono::high_resolution_clock::now();

            float_32_bit  max_change_of_variables = 0.0f;
            for (natural_32_bit i = 0U; i != get_num_constraints(); ++i)
            {
                matrix_element const&  jacobian_elem = m_jacobian.at(i);
                index_element const&  rb_ids = m_index.at(i);
                matrix_element const&  matrix_elem = m_inverted_mass_matrix_times_jacobian_transposed.at(i);

                float_32_bit&  lambda_ref = m_lambdas.at(i);
                linear_and_angular_vector&  rb_first_accel_ref = output_accelerations_from_constraints.at(rb_ids.first);
                linear_and_angular_vector&  rb_second_accel_ref = output_accelerations_from_constraints.at(rb_ids.second);

                float_32_bit const  raw_delta_lambda = (
                    m_rhs_vector.at(i) - 
                    (dot_product(jacobian_elem.first.m_linear, rb_first_accel_ref.m_linear) +
                        dot_product(jacobian_elem.first.m_angular, rb_first_accel_ref.m_angular)
                        ) -
                    (dot_product(jacobian_elem.second.m_linear, rb_second_accel_ref.m_linear) +
                        dot_product(jacobian_elem.second.m_angular, rb_second_accel_ref.m_angular)
                        )
                    ) / diagonal_elements.at(i);

                float_32_bit const  min_lambda =
                        m_variable_lower_bound_types.at(i) == VARIABLE_BOUND_TYPE::CONCRETE_VALUE ?
                                m_variable_lower_bounds.at(i).m_concrete_value :
                                m_lambdas.at(m_variable_lower_bounds.at(i).m_variable_index)
                                ;
                float_32_bit const  max_lambda =
                        m_variable_upper_bound_types.at(i) == VARIABLE_BOUND_TYPE::CONCRETE_VALUE ?
                                m_variable_upper_bounds.at(i).m_concrete_value :
                                m_lambdas.at(m_variable_upper_bounds.at(i).m_variable_index)
                                ;

                float_32_bit const  new_lambda = std::max(min_lambda, std::min(max_lambda, lambda_ref + raw_delta_lambda));
                float_32_bit const  delta_lambda = new_lambda - lambda_ref;

                lambda_ref = new_lambda;

                rb_first_accel_ref.m_linear += delta_lambda * matrix_elem.first.m_linear;
                rb_first_accel_ref.m_angular += delta_lambda * matrix_elem.first.m_angular;

                rb_second_accel_ref.m_linear += delta_lambda * matrix_elem.second.m_linear;
                rb_second_accel_ref.m_angular += delta_lambda * matrix_elem.second.m_angular;
            }

            std::chrono::high_resolution_clock::time_point const  iteration_end_time_point =
                    std::chrono::high_resolution_clock::now();

            ++output_statistics_ptr->m_num_performed_iterations;
            output_statistics_ptr->m_max_change_of_variables = max_change_of_variables;
            output_statistics_ptr->m_time_of_last_iteration_in_micro_seconds =
                std::chrono::high_resolution_clock::duration(iteration_end_time_point - iteration_start_time_point).count();
            output_statistics_ptr->m_total_time_of_all_performed_iterations_in_micro_seconds =
                std::chrono::high_resolution_clock::duration(iteration_end_time_point - start_time_point).count();
        }
        while (!terminate_comutation(*output_statistics_ptr));
    }

    return m_lambdas;
}


}
