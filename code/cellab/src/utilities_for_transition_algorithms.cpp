#include <cellab/utilities_for_transition_algorithms.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

#include <utility/development.hpp>

namespace cellab {


tissue_coordinates::tissue_coordinates(
        natural_32_bit const coord_along_x_axis,
        natural_32_bit const coord_along_y_axis,
        natural_32_bit const coord_along_columnar_axis
        )
    : m_coord_along_x_axis(coord_along_x_axis)
    , m_coord_along_y_axis(coord_along_y_axis)
    , m_coord_along_columnar_axis(coord_along_columnar_axis)
{}

natural_32_bit  tissue_coordinates::get_coord_along_x_axis() const
{
    return m_coord_along_x_axis;
}

natural_32_bit  tissue_coordinates::get_coord_along_y_axis() const
{
    return m_coord_along_y_axis;
}

natural_32_bit  tissue_coordinates::get_coord_along_columnar_axis() const
{
    return m_coord_along_columnar_axis;
}


spatial_neighbourhood::spatial_neighbourhood(
        tissue_coordinates const& center,
        shift_in_coordinates const& shift_to_low_corner,
        shift_in_coordinates const& shift_to_high_corner
        )
    : m_center_of_neighbourhood(center)
    , m_shift_to_low_corner(shift_to_low_corner)
    , m_shift_to_high_corner(shift_to_high_corner)
{
    ASSUMPTION(m_shift_to_low_corner.get_shift_along_x_axis() <= 0);
    ASSUMPTION(m_shift_to_low_corner.get_shift_along_y_axis() <= 0);
    ASSUMPTION(m_shift_to_low_corner.get_shift_along_columnar_axis() <= 0);
    ASSUMPTION(m_shift_to_high_corner.get_shift_along_x_axis() >= 0);
    ASSUMPTION(m_shift_to_high_corner.get_shift_along_y_axis() >= 0);
    ASSUMPTION(m_shift_to_high_corner.get_shift_along_columnar_axis() >= 0);
}

tissue_coordinates const& spatial_neighbourhood::get_center_of_neighbourhood() const
{
    return m_center_of_neighbourhood;
}

shift_in_coordinates const& spatial_neighbourhood::get_shift_to_low_corner() const
{
    return m_shift_to_low_corner;
}

shift_in_coordinates const& spatial_neighbourhood::get_shift_to_high_corner() const
{
    return m_shift_to_high_corner;
}

natural_32_bit  shift_coordinate_in_torus_axis(
        integer_64_bit coordinate,
        integer_64_bit const shift,
        natural_64_bit const length_of_axis
        )
{
    ASSUMPTION(coordinate >= 0 && natural_64_bit(coordinate) < length_of_axis);
    ASSUMPTION(natural_64_bit(shift < 0 ? -shift : shift) <= length_of_axis);
    ASSUMPTION(length_of_axis > 0U);
    integer_64_bit const result_coord = (length_of_axis + coordinate + shift) % length_of_axis;
    return static_cast<natural_32_bit>(result_coord);
}

natural_32_bit  shift_coordinate(
        natural_32_bit const coord,
        integer_64_bit const shift,
        natural_32_bit const length_of_axis,
        bool is_totus_axis
        )
{
    if (is_totus_axis)
        return shift_coordinate_in_torus_axis(coord,shift,length_of_axis);
    integer_64_bit const  result = (integer_64_bit)coord + shift;
    if (result < 0LL || result >= (integer_64_bit)length_of_axis)
        return length_of_axis;
    return (natural_32_bit)result;
}

bool operator==(tissue_coordinates const& left, tissue_coordinates const& right)
{
    return left.get_coord_along_x_axis() == right.get_coord_along_x_axis() &&
           left.get_coord_along_y_axis() == right.get_coord_along_y_axis() &&
           left.get_coord_along_columnar_axis() == right.get_coord_along_columnar_axis() ;
}

tissue_coordinates  shift_coordinates(
        tissue_coordinates const& coords,
        shift_in_coordinates const& shift,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        natural_32_bit const num_cells_along_columnar_axis
        )
{
    return tissue_coordinates(
                shift_coordinate_in_torus_axis(
                        coords.get_coord_along_x_axis(),
                        shift.get_shift_along_x_axis(),
                        num_cells_along_x_axis
                        ),
                shift_coordinate_in_torus_axis(
                        coords.get_coord_along_y_axis(),
                        shift.get_shift_along_y_axis(),
                        num_cells_along_y_axis
                        ),
                shift_coordinate_in_torus_axis(
                        coords.get_coord_along_columnar_axis(),
                        shift.get_shift_along_columnar_axis(),
                        num_cells_along_columnar_axis
                        )
                );
}

static natural_64_bit  go_to_next_value_modulo_range(
        natural_64_bit const current_value,
        natural_64_bit const range,
        natural_64_bit& extent
        )
{
    natural_64_bit const next_value = (current_value + extent) % range;
    extent = (current_value + extent) / range;
    return next_value;
}

bool  go_to_next_coordinates(
        natural_32_bit& x_coord, natural_32_bit& y_coord, natural_32_bit& c_coord,
        natural_32_bit const extent,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        natural_32_bit const num_cells_along_columnar_axis
        )
{
    natural_64_bit extent_64_bit = extent;
    c_coord = (natural_32_bit)go_to_next_value_modulo_range(c_coord,num_cells_along_columnar_axis,extent_64_bit);
    x_coord = (natural_32_bit)go_to_next_value_modulo_range(x_coord,num_cells_along_x_axis,extent_64_bit);
    y_coord = (natural_32_bit)go_to_next_value_modulo_range(y_coord,num_cells_along_y_axis,extent_64_bit);
    INVARIANT(extent_64_bit != 0ULL || (x_coord < num_cells_along_x_axis &&
                                        y_coord < num_cells_along_y_axis &&
                                        c_coord < num_cells_along_columnar_axis));
    return extent_64_bit == 0ULL;
}

bool  go_to_next_column(
        natural_32_bit& x_coord, natural_32_bit& y_coord,
        natural_32_bit const extent,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis
        )
{
    natural_64_bit extent_64_bit = extent;
    x_coord = (natural_32_bit)go_to_next_value_modulo_range(x_coord,num_cells_along_x_axis,extent_64_bit);
    y_coord = (natural_32_bit)go_to_next_value_modulo_range(y_coord,num_cells_along_y_axis,extent_64_bit);
    INVARIANT(extent_64_bit != 0ULL || (x_coord < num_cells_along_x_axis &&
                                        y_coord < num_cells_along_y_axis));
    return extent_64_bit == 0ULL;
}

bool  go_to_next_index(
        natural_32_bit& index,
        natural_32_bit const extent,
        natural_32_bit const size
        )
{
    natural_64_bit extent_64_bit = extent;
    index = (natural_32_bit)go_to_next_value_modulo_range(index,size,extent_64_bit);
    INVARIANT(extent_64_bit != 0ULL || index < size);
    return extent_64_bit == 0ULL;
}

integer_64_bit  clip_shift(
        integer_64_bit const shift,
        natural_32_bit const origin,
        natural_32_bit const length_of_axis
        )
{
    integer_64_bit const origin64 = origin;
    integer_64_bit const destination = origin64 + shift;
    integer_64_bit const length64 = length_of_axis;

    if (destination < 0ULL)
        return -origin64;

    if (destination >= length64)
        return (length64 - 1LL) - origin64;

    return shift;
}

integer_8_bit  clip_shift(
        integer_8_bit const shift,
        natural_32_bit const origin,
        natural_32_bit const length_of_axis,
        bool const is_it_torus_axis
        )
{
    if (is_it_torus_axis)
        return shift;
    return (natural_8_bit)clip_shift((integer_64_bit)shift,origin,length_of_axis);
}

void  write_tissue_coordinates_to_bits_of_coordinates(
        tissue_coordinates const& coords,
        bits_reference& bits_ref)
{
    natural_16_bit const num_bits = bits_ref.num_bits() / natural_16_bit(3U);
    INVARIANT(num_bits * natural_16_bit(3U) == bits_ref.num_bits());

    value_to_bits(coords.get_coord_along_x_axis(),bits_ref,0U,(natural_8_bit)num_bits);
    value_to_bits(coords.get_coord_along_y_axis(),bits_ref,(natural_8_bit)num_bits,(natural_8_bit)num_bits);
    value_to_bits(coords.get_coord_along_columnar_axis(),bits_ref,(natural_8_bit)(num_bits + num_bits),(natural_8_bit)num_bits);
}

tissue_coordinates  convert_bits_of_coordinates_to_tissue_coordinates(bits_reference const& bits_ref)
{
    natural_16_bit const num_bits = bits_ref.num_bits() / natural_16_bit(3U);
    INVARIANT(num_bits * natural_16_bit(3U) == bits_ref.num_bits());

    natural_32_bit coord_along_x_axis;
    bits_to_value(bits_ref,0U,(natural_8_bit)num_bits,coord_along_x_axis);

    natural_32_bit coord_along_y_axis;
    bits_to_value(bits_ref,(natural_8_bit)num_bits,(natural_8_bit)num_bits,coord_along_y_axis);

    natural_32_bit coord_along_columnar_axis;
    bits_to_value(bits_ref,(natural_8_bit)(num_bits + num_bits),(natural_8_bit)num_bits,coord_along_columnar_axis);

    return tissue_coordinates(coord_along_x_axis,coord_along_y_axis,coord_along_columnar_axis);
}

tissue_coordinates  get_coordinates_of_source_cell_of_synapse_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& coords_of_territorial_cell_of_synapse,
        natural_32_bit const index_of_synapse_in_territory
        )
{
    tissue_coordinates const  source_cell_coordinates =
        convert_bits_of_coordinates_to_tissue_coordinates(
                dynamic_state_of_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                            coords_of_territorial_cell_of_synapse.get_coord_along_x_axis(),
                            coords_of_territorial_cell_of_synapse.get_coord_along_y_axis(),
                            coords_of_territorial_cell_of_synapse.get_coord_along_columnar_axis(),
                            index_of_synapse_in_territory
                            )
                );
    INVARIANT(source_cell_coordinates.get_coord_along_x_axis() <
              dynamic_state_of_tissue->get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    INVARIANT(source_cell_coordinates.get_coord_along_y_axis() <
              dynamic_state_of_tissue->get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    INVARIANT(source_cell_coordinates.get_coord_along_columnar_axis() <
              dynamic_state_of_tissue->get_static_state_of_neural_tissue()->num_cells_along_columnar_axis() +
              dynamic_state_of_tissue->get_static_state_of_neural_tissue()->num_sensory_cells());

    return source_cell_coordinates;
}

tissue_coordinates  get_coordinates_of_source_cell_of_synapse_to_muscle(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const index_of_synapse_to_muscle
        )
{
    tissue_coordinates const  source_cell_coordinates =
        convert_bits_of_coordinates_to_tissue_coordinates(
                dynamic_state_of_tissue->find_bits_of_coords_of_source_cell_of_synapse_to_muscle(
                        index_of_synapse_to_muscle
                        )
                );
    INVARIANT(source_cell_coordinates.get_coord_along_x_axis() <
              dynamic_state_of_tissue->get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    INVARIANT(source_cell_coordinates.get_coord_along_y_axis() <
              dynamic_state_of_tissue->get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    INVARIANT(source_cell_coordinates.get_coord_along_columnar_axis() <
              dynamic_state_of_tissue->get_static_state_of_neural_tissue()->num_cells_along_columnar_axis() +
              dynamic_state_of_tissue->get_static_state_of_neural_tissue()->num_sensory_cells());

    return source_cell_coordinates;
}

natural_32_bit  get_begin_index_of_territorial_list_of_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& coordinates_of_cell,
        natural_8_bit const index_of_territorial_list
        )
{
    ASSUMPTION(index_of_territorial_list < 7U);

    if (index_of_territorial_list == 0U)
        return 0U;
    return bits_to_value<natural_32_bit>(
                dynamic_state_of_tissue->find_bits_of_delimiter_between_territorial_lists(
                        coordinates_of_cell.get_coord_along_x_axis(),
                        coordinates_of_cell.get_coord_along_y_axis(),
                        coordinates_of_cell.get_coord_along_columnar_axis(),
                        index_of_territorial_list - 1U
                        )
                );
}

natural_32_bit  get_end_index_of_territorial_list_of_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        tissue_coordinates const& coordinates_of_cell,
        natural_8_bit const index_of_territorial_list
        )
{
    ASSUMPTION(index_of_territorial_list < 7U);
    ASSUMPTION(coordinates_of_cell.get_coord_along_columnar_axis() < static_state_of_tissue->num_cells_along_columnar_axis());

    if (index_of_territorial_list == 6U)
        return static_state_of_tissue->num_synapses_in_territory_of_cell_kind(
                    static_state_of_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                            coordinates_of_cell.get_coord_along_columnar_axis()
                            )
                    );
    return bits_to_value<natural_32_bit>(
                dynamic_state_of_tissue->find_bits_of_delimiter_between_territorial_lists(
                        coordinates_of_cell.get_coord_along_x_axis(),
                        coordinates_of_cell.get_coord_along_y_axis(),
                        coordinates_of_cell.get_coord_along_columnar_axis(),
                        index_of_territorial_list
                        )
                );
}

void  swap_all_data_of_two_synapses(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& first_cell_coordinates,
        natural_32_bit const synapse_index_in_first_territory,
        tissue_coordinates const& second_cell_coordinates,
        natural_32_bit const synapse_index_in_second_territory
        )
{
    {
        bits_reference  arg1_bits_of_coords_of_source_cell =
            dynamic_state_of_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                    first_cell_coordinates.get_coord_along_x_axis(),
                    first_cell_coordinates.get_coord_along_y_axis(),
                    first_cell_coordinates.get_coord_along_columnar_axis(),
                    synapse_index_in_first_territory
                    );
        bits_reference  arg2_bits_of_coords_of_source_cell =
            dynamic_state_of_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                    second_cell_coordinates.get_coord_along_x_axis(),
                    second_cell_coordinates.get_coord_along_y_axis(),
                    second_cell_coordinates.get_coord_along_columnar_axis(),
                    synapse_index_in_second_territory
                    );
        swap_referenced_bits( arg1_bits_of_coords_of_source_cell, arg2_bits_of_coords_of_source_cell );
    }
    {
        bits_reference  arg1_bits_of_migration =
            dynamic_state_of_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(
                    first_cell_coordinates.get_coord_along_x_axis(),
                    first_cell_coordinates.get_coord_along_y_axis(),
                    first_cell_coordinates.get_coord_along_columnar_axis(),
                    synapse_index_in_first_territory
                    );
        bits_reference  arg2_bits_of_migration =
            dynamic_state_of_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(
                    second_cell_coordinates.get_coord_along_x_axis(),
                    second_cell_coordinates.get_coord_along_y_axis(),
                    second_cell_coordinates.get_coord_along_columnar_axis(),
                    synapse_index_in_second_territory
                    );
        swap_referenced_bits( arg1_bits_of_migration, arg2_bits_of_migration );
    }
    {
        bits_reference arg1_bits_of_synapse =
            dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(
                    first_cell_coordinates.get_coord_along_x_axis(),
                    first_cell_coordinates.get_coord_along_y_axis(),
                    first_cell_coordinates.get_coord_along_columnar_axis(),
                    synapse_index_in_first_territory
                    );
        bits_reference arg2_bits_of_synapse =
            dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(
                    second_cell_coordinates.get_coord_along_x_axis(),
                    second_cell_coordinates.get_coord_along_y_axis(),
                    second_cell_coordinates.get_coord_along_columnar_axis(),
                    synapse_index_in_second_territory
                    );
        swap_referenced_bits( arg1_bits_of_synapse, arg2_bits_of_synapse );
    }
}

std::pair<bits_const_reference,kind_of_cell>  get_signalling_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_coordinates const& shift
        )
{
    ASSUMPTION(shift.get_shift_along_x_axis() >=
               neighbourhood.get_shift_to_low_corner().get_shift_along_x_axis());
    ASSUMPTION(shift.get_shift_along_x_axis() <=
               neighbourhood.get_shift_to_high_corner().get_shift_along_x_axis());
    ASSUMPTION(shift.get_shift_along_y_axis() >=
               neighbourhood.get_shift_to_low_corner().get_shift_along_y_axis());
    ASSUMPTION(shift.get_shift_along_y_axis() <=
               neighbourhood.get_shift_to_high_corner().get_shift_along_y_axis());
    ASSUMPTION(shift.get_shift_along_columnar_axis() >=
               neighbourhood.get_shift_to_low_corner().get_shift_along_columnar_axis());
    ASSUMPTION(shift.get_shift_along_columnar_axis() <=
               neighbourhood.get_shift_to_high_corner().get_shift_along_columnar_axis());

    tissue_coordinates cell_coords =
            shift_coordinates(
                    neighbourhood.get_center_of_neighbourhood(),
                    shift,
                    static_state_of_tissue->num_cells_along_x_axis(),
                    static_state_of_tissue->num_cells_along_y_axis(),
                    static_state_of_tissue->num_cells_along_columnar_axis()
                    );
    ASSUMPTION(cell_coords.get_coord_along_columnar_axis() < static_state_of_tissue->num_cells_along_columnar_axis());
    return std::make_pair(
                bits_const_reference(
                        dynamic_state_of_tissue->find_bits_of_signalling(
                                cell_coords.get_coord_along_x_axis(),
                                cell_coords.get_coord_along_y_axis(),
                                cell_coords.get_coord_along_columnar_axis()
                                )
                        ),
                static_state_of_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                        cell_coords.get_coord_along_columnar_axis()
                        )
                );
}

std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>  get_synapse_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        tissue_coordinates const& target_cell,
        kind_of_cell const kind_of_target_cell,
        natural_32_bit const number_of_synapses_in_range,
        natural_32_bit const shift_to_start_index,
        natural_32_bit const shift_from_start_index
        )
{
    ASSUMPTION(shift_from_start_index < number_of_synapses_in_range);
    return std::make_tuple(
                bits_const_reference(
                    dynamic_state_of_tissue->find_bits_of_synapse_in_tissue(
                                target_cell.get_coord_along_x_axis(),
                                target_cell.get_coord_along_y_axis(),
                                target_cell.get_coord_along_columnar_axis(),
                                shift_to_start_index + shift_from_start_index
                                )
                    ),
                static_state_of_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                    get_coordinates_of_source_cell_of_synapse_in_tissue(
                            dynamic_state_of_tissue,
                            target_cell,
                            shift_to_start_index + shift_from_start_index
                            ).get_coord_along_columnar_axis()
                    ),
                kind_of_target_cell
                );
}

std::pair<bits_const_reference,kind_of_cell>  get_cell_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_coordinates const& shift
        )
{
    ASSUMPTION(shift.get_shift_along_x_axis() >=
               neighbourhood.get_shift_to_low_corner().get_shift_along_x_axis());
    ASSUMPTION(shift.get_shift_along_x_axis() <=
               neighbourhood.get_shift_to_high_corner().get_shift_along_x_axis());
    ASSUMPTION(shift.get_shift_along_y_axis() >=
               neighbourhood.get_shift_to_low_corner().get_shift_along_y_axis());
    ASSUMPTION(shift.get_shift_along_y_axis() <=
               neighbourhood.get_shift_to_high_corner().get_shift_along_y_axis());
    ASSUMPTION(shift.get_shift_along_columnar_axis() >=
               neighbourhood.get_shift_to_low_corner().get_shift_along_columnar_axis());
    ASSUMPTION(shift.get_shift_along_columnar_axis() <=
               neighbourhood.get_shift_to_high_corner().get_shift_along_columnar_axis());

    tissue_coordinates cell_coords =
            shift_coordinates(
                    neighbourhood.get_center_of_neighbourhood(),
                    shift,
                    static_state_of_tissue->num_cells_along_x_axis(),
                    static_state_of_tissue->num_cells_along_y_axis(),
                    static_state_of_tissue->num_cells_along_columnar_axis()
                    );
    ASSUMPTION(cell_coords.get_coord_along_columnar_axis() < static_state_of_tissue->num_cells_along_columnar_axis());

    return std::make_pair(
                bits_const_reference(
                        dynamic_state_of_tissue->find_bits_of_cell_in_tissue(
                                cell_coords.get_coord_along_x_axis(),
                                cell_coords.get_coord_along_y_axis(),
                                cell_coords.get_coord_along_columnar_axis()
                                )
                        ),
                static_state_of_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                        cell_coords.get_coord_along_columnar_axis()
                        )
                );
}


}
