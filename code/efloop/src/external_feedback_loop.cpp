#include <efloop/external_feedback_loop.hpp>
#include <efloop/access_to_sensory_cells.hpp>
#include <efloop/access_to_synapses_to_muscles.hpp>
#include <utility/thread_synchronisarion_barrier.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

namespace efloop { namespace {


void  thread_compute_next_state_of_neural_tissue(
        std::shared_ptr<cellab::neural_tissue>  neural_tissue_ptr,
        std::mutex* const  mutex_to_sensory_cells,
        std::mutex* const  mutex_to_synapses_to_muscles,
        thread_synchronisarion_barrier&  synchronisation_after_initialisations_of_locks,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    ASSUMPTION(neural_tissue_ptr.operator bool());
    ASSUMPTION(mutex_to_sensory_cells != nullptr);
    ASSUMPTION(mutex_to_synapses_to_muscles != nullptr);
    ASSUMPTION(num_threads_avalilable_for_computation > 0U);

    std::unique_lock<std::mutex>  lock_to_sensory_cells( *mutex_to_sensory_cells );
    std::unique_lock<std::mutex>  lock_to_synapses_to_muscles( *mutex_to_synapses_to_muscles );
    synchronisation_after_initialisations_of_locks.wait_for_other_threads();

    neural_tissue_ptr->apply_transition_of_synapses_to_muscles(  num_threads_avalilable_for_computation );
    lock_to_synapses_to_muscles.unlock();

    neural_tissue_ptr->apply_transition_of_synapses_of_tissue( num_threads_avalilable_for_computation );
    lock_to_sensory_cells.unlock();

    neural_tissue_ptr->apply_transition_of_territorial_lists_of_synapses( num_threads_avalilable_for_computation );
    neural_tissue_ptr->apply_transition_of_synaptic_migration_in_tissue( num_threads_avalilable_for_computation );
    neural_tissue_ptr->apply_transition_of_signalling_in_tissue( num_threads_avalilable_for_computation );
    neural_tissue_ptr->apply_transition_of_cells_of_tissue( num_threads_avalilable_for_computation );
}

void  thread_compute_next_state_of_environment(
        external_feedback_loop* const  feedback_loop,
        std::mutex  mutexes_to_sensory_cells[],
        std::mutex  mutexes_to_synapses_to_muscles[],
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    ASSUMPTION(feedback_loop != nullptr);
    ASSUMPTION(mutexes_to_sensory_cells != nullptr);
    ASSUMPTION(mutexes_to_synapses_to_muscles != nullptr);
    ASSUMPTION(num_threads_avalilable_for_computation > 0U);

    std::vector<access_to_sensory_cells> accesses_to_cells;
    std::vector<access_to_synapses_to_muscles> accesses_to_synapses;
    for (natural_32_bit i = 0; i < feedback_loop->num_neural_tissues(); ++i)
    {
        accesses_to_cells.push_back(
                    access_to_sensory_cells(
                            feedback_loop->get_neural_tissue(i),
                            &mutexes_to_sensory_cells[i]
                            )
                    );
        accesses_to_synapses.push_back(
                    access_to_synapses_to_muscles(
                            feedback_loop->get_neural_tissue(i),
                            &mutexes_to_synapses_to_muscles[i]
                            )
                    );
    }

    feedback_loop->get_rules_and_logic_of_environment()->compute_next_state(
                accesses_to_cells,
                accesses_to_synapses,
                num_threads_avalilable_for_computation
                );
}


}}

namespace efloop {


external_feedback_loop::external_feedback_loop(
        std::vector< std::shared_ptr<cellab::neural_tissue> > const&  neural_tissues,
        std::shared_ptr<envlab::rules_and_logic_of_environment>  rules_and_logic_of_environment
        )
    : m_neural_tissues(neural_tissues)
    , m_rules_and_logic_of_environment(rules_and_logic_of_environment)
{
    ASSUMPTION(!neural_tissues.empty());
    ASSUMPTION(
        [this]()->bool {
            for (auto ptr : m_neural_tissues)
                if (!ptr.operator bool())
                    return false;
            return true;
            }()
        );
    ASSUMPTION(m_rules_and_logic_of_environment.operator bool());
}

natural_32_bit external_feedback_loop::num_neural_tissues() const
{
    return (natural_32_bit)m_neural_tissues.size();
}

std::shared_ptr<cellab::neural_tissue> external_feedback_loop::get_neural_tissue(natural_32_bit const index)
{
    ASSUMPTION(index < num_neural_tissues());
    return m_neural_tissues.at(index);
}

std::shared_ptr<cellab::neural_tissue const> external_feedback_loop::get_neural_tissue(natural_32_bit const index) const
{
    ASSUMPTION(index < num_neural_tissues());
    return m_neural_tissues.at(index);
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

void external_feedback_loop::compute_next_state_of_neural_tissues_and_environment(
        std::vector<natural_32_bit> const&  num_threads_avalilable_for_computation_of_neural_tissues,
        natural_32_bit const  num_threads_avalilable_for_computation_of_environment
        )
{
    ASSUMPTION(num_threads_avalilable_for_computation_of_neural_tissues.size() == num_neural_tissues());
    ASSUMPTION(num_threads_avalilable_for_computation_of_environment > 0U);

    std::unique_ptr< std::mutex[] >  mutexes_to_sensory_cells(new std::mutex[num_neural_tissues()]);
    std::unique_ptr< std::mutex[] >  mutexes_to_synapses_to_muscles(new std::mutex[num_neural_tissues()]);
    {
        natural_32_bit const  num_neural_tissues_to_be_updated_in_this_thread =
                (natural_32_bit)std::count(num_threads_avalilable_for_computation_of_neural_tissues.cbegin(),
                                           num_threads_avalilable_for_computation_of_neural_tissues.cend(),
                                           0U);

        natural_32_bit const  num_neural_tissues_to_be_updated_in_separate_threads =
                (natural_32_bit)num_threads_avalilable_for_computation_of_neural_tissues.size() -
                num_neural_tissues_to_be_updated_in_this_thread;

        thread_synchronisarion_barrier  synchronisation_after_initialisations_of_locks {
                num_neural_tissues_to_be_updated_in_separate_threads + 1U
                };

        std::vector< std::thread >  threads_of_neural_tissues;
        std::vector<natural_32_bit>  this_thread_indices_of_neural_tissues;
        std::vector< std::unique_lock<std::mutex> >  this_thread_locks_to_sensory_cells;
        std::vector< std::unique_lock<std::mutex> >  this_thread_locks_to_synapses_to_muscles;

        for (natural_32_bit i = 0; i < num_neural_tissues(); ++i)
            if (num_threads_avalilable_for_computation_of_neural_tissues[i] > 0)
                threads_of_neural_tissues.push_back(
                            std::thread(
                                    &efloop::thread_compute_next_state_of_neural_tissue,
                                    get_neural_tissue(i),
                                    &mutexes_to_sensory_cells[i],
                                    &mutexes_to_synapses_to_muscles[i],
                                    std::ref(synchronisation_after_initialisations_of_locks),
                                    num_threads_avalilable_for_computation_of_neural_tissues[i]
                                    )
                            );
            else
            {
                this_thread_indices_of_neural_tissues.push_back(
                            i
                            );
                this_thread_locks_to_sensory_cells.push_back(
                            std::unique_lock<std::mutex>(mutexes_to_sensory_cells[i])
                            );
                this_thread_locks_to_synapses_to_muscles.push_back(
                            std::unique_lock<std::mutex>(mutexes_to_synapses_to_muscles[i])
                            );
            }

        INVARIANT(this_thread_indices_of_neural_tissues.size() == num_neural_tissues_to_be_updated_in_this_thread);
        INVARIANT(this_thread_indices_of_neural_tissues.size() == this_thread_locks_to_sensory_cells.size());
        INVARIANT(this_thread_indices_of_neural_tissues.size() == this_thread_locks_to_synapses_to_muscles.size());

        synchronisation_after_initialisations_of_locks.wait_for_other_threads();

        std::thread  thread_for_environment;

        if (num_threads_avalilable_for_computation_of_environment != 1U)
        {
            if (num_neural_tissues_to_be_updated_in_this_thread == 0U)
                efloop::thread_compute_next_state_of_environment(
                            this,
                            mutexes_to_sensory_cells.get(),
                            mutexes_to_synapses_to_muscles.get(),
                            num_threads_avalilable_for_computation_of_environment
                            );
            else
                thread_for_environment = std::thread(
                            &efloop::thread_compute_next_state_of_environment,
                            this,
                            mutexes_to_sensory_cells.get(),
                            mutexes_to_synapses_to_muscles.get(),
                            num_threads_avalilable_for_computation_of_environment
                            );
        }

        for (natural_32_bit i = 0; i < num_neural_tissues_to_be_updated_in_this_thread; ++i)
        {
            std::shared_ptr<cellab::neural_tissue>  neural_tissue_ptr =
                    get_neural_tissue( this_thread_indices_of_neural_tissues[i] );

            neural_tissue_ptr->apply_transition_of_synapses_to_muscles(1);
            this_thread_locks_to_synapses_to_muscles[i].unlock();

            neural_tissue_ptr->apply_transition_of_synapses_of_tissue(1);
            this_thread_locks_to_sensory_cells[i].unlock();
        }
        for (natural_32_bit i = 0; i < num_neural_tissues_to_be_updated_in_this_thread; ++i)
        {
            std::shared_ptr<cellab::neural_tissue>  neural_tissue_ptr =
                    get_neural_tissue( this_thread_indices_of_neural_tissues[i] );

            neural_tissue_ptr->apply_transition_of_territorial_lists_of_synapses(1);
            neural_tissue_ptr->apply_transition_of_synaptic_migration_in_tissue(1);
            neural_tissue_ptr->apply_transition_of_signalling_in_tissue(1);
            neural_tissue_ptr->apply_transition_of_cells_of_tissue(1);
        }

        if (num_threads_avalilable_for_computation_of_environment == 1U)
            efloop::thread_compute_next_state_of_environment(
                        this,
                        mutexes_to_sensory_cells.get(),
                        mutexes_to_synapses_to_muscles.get(),
                        1U
                        );

        for(std::thread& thread : threads_of_neural_tissues)
            thread.join();
        if (thread_for_environment.joinable())
            thread_for_environment.join();
    }
}


}
