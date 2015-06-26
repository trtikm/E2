#include <cellconnect/fill_territories.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <thread>

namespace cellconnect { namespace {


void  thread_fill_territories_in_columns(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        std::vector<natural_32_bit> const&  matrix,
        natural_32_bit  x_coord,
        natural_32_bit  y_coord,
        natural_32_bit const  extent_in_coordinates
        )
{
    do
    {
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


void  fill_territories(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector<natural_32_bit> const&  matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
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
                        &cellconnect::thread_fill_territories_in_columns,
                        dynamic_state_ptr,
                        static_state_ptr,
                        std::cref(matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds),
                        x_coord,y_coord,
                        num_threads_avalilable_for_computation
                        )
                    );
    }

    thread_fill_territories_in_columns(
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
