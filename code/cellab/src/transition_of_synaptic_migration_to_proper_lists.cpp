#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <memory>
#include <vector>
#include <array>
#include <thread>

#include <utility/development.hpp>

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

enum territorial_state_of_synapse
{
    SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY = 0,
    MIGRATION_ALONG_POSITIVE_X_AXIS = 1,
    MIGRATION_ALONG_NEGATIVE_X_AXIS = 2,
    MIGRATION_ALONG_POSITIVE_Y_AXIS = 3,
    MIGRATION_ALONG_NEGATIVE_Y_AXIS = 4,
    MIGRATION_ALONG_POSITIVE_COLUMNAR_AXIS = 5,
    MIGRATION_ALONG_NEGATIVE_COLUMNAR_AXIS = 6
};

natural_32_bit convert_territorial_state_of_synapse_to_territorial_list_index(
        territorial_state_of_synapse const territorial_state)
{
    return territorial_state;
}


bool go_to_next_coordinates(natural_32_bit& x_coord, natural_32_bit& y_coord, natural_32_bit& c_coord,
                            natural_32_bit const extent,
                            natural_32_bit const num_tissue_cells_along_x_axis,
                            natural_32_bit const num_tissue_cells_along_y_axis,
                            natural_32_bit const num_tissue_cells_along_columnar_axis);

integer_8_bit clip_shift(integer_8_bit const shift, natural_32_bit const origin,
                         natural_32_bit const length_of_axis, bool const is_it_torus_axis);


static void swap_all_data_of_two_synapses(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        tissue_coordinates const& first_cell_coordinates,
        natural_32_bit const synapse_index_in_first_territory,
        tissue_coordinates const& second_cell_coordinates,
        natural_32_bit const synapse_index_in_second_territory
        )
{
    {
        bits_reference const arg1_bits_of_coords_of_source_cell =
            dynamic_state_of_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                    first_cell_coordinates->get_coord_along_x_axis(),
                    first_cell_coordinates->get_coord_along_y_axis(),
                    first_cell_coordinates->get_coord_along_columnar_axis(),
                    synapse_index_in_first_territory
                    );
        bits_reference const arg2_bits_of_coords_of_source_cell =
            dynamic_state_of_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                    second_cell_coordinates->get_coord_along_x_axis(),
                    second_cell_coordinates->get_coord_along_y_axis(),
                    second_cell_coordinates->get_coord_along_columnar_axis(),
                    synapse_index_in_second_territory
                    );
        swap_referenced_bits( arg1_bits_of_coords_of_source_cell, arg2_bits_of_coords_of_source_cell );
    }
    {
        bits_reference const arg1_bits_of_migration =
            dynamic_state_of_tissue->find_bits_of_migration_of_synapse_in_tissue(
                    first_cell_coordinates->get_coord_along_x_axis(),
                    first_cell_coordinates->get_coord_along_y_axis(),
                    first_cell_coordinates->get_coord_along_columnar_axis(),
                    synapse_index_in_first_territory
                    );
        bits_reference const arg2_bits_of_migration =
            dynamic_state_of_tissue->find_bits_of_migration_of_synapse_in_tissue(
                    second_cell_coordinates->get_coord_along_x_axis(),
                    second_cell_coordinates->get_coord_along_y_axis(),
                    second_cell_coordinates->get_coord_along_columnar_axis(),
                    synapse_index_in_second_territory
                    );
        swap_referenced_bits( arg1_bits_of_migration, arg2_bits_of_migration );
    }
    {
        bits_reference arg1_bits_of_synapse =
            dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(
                    first_cell_coordinates->get_coord_along_x_axis(),
                    first_cell_coordinates->get_coord_along_y_axis(),
                    first_cell_coordinates->get_coord_along_columnar_axis(),
                    synapse_index_in_first_territory
                    );
        bits_reference arg2_bits_of_synapse =
            dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(
                    second_cell_coordinates->get_coord_along_x_axis(),
                    second_cell_coordinates->get_coord_along_y_axis(),
                    second_cell_coordinates->get_coord_along_columnar_axis(),
                    synapse_index_in_second_territory
                    );
        swap_referenced_bits( arg1_bits_of_synapse, arg2_bits_of_synapse );
    }
}


}

namespace cellab {


static void move_all_synapse_data_from_source_list_to_target_list(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        tissue_coordinates const& cell_coordinates,
        natural_32_bit source_list_index,
        natural_32_bit const target_list_index,
        natural_32_bit index_into_source_list,
        std::array<natural_32_bit,8U>& boundaries_of_lists
        )
{
    ASSUMPTION(source_list_index < 7U);
    ASSUMPTION(target_list_index < 7U);
    ASSUMPTION(index_into_source_list >= boundaries_of_lists.at(source_list_index));
    ASSUMPTION(index_into_source_list < boundaries_of_lists.at(source_list_index + 1U));
    for ( ; source_list_index < target_list_index; ++source_list_index)
    {
        natural_32_bit const next_list = source_list_index + 1U;
        natural_32_bit const target_index = boundaries_of_lists.at(next_list) - 1U;
        swap_all_data_of_two_synapses(
                dynamic_state_of_tissue,
                cell_coordinates,
                index_into_source_list,
                cell_coordinates,
                target_index
                );
        index_into_source_list = target_index;
        boundaries_of_lists.at(next_list) = target_index;
    }
    for ( ; target_list_index < source_list_index; --source_list_index)
    {
        natural_32_bit const target_index = boundaries_of_lists.at(source_list_index);
        swap_all_data_of_two_synapses(
                dynamic_state_of_tissue,
                cell_coordinates,
                index_into_source_list,
                cell_coordinates,
                target_index
                );
        index_into_source_list = target_index;
        boundaries_of_lists.at(source_list_index) = target_index + 1U;
    }
}

static void move_synapses_into_proper_lists_in_territory_of_one_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_32_bit const x_coord,
        natural_32_bit const y_coord,
        natural_32_bit const c_coord
        )
{
    std::array<bits_reference,6U> bits_of_delimiters = {
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 0U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 1U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 2U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 3U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 4U
                    ),
            dynamic_state_of_tissue->find_bits_of_delimiter_between_teritorial_lists(
                    x_coord, y_coord, c_coord, 5U
                    ),
            };

    std::array<natural_32_bit,8U> boundaries_of_lists = {
            0U,
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(0U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(1U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(2U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(3U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(4U)),
            bits_to_value<natural_32_bit>(bits_of_delimiters.at(5U)),
            static_state_of_tissue->num_synapses_in_territory_of_cell(
                    static_state_of_tissue->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                            c_coord
                            )
                    )
            };

    for (natural_32_bit list_index = 0U; list_index < 7U; ++list_index)
        for (natural_32_bit index_into_list = boundaries_of_lists.at(list_index);
             index_into_list < boundaries_of_lists.at(list_index + 1U);
             )
        {
            natural_32_bit const target_list_index =
                convert_territorial_state_of_synapse_to_territorial_list_index(
                        bits_to_value<natural_32_bit>(
                                dynamic_state_of_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(
                                        x_coord, y_coord, c_coord,
                                        index_into_list
                                        )
                                )
                        );
            INVARIANT(target_list_index < 7U);
            if (target_list_index == list_index)
                ++index_into_list;
            else
            {
                move_all_synapse_data_from_source_list_to_target_list(
                        dynamic_state_of_tissue,
                        tissue_coordinates(x_coord, y_coord, c_coord),
                        list_index,
                        target_list_index,
                        index_into_list,
                        boundaries_of_lists
                        );
                if (index_into_list < boundaries_of_lists.at(list_index))
                {
                    ++index_into_list;
                    INVARIANT(index_into_list == boundaries_of_lists.at(list_index));
                }
            }
        }

    for (natural_32_bit list_index = 0U; list_index < 7U; ++list_index)
    {
        INVARIANT( boundaries_of_lists.at(list_index) <= boundaries_of_lists.at(list_index + 1U) );
        value_to_bits( boundaries_of_lists.at(list_index + 1U), bits_of_delimiters.at(list_index) );
    }
}

static void thread_move_synapses_into_proper_lists(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_32_bit x_coord,
        natural_32_bit y_coord,
        natural_32_bit c_coord,
        natural_32_bit const extent_in_coordinates
        )
{
    do
    {
        move_synapses_into_proper_lists_in_territory_of_one_cell(
                    dynamic_state_of_tissue,
                    static_state_of_tissue,
                    x_coord,y_coord,c_coord
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


void move_synapses_into_proper_lists_by_their_migration_attempts_in_territories_of_all_cells(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
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
                        &cellab::thread_move_synapses_into_proper_lists,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        x_coord,y_coord,c_coord,
                        num_avalilable_thread_for_creation_and_use
                        )
                    );
    }

    for(std::thread& thread : threads)
        thread.join();
}


}}
