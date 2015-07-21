#include <cellconnect/spread_synapses_into_local_neighbourhoods.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <thread>
#include <algorithm>
#include <functional>

namespace cellconnect { namespace {


void  thread_check_consistency_of_matrix_and_column(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
        natural_32_bit  x,
        natural_32_bit  y,
        natural_32_bit const  extent_in_coordinates,
        cellab::kind_of_cell const  target_kind,
        cellab::kind_of_cell const  source_kind,
        natural_64_bit const SUM,
        std::vector<bool>& results,
        natural_32_bit const  my_result_index
        )
{
    do
    {
        natural_64_bit num_synapses = 0ULL;
        natural_32_bit const  c0 = static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(target_kind);
        for (natural_32_bit i = 0U; i < static_state_ptr->num_cells_of_cell_kind(target_kind); ++i)
            for (natural_32_bit j = 0U; j < static_state_ptr->num_synapses_in_territory_of_cell_kind(target_kind); ++j)
            {
                cellab::tissue_coordinates const  coords =
                    cellab::get_coordinates_of_source_cell_of_synapse_in_tissue(
                            dynamic_state_ptr,
                            cellab::tissue_coordinates(x,y,c0 + i),
                            j);
                cellab::kind_of_cell const  source_cell_kind_of_synapse =
                    static_state_ptr->compute_kind_of_cell_from_its_position_along_columnar_axis(
                            coords.get_coord_along_columnar_axis()
                            );
                if (source_cell_kind_of_synapse == source_kind)
                    ++num_synapses;
            }
        if (SUM != num_synapses)
        {
            results[my_result_index] = false;
            break;
        }
    }
    while (cellab::go_to_next_column(
                    x,y,
                    extent_in_coordinates,
                    static_state_ptr->num_cells_along_x_axis(),
                    static_state_ptr->num_cells_along_y_axis()
                    ));
}


bool  check_consistency_of_matrix_and_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
        cellab::kind_of_cell const  target_kind,
        cellab::kind_of_cell const  source_kind,
        std::vector<natural_32_bit> const&  matrix,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    natural_64_bit SUM = 0ULL;
    for (natural_32_bit const count : matrix)
        SUM += count;

    std::vector<bool> results(num_threads_avalilable_for_computation,true);

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
                        &cellconnect::thread_check_consistency_of_matrix_and_column,
                        dynamic_state_ptr,
                        static_state_ptr,
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation,
                        target_kind,
                        source_kind,
                        SUM,
                        std::ref(results),
                        i
                        )
                    );
    }

    cellconnect::thread_check_consistency_of_matrix_and_column(
            dynamic_state_ptr,
            static_state_ptr,
            0U,0U,
            num_threads_avalilable_for_computation,
            target_kind,
            source_kind,
            SUM,
            std::ref(results),
            0
            );

    for(std::thread& thread : threads)
        thread.join();

    return std::find(results.begin(),results.end(),false) == results.end();
}


}}

namespace cellconnect { namespace {



void  thread_spread_synapses(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
        cellab::kind_of_cell const  target_kind,
        cellab::kind_of_cell const  source_kind
        )
{
}


bool go_to_next_task(
        natural_32_bit&  row,
        natural_32_bit&  column,
        natural_32_bit&  index,
        natural_64_bit&  shift,
        natural_32_bit const  diameter_x,
        natural_32_bit const  diameter_y,
        std::vector<natural_32_bit> const&  matrix
        )
{
    ++shift;
    ++index;
    if (index >= matrix.at(row * diameter_x + column))
    {
        index = 0U;
        ++column;
        if (column >= diameter_x)
        {
            column = 0U;
            ++row;
            if (row >= diameter_y)
            {
                return false;
            }
        }
        else if (column == diameter_x / 2U && row == diameter_y / 2U)
        {
            shift += matrix.at(row * diameter_x + column);
            ++column;
        }
    }
    return true;
}


}}

namespace cellconnect {


void  spread_synapses_into_local_neighbourhoods(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_target_cells_of_synapses,
        cellab::kind_of_cell const  kind_of_source_cells_of_synapses,
        natural_32_bit const  diameter_x,
        natural_32_bit const  diameter_y,
        std::vector<natural_32_bit> const&  matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(dynamic_state_ptr.operator bool());
    ASSUMPTION(num_threads_avalilable_for_computation > 0U);
    ASSUMPTION(diameter_x > 0U && (diameter_x % 2U) == 1U);
    ASSUMPTION(diameter_y > 0U && (diameter_y % 2U) == 1U);
    ASSUMPTION(diameter_x > 2U || diameter_y > 2U);

    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

    ASSUMPTION(static_state_ptr.operator bool());
    ASSUMPTION(diameter_x < static_state_ptr->num_cells_along_x_axis() && diameter_y < static_state_ptr->num_cells_along_y_axis());
    ASSUMPTION(matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood.size() == diameter_x * diameter_y);
    ASSUMPTION(kind_of_target_cells_of_synapses < static_state_ptr->num_kinds_of_tissue_cells());
    ASSUMPTION(kind_of_source_cells_of_synapses < static_state_ptr->num_kinds_of_cells());
    ASSUMPTION(
            check_consistency_of_matrix_and_tissue(
                    dynamic_state_ptr,
                    static_state_ptr,
                    kind_of_target_cells_of_synapses,
                    kind_of_source_cells_of_synapses,
                    matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood,
                    num_threads_avalilable_for_computation
                    )
            );

    natural_32_bit row = 0U;
    natural_32_bit column = 0U;
    natural_32_bit index = 0U;
    natural_64_bit shift = 0ULL;
    bool done = false;
    do
    {
        std::vector<std::thread> threads;
        for (natural_32_bit i = 1U; i < num_threads_avalilable_for_computation && !done; ++i)
        {
            threads.push_back(
                        std::thread(
                            &cellconnect::thread_spread_synapses,
                            dynamic_state_ptr,
                            static_state_ptr,
                            kind_of_target_cells_of_synapses,
                            kind_of_source_cells_of_synapses
                            )
                        );

            done = !cellconnect::go_to_next_task(
                            row,column,index,shift,
                            diameter_x,diameter_y,
                            matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood
                            );
        }
        if (!done)
        {
            cellconnect::thread_spread_synapses(
                    dynamic_state_ptr,
                    static_state_ptr,
                    kind_of_target_cells_of_synapses,
                    kind_of_source_cells_of_synapses
                    );
            done = !cellconnect::go_to_next_task(
                            row,column,index,shift,
                            diameter_x,diameter_y,
                            matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood
                            );
        }
        for(std::thread& thread : threads)
            thread.join();
    }
    while(!done);
    INVARIANT(index == 0U && column == 0U && row == diameter_y);
}



}
