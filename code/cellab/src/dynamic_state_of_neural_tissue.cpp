#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/bit_count.hpp>
#include <utility/assumptions.hpp>
#include <utility/log.hpp>
#include <algorithm>


static natural_64_bit compute_columnar_index_of_synapse_in_territorial_of_cell(
        natural_64_bit const columnar_index_of_cell_in_slice_of_cell_kind,
        natural_64_bit const num_synapses_in_territory_of_cell_kind,
        natural_64_bit const index_of_synapse_in_territory_of_cell
        )
{
    return columnar_index_of_cell_in_slice_of_cell_kind * num_synapses_in_territory_of_cell_kind
           + index_of_synapse_in_territory_of_cell
           ;
}

namespace cellab {


dynamic_state_of_neural_tissue::dynamic_state_of_neural_tissue(
        std::shared_ptr<static_state_of_neural_tissue const> const pointer_to_static_state_of_neural_tissue
        )
    : m_static_state_of_neural_tissue(pointer_to_static_state_of_neural_tissue)
    , m_num_bits_per_source_cell_coordinate(
          compute_byte_aligned_num_of_bits_to_store_number(
              std::max(m_static_state_of_neural_tissue->num_cells_along_x_axis(),
                       std::max(m_static_state_of_neural_tissue->num_cells_along_y_axis(),
                                checked_add_32_bit(m_static_state_of_neural_tissue->num_cells_along_columnar_axis(),
                                                   m_static_state_of_neural_tissue->num_sensory_cells())
                                ))
              )
          )
    , m_num_bits_per_delimiter_number(m_static_state_of_neural_tissue->num_kinds_of_tissue_cells())
    , m_slices_of_cells(m_static_state_of_neural_tissue->num_kinds_of_tissue_cells())
    , m_slices_of_synapses(m_static_state_of_neural_tissue->num_kinds_of_tissue_cells())
    , m_slices_of_territorial_states_of_synapses(m_static_state_of_neural_tissue->num_kinds_of_tissue_cells())
    , m_slices_of_source_cell_coords_of_synapses(m_static_state_of_neural_tissue->num_kinds_of_tissue_cells())
    , m_slices_of_signalling_data(m_static_state_of_neural_tissue->num_kinds_of_tissue_cells())
    , m_slices_of_delimiters_between_territorial_lists(m_static_state_of_neural_tissue->num_kinds_of_tissue_cells())
    , m_bits_of_sensory_cells(m_static_state_of_neural_tissue->num_bits_per_cell(),
                              m_static_state_of_neural_tissue->num_sensory_cells())
    , m_bits_of_synapses_to_muscles(m_static_state_of_neural_tissue->num_bits_per_synapse(),
                                    m_static_state_of_neural_tissue->num_synapses_to_muscles())
    , m_bits_of_source_cell_coords_of_synapses_to_muscles(checked_mul_16_bit(3U,m_num_bits_per_source_cell_coordinate),
                                                          m_static_state_of_neural_tissue->num_synapses_to_muscles())
{
    for (kind_of_cell kind = 0U; kind < m_static_state_of_neural_tissue->num_kinds_of_tissue_cells(); ++kind)
    {
        m_num_bits_per_delimiter_number.at(kind) =
                compute_byte_aligned_num_of_bits_to_store_number(
                        m_static_state_of_neural_tissue->num_synapses_in_territory_of_cell_kind(kind)
                        );
        checked_add_8_bit(checked_mul_8_bit(num_delimiters()-1U,m_num_bits_per_delimiter_number.at(kind)),7U);

        m_slices_of_cells.at(kind) =
                pointer_to_homogenous_slice_of_tissue(
                    new homogenous_slice_of_tissue(
                        m_static_state_of_neural_tissue->num_bits_per_cell(),
                        m_static_state_of_neural_tissue->num_cells_along_x_axis(),
                        m_static_state_of_neural_tissue->num_cells_along_y_axis(),
                        m_static_state_of_neural_tissue->num_tissue_cells_of_cell_kind(kind)
                        )
                    );
        m_slices_of_synapses.at(kind) =
                pointer_to_homogenous_slice_of_tissue(
                    new homogenous_slice_of_tissue(
                        m_static_state_of_neural_tissue->num_bits_per_synapse(),
                        m_static_state_of_neural_tissue->num_cells_along_x_axis(),
                        m_static_state_of_neural_tissue->num_cells_along_y_axis(),
                        checked_mul_64_bit(
                            m_static_state_of_neural_tissue->num_tissue_cells_of_cell_kind(kind),
                            m_static_state_of_neural_tissue->num_synapses_in_territory_of_cell_kind(kind)
                            )
                        )
                    );
        m_slices_of_territorial_states_of_synapses.at(kind) =
                pointer_to_homogenous_slice_of_tissue(
                    new homogenous_slice_of_tissue(
                        num_of_bits_to_store_territorial_state_of_synapse(),
                        m_static_state_of_neural_tissue->num_cells_along_x_axis(),
                        m_static_state_of_neural_tissue->num_cells_along_y_axis(),
                        checked_mul_64_bit(
                            m_static_state_of_neural_tissue->num_tissue_cells_of_cell_kind(kind),
                            m_static_state_of_neural_tissue->num_synapses_in_territory_of_cell_kind(kind)
                            )
                        )
                    );
        m_slices_of_source_cell_coords_of_synapses.at(kind) =
                pointer_to_homogenous_slice_of_tissue(
                    new homogenous_slice_of_tissue(
                        checked_mul_16_bit(3U,m_num_bits_per_source_cell_coordinate),
                        m_static_state_of_neural_tissue->num_cells_along_x_axis(),
                        m_static_state_of_neural_tissue->num_cells_along_y_axis(),
                        checked_mul_64_bit(
                            m_static_state_of_neural_tissue->num_tissue_cells_of_cell_kind(kind),
                            m_static_state_of_neural_tissue->num_synapses_in_territory_of_cell_kind(kind)
                            )
                        )
                    );
        m_slices_of_signalling_data.at(kind) =
                pointer_to_homogenous_slice_of_tissue(
                    new homogenous_slice_of_tissue(
                        m_static_state_of_neural_tissue->num_bits_per_signalling(),
                        m_static_state_of_neural_tissue->num_cells_along_x_axis(),
                        m_static_state_of_neural_tissue->num_cells_along_y_axis(),
                        m_static_state_of_neural_tissue->num_tissue_cells_of_cell_kind(kind)
                        )
                    );
        m_slices_of_delimiters_between_territorial_lists.at(kind) =
                pointer_to_homogenous_slice_of_tissue(
                    new homogenous_slice_of_tissue(
                        checked_mul_16_bit(num_delimiters(),m_num_bits_per_delimiter_number.at(kind)),
                        m_static_state_of_neural_tissue->num_cells_along_x_axis(),
                        m_static_state_of_neural_tissue->num_cells_along_y_axis(),
                        m_static_state_of_neural_tissue->num_tissue_cells_of_cell_kind(kind)
                        )
                    );
    }

    LOG(debug,FUNCTION_PROTOTYPE());
}

dynamic_state_of_neural_tissue::~dynamic_state_of_neural_tissue()
{
    LOG(debug,FUNCTION_PROTOTYPE());
}

std::shared_ptr<static_state_of_neural_tissue const>
dynamic_state_of_neural_tissue::get_static_state_of_neural_tissue() const
{
    return m_static_state_of_neural_tissue;
}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_cell(
        natural_32_bit const coord_along_x_axis,
        natural_32_bit const coord_along_y_axis,
        kind_of_cell const cell_kind,
        natural_32_bit const relative_index_of_cell
        )
{
    ASSUMPTION(coord_along_x_axis < get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    ASSUMPTION(coord_along_y_axis < get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    ASSUMPTION(cell_kind < get_static_state_of_neural_tissue()->num_kinds_of_cells());
    return cell_kind < get_static_state_of_neural_tissue()->num_kinds_of_tissue_cells() ?
                m_slices_of_cells.at(cell_kind)->find_bits_of_unit(coord_along_x_axis,
                                                                   coord_along_y_axis,
                                                                   relative_index_of_cell)
                :
                find_bits_of_sensory_cell(
                    get_static_state_of_neural_tissue()->compute_index_of_first_sensory_cell_of_kind(cell_kind)
                    + relative_index_of_cell
                    )
                ;

}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_cell_in_tissue(
        natural_32_bit const coord_along_x_axis,
        natural_32_bit const coord_along_y_axis,
        natural_32_bit const coord_along_columnar_axis
        )
{
    ASSUMPTION(coord_along_x_axis < get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    ASSUMPTION(coord_along_y_axis < get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    ASSUMPTION(coord_along_columnar_axis < get_static_state_of_neural_tissue()->num_cells_along_columnar_axis());
    std::pair<kind_of_cell,natural_32_bit> const kind_and_index =
        get_static_state_of_neural_tissue()->
            compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
                coord_along_columnar_axis
                );
    return m_slices_of_cells.at(kind_and_index.first)->find_bits_of_unit(
                coord_along_x_axis,
                coord_along_y_axis,
                kind_and_index.second
                );
}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_synapse_in_tissue(
        natural_32_bit const coord_to_cell_along_x_axis,
        natural_32_bit const coord_to_cell_along_y_axis,
        natural_32_bit const coord_to_cell_along_columnar_axis,
        natural_32_bit const index_of_synapse_in_territory_of_cell
        )
{
    ASSUMPTION(coord_to_cell_along_x_axis < get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    ASSUMPTION(coord_to_cell_along_y_axis < get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    ASSUMPTION(coord_to_cell_along_columnar_axis < get_static_state_of_neural_tissue()->num_cells_along_columnar_axis());
    std::pair<kind_of_cell,natural_32_bit> const kind_and_index =
        get_static_state_of_neural_tissue()->
            compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
                coord_to_cell_along_columnar_axis
                );
    ASSUMPTION(index_of_synapse_in_territory_of_cell <
               get_static_state_of_neural_tissue()->num_synapses_in_territory_of_cell_kind(kind_and_index.first));
    return m_slices_of_synapses.at(kind_and_index.first)->find_bits_of_unit(
                coord_to_cell_along_x_axis,
                coord_to_cell_along_y_axis,
                compute_columnar_index_of_synapse_in_territorial_of_cell(
                    kind_and_index.second,
                    get_static_state_of_neural_tissue()->num_synapses_in_territory_of_cell_kind(
                        kind_and_index.first
                        ),
                    index_of_synapse_in_territory_of_cell
                    )
                );
}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_territorial_state_of_synapse_in_tissue(
        natural_32_bit const coord_to_cell_along_x_axis,
        natural_32_bit const coord_to_cell_along_y_axis,
        natural_32_bit const coord_to_cell_along_columnar_axis,
        natural_32_bit const index_of_synapse_in_territory_of_cell
        )
{
    ASSUMPTION(coord_to_cell_along_x_axis < get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    ASSUMPTION(coord_to_cell_along_y_axis < get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    ASSUMPTION(coord_to_cell_along_columnar_axis < get_static_state_of_neural_tissue()->num_cells_along_columnar_axis());
    std::pair<kind_of_cell,natural_32_bit> const kind_and_index =
        get_static_state_of_neural_tissue()->
            compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
                coord_to_cell_along_columnar_axis
                );
    ASSUMPTION(index_of_synapse_in_territory_of_cell <
               get_static_state_of_neural_tissue()->num_synapses_in_territory_of_cell_kind(kind_and_index.first));
    return m_slices_of_territorial_states_of_synapses.at(kind_and_index.first)->find_bits_of_unit(
                coord_to_cell_along_x_axis,
                coord_to_cell_along_y_axis,
                compute_columnar_index_of_synapse_in_territorial_of_cell(
                    kind_and_index.second,
                    get_static_state_of_neural_tissue()->num_synapses_in_territory_of_cell_kind(
                        kind_and_index.first
                        ),
                    index_of_synapse_in_territory_of_cell
                    )
                );
}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
        natural_32_bit const coord_to_cell_along_x_axis,
        natural_32_bit const coord_to_cell_along_y_axis,
        natural_32_bit const coord_to_cell_along_columnar_axis,
        natural_32_bit const index_of_synapse_in_territory_of_cell
        )
{
    ASSUMPTION(coord_to_cell_along_x_axis < get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    ASSUMPTION(coord_to_cell_along_y_axis < get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    ASSUMPTION(coord_to_cell_along_columnar_axis < get_static_state_of_neural_tissue()->num_cells_along_columnar_axis());
    std::pair<kind_of_cell,natural_32_bit> const kind_and_index =
        get_static_state_of_neural_tissue()->
            compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
                coord_to_cell_along_columnar_axis
                );
    ASSUMPTION(index_of_synapse_in_territory_of_cell <
               get_static_state_of_neural_tissue()->num_synapses_in_territory_of_cell_kind(kind_and_index.first));
    return m_slices_of_source_cell_coords_of_synapses.at(kind_and_index.first)->find_bits_of_unit(
                coord_to_cell_along_x_axis,
                coord_to_cell_along_y_axis,
                compute_columnar_index_of_synapse_in_territorial_of_cell(
                    kind_and_index.second,
                    get_static_state_of_neural_tissue()->num_synapses_in_territory_of_cell_kind(
                        kind_and_index.first
                        ),
                    index_of_synapse_in_territory_of_cell
                    )
                );
}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_signalling(
        natural_32_bit const coord_to_cell_along_x_axis,
        natural_32_bit const coord_to_cell_along_y_axis,
        natural_32_bit const coord_to_cell_along_columnar_axis
        )
{
    ASSUMPTION(coord_to_cell_along_x_axis < get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    ASSUMPTION(coord_to_cell_along_y_axis < get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    ASSUMPTION(coord_to_cell_along_columnar_axis < get_static_state_of_neural_tissue()->num_cells_along_columnar_axis());
    std::pair<kind_of_cell,natural_32_bit> const kind_and_index =
        get_static_state_of_neural_tissue()->
            compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
                coord_to_cell_along_columnar_axis
                );
    return m_slices_of_signalling_data.at(kind_and_index.first)->find_bits_of_unit(
                coord_to_cell_along_x_axis,
                coord_to_cell_along_y_axis,
                kind_and_index.second
                );
}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_delimiter_between_territorial_lists(
        natural_32_bit const coord_to_cell_along_x_axis,
        natural_32_bit const coord_to_cell_along_y_axis,
        natural_32_bit const coord_to_cell_along_columnar_axis,
        natural_8_bit const index_of_delimiter
        )
{
    ASSUMPTION(coord_to_cell_along_x_axis < get_static_state_of_neural_tissue()->num_cells_along_x_axis());
    ASSUMPTION(coord_to_cell_along_y_axis < get_static_state_of_neural_tissue()->num_cells_along_y_axis());
    ASSUMPTION(coord_to_cell_along_columnar_axis < get_static_state_of_neural_tissue()->num_cells_along_columnar_axis());
    ASSUMPTION(index_of_delimiter < num_delimiters());
    std::pair<kind_of_cell,natural_32_bit> const kind_and_index =
        get_static_state_of_neural_tissue()->
            compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
                coord_to_cell_along_columnar_axis
                );
    bits_reference bits_of_all_delimiters =
            m_slices_of_delimiters_between_territorial_lists.at(kind_and_index.first)->find_bits_of_unit(
                coord_to_cell_along_x_axis,
                coord_to_cell_along_y_axis,
                kind_and_index.second
                );
    natural_8_bit const shift =
            ( index_of_delimiter * m_num_bits_per_delimiter_number.at(kind_and_index.first) )
            + bits_of_all_delimiters.shift_in_the_first_byte()
            ;
    return bits_reference(
                bits_of_all_delimiters.first_byte_ptr() + (shift >> 3U),
                shift & 7U,
                m_num_bits_per_delimiter_number.at(kind_and_index.first)
                );
}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_sensory_cell(
        natural_32_bit const index_of_sensory_cell
        )
{
    ASSUMPTION(index_of_sensory_cell < get_static_state_of_neural_tissue()->num_sensory_cells());
    return m_bits_of_sensory_cells.find_bits_of_unit(index_of_sensory_cell);
}

bits_reference  dynamic_state_of_neural_tissue::find_bits_of_synapse_to_muscle(
        natural_32_bit const index_of_synapse_to_muscle
        )
{
    ASSUMPTION(index_of_synapse_to_muscle < get_static_state_of_neural_tissue()->num_synapses_to_muscles());
    return m_bits_of_synapses_to_muscles.find_bits_of_unit(index_of_synapse_to_muscle);
}


bits_reference  dynamic_state_of_neural_tissue::find_bits_of_coords_of_source_cell_of_synapse_to_muscle(
        natural_32_bit const index_of_synapse_to_muscle
        )
{
    ASSUMPTION(index_of_synapse_to_muscle < get_static_state_of_neural_tissue()->num_synapses_to_muscles());
    return m_bits_of_source_cell_coords_of_synapses_to_muscles.find_bits_of_unit(index_of_synapse_to_muscle);
}


natural_8_bit  dynamic_state_of_neural_tissue::num_bits_per_source_cell_coordinate() const
{
    return m_num_bits_per_source_cell_coordinate;
}

natural_8_bit  dynamic_state_of_neural_tissue::num_bits_per_delimiter_number(kind_of_cell const  kind_of_tissue_cell) const
{
    ASSUMPTION(kind_of_tissue_cell < get_static_state_of_neural_tissue()->num_kinds_of_tissue_cells());
    return m_num_bits_per_delimiter_number.at(kind_of_tissue_cell);
}


natural_16_bit num_of_bits_to_store_territorial_state_of_synapse()
{
    return compute_byte_aligned_num_of_bits_to_store_number(6U);
}

natural_8_bit num_delimiters()
{
    return 6U;
}

boost::multiprecision::int128_t compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
        static_state_of_neural_tissue const& static_state_of_tissue
        )
{
    ASSUMPTION(num_delimiters() > 0U);
    ASSUMPTION(num_of_bits_to_store_territorial_state_of_synapse() > 0U);

    natural_8_bit const num_bits_per_source_cell_coordinate =
          compute_byte_aligned_num_of_bits_to_store_number(
              std::max(static_state_of_tissue.num_cells_along_x_axis(),
                       std::max(static_state_of_tissue.num_cells_along_y_axis(),
                                checked_add_32_bit(static_state_of_tissue.num_cells_along_columnar_axis(),
                                                   static_state_of_tissue.num_sensory_cells())))
              );
    ASSUMPTION(num_bits_per_source_cell_coordinate > 0U);
    ASSUMPTION(num_bits_per_source_cell_coordinate < 32U);

    // Let's start with 'sizeof' of our data structures
    boost::multiprecision::int128_t num_bits =
            checked_add_64_bit(
                sizeof(dynamic_state_of_neural_tissue),
                checked_mul_64_bit(
                    static_state_of_tissue.num_kinds_of_tissue_cells(),
                    sizeof(natural_8_bit) + 6U * (
                        sizeof(std::shared_ptr<homogenous_slice_of_tissue>) +
                        sizeof(homogenous_slice_of_tissue)
                        )
                    )
                )
            ;
    // now let's compute the size of the memory to be allocated to store the tissue itself
    for (kind_of_cell kind = 0U; kind < static_state_of_tissue.num_kinds_of_tissue_cells(); ++kind)
    {
        // slice of cells
        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
                        static_state_of_tissue.num_bits_per_cell(),
                        static_state_of_tissue.num_cells_along_x_axis(),
                        static_state_of_tissue.num_cells_along_y_axis(),
                        static_state_of_tissue.num_tissue_cells_of_cell_kind(kind)
                        );
        // slice of synapses
        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
                        static_state_of_tissue.num_bits_per_synapse(),
                        static_state_of_tissue.num_cells_along_x_axis(),
                        static_state_of_tissue.num_cells_along_y_axis(),
                        checked_mul_64_bit(
                            static_state_of_tissue.num_tissue_cells_of_cell_kind(kind),
                            static_state_of_tissue.num_synapses_in_territory_of_cell_kind(kind)
                            )
                        );
        // slice of territorial states of synapses
        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
                        num_of_bits_to_store_territorial_state_of_synapse(),
                        static_state_of_tissue.num_cells_along_x_axis(),
                        static_state_of_tissue.num_cells_along_y_axis(),
                        checked_mul_64_bit(
                            static_state_of_tissue.num_tissue_cells_of_cell_kind(kind),
                            static_state_of_tissue.num_synapses_in_territory_of_cell_kind(kind)
                            )
                        );
        // slices of source cell coords of synapses
        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
                        checked_mul_16_bit(3U,num_bits_per_source_cell_coordinate),
                        static_state_of_tissue.num_cells_along_x_axis(),
                        static_state_of_tissue.num_cells_along_y_axis(),
                        checked_mul_64_bit(
                            static_state_of_tissue.num_tissue_cells_of_cell_kind(kind),
                            static_state_of_tissue.num_synapses_in_territory_of_cell_kind(kind)
                            )
                        );
        // slices of signalling
        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
                        static_state_of_tissue.num_bits_per_signalling(),
                        static_state_of_tissue.num_cells_along_x_axis(),
                        static_state_of_tissue.num_cells_along_y_axis(),
                        static_state_of_tissue.num_tissue_cells_of_cell_kind(kind)
                        );
        // slices of delimiters between territorial lists
        natural_8_bit const num_bits_per_delimiter_number =
                compute_byte_aligned_num_of_bits_to_store_number(
                        static_state_of_tissue.num_synapses_in_territory_of_cell_kind(kind)
                        );
        checked_add_8_bit(checked_mul_8_bit(num_delimiters()-1U,num_bits_per_delimiter_number),7U);
        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
                        checked_mul_16_bit(num_delimiters(),num_bits_per_delimiter_number),
                        static_state_of_tissue.num_cells_along_x_axis(),
                        static_state_of_tissue.num_cells_along_y_axis(),
                        static_state_of_tissue.num_tissue_cells_of_cell_kind(kind)
                        );
    }
    // sensory cells
    num_bits += compute_num_bits_of_all_array_units_with_checked_operations(
                        static_state_of_tissue.num_bits_per_cell(),
                        static_state_of_tissue.num_sensory_cells()
                        );
    // synapses to muscles
    num_bits += compute_num_bits_of_all_array_units_with_checked_operations(
                        static_state_of_tissue.num_bits_per_synapse(),
                        static_state_of_tissue.num_synapses_to_muscles()
                        );
    // coords of source cell of synapses to muscles
    num_bits += compute_num_bits_of_all_array_units_with_checked_operations(
                        checked_mul_16_bit(3U,num_bits_per_source_cell_coordinate),
                        static_state_of_tissue.num_synapses_to_muscles());

    return num_bits;
}

boost::multiprecision::int128_t compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_ptr
        )
{
    ASSUMPTION(static_state_ptr.operator bool());
    return compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(*static_state_ptr.get());
}


}
