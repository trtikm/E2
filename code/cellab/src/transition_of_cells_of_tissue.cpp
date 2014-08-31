#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/territorial_state_of_synapse.hpp>
#include <cellab/shift_in_coordinates.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <tuple>
#include <thread>

namespace cellab {


static void thread_apply_transition_of_cells_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        std::function<
            void(
            bits_reference& bits_of_cell_to_be_updated,
            kind_of_cell kind_of_cell_to_be_updated,
            natural_32_bit num_of_synapses_connected_to_the_cell,
            std::function<std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>(natural_32_bit)>
                get_connected_synapse_at_index,
            shift_in_coordinates const& shift_to_low_corner,
            shift_in_coordinates const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)>
                get_signalling
            )> single_threaded_transition_function_of_packed_dynamic_state_of_cell,
        natural_32_bit x_coord,
        natural_32_bit y_coord,
        natural_32_bit c_coord,
        natural_32_bit const extent_in_coordinates
        )
{
    do
    {
        bits_reference bits_of_cell =
            dynamic_state_of_tissue->find_bits_of_cell_in_tissue(x_coord,y_coord,c_coord);

        kind_of_cell const cell_kind =
            static_state_of_tissue->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(c_coord);

        tissue_coordinates const cell_coordinates(x_coord,y_coord,c_coord);

        shift_in_coordinates shift_to_low_corner(
            clip_shift(-static_state_of_tissue->get_x_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_tissue_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_y_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_tissue_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_columnar_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        shift_in_coordinates shift_to_high_corner(
            clip_shift(static_state_of_tissue->get_x_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_tissue_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_y_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_tissue_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_columnar_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        spatial_neighbourhood const cell_neighbourhood(cell_coordinates,shift_to_low_corner,shift_to_high_corner);

        natural_32_bit const list_index_of_connected_synapses =
                convert_territorial_state_of_synapse_to_territorial_list_index(
                        SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY
                        );
        natural_32_bit const begin_index_in_list_of_synapses =
                get_begin_index_of_territorial_list_of_cell(
                        dynamic_state_of_tissue,
                        cell_coordinates,
                        list_index_of_connected_synapses
                        );
        natural_32_bit const end_index_in_list_of_synapses =
                get_end_index_of_territorial_list_of_cell(
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        cell_coordinates,
                        list_index_of_connected_synapses
                        );
        natural_32_bit const number_of_connected_synapses =
                end_index_in_list_of_synapses - begin_index_in_list_of_synapses;

        single_threaded_transition_function_of_packed_dynamic_state_of_cell(
                    bits_of_cell,
                    cell_kind,
                    number_of_connected_synapses,
                    std::bind(&cellab::get_synapse_callback_function, dynamic_state_of_tissue,
                              static_state_of_tissue, std::cref(cell_coordinates), cell_kind,
                              number_of_connected_synapses, begin_index_in_list_of_synapses,
                              std::placeholders::_1),
                    shift_to_low_corner,
                    shift_to_high_corner,
                    std::bind(&cellab::get_signalling_callback_function, dynamic_state_of_tissue,
                              static_state_of_tissue, std::cref(cell_neighbourhood), std::placeholders::_1)
                    );
    }
    while (go_to_next_coordinates(
                    x_coord,y_coord,c_coord,
                    extent_in_coordinates,
                    static_state_of_tissue->num_tissue_cells_along_x_axis(),
                    static_state_of_tissue->num_tissue_cells_along_y_axis(),
                    static_state_of_tissue->num_tissue_cells_along_columnar_axis()
                    ));
}

void apply_transition_of_cells_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_cell_to_be_updated,
            kind_of_cell kind_of_cell_to_be_updated,
            natural_32_bit num_of_synapses_connected_to_the_cell,
            std::function<std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>(natural_32_bit)>
                get_connected_synapse_at_index,
            shift_in_coordinates const& shift_to_low_corner,
            shift_in_coordinates const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)>
                get_signalling
            )> single_threaded_transition_function_of_packed_dynamic_state_of_cell,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    std::vector<std::thread> threads;
    for (natural_32_bit i = 0U; i < num_avalilable_thread_for_creation_and_use; ++i)
    {
        natural_32_bit x_coord = 0U;
        natural_32_bit y_coord = 0U;
        natural_32_bit c_coord = 0U;
        if (!go_to_next_coordinates(
                    x_coord,y_coord,c_coord,
                    i,
                    static_state_of_tissue->num_tissue_cells_along_x_axis(),
                    static_state_of_tissue->num_tissue_cells_along_y_axis(),
                    static_state_of_tissue->num_tissue_cells_along_columnar_axis()
                    ))
            break;

        threads.push_back(
                    std::thread(
                        &cellab::thread_apply_transition_of_cells_of_tissue,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        single_threaded_transition_function_of_packed_dynamic_state_of_cell,
                        x_coord,y_coord,c_coord,
                        num_avalilable_thread_for_creation_and_use
                        )
                    );
    }

    for(std::thread& thread : threads)
        thread.join();
}


}
