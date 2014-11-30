#include "./my_environment.hpp"
#include "./my_cell.hpp"
#include "./my_synapse.hpp"
#include "./my_signalling.hpp"
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/random.hpp>
#include <utility/test.hpp>
#include <boost/chrono.hpp>
#include <vector>
#include <thread>

static void wait_milliseconds(natural_32_bit const num_millisecond)
{
    double const  max_duration = num_millisecond / 1000.0;
    for (boost::chrono::system_clock::time_point const  start = boost::chrono::system_clock::now();
         boost::chrono::duration<double>(boost::chrono::system_clock::now() - start).count() < max_duration;
         )
    {}
}


static void  thread_update_sensory_cells(
        efloop::access_to_sensory_cells const&  access_to_sensory_cells,
        efloop::access_to_synapses_to_muscles const&  access_to_synapses_to_muscles,
        natural_32_bit  cell_index,
        natural_32_bit const  shift_to_next_cell
        )
{
    wait_milliseconds(get_random_natural_32_bit_in_range(0U,1000U));

    for ( ; cell_index < access_to_sensory_cells.num_sensory_cells(); cell_index += shift_to_next_cell)
    {
        instance_wrapper<my_cell> cell;
        access_to_sensory_cells.read_sensory_cell(cell_index,cell);
        cell->increment();

        for (natural_32_bit i = 0, n = get_random_natural_32_bit_in_range(0U,10U); i < n; ++i)
        {
            instance_wrapper<my_synapse> synapse;
            access_to_synapses_to_muscles.read_synapse_to_muscle(
                        get_random_natural_32_bit_in_range(0U,access_to_synapses_to_muscles.num_synapses_to_muscles()-1U),
                        synapse);
            TEST_SUCCESS(synapse->count() == cell->count());
        }
    }
}


void my_environment::compute_next_state(
        efloop::access_to_sensory_cells const&  access_to_sensory_cells,
        efloop::access_to_synapses_to_muscles const&  access_to_synapses_to_muscles,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    std::vector<std::thread> threads;
    for (natural_32_bit i = 1U; i < num_threads_avalilable_for_computation; ++i)
        threads.push_back(
                    std::thread(
                        &thread_update_sensory_cells,
                        std::cref(access_to_sensory_cells),
                        std::cref(access_to_synapses_to_muscles),
                        i,
                        num_threads_avalilable_for_computation
                        )
                    );
    thread_update_sensory_cells(
                access_to_sensory_cells,
                access_to_synapses_to_muscles,
                0U,
                num_threads_avalilable_for_computation
                );
    for(std::thread& thread : threads)
        thread.join();
}
