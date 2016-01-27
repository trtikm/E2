#include <cellconnect/fill_delimiters_between_territorial_lists.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <array>
#include <thread>

namespace cellconnect { namespace {


void  thread_fill_delimiters_between_territorial_lists_in_territory_of_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        delimiters_fill_kind const  fill_kind,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        cellab::kind_of_cell const  cell_kind,
        natural_32_bit const  index_of_cell
        )
{
    TMPROF_BLOCK();

    natural_32_bit const  c_coord =
        static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(cell_kind) + index_of_cell;

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

    switch (fill_kind)
    {
    case delimiters_fill_kind::SYNAPSES_DISTRIBUTED_REGULARLY:
        {
            natural_32_bit const  count =
                    static_state_ptr->num_synapses_in_territory_of_cell_kind(cell_kind) / 7U;
            value_to_bits(1U * count, bits_of_delimiters.at(0U));
            value_to_bits(2U * count, bits_of_delimiters.at(1U));
            value_to_bits(3U * count, bits_of_delimiters.at(2U));
            value_to_bits(4U * count, bits_of_delimiters.at(3U));
            value_to_bits(5U * count, bits_of_delimiters.at(4U));
            value_to_bits(6U * count, bits_of_delimiters.at(5U));
        }
        break;
    case delimiters_fill_kind::NONE_CONNECTED_OTHERWISE_SYNAPSES_DISTRIBUTED_REGULARLY:
        {
            natural_32_bit const  count =
                    static_state_ptr->num_synapses_in_territory_of_cell_kind(cell_kind) / 6U;
            value_to_bits(0U, bits_of_delimiters.at(0U));
            value_to_bits(1U * count, bits_of_delimiters.at(1U));
            value_to_bits(2U * count, bits_of_delimiters.at(2U));
            value_to_bits(3U * count, bits_of_delimiters.at(3U));
            value_to_bits(4U * count, bits_of_delimiters.at(4U));
            value_to_bits(5U * count, bits_of_delimiters.at(5U));
        }
        break;
    case delimiters_fill_kind::ALL_SYNAPSES_CONNECTED:
        {
            ASSUMPTION(static_state_ptr->num_synapses_in_territory_of_cell_kind(cell_kind) > 0U);
            natural_32_bit const  count =
                    static_state_ptr->num_synapses_in_territory_of_cell_kind(cell_kind) - 1U;
            value_to_bits(count, bits_of_delimiters.at(0U));
            value_to_bits(count, bits_of_delimiters.at(1U));
            value_to_bits(count, bits_of_delimiters.at(2U));
            value_to_bits(count, bits_of_delimiters.at(3U));
            value_to_bits(count, bits_of_delimiters.at(4U));
            value_to_bits(count, bits_of_delimiters.at(5U));
        }
        break;
    default:
        UNREACHABLE();
    }
}

void  thread_fill_delimiters_between_territorial_lists_in_columns(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        delimiters_fill_kind const  fill_kind,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates
        )
{
    TMPROF_BLOCK();

    do
    {
        for (cellab::kind_of_cell i = 0U; i < static_state_ptr->num_kinds_of_tissue_cells(); ++i)
            for (natural_32_bit j = 0U; j < static_state_ptr->num_tissue_cells_of_cell_kind(i); ++j)
                thread_fill_delimiters_between_territorial_lists_in_territory_of_cell(
                            dynamic_state_ptr,
                            static_state_ptr,
                            fill_kind,
                            x_coord,
                            y_coord,
                            i,
                            j
                            );
    }
    while (cellab::go_to_next_column(
                    x_coord,y_coord,
                    extent_in_coordinates,
                    static_state_ptr->num_cells_along_x_axis(),
                    static_state_ptr->num_cells_along_y_axis()
                    ));
}


void  thread_fill_delimiters_between_territorial_lists_for_cell_kind(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        delimiters_fill_kind const  fill_kind,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates
        )
{
    TMPROF_BLOCK();

    do
    {
        for (natural_32_bit j = 0U;
             j < static_state_ptr->num_tissue_cells_of_cell_kind(kind_of_cells_to_be_considered);
             ++j)
            thread_fill_delimiters_between_territorial_lists_in_territory_of_cell(
                        dynamic_state_ptr,
                        static_state_ptr,
                        fill_kind,
                        x_coord,
                        y_coord,
                        kind_of_cells_to_be_considered,
                        j
                        );
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


void  fill_delimiters_between_territorial_lists(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        delimiters_fill_kind const  fill_kind,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    TMPROF_BLOCK();

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
                        &cellconnect::thread_fill_delimiters_between_territorial_lists_in_columns,
                        dynamic_state_ptr,
                        static_state_ptr,
                        fill_kind,
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation
                        )
                    );
    }

    cellconnect::thread_fill_delimiters_between_territorial_lists_in_columns(
            dynamic_state_ptr,
            static_state_ptr,
            fill_kind,
            0U,0U,
            num_threads_avalilable_for_computation
            );

    for(std::thread& thread : threads)
        thread.join();
}


void  fill_delimiters_between_territorial_lists_for_cell_kind(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        delimiters_fill_kind const  fill_kind,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    TMPROF_BLOCK();

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
                        &cellconnect::thread_fill_delimiters_between_territorial_lists_for_cell_kind,
                        dynamic_state_ptr,
                        static_state_ptr,
                        fill_kind,
                        kind_of_cells_to_be_considered,
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation
                        )
                    );
    }

    cellconnect::thread_fill_delimiters_between_territorial_lists_for_cell_kind(
            dynamic_state_ptr,
            static_state_ptr,
            fill_kind,
            kind_of_cells_to_be_considered,
            0U,0U,
            num_threads_avalilable_for_computation
            );

    for(std::thread& thread : threads)
        thread.join();
}


}
