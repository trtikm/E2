#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <tuple>
#include <thread>

#include <utility/development.hpp>

namespace cellab {

typedef natural_16_bit kind_of_cell;

struct tissue_coordinates;
struct shift_in_spatial_neighbourhood;
struct spatial_neighbourhood;

enum migration_attempt_of_synapse
{
    CONNECT_TO_TERRITORY_CELL = 0,
    DISCONNECT_FROM_TERRITORY_CELL = 1,
    MOVE_ALONG_POSITIVE_X_AXIS = 2,
    MOVE_ALONG_NEGATIVE_X_AXIS = 3,
    MOVE_ALONG_POSITIVE_Y_AXIS = 4,
    MOVE_ALONG_NEGATIVE_Y_AXIS = 5,
    MOVE_ALONG_POSITIVE_COLUMNAR_AXIS = 6,
    MOVE_ALONG_NEGATIVE_COLUMNAR_AXIS = 7
};


static std::pair<bits_const_reference,kind_of_cell> get_signalling(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_spatial_neighbourhood const& shift);


static void thread_apply_transition_of_synapses_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        std::function<
            migration_attempt_of_synapse(
            bits_reference& bits_of_synapse_to_be_updated,
            kind_of_cell kind_of_source_cell,
            bits_const_reference const& bits_of_source_cell,
            kind_of_cell kind_of_territory_cell,
            bits_const_reference const& bits_of_territory_cell,
            bool is_connected_to_territory_cell,
            migration_attempt_of_synapse current_migration_attempt,
            shift_in_spatial_neighbourhood const& shift_to_low_corner,
            shift_in_spatial_neighbourhood const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_spatial_neighbourhood const&)>
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
            static_state_of_tissue->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(c_coord);

        tissue_coordinates const territory_cell_coordinates(x_coord,y_coord,c_coord);

        shift_in_spatial_neighbourhood shift_to_low_corner(
            clip_shift(-static_state_of_tissue->get_x_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_tissue_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_y_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_tissue_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_columnar_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        shift_in_spatial_neighbourhood shift_to_high_corner(
            clip_shift(static_state_of_tissue->get_x_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_tissue_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_y_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_tissue_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_columnar_radius_of_signalling_neighbourhood_of_synapse(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        spatial_neighbourhood const synapse_neighbourhood(
                    territory_cell_coordinates,shift_to_low_corner,shift_to_high_corner
                    );

        natural_32_bit const number_of_connected_synapses =
            dynamic_state_of_tissue->get_num_of_synapses_connected_to_cell(x_coord,y_coord,c_coord);
        natural_32_bit const start_index_of_connected_synapses =
            dynamic_state_of_tissue->get_start_index_of_synapses_connected_to_cell(x_coord,y_coord,c_coord);

        for (natural_32_bit synapse_index = 0U;
             synapse_index < static_state_of_tissue->num_synapses_in_territory_of_cell(kind_of_territory_cell);
             ++synapse_index)
        {
            bits_reference bits_of_synapse =
                dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(x_coord,y_coord,c_coord,synapse_index);

            tissue_coordinates const source_cell_coords =
                dynamic_state_of_tissue->get_coordinates_of_source_cell_of_synapse(
                        x_coord,y_coord,c_coord,synapse_index
                        );

            natural_16_bit const kind_of_source_cell =
                static_state_of_tissue->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                        source_cell_coords.get_coord_along_columnar_axis()
                        );

            bits_const_reference const bits_of_source_cell =
                    dynamic_state_of_tissue->find_bits_of_cell_in_tissue(
                        source_cell_coords.get_coord_along_x_axis(),
                        source_cell_coords.get_coord_along_y_axis(),
                        source_cell_coords.get_coord_along_columnar_axis()
                        );

            bool const is_connected =
                    start_index_of_connected_synapses <= synapse_index &&
                    synapse_index < number_of_connected_synapses
                    ;

            bits_reference const bits_of_migration =
                dynamic_state_of_tissue->find_bits_of_migration_of_synapse_in_tissue(
                        x_coord,y_coord,c_coord,synapse_index
                        );
            natural_32_bit current_migration_attempt;
            bits_to_value(bits_of_migration,0uc,bits_of_migration.num_bits(),current_migration_attempt);
            INVARIANT(current_migration_attempt < (1U << bits_of_migration.num_bits()));

            migration_attempt_of_synapse const new_migration_attempt =
                single_threaded_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue(
                            bits_of_synapse,
                            kind_of_source_cell, bits_of_source_cell,
                            kind_of_territory_cell, bits_of_territory_cell,
                            is_connected,
                            static_cast<migration_attempt_of_synapse>(current_migration_attempt),
                            shift_to_low_corner,
                            shift_to_high_corner,
                            std::bind(&cellab::get_signalling,dynamic_state_of_tissue,
                                      std::cref(synapse_neighbourhood),std::placeholders::_1)
                            );

            natural_32_bit const migration_attempt = static_cast<natural_32_bit>(new_migration_attempt);
            ASSUMPTION(migration_attempt < (1U << bits_of_migration.num_bits()));
            value_to_bits(migration_attempt,bits_of_migration.num_bits(),bits_of_migration.num_bits(),0uc);
        }
    }
    while (seek_to_next_coordinates(
                    x_coord,y_coord,c_coord,
                    extent_in_coordinates,
                    static_state_of_tissue->num_tissue_cells_along_x_axis(),
                    static_state_of_tissue->num_tissue_cells_along_y_axis(),
                    static_state_of_tissue->num_tissue_cells_along_columnar_axis()
                    ));
}

void apply_transition_of_synapses_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            migration_attempt_of_synapse(
            bits_reference& bits_of_synapse_to_be_updated,
            kind_of_cell kind_of_source_cell,
            bits_const_reference const& bits_of_source_cell,
            kind_of_cell kind_of_territory_cell,
            bits_const_reference const& bits_of_territory_cell,
            bool is_connected_to_territory_cell,
            migration_attempt_of_synapse current_migration_attempt,
            shift_in_spatial_neighbourhood const& shift_to_low_corner,
            shift_in_spatial_neighbourhood const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_spatial_neighbourhood const&)>
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
        if (!seek_to_next_coordinates(
                    x_coord,y_coord,c_coord,
                    i,
                    static_state_of_tissue->num_tissue_cells_along_x_axis(),
                    static_state_of_tissue->num_tissue_cells_along_y_axis(),
                    static_state_of_tissue->num_tissue_cells_along_columnar_axis()
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
