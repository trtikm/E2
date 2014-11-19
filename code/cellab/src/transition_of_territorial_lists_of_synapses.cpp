#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/territorial_state_of_synapse.hpp>
#include <cellab/shift_in_coordinates.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <memory>
#include <vector>
#include <array>
#include <thread>

namespace cellab {


static void move_all_synapse_data_from_source_list_to_target_list(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& cell_coordinates,
        natural_32_bit source_list_index,
        natural_32_bit const target_list_index,
        natural_32_bit index_into_source_list,
        std::array<natural_32_bit,8U>& boundaries_of_lists
        )
{
    ASSUMPTION(source_list_index < 7U);
    ASSUMPTION(target_list_index < 7U);
    ASSUMPTION(index_into_source_list >= boundaries_of_lists.at(source_list_index));
    ASSUMPTION(index_into_source_list < boundaries_of_lists.at(source_list_index + 1U));
    for ( ; source_list_index < target_list_index; ++source_list_index)
    {
        natural_32_bit const next_list = source_list_index + 1U;
        natural_32_bit const target_index = boundaries_of_lists.at(next_list) - 1U;
        swap_all_data_of_two_synapses(
                dynamic_state_of_tissue,
                cell_coordinates,
                index_into_source_list,
                cell_coordinates,
                target_index
                );
        index_into_source_list = target_index;
        boundaries_of_lists.at(next_list) = target_index;
    }
    for ( ; target_list_index < source_list_index; --source_list_index)
    {
        natural_32_bit const target_index = boundaries_of_lists.at(source_list_index);
        swap_all_data_of_two_synapses(
                dynamic_state_of_tissue,
                cell_coordinates,
                index_into_source_list,
                cell_coordinates,
                target_index
                );
        index_into_source_list = target_index;
        boundaries_of_lists.at(source_list_index) = target_index + 1U;
    }
}

static void move_synapses_into_proper_lists_in_territory_of_one_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_32_bit const x_coord,
        natural_32_bit const y_coord,
        natural_32_bit const c_coord
        )
{
    ASSUMPTION(c_coord < static_state_of_tissue->num_cells_along_columnar_axis());

    std::array<bits_reference,6U> bits_of_delimiters = {
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 0U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 1U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 2U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 3U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 4U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 5U
                    ),
            };

    std::array<natural_32_bit,8U> boundaries_of_lists = {
            0U,
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(0U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(1U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(2U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(3U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(4U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(5U)),
            static_state_of_tissue->num_synapses_in_territory_of_cell_kind(
                    static_state_of_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                            c_coord
                            )
                    )
            };

    for (natural_32_bit list_index = 0U; list_index < 7U; ++list_index)
        for (natural_32_bit index_into_list = boundaries_of_lists.at(list_index);
             index_into_list < boundaries_of_lists.at(list_index + 1U);
             )
        {
            natural_32_bit const target_list_index =
                    convert_territorial_state_of_synapse_to_territorial_list_index(
                        static_cast<territorial_state_of_synapse>(
                            bits_to_value<natural_32_bit>(
                                dynamic_state_of_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(
                                        x_coord, y_coord, c_coord,
                                        index_into_list
                                        )
                                )
                            )
                        );
            INVARIANT(target_list_index < 7U);
            if (target_list_index == list_index)
                ++index_into_list;
            else
            {
                move_all_synapse_data_from_source_list_to_target_list(
                        dynamic_state_of_tissue,
                        tissue_coordinates(x_coord, y_coord, c_coord),
                        list_index,
                        target_list_index,
                        index_into_list,
                        boundaries_of_lists
                        );
                if (index_into_list < boundaries_of_lists.at(list_index))
                {
                    ++index_into_list;
                    INVARIANT(index_into_list == boundaries_of_lists.at(list_index));
                }
            }
        }

    for (natural_32_bit delimiter_index = 0U; delimiter_index < bits_of_delimiters.size(); ++delimiter_index)
    {
        INVARIANT( boundaries_of_lists.at(delimiter_index) <= boundaries_of_lists.at(delimiter_index + 1U) );
        value_to_bits( boundaries_of_lists.at(delimiter_index + 1U), bits_of_delimiters.at(delimiter_index) );
    }
}

static void thread_apply_transition_of_territorial_lists_of_synapses(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_32_bit x_coord,
        natural_32_bit y_coord,
        natural_32_bit c_coord,
        natural_32_bit const extent_in_coordinates
        )
{
    do
    {
        move_synapses_into_proper_lists_in_territory_of_one_cell(
                    dynamic_state_of_tissue,
                    static_state_of_tissue,
                    x_coord,y_coord,c_coord
                    );
    }
    while (go_to_next_coordinates(
               x_coord,y_coord,c_coord,
               extent_in_coordinates,
               static_state_of_tissue->num_cells_along_x_axis(),
               static_state_of_tissue->num_cells_along_y_axis(),
               static_state_of_tissue->num_cells_along_columnar_axis()
               ));
}

void apply_transition_of_territorial_lists_of_synapses(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    std::vector<std::thread> threads;
    for (natural_32_bit i = 1U; i < num_threads_avalilable_for_computation; ++i)
    {
        natural_32_bit x_coord = 0U;
        natural_32_bit y_coord = 0U;
        natural_32_bit c_coord = 0U;
        if (!go_to_next_coordinates(
                    x_coord,y_coord,c_coord,
                    i,
                    static_state_of_tissue->num_cells_along_x_axis(),
                    static_state_of_tissue->num_cells_along_y_axis(),
                    static_state_of_tissue->num_cells_along_columnar_axis()
                    ))
            break;

        threads.push_back(
                    std::thread(
                        &cellab::thread_apply_transition_of_territorial_lists_of_synapses,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        x_coord,y_coord,c_coord,
                        num_threads_avalilable_for_computation
                        )
                    );
    }

    thread_apply_transition_of_territorial_lists_of_synapses(
            dynamic_state_of_tissue,
            static_state_of_tissue,
            0U,0U,0U,
            num_threads_avalilable_for_computation
            );

    for(std::thread& thread : threads)
        thread.join();
}


}
