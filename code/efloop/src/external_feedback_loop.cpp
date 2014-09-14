#include <efloop/external_feedback_loop.hpp>
#include <efloop/access_to_sensory_cells.hpp>
#include <efloop/access_to_synapses_to_muscles.hpp>
#include <thread>
#include <mutex>

namespace efloop {


static void  thread_compute_next_state_of_environment(external_feedback_loop* const  feedback_loop,
                                                      std::mutex*  mutex_to_sensory_cells,
                                                      std::mutex*  mutex_to_synapses_to_muscles,
                                                      natural_32_bit  max_number_of_threads
                                                      )
{
    access_to_sensory_cells access_to_cells(
                feedback_loop->get_neural_tissue(),
                mutex_to_sensory_cells
                );
    access_to_synapses_to_muscles access_to_synapses(
                feedback_loop->get_neural_tissue(),
                mutex_to_synapses_to_muscles
                );

    feedback_loop->get_rules_and_logic_of_environment()->compute_next_state(
                access_to_cells,
                access_to_synapses,
                max_number_of_threads
                );
}


external_feedback_loop::external_feedback_loop(
        std::shared_ptr<cellab::neural_tissue> neural_tissue,
        std::shared_ptr<envlab::rules_and_logic_of_environment> rules_and_logic_of_environment
        )
    : m_neural_tissue(neural_tissue)
    , m_rules_and_logic_of_environment(rules_and_logic_of_environment)
{
    ASSUMPTION(m_neural_tissue.operator bool());
    ASSUMPTION(m_rules_and_logic_of_environment.operator bool());
}

std::shared_ptr<cellab::neural_tissue> external_feedback_loop::get_neural_tissue()
{
    return m_neural_tissue;
}

std::shared_ptr<cellab::neural_tissue const> external_feedback_loop::get_neural_tissue() const
{
    return m_neural_tissue;
}

std::shared_ptr<envlab::rules_and_logic_of_environment>
external_feedback_loop::get_rules_and_logic_of_environment()
{
    return m_rules_and_logic_of_environment;
}

std::shared_ptr<envlab::rules_and_logic_of_environment const>
external_feedback_loop::get_rules_and_logic_of_environment() const
{
    return m_rules_and_logic_of_environment;
}

void external_feedback_loop::compute_next_state_of_both_neural_tissue_and_environment(
        natural_32_bit max_number_of_threads_neural_tissue_can_create_and_run_simultaneously,
        natural_32_bit max_number_of_threads_environment_can_create_and_run_simultaneously)
{
    std::thread thread_for_environment;

    std::mutex  mutex_to_sensory_cells;
    {
        std::lock_guard<std::mutex> const  lock_access_to_sensory_cells(mutex_to_sensory_cells);

        std::mutex  mutex_to_synapses_to_muscles;
        {
            std::lock_guard<std::mutex> const  lock_access_to_synapses_to_muscles(mutex_to_synapses_to_muscles);

            thread_for_environment = std::thread(
                        &efloop::thread_compute_next_state_of_environment,
                        this,
                        &mutex_to_sensory_cells,
                        &mutex_to_synapses_to_muscles,
                        max_number_of_threads_environment_can_create_and_run_simultaneously
                        );

            get_neural_tissue()->apply_transition_of_synapses_to_muscles(
                        max_number_of_threads_neural_tissue_can_create_and_run_simultaneously
                        );
        }

        get_neural_tissue()->apply_transition_of_synapses_of_tissue(
                    max_number_of_threads_neural_tissue_can_create_and_run_simultaneously
                    );
    }

    get_neural_tissue()->apply_transition_of_territorial_lists_of_synapses(
                max_number_of_threads_neural_tissue_can_create_and_run_simultaneously
                );

    get_neural_tissue()->apply_transition_of_synaptic_migration_in_tissue(
                max_number_of_threads_neural_tissue_can_create_and_run_simultaneously
                );

    get_neural_tissue()->apply_transition_of_signalling_in_tissue(
                max_number_of_threads_neural_tissue_can_create_and_run_simultaneously
                );

    get_neural_tissue()->apply_transition_of_cells_of_tissue(
                max_number_of_threads_neural_tissue_can_create_and_run_simultaneously
                );

    thread_for_environment.join();
}


}
