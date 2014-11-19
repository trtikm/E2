#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/territorial_state_of_synapse.hpp>
#include <cellab/shift_in_coordinates.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <memory>
#include <vector>
#include <thread>

namespace cellab {


static void  exchange_all_data_of_synapses_between_territorial_lists_of_cells_at_given_coordinates(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& coordinates_of_first_cell,
        natural_32_bit const begin_index_of_first_list,
        natural_32_bit const end_index_of_first_list,
        tissue_coordinates const& coordinates_of_second_cell,
        natural_32_bit const begin_index_of_second_list,
        natural_32_bit const num_synapses_to_be_exchanged
        )
{
    if (num_synapses_to_be_exchanged == 0U)
        return;

    INVARIANT(begin_index_of_first_list < end_index_of_first_list);
    natural_32_bit index_of_current_synapse_in_first_list =
            get_random_natural_32_bit_in_range(
                begin_index_of_first_list,
                end_index_of_first_list - 1U
                );
    for (natural_32_bit i = 0U; i < num_synapses_to_be_exchanged; ++i)
    {
        swap_all_data_of_two_synapses(
                dynamic_state_of_tissue,
                coordinates_of_first_cell,
                index_of_current_synapse_in_first_list,
                coordinates_of_second_cell,
                begin_index_of_second_list + i
                );

        ++index_of_current_synapse_in_first_list;
        if (index_of_current_synapse_in_first_list == end_index_of_first_list)
            index_of_current_synapse_in_first_list = begin_index_of_first_list;
    }
}

static void  exchange_synapses_between_territorial_lists_of_cells_at_given_coordinates(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        tissue_coordinates const& coordinates_of_first_cell,
        natural_8_bit const list_index_in_first_cell,
        tissue_coordinates const& coordinates_of_second_cell,
        natural_8_bit const list_index_in_second_cell
        )
{
    natural_32_bit const begin_index_of_first_list =
            get_begin_index_of_territorial_list_of_cell(
                    dynamic_state_of_tissue,
                    coordinates_of_first_cell,
                    list_index_in_first_cell
                    );
    natural_32_bit const end_index_of_first_list =
            get_end_index_of_territorial_list_of_cell(
                    dynamic_state_of_tissue,
                    static_state_of_tissue,
                    coordinates_of_first_cell,
                    list_index_in_first_cell
                    );
    INVARIANT(begin_index_of_first_list <= end_index_of_first_list);

    natural_32_bit const begin_index_of_second_list =
            get_begin_index_of_territorial_list_of_cell(
                    dynamic_state_of_tissue,
                    coordinates_of_second_cell,
                    list_index_in_second_cell
                    );
    natural_32_bit const end_index_of_second_list =
            get_end_index_of_territorial_list_of_cell(
                    dynamic_state_of_tissue,
                    static_state_of_tissue,
                    coordinates_of_second_cell,
                    list_index_in_second_cell
                    );
    INVARIANT(begin_index_of_second_list <= end_index_of_second_list);

    if (end_index_of_first_list - begin_index_of_first_list >=
            end_index_of_second_list - begin_index_of_second_list)
        exchange_all_data_of_synapses_between_territorial_lists_of_cells_at_given_coordinates(
                dynamic_state_of_tissue,
                coordinates_of_first_cell,
                begin_index_of_first_list,
                end_index_of_first_list,
                coordinates_of_second_cell,
                begin_index_of_second_list,
                end_index_of_second_list - begin_index_of_second_list
                );
    else
        exchange_all_data_of_synapses_between_territorial_lists_of_cells_at_given_coordinates(
                dynamic_state_of_tissue,
                coordinates_of_second_cell,
                begin_index_of_second_list,
                end_index_of_second_list,
                coordinates_of_first_cell,
                begin_index_of_first_list,
                end_index_of_first_list - begin_index_of_first_list
                );
}

static void  thread_exchange_synapses_between_territorial_lists_of_all_cells(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_8_bit const list_index_in_pivot_cells,
        shift_in_coordinates const& shift,
        natural_8_bit const list_index_in_shift_cells,
        natural_32_bit x_coord,
        natural_32_bit y_coord,
        natural_32_bit c_coord,
        natural_32_bit const extent_in_coordinates
        )
{
    do
    {
        exchange_synapses_between_territorial_lists_of_cells_at_given_coordinates(
                    dynamic_state_of_tissue,
                    static_state_of_tissue,
                    tissue_coordinates(x_coord,y_coord,c_coord),
                    list_index_in_pivot_cells,
                    shift_coordinates(
                            tissue_coordinates(x_coord,y_coord,c_coord),
                            shift_in_coordinates(
                                clip_shift(shift.get_shift_along_x_axis(),x_coord,
                                           static_state_of_tissue->num_cells_along_x_axis(),
                                           static_state_of_tissue->is_x_axis_torus_axis()),
                                clip_shift(shift.get_shift_along_y_axis(),y_coord,
                                           static_state_of_tissue->num_cells_along_y_axis(),
                                           static_state_of_tissue->is_y_axis_torus_axis()),
                                clip_shift(shift.get_shift_along_columnar_axis(),c_coord,
                                           static_state_of_tissue->num_cells_along_columnar_axis(),
                                           static_state_of_tissue->is_columnar_axis_torus_axis())
                                ),
                            static_state_of_tissue->num_cells_along_x_axis(),
                            static_state_of_tissue->num_cells_along_y_axis(),
                            static_state_of_tissue->num_cells_along_columnar_axis()
                            ),
                    list_index_in_shift_cells
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

static void  exchange_synapses_between_territorial_lists_of_all_cells(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_8_bit const list_index_in_pivot_cells,
        shift_in_coordinates const& shift,
        natural_8_bit const list_index_in_shift_cells,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    ASSUMPTION(list_index_in_pivot_cells == 1U ||
               list_index_in_pivot_cells == 3U ||
               list_index_in_pivot_cells == 5U);
    ASSUMPTION(list_index_in_shift_cells == list_index_in_pivot_cells + 1U);

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
                        &cellab::thread_exchange_synapses_between_territorial_lists_of_all_cells,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        list_index_in_pivot_cells,
                        shift,
                        list_index_in_shift_cells,
                        x_coord,y_coord,c_coord,
                        num_threads_avalilable_for_computation
                        )
                    );
    }

    thread_exchange_synapses_between_territorial_lists_of_all_cells(
            dynamic_state_of_tissue,
            static_state_of_tissue,
            list_index_in_pivot_cells,
            shift,
            list_index_in_shift_cells,
            0U,0U,0U,
            num_threads_avalilable_for_computation
            );

    for(std::thread& thread : threads)
        thread.join();
}

void  apply_transition_of_synaptic_migration_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    exchange_synapses_between_territorial_lists_of_all_cells(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                convert_territorial_state_of_synapse_to_territorial_list_index(MIGRATION_ALONG_POSITIVE_X_AXIS),
                shift_in_coordinates(1,0,0),
                convert_territorial_state_of_synapse_to_territorial_list_index(MIGRATION_ALONG_NEGATIVE_X_AXIS),
                num_threads_avalilable_for_computation
                );
    exchange_synapses_between_territorial_lists_of_all_cells(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                convert_territorial_state_of_synapse_to_territorial_list_index(MIGRATION_ALONG_POSITIVE_Y_AXIS),
                shift_in_coordinates(0,1,0),
                convert_territorial_state_of_synapse_to_territorial_list_index(MIGRATION_ALONG_NEGATIVE_Y_AXIS),
                num_threads_avalilable_for_computation
                );
    exchange_synapses_between_territorial_lists_of_all_cells(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                convert_territorial_state_of_synapse_to_territorial_list_index(MIGRATION_ALONG_POSITIVE_COLUMNAR_AXIS),
                shift_in_coordinates(0,0,1),
                convert_territorial_state_of_synapse_to_territorial_list_index(MIGRATION_ALONG_NEGATIVE_COLUMNAR_AXIS),
                num_threads_avalilable_for_computation
                );
}


}
