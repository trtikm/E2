#ifndef ANGEO_MOTION_CONSTRAINT_SYSTEM_HPP_INCLUDED
#   define ANGEO_MOTION_CONSTRAINT_SYSTEM_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/rigid_body.hpp>
#   include <vector>
#   include <functional>
#   include <tuple>

namespace angeo {


struct  motion_constraint_system
{
    enum struct VARIABLE_BOUND_TYPE : natural_8_bit
    {
        CONCRETE_VALUE = 0,
        VARIABLE_INDEX = 1
    };

    union  variable_bound
    {
        float_32_bit  m_concrete_value;
        natural_32_bit  m_variable_index;
    };

    using  constraint_id = natural_32_bit;

    struct  computation_statistics
    {
        natural_32_bit  m_num_constraints_in_system;    // So, it also says how many variables there are (for each constraint one variable).
        natural_32_bit  m_num_performed_iterations;     // A single iteration performs for each constraint in the system
                                                        // an update of the corresponding variable to be a bit closer to a solution.
        float_32_bit  m_max_change_of_variables;        // The absolute value of the maximal change of any variable of the system
                                                        // in the last iteration.
        natural_32_bit  m_time_of_last_iteration_in_micro_seconds;
        natural_32_bit  m_total_time_of_all_performed_iterations_in_micro_seconds;
    };

    // It is assumed, that ids 'rb_0' and 'rb_1' will be interpreted in the method 'solve'
    // as indices to 'rigid_bosies' vector (passed to that function via the first parameter).
    constraint_id  insert_constraint(
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
            float_32_bit const  variable_initial_value = 0.0f
            );

    void  clear();

    natural_32_bit  get_num_constraints() const { return (natural_32_bit)m_lambdas.size(); }

    static bool  default_computation_terminator(
            natural_32_bit const  max_num_iterations,
            float_32_bit const  min_change_of_variables,
            natural_32_bit const  max_computation_time_in_micro_seconds,
            computation_statistics const&  statistics
            );

    // Solves the constraint system and returns a reference to the solution, i.e. values of unknown
    // variables. The order of variables (and their values) is the same as the order in which the
    // constraints were added to the system.
    std::vector<float_32_bit> const&  solve(
            std::vector<rigid_body> const&  rigid_bosies,   // It is assumed, that ids 'rb_0' and 'rb_1' passed to
                                                            // 'insert_constraint' method all relate to this vector.
            std::vector<linear_and_angular_vector> const&  accelerations_from_external_forces,
                    // For each rigid body: The external force (i.e. the linear component) is multiplied by the
                    // inverted mass, and the external torque (i.e. the second component) is multiplied (from the left)
                    // by the inverted inertia tensor. When no exteral force and torque is acting on the body, then
                    // both components are zero vectors.
            std::function<bool(computation_statistics const&)> const&  terminate_comutation,
                    // The return value 'true' will terminate the computation.
            float_32_bit const  time_step_in_seconds,
            std::vector<linear_and_angular_vector>& output_accelerations_from_constraints,
                    // For each rigid body, both vecors (i.e. the linear and angular component) must be zero vector.
                    // The function then computes and writes both accelerations for each rigid body, whose id was
                    // passed to any inserted constraint (see the method 'insert_constraint'). Other records in the
                    // vector are not accessed.
            computation_statistics* const  output_statistics_ptr = nullptr
            );

private:

    // All vectors below have the same size, which is the number of inserted constraints. 

    std::vector<float_32_bit>  m_lambdas;   // Unknown variables of the system.
    std::vector<VARIABLE_BOUND_TYPE>  m_variable_lower_bound_types;
    std::vector<variable_bound>  m_variable_lower_bounds;
    std::vector<VARIABLE_BOUND_TYPE>  m_variable_upper_bound_types;
    std::vector<variable_bound>  m_variable_upper_bounds;

    using  index_element = std::pair<rigid_body_id, rigid_body_id>;

    std::vector<index_element>  m_index;    // Look up table from constraint ids to constraoined pair of rigid body ids.

    using  matrix_element = std::pair<linear_and_angular_vector, linear_and_angular_vector>;

    std::vector<matrix_element>  m_jacobian;    // The constraints.
    std::vector<matrix_element>  m_inverted_mass_matrix_times_jacobian_transposed;
    std::vector<float_32_bit>  m_rhs_vector;
};


}

#endif
