#include <cellconnect/check_for_network_properties.hpp>
#include <cellconnect/fill_delimiters_between_territorial_lists.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <cellab/territorial_state_of_synapse.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <thread>
#include <mutex>

namespace cellconnect { namespace {


natural_32_bit*  find_counter_of_cell_at_coordinates(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit  c_coord
        )
{
    return reinterpret_cast<natural_32_bit*>(
            dynamic_state_ptr->find_bits_of_delimiter_between_territorial_lists(
                    x_coord,
                    y_coord,
                    c_coord,
                    0U
                    ).first_byte_ptr()
            );
}

void  thread_clear_counters_inside_memory_of_delimiters(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered
        )
{
    TMPROF_BLOCK();

    natural_32_bit const  start_columnar_coord =
            static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(kind_of_cells_to_be_considered);

    do
    {
        for (natural_32_bit  i = 0U; i < static_state_ptr->num_cells_of_cell_kind(kind_of_cells_to_be_considered); ++i)
        {
            natural_32_bit* const  counter_ptr =
                    find_counter_of_cell_at_coordinates(dynamic_state_ptr,x_coord,y_coord,start_columnar_coord + i);

            *counter_ptr = 0U;
        }
    }
    while (cellab::go_to_next_column(
                    x_coord,y_coord,
                    extent_in_coordinates,
                    static_state_ptr->num_cells_along_x_axis(),
                    static_state_ptr->num_cells_along_y_axis()
                    ));
}

void  clear_counters_inside_memory_of_delimiters(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    TMPROF_BLOCK();

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
                        &cellconnect::thread_clear_counters_inside_memory_of_delimiters,
                        dynamic_state_ptr,
                        static_state_ptr,
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation,
                        kind_of_cells_to_be_considered
                        )
                    );
    }

    cellconnect::thread_clear_counters_inside_memory_of_delimiters(
            dynamic_state_ptr,
            static_state_ptr,
            0U,0U,
            num_threads_avalilable_for_computation,
            kind_of_cells_to_be_considered
            );

    for(std::thread& thread : threads)
        thread.join();
}

void  thread_compute_counters_inside_memory_of_delimiters(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        cellab::territorial_state_of_synapse const  territorial_state_to_be_considered,
        std::mutex&  mutex_to_counters
        )
{
    TMPROF_BLOCK();

    natural_32_bit const  begin_columnar_coord =
            static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(kind_of_cells_to_be_considered);
    natural_32_bit const  end_columnar_coord =
            begin_columnar_coord + static_state_ptr->num_tissue_cells_of_cell_kind(kind_of_cells_to_be_considered);

    do
    {
        for (cellab::kind_of_cell i = 0U; i < static_state_ptr->num_kinds_of_tissue_cells(); ++i)
            for (natural_32_bit j = 0U; j < static_state_ptr->num_tissue_cells_of_cell_kind(i); ++j)
            {
                natural_32_bit const  c_coord =
                    static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(i) + j;

                for (natural_32_bit  synapse_index = 0U;
                     synapse_index < static_state_ptr->num_synapses_in_territory_of_cell_kind(i);
                     ++synapse_index)
                {
                    bits_reference bits_of_territorial_state_of_synapse =
                            dynamic_state_ptr->find_bits_of_territorial_state_of_synapse_in_tissue(
                                x_coord,y_coord,c_coord,
                                synapse_index
                                );
                    natural_32_bit const territorial_state_of_synapse =
                            bits_to_value<natural_32_bit>(bits_of_territorial_state_of_synapse);
                    INVARIANT( territorial_state_of_synapse < 7U );

                    if (territorial_state_of_synapse != territorial_state_to_be_considered)
                        continue;

                    cellab::tissue_coordinates const  source_coords =
                            cellab::get_coordinates_of_source_cell_of_synapse_in_tissue(
                                    dynamic_state_ptr,
                                    cellab::tissue_coordinates{x_coord,y_coord,c_coord},
                                    synapse_index
                                    );

                    if (source_coords.get_coord_along_columnar_axis() < begin_columnar_coord ||
                        source_coords.get_coord_along_columnar_axis() >= end_columnar_coord)
                        continue;

                    natural_32_bit* const  counter_ptr =
                            find_counter_of_cell_at_coordinates(
                                    dynamic_state_ptr,
                                    source_coords.get_coord_along_x_axis(),
                                    source_coords.get_coord_along_y_axis(),
                                    source_coords.get_coord_along_columnar_axis()
                                    );

                    {
                        std::lock_guard<std::mutex>  lock(mutex_to_counters);
                        ++*counter_ptr;
                    }
                }
            }
    }
    while (cellab::go_to_next_column(
                    x_coord,y_coord,
                    extent_in_coordinates,
                    static_state_ptr->num_cells_along_x_axis(),
                    static_state_ptr->num_cells_along_y_axis()
                    ));
}

void  compute_counters_inside_memory_of_delimiters(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        cellab::territorial_state_of_synapse const  territorial_state_to_be_considered,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    TMPROF_BLOCK();

    std::mutex  mutex_to_counters;
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
                        &cellconnect::thread_compute_counters_inside_memory_of_delimiters,
                        dynamic_state_ptr,
                        static_state_ptr,
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation,
                        kind_of_cells_to_be_considered,
                        territorial_state_to_be_considered,
                        std::ref(mutex_to_counters)
                        )
                    );
    }

    cellconnect::thread_compute_counters_inside_memory_of_delimiters(
            dynamic_state_ptr,
            static_state_ptr,
            0U,0U,
            num_threads_avalilable_for_computation,
            kind_of_cells_to_be_considered,
            territorial_state_to_be_considered,
            mutex_to_counters
            );

    for(std::thread& thread : threads)
        thread.join();

}

void thread_build_output_based_on_counters_inside_memory_of_delimiters(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        natural_32_bit const  num_rows_in_output_distribution_matrix,
        natural_32_bit const  num_columns_in_output_distribution_matrix,
        std::mutex&  mutex_to_output,
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&  output_matrix_with_distribution_of_in_degrees
        )
{
    TMPROF_BLOCK();

    natural_32_bit const  start_columnar_coord =
            static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(kind_of_cells_to_be_considered);

    do
    {
        for (natural_32_bit  i = 0U; i < static_state_ptr->num_cells_of_cell_kind(kind_of_cells_to_be_considered); ++i)
        {
            natural_32_bit* const  counter_ptr =
                    find_counter_of_cell_at_coordinates(dynamic_state_ptr,x_coord,y_coord,start_columnar_coord + i);

            natural_32_bit const  out_degree = *counter_ptr;

            natural_32_bit const row =
                    (num_rows_in_output_distribution_matrix * x_coord) / static_state_ptr->num_cells_along_x_axis();
            INVARIANT(row < num_rows_in_output_distribution_matrix);
            natural_32_bit const column =
                    (num_columns_in_output_distribution_matrix * y_coord) / static_state_ptr->num_cells_along_y_axis();
            INVARIANT(column < num_columns_in_output_distribution_matrix);
            natural_64_bit const  index =
                    row * num_columns_in_output_distribution_matrix + column;
            INVARIANT(index < output_matrix_with_distribution_of_in_degrees.size());

            std::unordered_map<natural_32_bit,natural_64_bit>& target_map =
                    output_matrix_with_distribution_of_in_degrees.at(index);

            {
                std::lock_guard<std::mutex>  lock(mutex_to_output);

                std::unordered_map<natural_32_bit,natural_64_bit>::iterator const  it = target_map.find(out_degree);
                if (it == target_map.end())
                    target_map.insert({out_degree,1ULL});
                else
                    ++it->second;
            }
        }
    }
    while (cellab::go_to_next_column(
                    x_coord,y_coord,
                    extent_in_coordinates,
                    static_state_ptr->num_cells_along_x_axis(),
                    static_state_ptr->num_cells_along_y_axis()
                    ));
}

void  build_output_based_on_counters_inside_memory_of_delimiters(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        natural_32_bit const  num_rows_in_output_distribution_matrix,
        natural_32_bit const  num_columns_in_output_distribution_matrix,
        natural_32_bit const  num_threads_avalilable_for_computation,
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&  output_matrix_with_distribution_of_in_degrees
        )
{
    TMPROF_BLOCK();

    std::mutex  mutex_to_output;

    output_matrix_with_distribution_of_in_degrees.resize(
                num_rows_in_output_distribution_matrix * num_columns_in_output_distribution_matrix
                );

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
                        &cellconnect::thread_build_output_based_on_counters_inside_memory_of_delimiters,
                        dynamic_state_ptr,
                        static_state_ptr,
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation,
                        kind_of_cells_to_be_considered,
                        num_rows_in_output_distribution_matrix,
                        num_columns_in_output_distribution_matrix,
                        std::ref(mutex_to_output),
                        std::ref(output_matrix_with_distribution_of_in_degrees)
                        )
                    );
    }

    cellconnect::thread_build_output_based_on_counters_inside_memory_of_delimiters(
            dynamic_state_ptr,
            static_state_ptr,
            0U,0U,
            num_threads_avalilable_for_computation,
            kind_of_cells_to_be_considered,
            num_rows_in_output_distribution_matrix,
            num_columns_in_output_distribution_matrix,
            mutex_to_output,
            output_matrix_with_distribution_of_in_degrees
            );

    for(std::thread& thread : threads)
        thread.join();
}


}}

namespace cellconnect {


void  compute_out_degrees_of_tissue_cells_of_given_kind(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        natural_32_bit const  num_rows_in_output_distribution_matrix,
        natural_32_bit const  num_columns_in_output_distribution_matrix,
        natural_32_bit const  num_threads_avalilable_for_computation,
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&  output_matrix_with_distribution_of_in_degrees,
        cellab::territorial_state_of_synapse const  territorial_state_to_be_considered
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(dynamic_state_ptr.operator bool());

    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

    ASSUMPTION(static_state_ptr.operator bool());
    ASSUMPTION(kind_of_cells_to_be_considered < static_state_ptr->num_kinds_of_tissue_cells());
    ASSUMPTION(num_rows_in_output_distribution_matrix <= static_state_ptr->num_cells_along_x_axis());
    ASSUMPTION(num_columns_in_output_distribution_matrix  <= static_state_ptr->num_cells_along_y_axis());
    ASSUMPTION((natural_64_bit)num_rows_in_output_distribution_matrix *
               (natural_64_bit)static_state_ptr->num_cells_along_x_axis() <=
               std::numeric_limits<natural_32_bit>::max());
    ASSUMPTION((natural_64_bit)num_columns_in_output_distribution_matrix *
               (natural_64_bit)static_state_ptr->num_cells_along_y_axis() <=
               std::numeric_limits<natural_32_bit>::max());
    ASSUMPTION((natural_64_bit)num_rows_in_output_distribution_matrix *
               (natural_64_bit)num_columns_in_output_distribution_matrix <
               std::numeric_limits<natural_32_bit>::max());
    ASSUMPTION(output_matrix_with_distribution_of_in_degrees.empty() ||
               output_matrix_with_distribution_of_in_degrees.size() ==
                        num_rows_in_output_distribution_matrix * num_columns_in_output_distribution_matrix);
    ASSUMPTION(dynamic_state_ptr->num_bits_per_delimiter_number(kind_of_cells_to_be_considered) *
               cellab::num_delimiters() >= 32);

    clear_counters_inside_memory_of_delimiters(
                dynamic_state_ptr,
                static_state_ptr,
                kind_of_cells_to_be_considered,
                num_threads_avalilable_for_computation
                );

    compute_counters_inside_memory_of_delimiters(
                dynamic_state_ptr,
                static_state_ptr,
                kind_of_cells_to_be_considered,
                territorial_state_to_be_considered,
                num_threads_avalilable_for_computation
                );

    build_output_based_on_counters_inside_memory_of_delimiters(
                dynamic_state_ptr,
                static_state_ptr,
                kind_of_cells_to_be_considered,
                num_rows_in_output_distribution_matrix,
                num_columns_in_output_distribution_matrix,
                num_threads_avalilable_for_computation,
                output_matrix_with_distribution_of_in_degrees
                );

    fill_delimiters_between_territorial_lists_for_cell_kind(
                dynamic_state_ptr,
                delimiters_fill_kind::SYNAPSES_DISTRIBUTED_REGULARLY,
                kind_of_cells_to_be_considered,
                num_threads_avalilable_for_computation
                );
}


}
