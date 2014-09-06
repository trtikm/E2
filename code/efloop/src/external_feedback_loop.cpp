#include <efloop/external_feedback_loop.hpp>
#include <envlab/rules_and_logic_of_environment.hpp>

namespace efloop {


external_feedback_loop::external_feedback_loop(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_state_of_neural_tissue,
        std::shared_ptr<envlab::rules_and_logic_of_environment> rules_and_logic_of_environment
        )
    : m_dynamic_state_of_neural_tissue(dynamic_state_of_neural_tissue)
    , m_rules_and_logic_of_environment(rules_and_logic_of_environment)
{}

void external_feedback_loop::compute_next_state_of_both_neural_tissue_and_environment(
        natural_32_bit max_number_of_threads_neural_tissue_can_create_and_run_simultaneously,
        natural_32_bit max_number_of_threads_environment_can_create_and_run_simultaneously)
{
}



}
