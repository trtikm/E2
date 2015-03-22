#include <utility/thread_synchronisarion_barrier.hpp>
#include <utility/assumptions.hpp>


thread_synchronisarion_barrier::thread_synchronisarion_barrier(natural_32_bit const num_threads_to_synchronise)
    : m_num_threads_to_wait_for(num_threads_to_synchronise)
    , m_mutex_to_m_num_threads_to_wait_for()
    , m_condition_variable()
{
    ASSUMPTION(num_threads_to_synchronise > 0U);
}

thread_synchronisarion_barrier::~thread_synchronisarion_barrier()
{
    //m_condition_variable.notify_all();
}

void thread_synchronisarion_barrier::wait_for_other_threads()
{
    std::unique_lock<std::mutex> lock_m_num_threads_to_wait_for{m_mutex_to_m_num_threads_to_wait_for};

    ASSUMPTION(m_num_threads_to_wait_for > 0U);

    --m_num_threads_to_wait_for;
    if (m_num_threads_to_wait_for == 0U)
        m_condition_variable.notify_all();
    else
        m_condition_variable.wait(lock_m_num_threads_to_wait_for, [this] { return m_num_threads_to_wait_for == 0; });
}
