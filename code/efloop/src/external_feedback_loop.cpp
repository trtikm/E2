#include <efloop/external_feedback_loop.hpp>
#include <envlab/rules_and_logic_of_environment.hpp>
#include <thread>

namespace efloop {


external_feedback_loop::external_feedback_loop(
        std::shared_ptr<cellab::neural_tissue> neural_tissue,
        std::shared_ptr<envlab::rules_and_logic_of_environment> rules_and_logic_of_environment
        )
    : m_neural_tissue(neural_tissue)
    , m_rules_and_logic_of_environment(rules_and_logic_of_environment)
{}

std::shared_ptr<cellab::neural_tissue> external_feedback_loop::get_neural_tissue()
{
    return m_neural_tissue;
}

std::shared_ptr<cellab::neural_tissue const> external_feedback_loop::get_neural_tissue() const
{
    return m_neural_tissue;
}

void external_feedback_loop::compute_next_state_of_both_neural_tissue_and_environment(
        natural_32_bit max_number_of_threads_neural_tissue_can_create_and_run_simultaneously,
        natural_32_bit max_number_of_threads_environment_can_create_and_run_simultaneously)
{
}


}
