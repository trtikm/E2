#include <cellconnect/spread_synapses_into_local_neighbourhoods.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <thread>
#include <algorithm>
#include <functional>

namespace cellconnect {


static void  thread_check_consistency_of_matrix_and_column(
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


static bool  check_consistency_of_matrix_and_tissue(
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


static natural_32_bit  read_x_coord_from_bits_of_coordinates(bits_reference const& bits_ref)
{
    natural_16_bit const num_bits = bits_ref.num_bits() / natural_16_bit(3U);
    INVARIANT(num_bits * natural_16_bit(3U) == bits_ref.num_bits());

    natural_32_bit coord_along_x_axis;
    bits_to_value(bits_ref,0U,(natural_8_bit)num_bits,coord_along_x_axis);

    return coord_along_x_axis;
}


static natural_32_bit  read_y_coord_from_bits_of_coordinates(bits_reference const& bits_ref)
{
    natural_16_bit const num_bits = bits_ref.num_bits() / natural_16_bit(3U);
    INVARIANT(num_bits * natural_16_bit(3U) == bits_ref.num_bits());

    natural_32_bit coord_along_y_axis;
    bits_to_value(bits_ref,(natural_8_bit)num_bits,(natural_8_bit)num_bits,coord_along_y_axis);

    return coord_along_y_axis;
}


static natural_32_bit  read_columnar_coord_from_bits_of_coordinates(bits_reference const& bits_ref)
{
    natural_16_bit const num_bits = bits_ref.num_bits() / natural_16_bit(3U);
    INVARIANT(num_bits * natural_16_bit(3U) == bits_ref.num_bits());

    natural_32_bit coord_along_columnar_axis;
    bits_to_value(bits_ref,(natural_8_bit)(num_bits + num_bits),(natural_8_bit)num_bits,coord_along_columnar_axis);

    return coord_along_columnar_axis;
}


static void  write_x_coord_to_bits_of_coordinates(natural_32_bit const  x_coord, bits_reference& bits_ref)
{
    natural_16_bit const num_bits = bits_ref.num_bits() / natural_16_bit(3U);
    INVARIANT(num_bits * natural_16_bit(3U) == bits_ref.num_bits());

    value_to_bits(x_coord,bits_ref,0U,(natural_8_bit)num_bits);
}


static void  write_y_coord_to_bits_of_coordinates(natural_32_bit const  y_coord, bits_reference& bits_ref)
{
    natural_16_bit const num_bits = bits_ref.num_bits() / natural_16_bit(3U);
    INVARIANT(num_bits * natural_16_bit(3U) == bits_ref.num_bits());

    value_to_bits(y_coord,bits_ref,(natural_8_bit)num_bits,(natural_8_bit)num_bits);
}


static bits_reference  find_bits_of_source_coords_of_synapse(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
        natural_32_bit const  x,
        natural_32_bit const  y,
        cellab::kind_of_cell const  target_kind,
        cellab::kind_of_cell const  source_kind,
        natural_64_bit const  shift_to_synapse
        )
{
    TMPROF_BLOCK();

    natural_32_bit const  c0 = static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(target_kind);
    natural_64_bit synapse_index = 0U;
    natural_32_bit cell_index = 0U;
    natural_32_bit territory_index = 0U;
    while (true)
    {
        INVARIANT(territory_index < static_state_ptr->num_synapses_in_territory_of_cell_kind(target_kind));

        bits_reference const  bits_of_coords =
                dynamic_state_ptr->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                        x, y, c0 + cell_index, territory_index
                        );
        natural_32_bit const  c = read_columnar_coord_from_bits_of_coordinates(bits_of_coords);
        if (static_state_ptr->compute_kind_of_cell_from_its_position_along_columnar_axis(c) == source_kind)
        {
            if (synapse_index == shift_to_synapse)
                return bits_of_coords;
            ++synapse_index;
        }

        ++cell_index;
        if (cell_index == static_state_ptr->num_cells_of_cell_kind(target_kind))
        {
            cell_index = 0U;
            ++territory_index;
        }
    }
}


static void  thread_spread_synapses(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
        cellab::kind_of_cell const  target_kind,
        cellab::kind_of_cell const  source_kind,
        integer_64_bit const  shift_x,
        integer_64_bit const  shift_y,
        natural_64_bit const  shift_to_synapse
        )
{
    TMPROF_BLOCK();

    for (natural_32_bit  x = 0U; x < static_state_ptr->num_cells_along_x_axis(); ++x)
        for (natural_32_bit  y = 0U; y < static_state_ptr->num_cells_along_y_axis(); ++y)
        {
            bits_reference  bits_of_coords =
                    cellconnect::find_bits_of_source_coords_of_synapse(
                            dynamic_state_ptr,
                            static_state_ptr,
                            x,
                            y,
                            target_kind,
                            source_kind,
                            shift_to_synapse
                            );

            natural_32_bit  current_x = cellconnect::read_x_coord_from_bits_of_coordinates(bits_of_coords);
            if (current_x != x)
                continue;
            natural_32_bit  current_y = cellconnect::read_y_coord_from_bits_of_coordinates(bits_of_coords);
            if (current_y != y)
                continue;

            do
            {
                natural_32_bit const  shifted_x = cellab::shift_coordinate(
                                                        current_x,
                                                        shift_x,
                                                        static_state_ptr->num_cells_along_x_axis(),
                                                        static_state_ptr->is_x_axis_torus_axis()
                                                        );
                if (shifted_x == static_state_ptr->num_cells_along_x_axis())
                    break;

                natural_32_bit const  shifted_y = cellab::shift_coordinate(
                                                        current_y,
                                                        shift_y,
                                                        static_state_ptr->num_cells_along_y_axis(),
                                                        static_state_ptr->is_y_axis_torus_axis()
                                                        );
                if (shifted_y == static_state_ptr->num_cells_along_y_axis())
                    break;

                bits_of_coords = cellconnect::find_bits_of_source_coords_of_synapse(
                                        dynamic_state_ptr,
                                        static_state_ptr,
                                        shifted_x,
                                        shifted_y,
                                        target_kind,
                                        source_kind,
                                        shift_to_synapse
                                        );

                if (cellconnect::read_x_coord_from_bits_of_coordinates(bits_of_coords) == current_x &&
                    cellconnect::read_y_coord_from_bits_of_coordinates(bits_of_coords) == current_y )
                    break;

                INVARIANT(
                        cellconnect::read_x_coord_from_bits_of_coordinates(bits_of_coords) == shifted_x &&
                        cellconnect::read_y_coord_from_bits_of_coordinates(bits_of_coords) == shifted_y
                        );

                cellconnect::write_x_coord_to_bits_of_coordinates(current_x, bits_of_coords);
                cellconnect::write_y_coord_to_bits_of_coordinates(current_y, bits_of_coords);

                current_x = shifted_x;
                current_y = shifted_y;
            }
            while (current_x != x || current_y != y);
        }
}


static bool go_to_next_task(
        natural_32_bit&  row,
        natural_32_bit&  column,
        natural_32_bit&  index,
        natural_64_bit&  shift,
        natural_32_bit const  diameter_x,
        natural_32_bit const  diameter_y,
        std::vector<natural_32_bit> const&  matrix
        )
{
    do
    {
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
                    return false;
            }
        }
    }
    while (matrix.at(row * diameter_x + column) == 0U);

    ++shift;

    if (column == diameter_x / 2U && row == diameter_y / 2U)
    {
        INVARIANT(index == 0U);
        index = matrix.at(row * diameter_x + column) - 1U;
        shift += index;
        return go_to_next_task(row,column,index,shift,diameter_x,diameter_y,matrix);
    }

    return true;
}


}

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
    bool not_done = true;
    if (matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood.at(row * diameter_x + column) == 0U)
    {
        not_done = go_to_next_task(row,column,index,shift,
                                   diameter_x,diameter_y,
                                   matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood
                                   );
        INVARIANT(shift > 0ULL);
        --shift;
    }

    do
    {
        std::vector<std::thread> threads;
        for (natural_32_bit i = 1U; i < num_threads_avalilable_for_computation && not_done; ++i)
        {
            threads.push_back(
                        std::thread(
                            &cellconnect::thread_spread_synapses,
                            dynamic_state_ptr,
                            static_state_ptr,
                            kind_of_target_cells_of_synapses,
                            kind_of_source_cells_of_synapses,
                            (integer_64_bit)column - (integer_64_bit)(diameter_x / 2U),
                            (integer_64_bit)row - (integer_64_bit)(diameter_y / 2U),
                            shift
                            )
                        );

            not_done = cellconnect::go_to_next_task(
                            row,column,index,shift,
                            diameter_x,diameter_y,
                            matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood
                            );
        }
        if (not_done)
        {
            cellconnect::thread_spread_synapses(
                    dynamic_state_ptr,
                    static_state_ptr,
                    kind_of_target_cells_of_synapses,
                    kind_of_source_cells_of_synapses,
                    (integer_64_bit)column - (integer_64_bit)(diameter_x / 2U),
                    (integer_64_bit)row - (integer_64_bit)(diameter_y / 2U),
                    shift
                    );
            not_done = cellconnect::go_to_next_task(
                            row,column,index,shift,
                            diameter_x,diameter_y,
                            matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood
                            );
        }
        for (std::thread& thread : threads)
            thread.join();
    }
    while (not_done);
    INVARIANT(index == 0U && column == 0U && row == diameter_y);
}



}
