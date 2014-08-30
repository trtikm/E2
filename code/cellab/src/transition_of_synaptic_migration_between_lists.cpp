#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <memory>
#include <vector>
#include <thread>

namespace cellab {



typedef natural_16_bit kind_of_cell;

struct tissue_coordinates
{
    tissue_coordinates(natural_32_bit const coord_along_x_axis,
                       natural_32_bit const coord_along_y_axis,
                       natural_32_bit const coord_along_columnar_axis);
    natural_32_bit get_coord_along_x_axis() const;
    natural_32_bit get_coord_along_y_axis() const;
    natural_32_bit get_coord_along_columnar_axis() const;
private:
    natural_32_bit m_coord_along_x_axis;
    natural_32_bit m_coord_along_y_axis;
    natural_32_bit m_coord_along_columnar_axis;
};

struct shift_in_coordinates
{
    shift_in_coordinates(integer_8_bit const shift_along_x_axis,
                         integer_8_bit const shift_along_y_axis,
                         integer_8_bit const shift_along_columnar_axis);
    integer_8_bit get_shift_along_x_axis() const;
    integer_8_bit get_shift_along_y_axis() const;
    integer_8_bit get_shift_along_columnar_axis() const;
private:
    integer_8_bit m_shift_along_x_axis;
    integer_8_bit m_shift_along_y_axis;
    integer_8_bit m_shift_along_columnar_axis;
};

struct spatial_neighbourhood;

enum migration_attempt_of_synapse
{
    STAY_IN_TERRITORY_PROVIDE_SIGNAL_DELIVERY_TO_CELL_OF_THIS_TERRITORY = 0,
    MOVE_ALONG_POSITIVE_X_AXIS = 1,
    MOVE_ALONG_NEGATIVE_X_AXIS = 2,
    MOVE_ALONG_POSITIVE_Y_AXIS = 3,
    MOVE_ALONG_NEGATIVE_Y_AXIS = 4,
    MOVE_ALONG_POSITIVE_COLUMNAR_AXIS = 5,
    MOVE_ALONG_NEGATIVE_COLUMNAR_AXIS = 6
};

bool go_to_next_coordinates(natural_32_bit& x_coord, natural_32_bit& y_coord, natural_32_bit& c_coord,
                            natural_32_bit const extent,
                            natural_32_bit const num_tissue_cells_along_x_axis,
                            natural_32_bit const num_tissue_cells_along_y_axis,
                            natural_32_bit const num_tissue_cells_along_columnar_axis);

integer_8_bit clip_shift(integer_8_bit const shift, natural_32_bit const origin,
                         natural_32_bit const length_of_axis, bool const is_it_torus_axis);



static void exchange_synapses_in_lists_in_territories_of_cells_at_given_coordinates(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        tissue_coordinates const& coordinates_of_first_cell,
        natural_8_bit const list_index_in_first_cell,
        tissue_coordinates const& coordinates_of_second_cell,
        natural_8_bit const list_index_in_second_cell
        )
{
    natural_32_bit const size_of_first_list =
            dynamic_state_of_tissue->get_size_of_migration_list(
                    coordinates_of_first_cell.get_coord_along_x_axis(),
                    coordinates_of_first_cell.get_coord_along_y_axis(),
                    coordinates_of_first_cell.get_coord_along_columnar_axis(),
                    list_index_in_first_cell
                    );
    natural_32_bit const size_of_second_list =
            dynamic_state_of_tissue->get_size_of_migration_list(
                    coordinates_of_second_cell.get_coord_along_x_axis(),
                    coordinates_of_second_cell.get_coord_along_y_axis(),
                    coordinates_of_second_cell.get_coord_along_columnar_axis(),
                    list_index_in_second_cell
                    );

    tissue_coordinates const* first_cell_coordinates;
    natural_32_bit index_of_first_synapse_in_first_list;
    natural_32_bit end_index_of_first_list;
    tissue_coordinates const* second_cell_coordinates;
    natural_32_bit index_of_first_synapse_in_second_list;
    natural_32_bit num_synapses_to_be_exchanged;
    {
        if (size_of_first_list >= size_of_second_list)
        {
            first_cell_coordinates = &coordinates_of_first_cell;
            index_of_first_synapse_in_first_list =
                    dynamic_state_of_tissue->get_index_of_first_synapse_in_migration_list(
                            coordinates_of_first_cell.get_coord_along_x_axis(),
                            coordinates_of_first_cell.get_coord_along_y_axis(),
                            coordinates_of_first_cell.get_coord_along_columnar_axis(),
                            list_index_in_first_cell
                            );
            end_index_of_first_list = index_of_first_synapse_in_first_list + size_of_first_list;
            second_cell_coordinates = &coordinates_of_second_cell;
            index_of_first_synapse_in_second_list =
                    dynamic_state_of_tissue->get_index_of_first_synapse_in_migration_list(
                            coordinates_of_second_cell.get_coord_along_x_axis(),
                            coordinates_of_second_cell.get_coord_along_y_axis(),
                            coordinates_of_second_cell.get_coord_along_columnar_axis(),
                            list_index_in_second_cell
                            );
            num_synapses_to_be_exchanged = size_of_second_list;
        }
        else
        {
            first_cell_coordinates = &coordinates_of_second_cell;
            index_of_first_synapse_in_first_list =
                    dynamic_state_of_tissue->get_index_of_first_synapse_in_migration_list(
                            coordinates_of_second_cell.get_coord_along_x_axis(),
                            coordinates_of_second_cell.get_coord_along_y_axis(),
                            coordinates_of_second_cell.get_coord_along_columnar_axis(),
                            list_index_in_second_cell
                            );
            end_index_of_first_list = index_of_first_synapse_in_second_list + size_of_second_list;
            second_cell_coordinates = &coordinates_of_first_cell;
            index_of_first_synapse_in_second_list =
                    dynamic_state_of_tissue->get_index_of_first_synapse_in_migration_list(
                            coordinates_of_first_cell.get_coord_along_x_axis(),
                            coordinates_of_first_cell.get_coord_along_y_axis(),
                            coordinates_of_first_cell.get_coord_along_columnar_axis(),
                            list_index_in_first_cell
                            );
            num_synapses_to_be_exchanged = size_of_first_list;
        }

    }

    natural_32_bit index_of_current_synapse_in_first_list =
            get_random_natural_32_bit_from_range(
                index_of_first_synapse_in_first_list,
                end_index_of_first_list
                );
    for (natural_32_bit i = 0U; i < num_synapses_to_be_exchanged; ++i)
    {
        {
            bits_reference const arg1_bits_of_coords_of_source_cell =
                dynamic_state_of_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                        first_cell_coordinates->get_coord_along_x_axis(),
                        first_cell_coordinates->get_coord_along_y_axis(),
                        first_cell_coordinates->get_coord_along_columnar_axis(),
                        index_of_current_synapse_in_first_list
                        );
            bits_reference const arg2_bits_of_coords_of_source_cell =
                dynamic_state_of_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                        second_cell_coordinates->get_coord_along_x_axis(),
                        second_cell_coordinates->get_coord_along_y_axis(),
                        second_cell_coordinates->get_coord_along_columnar_axis(),
                        index_of_first_synapse_in_second_list + i
                        );
            swap_referenced_bits( arg1_bits_of_coords_of_source_cell, arg2_bits_of_coords_of_source_cell );
        }
        {
            bits_reference const arg1_bits_of_migration =
                dynamic_state_of_tissue->find_bits_of_migration_of_synapse_in_tissue(
                        first_cell_coordinates->get_coord_along_x_axis(),
                        first_cell_coordinates->get_coord_along_y_axis(),
                        first_cell_coordinates->get_coord_along_columnar_axis(),
                        index_of_current_synapse_in_first_list
                        );
            bits_reference const arg2_bits_of_migration =
                dynamic_state_of_tissue->find_bits_of_migration_of_synapse_in_tissue(
                        second_cell_coordinates->get_coord_along_x_axis(),
                        second_cell_coordinates->get_coord_along_y_axis(),
                        second_cell_coordinates->get_coord_along_columnar_axis(),
                        index_of_first_synapse_in_second_list + i
                        );
            swap_referenced_bits( arg1_bits_of_migration, arg2_bits_of_migration );
        }
        {
            bits_reference arg1_bits_of_synapse =
                dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(
                        first_cell_coordinates->get_coord_along_x_axis(),
                        first_cell_coordinates->get_coord_along_y_axis(),
                        first_cell_coordinates->get_coord_along_columnar_axis(),
                        index_of_current_synapse_in_first_list
                        );
            bits_reference arg2_bits_of_synapse =
                dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(
                        second_cell_coordinates->get_coord_along_x_axis(),
                        second_cell_coordinates->get_coord_along_y_axis(),
                        second_cell_coordinates->get_coord_along_columnar_axis(),
                        index_of_first_synapse_in_second_list + i
                        );
            swap_referenced_bits( arg1_bits_of_synapse, arg2_bits_of_synapse );
        }

        ++index_of_current_synapse_in_first_list;
        if (index_of_current_synapse_in_first_list == end_index_of_first_list)
            index_of_current_synapse_in_first_list = index_of_first_synapse_in_first_list;
    }
}

static void thread_exchange_synapses_in_lists_in_territories_of_all_cells(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_8_bit const list_index_in_pivot_cell,
        shift_in_coordinates const& shift,
        natural_8_bit const list_index_in_shift_cell,
        natural_32_bit x_coord,
        natural_32_bit y_coord,
        natural_32_bit c_coord,
        natural_32_bit const extent_in_coordinates
        )
{
    do
    {
        exchange_synapses_in_lists_in_territories_of_cells_at_given_coordinates(
                    dynamic_state_of_tissue,
                    static_state_of_tissue,
                    tissue_coordinates(x_coord,y_coord,c_coord),
                    list_index_in_pivot_cell,
                    tissue_coordinates(
                        clip_shift(shift.get_shift_along_x_axis(),x_coord,
                                   static_state_of_tissue->num_tissue_cells_along_x_axis(),
                                   static_state_of_tissue->is_x_axis_torus_axis()),
                        clip_shift(shift.get_shift_along_y_axis(),y_coord,
                                   static_state_of_tissue->num_tissue_cells_along_y_axis(),
                                   static_state_of_tissue->is_y_axis_torus_axis()),
                        clip_shift(shift.get_shift_along_columnar_axis(),c_coord,
                                   static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                                   static_state_of_tissue->is_columnar_axis_torus_axis()),
                        ),
                    list_index_in_shift_cell
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


}

namespace cellab { namespace private_interal_implementation_details {


void exchange_synapses_in_lists_in_territories_of_all_cells(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_8_bit const list_index_in_pivot_cell,
        shift_in_coordinates const& shift,
        natural_8_bit const list_index_in_shift_cell,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
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
                        &cellab::thread_exchange_synapses_in_lists_in_territories_of_all_cells,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        list_index_in_pivot_cell,
                        shift,
                        list_index_in_shift_cell,
                        x_coord,y_coord,c_coord,
                        num_avalilable_thread_for_creation_and_use
                        )
                    );
    }

    for(std::thread& thread : threads)
        thread.join();
}


}}
