#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/territorial_state_of_synapse.hpp>
#include <cellab/shift_in_coordinates.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <tuple>
#include <thread>

namespace cellab {


static void thread_apply_transition_of_synapses_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        std::function<
            territorial_state_of_synapse(
            bits_reference& bits_of_synapse_to_be_updated,
            kind_of_cell kind_of_source_cell,
            bits_const_reference const& bits_of_source_cell,
            kind_of_cell kind_of_territory_cell,
            bits_const_reference const& bits_of_territory_cell,
            territorial_state_of_synapse current_territorial_state_of_synapse,
            shift_in_coordinates const& shift_to_low_corner,
            shift_in_coordinates const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)>
                get_signalling
            )> single_threaded_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue,
        natural_32_bit x_coord,
        natural_32_bit y_coord,
        natural_32_bit c_coord,
        natural_32_bit const extent_in_coordinates
        )
{
    do
    {
        bits_const_reference const bits_of_territory_cell =
            dynamic_state_of_tissue->find_bits_of_cell_in_tissue(x_coord,y_coord,c_coord);

        natural_16_bit const kind_of_territory_cell =
            static_state_of_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c_coord);

        tissue_coordinates const territory_cell_coordinates(x_coord,y_coord,c_coord);

        shift_in_coordinates shift_to_low_corner(
            clip_shift(-static_state_of_tissue->get_x_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_y_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_columnar_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        shift_in_coordinates shift_to_high_corner(
            clip_shift(static_state_of_tissue->get_x_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_y_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_columnar_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        spatial_neighbourhood const synapse_neighbourhood(
                    territory_cell_coordinates, shift_to_low_corner, shift_to_high_corner
                    );

        for (natural_32_bit synapse_index = 0U;
             synapse_index < static_state_of_tissue->num_synapses_in_territory_of_cell_kind(kind_of_territory_cell);
             ++synapse_index)
        {
            bits_reference bits_of_synapse =
                dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(x_coord,y_coord,c_coord,synapse_index);

            tissue_coordinates const source_cell_coords =
                get_coordinates_of_source_cell_of_synapse_in_tissue(
                        dynamic_state_of_tissue,
                        territory_cell_coordinates,
                        synapse_index
                        );

            natural_16_bit const kind_of_source_cell =
                static_state_of_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                        source_cell_coords.get_coord_along_columnar_axis()
                        );

            bits_const_reference const bits_of_source_cell =
                    dynamic_state_of_tissue->find_bits_of_cell_in_tissue(
                        source_cell_coords.get_coord_along_x_axis(),
                        source_cell_coords.get_coord_along_y_axis(),
                        source_cell_coords.get_coord_along_columnar_axis()
                        );

            bits_reference bits_of_territorial_state_of_synapse =
                    dynamic_state_of_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(
                        x_coord,y_coord,c_coord,
                        synapse_index
                        );
            natural_32_bit const current_territorial_state_of_synapse =
                    bits_to_value<natural_32_bit>(bits_of_territorial_state_of_synapse);
            INVARIANT( current_territorial_state_of_synapse < 7U );

            territorial_state_of_synapse const new_territorial_state_of_synapse =
                single_threaded_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue(
                            bits_of_synapse,
                            kind_of_source_cell, bits_of_source_cell,
                            kind_of_territory_cell, bits_of_territory_cell,
                            static_cast<territorial_state_of_synapse>(current_territorial_state_of_synapse),
                            shift_to_low_corner,
                            shift_to_high_corner,
                            std::bind(&cellab::get_signalling_callback_function,dynamic_state_of_tissue,
                                      static_state_of_tissue,std::cref(synapse_neighbourhood),
                                      std::placeholders::_1)
                            );

            natural_32_bit const territorial_state_value =
                    static_cast<natural_32_bit>(new_territorial_state_of_synapse);
            ASSUMPTION( territorial_state_value < 7U );
            value_to_bits( territorial_state_value, bits_of_territorial_state_of_synapse );
        }
    }
    while (go_to_next_coordinates(
                    x_coord,y_coord,c_coord,
                    extent_in_coordinates,
                    static_state_of_tissue->num_cells_along_x_axis(),
                    static_state_of_tissue->num_cells_along_y_axis(),
                    static_state_of_tissue->num_cells_along_columnar_axis()
                    ));
}

void apply_transition_of_synapses_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::function<
            territorial_state_of_synapse(
            bits_reference& bits_of_synapse_to_be_updated,
            kind_of_cell kind_of_source_cell,
            bits_const_reference const& bits_of_source_cell,
            kind_of_cell kind_of_territory_cell,
            bits_const_reference const& bits_of_territory_cell,
            territorial_state_of_synapse current_migration_attempt,
            shift_in_coordinates const& shift_to_low_corner,
            shift_in_coordinates const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)>
                get_signalling
            )> single_threaded_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    std::vector<std::thread> threads;
    for (natural_32_bit i = 0; i < num_avalilable_thread_for_creation_and_use; ++i)
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
                        &cellab::thread_apply_transition_of_synapses_of_tissue,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        single_threaded_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue,
                        x_coord,y_coord,c_coord,
                        num_avalilable_thread_for_creation_and_use
                        )
                    );
    }

    for(std::thread& thread : threads)
        thread.join();
}


}
