#include <cellconnect/fill_coords_of_source_cells_of_synapses_in_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <thread>

namespace cellconnect { namespace {


void  thread_fill_coords_of_source_cells_of_synapses_in_tissue_in_columns(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        std::vector<natural_32_bit> const&  matrix,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates
        )
{
    TMPROF_BLOCK();

if (matrix.size() == 42U)
{
    int iii = 0;
}

    do
    {
        cellab::kind_of_cell  i = 0U;
        natural_32_bit k = 0U;
        natural_32_bit t = 0U;
        do
        {
            for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_tissue_cells(); ++j)
                for (natural_32_bit l = 0U; l < static_state_ptr->num_tissue_cells_of_cell_kind(j); ++l)
                    for (natural_32_bit r = 0U; r < matrix.at(i * static_state_ptr->num_kinds_of_cells() + j); ++r)
                    {
                        INVARIANT(i < static_state_ptr->num_kinds_of_tissue_cells());

                        bits_reference  coords_bits =
                            dynamic_state_ptr->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                                        x_coord,
                                        y_coord,
                                        static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(i) + k,
                                        t
                                        );
                        cellab::tissue_coordinates const coords(
                                    x_coord,
                                    y_coord,
                                    static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(j) + l
                                    );
                        cellab::write_tissue_coordinates_to_bits_of_coordinates(coords,coords_bits);

                        ++t;
                        if (t == static_state_ptr->num_synapses_in_territory_of_cell_kind(i))
                        {
                            t = 0U;
                            ++k;
                            if (k == static_state_ptr->num_tissue_cells_of_cell_kind(i))
                            {
                                INVARIANT(
                                        [](cellab::kind_of_cell const  min_sensory_kind,
                                           natural_16_bit const  num_cell_kinds,
                                           natural_16_bit const  shift_to_row,
                                           std::vector<natural_32_bit> const&  matrix)
                                        {
                                            for (cellab::kind_of_cell j = min_sensory_kind; j < num_cell_kinds; ++j)
                                                if (matrix.at(shift_to_row + j) != 0U)
                                                    return false;
                                            return true;
                                        }(static_state_ptr->lowest_kind_of_sensory_cells(),
                                          static_state_ptr->num_kinds_of_cells(),
                                          i * static_state_ptr->num_kinds_of_cells(),
                                          matrix)
                                        );
                                k = 0U;
                                ++i;
                            }
                        }
                    }

            for (cellab::kind_of_cell j = static_state_ptr->lowest_kind_of_sensory_cells(); j < static_state_ptr->num_kinds_of_cells(); ++j)
                for (natural_32_bit l = 0U; l < static_state_ptr->num_sensory_cells_of_cell_kind(j); ++l)
                    for (natural_32_bit r = 0U; r < matrix.at(i * static_state_ptr->num_kinds_of_cells() + j); ++r)
                    {
                        INVARIANT(i < static_state_ptr->num_kinds_of_tissue_cells());

                        bits_reference  coords_bits =
                            dynamic_state_ptr->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                                        x_coord,
                                        y_coord,
                                        static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(i) + k,
                                        t
                                        );
                        cellab::tissue_coordinates const coords(
                                    x_coord,
                                    y_coord,
                                    static_state_ptr->num_cells_along_columnar_axis() +
                                        static_state_ptr->compute_index_of_first_sensory_cell_of_kind(j) +
                                        l
                                    );
                        cellab::write_tissue_coordinates_to_bits_of_coordinates(coords,coords_bits);

                        ++t;
                        if (t == static_state_ptr->num_synapses_in_territory_of_cell_kind(i))
                        {
                            t = 0U;
                            ++k;
                            if (k == static_state_ptr->num_tissue_cells_of_cell_kind(i))
                            {
                                k = 0U;
                                ++i;
                                if (i == static_state_ptr->num_kinds_of_tissue_cells())
                                {
                                    INVARIANT(l + 1U == static_state_ptr->num_sensory_cells_of_cell_kind(j));
                                    INVARIANT(j + 1U == static_state_ptr->num_kinds_of_cells());
                                    break;
                                }
                            }
                        }
                    }

            INVARIANT(k == 0U && t == 0U);
        }
        while (i < static_state_ptr->num_kinds_of_tissue_cells());
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


void  fill_coords_of_source_cells_of_synapses_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector<natural_32_bit> const&  matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    TMPROF_BLOCK();

    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

    ASSUMPTION(matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds.size() ==
               (std::size_t)(static_state_ptr->num_kinds_of_tissue_cells() * static_state_ptr->num_kinds_of_cells()));
    ASSUMPTION(
            [](std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
               std::vector<natural_32_bit> const&  matrix)
            {
                for (cellab::kind_of_cell i = 0U; i < static_state_ptr->num_kinds_of_tissue_cells(); ++i)
                {
                    natural_64_bit sum = 0ULL;
                    for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_cells(); ++j)
                        sum += (natural_64_bit)matrix.at(i * static_state_ptr->num_kinds_of_cells() + j) *
                               (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j);
                    natural_64_bit correct_sum =
                            (natural_64_bit)static_state_ptr->num_synapses_in_territory_of_cell_kind(i) *
                            (natural_64_bit)static_state_ptr->num_tissue_cells_of_cell_kind(i);
                    if (sum != correct_sum)
                        return false;
                }
                return true;
            }(static_state_ptr,matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds)
            );
    ASSUMPTION(num_threads_avalilable_for_computation > 0U);

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
                        &cellconnect::thread_fill_coords_of_source_cells_of_synapses_in_tissue_in_columns,
                        dynamic_state_ptr,
                        static_state_ptr,
                        std::cref(matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds),
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation
                        )
                    );
    }

    thread_fill_coords_of_source_cells_of_synapses_in_tissue_in_columns(
            dynamic_state_ptr,
            static_state_ptr,
            matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds,
            0U,0U,
            num_threads_avalilable_for_computation
            );

    for(std::thread& thread : threads)
        thread.join();
}


}
