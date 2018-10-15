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
    NOT_IMPLEMENTED_YET();
}


std::vector<float_32_bit> const&  motion_constraint_system::solve(
        std::vector<rigid_body> const&  rigid_bosies,
        std::vector<linear_and_angular_vector> const&  accelerations_from_external_forces,
        std::function<bool(computation_statistics const&)> const&  terminate_comutation,
        float_32_bit const  time_step_in_seconds,
        std::vector<linear_and_angular_vector>& output_accelerations_from_constraints,
        computation_statistics* const  output_statistics_ptr
        )
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


}
