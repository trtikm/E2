#include <cellconnect/fill_delimeters_between_territorial_lists.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <array>
#include <thread>

namespace cellconnect { namespace {


void  thread_fill_delimeters_between_territorial_lists_in_columns(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates
        )
{
    do
    {
        for (cellab::kind_of_cell i = 0U; i < static_state_ptr->num_kinds_of_tissue_cells(); ++i)
            for (natural_32_bit j = 0U; j < static_state_ptr->num_tissue_cells_of_cell_kind(i); ++j)
            {
                natural_32_bit const  c_coord =
                    static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(i) + j;

                std::array<bits_reference,6U> bits_of_delimiters = {
                        dynamic_state_ptr->find_bits_of_delimiter_between_territorial_lists(
                                x_coord, y_coord, c_coord, 0U
                                ),
                        dynamic_state_ptr->find_bits_of_delimiter_between_territorial_lists(
                                x_coord, y_coord, c_coord, 1U
                                ),
                        dynamic_state_ptr->find_bits_of_delimiter_between_territorial_lists(
                                x_coord, y_coord, c_coord, 2U
                                ),
                        dynamic_state_ptr->find_bits_of_delimiter_between_territorial_lists(
                                x_coord, y_coord, c_coord, 3U
                                ),
                        dynamic_state_ptr->find_bits_of_delimiter_between_territorial_lists(
                                x_coord, y_coord, c_coord, 4U
                                ),
                        dynamic_state_ptr->find_bits_of_delimiter_between_territorial_lists(
                                x_coord, y_coord, c_coord, 5U
                                ),
                        };

                natural_32_bit const  count = static_state_ptr->num_synapses_in_territory_of_cell_kind(i) / 6U;
                value_to_bits(0U * count, bits_of_delimiters.at(0U));
                value_to_bits(1U * count, bits_of_delimiters.at(1U));
                value_to_bits(2U * count, bits_of_delimiters.at(2U));
                value_to_bits(3U * count, bits_of_delimiters.at(3U));
                value_to_bits(4U * count, bits_of_delimiters.at(4U));
                value_to_bits(5U * count, bits_of_delimiters.at(5U));
            }
    }
    while (cellab::go_to_next_column(
                    x_coord,y_coord,
                    extent_in_coordinates,
                    static_state_ptr->num_cells_along_x_axis(),
                    static_state_ptr->num_cells_along_y_axis()
                    ));
}


}}

namespace cellconnect {


void  fill_delimeters_between_territorial_lists(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    ASSUMPTION(num_threads_avalilable_for_computation > 0U);

    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

    std::vector<std::thread> threads;
    for (natural_32_bit i = 1U; i < num_threads_avalilable_for_computation; ++i)
    {
        natural_32_bit x_coord = 0U;
        natural_32_bit y_coord = 0U;
        if (!cellab::go_to_next_column(
                    x_coord,y_coord,
                    i,
                    static_state_ptr->num_cells_along_x_axis(),
                    static_state_ptr->num_cells_along_y_axis()
                    ))
            break;

        threads.push_back(
                    std::thread(
                        &cellconnect::thread_fill_delimeters_between_territorial_lists_in_columns,
                        dynamic_state_ptr,
                        static_state_ptr,
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation
                        )
                    );
    }

    thread_fill_delimeters_between_territorial_lists_in_columns(
            dynamic_state_ptr,
            static_state_ptr,
            0U,0U,
            num_threads_avalilable_for_computation
            );

    for(std::thread& thread : threads)
        thread.join();
}


}
