#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <tuple>
#include <thread>

#include <utility/development.hpp>

static natural_64_bit go_to_next_value_modulo_range(natural_64_bit const current_value,
                                                    natural_64_bit const range,
                                                    natural_64_bit& extent);

static bool go_to_next_index(natural_32_bit& index, natural_32_bit const extent, natural_32_bit const size)
{
    natural_64_bit extent_64_bit = extent;
    index = go_to_next_value_modulo_range(index,size,extent_64_bit);
    return extent_64_bit == 0ULL;
}

namespace cellab {


typedef natural_16_bit kind_of_cell;


static void thread_apply_transition_of_synapses_to_muscles(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        std::function<
            void(
            bits_reference& bits_of_synapse_to_be_updated,
            kind_of_cell kind_of_source_cell,
            bits_const_reference const& bits_of_source_cell,
            )> single_threaded_transition_function_of_dynamic_state_of_synapse_to_muscle,
        natural_32_bit index,
        natural_32_bit const extent_of_index
        )
{
    do
    {
        bits_reference bits_of_synapse =
            dynamic_state_of_tissue->find_bits_of_synapse_to_muscle(index);

        tissue_coordinates const source_cell_coords =
            dynamic_state_of_tissue->get_coordinates_of_source_cell_of_synapse_to_muscle(index);

        natural_16_bit const kind_of_source_cell =
            static_state_of_tissue->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                    source_cell_coords.get_coord_along_columnar_axis()
                    );

        bits_const_reference const bits_of_source_cell =
                dynamic_state_of_tissue->find_bits_of_cell_in_tissue(
                    source_cell_coords.get_coord_along_x_axis(),
                    source_cell_coords.get_coord_along_y_axis(),
                    source_cell_coords.get_coord_along_columnar_axis()
                    );

        single_threaded_transition_function_of_dynamic_state_of_synapse_to_muscle(
                    bits_of_synapse,
                    kind_of_source_cell,
                    bits_of_source_cell,
                    );
    }
    while (go_to_next_index(index,extent_of_index,static_state_of_tissue->num_synapses_to_muscles()));
}

void apply_transition_of_synapses_to_muscles(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_synapse_to_be_updated,
            static_state_of_synapse const& static_state_of_updated_synapse,
            bits_const_reference bits_of_source_cell,
            static_state_of_cell const& static_state_of_source_cell
            )> single_threaded_transition_function_of_dynamic_state_of_synapse_to_muscle,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    std::vector<std::thread> threads;
    for (natural_32_bit i = 0; i < num_avalilable_thread_for_creation_and_use; ++i)
    {
        natural_32_bit index = 0U;
        if (!go_to_next_index(index,i,static_state_of_tissue->num_synapses_to_muscles()))
            break;

        threads.push_back(
                    std::thread(
                        &cellab::thread_apply_transition_of_synapses_to_muscles,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        single_threaded_transition_function_of_dynamic_state_of_synapse_to_muscle,
                        index,
                        num_avalilable_thread_for_creation_and_use
                        )
                    );
    }

    for(std::thread& thread : threads)
        thread.join();
}



}
