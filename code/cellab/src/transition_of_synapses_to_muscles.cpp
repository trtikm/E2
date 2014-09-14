#include <cellab/transition_algorithms.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/invariants.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <thread>

namespace cellab {


static void thread_apply_transition_of_synapses_to_muscles(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle const&
            transition_function_of_packed_synapse_to_muscle,
        natural_32_bit index,
        natural_32_bit const extent_of_index
        )
{
    do
    {
        bits_reference bits_of_synapse =
            dynamic_state_of_tissue->find_bits_of_synapse_to_muscle(index);

        tissue_coordinates const source_cell_coords(
                    get_coordinates_of_source_cell_of_synapse_to_muscle(
                            dynamic_state_of_tissue,
                            index
                            )
                    );

        std::pair<kind_of_cell,natural_32_bit> const kind_and_index_of_source_cell =
            static_state_of_tissue->compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
                    source_cell_coords.get_coord_along_columnar_axis()
                    );
        //INVARIANT(kind_and_index_of_source_cell.first < static_state_of_tissue->num_kinds_of_tissue_cells());

        bits_const_reference const bits_of_source_cell =
                dynamic_state_of_tissue->find_bits_of_cell(
                    source_cell_coords.get_coord_along_x_axis(),
                    source_cell_coords.get_coord_along_y_axis(),
                    kind_and_index_of_source_cell.first,
                    kind_and_index_of_source_cell.second
                    );

        transition_function_of_packed_synapse_to_muscle(
                    bits_of_synapse,
                    kind_and_index_of_source_cell.first,
                    bits_of_source_cell
                    );
    }
    while (go_to_next_index(index,extent_of_index,static_state_of_tissue->num_synapses_to_muscles()));
}

void apply_transition_of_synapses_to_muscles(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle const&
            transition_function_of_packed_synapse_to_muscle,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    std::vector<std::thread> threads;
    for (natural_32_bit i = 1U; i < num_threads_avalilable_for_computation; ++i)
    {
        natural_32_bit index = 0U;
        if (!go_to_next_index(index,i,static_state_of_tissue->num_synapses_to_muscles()))
            break;

        threads.push_back(
                    std::thread(
                        &cellab::thread_apply_transition_of_synapses_to_muscles,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        transition_function_of_packed_synapse_to_muscle,
                        index,
                        num_threads_avalilable_for_computation
                        )
                    );
    }

    thread_apply_transition_of_synapses_to_muscles(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                transition_function_of_packed_synapse_to_muscle,
                0U,
                num_threads_avalilable_for_computation
                );

    for(std::thread& thread : threads)
        thread.join();
}


}
