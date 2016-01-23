#include <cellconnect/check_for_network_properties.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <thread>
#include <mutex>

namespace cellconnect { namespace {

natural_64_bit  thread_compute_in_degree_of_tissue_cell_using_territorial_states_of_all_synapses(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        cellab::kind_of_cell const  kind_of_cell,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit  c_coord
        )
{
//    for (natural_32_bit  i = 0U; i < static_state_ptr->num_synapses_in_territory_of_cell_kind(kind_of_cell); ++i)
//    {

//    }

    return 0ULL;
}

natural_64_bit  thread_compute_in_degree_of_tissue_cell_using_delimiters_lists(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit  c_coord
        )
{
    return 0ULL;
}

void thread_compute_in_degrees_of_tissue_cells_of_given_kind(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        bool const  ignore_delimiters_lists_and_check_territorial_states_of_all_synapses,
        natural_32_bit const  num_rows_in_output_distribution_matrix,
        natural_32_bit const  num_columns_in_output_distribution_matrix,
        natural_32_bit const  minimal_difference_between_two_in_degrees_to_be_considered_different,
        std::vector< std::unique_ptr<std::mutex> > const&  mutexes_to_elements_of_output_matrix,
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
            natural_64_bit const  in_degree = ignore_delimiters_lists_and_check_territorial_states_of_all_synapses ?
                    thread_compute_in_degree_of_tissue_cell_using_territorial_states_of_all_synapses(
                            dynamic_state_ptr,
                            static_state_ptr,
                            x_coord,
                            y_coord,
                            start_columnar_coord + i,
                            kind_of_cells_to_be_considered
                            ) :
                    thread_compute_in_degree_of_tissue_cell_using_delimiters_lists(
                            dynamic_state_ptr,
                            static_state_ptr,
                            x_coord,
                            y_coord,
                            start_columnar_coord + i
                            );



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


void  compute_in_degrees_of_tissue_cells_of_given_kind(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        bool const  ignore_delimiters_lists_and_check_territorial_states_of_all_synapses,
        natural_32_bit const  num_rows_in_output_distribution_matrix,
        natural_32_bit const  num_columns_in_output_distribution_matrix,
        natural_32_bit const  minimal_difference_between_two_in_degrees_to_be_considered_different,
        natural_32_bit const  num_threads_avalilable_for_computation,
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&  output_matrix_with_distribution_of_in_degrees
        )
{
    TMPROF_BLOCK();

    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

    std::vector< std::unique_ptr<std::mutex> >  mutexes_to_elements_of_output_matrix(
                num_rows_in_output_distribution_matrix * num_columns_in_output_distribution_matrix);
    {
        INVARIANT(mutexes_to_elements_of_output_matrix.size() ==
                  num_rows_in_output_distribution_matrix * num_columns_in_output_distribution_matrix);
        for (natural_64_bit  i = 0ULL; i < mutexes_to_elements_of_output_matrix.size(); ++i)
            mutexes_to_elements_of_output_matrix.at(i) = std::move(std::unique_ptr<std::mutex>(new std::mutex));
    }

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
                        &cellconnect::thread_compute_in_degrees_of_tissue_cells_of_given_kind,
                        dynamic_state_ptr,
                        static_state_ptr,
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation,
                        kind_of_cells_to_be_considered,
                        ignore_delimiters_lists_and_check_territorial_states_of_all_synapses,
                        num_rows_in_output_distribution_matrix,
                        num_columns_in_output_distribution_matrix,
                        minimal_difference_between_two_in_degrees_to_be_considered_different,
                        std::cref(mutexes_to_elements_of_output_matrix),
                        std::ref(output_matrix_with_distribution_of_in_degrees)
                        )
                    );
    }

    cellconnect::thread_compute_in_degrees_of_tissue_cells_of_given_kind(
            dynamic_state_ptr,
            static_state_ptr,
            0U,0U,
            num_threads_avalilable_for_computation,
            kind_of_cells_to_be_considered,
            ignore_delimiters_lists_and_check_territorial_states_of_all_synapses,
            num_rows_in_output_distribution_matrix,
            num_columns_in_output_distribution_matrix,
            minimal_difference_between_two_in_degrees_to_be_considered_different,
            mutexes_to_elements_of_output_matrix,
            output_matrix_with_distribution_of_in_degrees
            );

    for(std::thread& thread : threads)
        thread.join();
}


}
