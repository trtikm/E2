#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/assumptions.hpp>


static natural_64_bit num_bytes_to_store_bits(natural_64_bit const num_bits_to_store)
{
    return (num_bits_to_store >> 3U) + ((num_bits_to_store & 7U) == 0U) ? 0U : 1U;
}

static natural_32_bit total_count_of_numbers_in_seven_heads_and_tails_pairs_of_indices()
{
    return 7U * 2U;
}


namespace cellab {


reference_to_list_of_indices_of_synampses::reference_to_list_of_indices_of_synampses(
        natural_16_bit* const pointer_to_pair_of_head_and_tail_indices_of_the_list,
        natural_16_bit const max_index_of_any_synapse_in_the_list
        )
    : m_pointer_to_pair_of_head_and_tail_indices_of_the_list(
          pointer_to_pair_of_head_and_tail_indices_of_the_list
          )
    , m_max_index_of_any_synapse(max_index_of_any_synapse_in_the_list)
{}

natural_16_bit reference_to_list_of_indices_of_synampses::head() const
{
    return m_pointer_to_pair_of_head_and_tail_indices_of_the_list[0];
}

natural_16_bit reference_to_list_of_indices_of_synampses::tail() const
{
    return m_pointer_to_pair_of_head_and_tail_indices_of_the_list[1];
}

void reference_to_list_of_indices_of_synampses::set_head(natural_16_bit const index)
{
    m_pointer_to_pair_of_head_and_tail_indices_of_the_list[0] = index;
}

void reference_to_list_of_indices_of_synampses::set_tail(natural_16_bit const index)
{
    m_pointer_to_pair_of_head_and_tail_indices_of_the_list[1] = index;
}

natural_16_bit reference_to_list_of_indices_of_synampses::max_index_of_any_synapse() const
{
    return m_max_index_of_any_synapse;
}



reference_to_synapses_in_territory_of_cell::reference_to_synapses_in_territory_of_cell(
        natural_16_bit* const array_of_seven_pairs_of_head_and_tail_synapse_indices,
        natural_8_bit* const pointer_to_memory_with_bits_of_all_synapses,
        natural_8_bit const seek_in_first_byte_in_memory_with_synapses,
        natural_8_bit const num_bits_per_synapse,
        natural_16_bit const total_number_of_synapses_in_the_territory
        )
    : m_array_of_seven_pairs_of_head_and_tail_synapse_indices(
        array_of_seven_pairs_of_head_and_tail_synapse_indices
        )
    , m_bits_of_all_synapses(pointer_to_memory_with_bits_of_all_synapses)
    , m_seek_in_first_byte_in_memory_with_synapses(seek_in_first_byte_in_memory_with_synapses)
    , m_num_bits_per_synapse(num_bits_per_synapse)
    , m_number_of_synapses(total_number_of_synapses_in_the_territory)
{}

bits_reference reference_to_synapses_in_territory_of_cell::find_bits_of_synapse(
        natural_16_bit const index_of_synapse_in_range_from_0_to_number_of_synapses_minus_1)
{
    ASSUMPTION(index_of_synapse_in_range_from_0_to_number_of_synapses_minus_1 < number_of_synapses());

    natural_32_bit const first_bit_index =
            m_seek_in_first_byte_in_memory_with_synapses
            + index_of_synapse_in_range_from_0_to_number_of_synapses_minus_1 * num_bits_per_synapse()
            ;

    return bits_reference(
                &m_bits_of_all_synapses[first_bit_index >> 3U],
                first_bit_index & 7U,
                num_bits_per_synapse()
                );
}

natural_16_bit reference_to_synapses_in_territory_of_cell::number_of_synapses() const
{
    return m_number_of_synapses;
}

natural_8_bit reference_to_synapses_in_territory_of_cell::num_bits_per_synapse() const
{
    return m_num_bits_per_synapse;
}

reference_to_list_of_indices_of_synampses
reference_to_synapses_in_territory_of_cell::indices_of_connected_synapses()
{
    return reference_to_list_of_indices_of_synampses(
                &m_array_of_seven_pairs_of_head_and_tail_synapse_indices[0U],
                number_of_synapses()
                );
}

reference_to_list_of_indices_of_synampses
reference_to_synapses_in_territory_of_cell::indices_of_synapses_to_be_moved_along_positive_x_axis()
{
    return reference_to_list_of_indices_of_synampses(
                &m_array_of_seven_pairs_of_head_and_tail_synapse_indices[2U],
                number_of_synapses()
                );
}

reference_to_list_of_indices_of_synampses
reference_to_synapses_in_territory_of_cell::indices_of_synapses_to_be_moved_along_negative_x_axis()
{
    return reference_to_list_of_indices_of_synampses(
                &m_array_of_seven_pairs_of_head_and_tail_synapse_indices[4U],
                number_of_synapses()
                );
}

reference_to_list_of_indices_of_synampses
reference_to_synapses_in_territory_of_cell::indices_of_synapses_to_be_moved_along_positive_y_axis()
{
    return reference_to_list_of_indices_of_synampses(
                &m_array_of_seven_pairs_of_head_and_tail_synapse_indices[6U],
                number_of_synapses()
                );
}

reference_to_list_of_indices_of_synampses
reference_to_synapses_in_territory_of_cell::indices_of_synapses_to_be_moved_along_negative_y_axis()
{
    return reference_to_list_of_indices_of_synampses(
                &m_array_of_seven_pairs_of_head_and_tail_synapse_indices[8U],
                number_of_synapses()
                );
}

reference_to_list_of_indices_of_synampses
reference_to_synapses_in_territory_of_cell::indices_of_synapses_to_be_moved_along_positive_columnar_axis()
{
    return reference_to_list_of_indices_of_synampses(
                &m_array_of_seven_pairs_of_head_and_tail_synapse_indices[10U],
                number_of_synapses()
                );
}

reference_to_list_of_indices_of_synampses
reference_to_synapses_in_territory_of_cell::indices_of_synapses_to_be_moved_along_negative_columnar_axis()
{
    return reference_to_list_of_indices_of_synampses(
                &m_array_of_seven_pairs_of_head_and_tail_synapse_indices[12U],
                number_of_synapses()
                );
}



dynamic_state_of_neural_tissue::dynamic_state_of_neural_tissue(
        std::shared_ptr<static_state_of_neural_tissue const> const pointer_to_static_state_of_neural_tissue
        )
    : m_static_state_of_neural_tissue(pointer_to_static_state_of_neural_tissue)
    , m_bits_of_all_cells_organised_in_3D_array_with_appendix_of_sensory_cells(
        new natural_8_bit[
            num_bytes_to_store_bits(
                    (static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis())
                    * static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis())
                    * static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_tissue_cells_along_columnar_axis())
                    + static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_sensory_cells())
                    ) * static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_bits_per_cell())
                )
            ]
        )
    , m_num_synapses_in_all_cell_territories_along_columnar_axis(
            [](std::shared_ptr<static_state_of_neural_tissue const> const tissue_props) {
                natural_64_bit num_sysnapses_in_column = 0;
                for (natural_16_bit kind = 0; kind < tissue_props->num_kinds_of_cells_in_neural_tissue(); ++kind)
                    num_sysnapses_in_column +=
                        static_cast<natural_64_bit>(tissue_props->num_tissue_cells_in_column(kind))
                        * static_cast<natural_64_bit>(tissue_props->get_static_state_of_tissue_cell(kind)
                                                                  .num_synapses_in_territory_of_cell())
                        ;
                return num_sysnapses_in_column;
            }(m_static_state_of_neural_tissue)
        )
    , m_bits_of_synapses_in_teritories_of_all_cells_organised_in_3D_array_with_appendix_of_synapses_to_muscles(
        new natural_8_bit[
            num_bytes_to_store_bits(
                    (static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis())
                    * static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis())
                    * static_cast<natural_64_bit>(m_num_synapses_in_all_cell_territories_along_columnar_axis)
                    + static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_synapses_to_muscles())
                    ) * static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_bits_per_synapse())
                )
            ]
        )
    , m_heads_and_tails_of_seven_lists_of_indices_of_synapses_in_territories_of_all_cells_organised_in_3D_array(
        new natural_16_bit[
            m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis()
            * m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis()
            * m_static_state_of_neural_tissue->num_tissue_cells_along_columnar_axis()
            * total_count_of_numbers_in_seven_heads_and_tails_pairs_of_indices()
            ]
        )
{}

std::shared_ptr<static_state_of_neural_tissue const>
dynamic_state_of_neural_tissue::get_static_state_of_neural_tissue() const
{
    return m_static_state_of_neural_tissue;
}

bits_reference dynamic_state_of_neural_tissue::find_bits_of_cell(
        natural_32_bit const seek_along_x_axis,
        natural_32_bit const seek_along_y_axis,
        natural_32_bit const seek_along_columnar_axis
        )
{
    ASSUMPTION(seek_along_x_axis < m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis());
    ASSUMPTION(seek_along_y_axis < m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis());
    ASSUMPTION(seek_along_columnar_axis < m_static_state_of_neural_tissue->num_tissue_cells_along_columnar_axis());

    natural_64_bit const x = seek_along_x_axis;
    natural_64_bit const y = seek_along_y_axis;
    natural_64_bit const c = seek_along_columnar_axis;
    natural_64_bit const Nx = m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis();
    natural_64_bit const Nc = m_static_state_of_neural_tissue->num_tissue_cells_along_columnar_axis();
    natural_64_bit const Nbits = m_static_state_of_neural_tissue->num_bits_per_cell();

    natural_64_bit const first_bit_index = (y * Nx*Nc + x * Nc + c) * Nbits;

    return bits_reference(
                &m_bits_of_all_cells_organised_in_3D_array_with_appendix_of_sensory_cells[first_bit_index >> 3U],
                first_bit_index & 7U,
                m_static_state_of_neural_tissue->num_bits_per_cell()
                );
}

reference_to_synapses_in_territory_of_cell dynamic_state_of_neural_tissue::find_synapses_in_territory_of_cell(
        natural_32_bit const seek_along_x_axis,
        natural_32_bit const seek_along_y_axis,
        natural_32_bit seek_along_columnar_axis
        )
{
    ASSUMPTION(seek_along_x_axis < m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis());
    ASSUMPTION(seek_along_y_axis < m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis());
    ASSUMPTION(seek_along_columnar_axis < m_static_state_of_neural_tissue->num_tissue_cells_along_columnar_axis());

    natural_64_bit const x = seek_along_x_axis;
    natural_64_bit const y = seek_along_y_axis;
    natural_64_bit const c = seek_along_columnar_axis;
    natural_64_bit const Nx = m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis();
    natural_64_bit const Nc = m_num_synapses_in_all_cell_territories_along_columnar_axis;

    natural_32_bit const first_number_of_terriroty_lists =
            (seek_along_y_axis
                * m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis()
                * m_static_state_of_neural_tissue->num_tissue_cells_along_columnar_axis()
            + seek_along_x_axis
                * m_static_state_of_neural_tissue->num_tissue_cells_along_columnar_axis()
            + seek_along_columnar_axis
            ) * total_count_of_numbers_in_seven_heads_and_tails_pairs_of_indices()
            ;

    natural_64_bit const seek_to_column_with_synapses_of_territory = y * Nx*Nc + x * Nc;

    natural_64_bit seek_in_column_to_sysnapses_of_territory = 0;
    natural_16_bit cell_kind_of_territory = 0;
    {
        while (seek_along_columnar_axis >=
               m_static_state_of_neural_tissue->num_tissue_cells_in_column(cell_kind_of_territory))
        {
            seek_along_columnar_axis -=
                    m_static_state_of_neural_tissue->num_tissue_cells_in_column(cell_kind_of_territory);
            seek_in_column_to_sysnapses_of_territory +=
                static_cast<natural_64_bit>(
                        m_static_state_of_neural_tissue->num_tissue_cells_in_column(cell_kind_of_territory))
                * static_cast<natural_64_bit>(
                        m_static_state_of_neural_tissue->get_static_state_of_tissue_cell(cell_kind_of_territory)
                                                       .num_synapses_in_territory_of_cell())
                ;
            ++cell_kind_of_territory;
        }
        seek_in_column_to_sysnapses_of_territory +=
            c * static_cast<natural_64_bit>(
                    m_static_state_of_neural_tissue->get_static_state_of_tissue_cell(cell_kind_of_territory)
                                                   .num_synapses_in_territory_of_cell())
            ;
    }

    natural_64_bit const first_bit_of_first_synapse_in_territory =
            (seek_to_column_with_synapses_of_territory + seek_in_column_to_sysnapses_of_territory)
            * static_cast<natural_64_bit>(m_static_state_of_neural_tissue->num_bits_per_synapse());

    return reference_to_synapses_in_territory_of_cell(
                &m_heads_and_tails_of_seven_lists_of_indices_of_synapses_in_territories_of_all_cells_organised_in_3D_array
                    [first_number_of_terriroty_lists],
                &m_bits_of_synapses_in_teritories_of_all_cells_organised_in_3D_array_with_appendix_of_synapses_to_muscles
                    [first_bit_of_first_synapse_in_territory >> 3U],
                first_bit_of_first_synapse_in_territory & 7U,
                m_static_state_of_neural_tissue->num_bits_per_synapse(),
                m_static_state_of_neural_tissue->get_static_state_of_tissue_cell(cell_kind_of_territory)
                                               .num_synapses_in_territory_of_cell()
                )
                ;
}

bits_reference dynamic_state_of_neural_tissue::find_bits_of_sensory_cell(
        natural_32_bit const index_of_sensory_cell)
{
    ASSUMPTION(index_of_sensory_cell < m_static_state_of_neural_tissue->num_sensory_cells());

    natural_64_bit const idx = index_of_sensory_cell;
    natural_64_bit const Nx = m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis();
    natural_64_bit const Ny = m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis();
    natural_64_bit const Nc = m_static_state_of_neural_tissue->num_tissue_cells_along_columnar_axis();
    natural_64_bit const Nbits = m_static_state_of_neural_tissue->num_bits_per_cell();

    natural_64_bit const first_bit_index = (Nx * Ny * Nc + idx) * Nbits;

    return bits_reference(
                &m_bits_of_all_cells_organised_in_3D_array_with_appendix_of_sensory_cells[first_bit_index >> 3U],
                first_bit_index & 7U,
                m_static_state_of_neural_tissue->num_bits_per_cell()
                );
}

bits_reference dynamic_state_of_neural_tissue::find_bits_of_synapse_to_muscle(
        natural_32_bit const index_of_synapse_to_muscle)
{
    ASSUMPTION(index_of_synapse_to_muscle < m_static_state_of_neural_tissue->num_synapses_to_muscles());

    natural_64_bit const idx = index_of_synapse_to_muscle;
    natural_64_bit const Nx = m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis();
    natural_64_bit const Ny = m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis();
    natural_64_bit const Nc = m_num_synapses_in_all_cell_territories_along_columnar_axis;
    natural_64_bit const Nbits = m_static_state_of_neural_tissue->num_bits_per_synapse();

    natural_64_bit const first_bit_index = (Nx * Ny * Nc + idx) * Nbits;

    return bits_reference(
                &m_bits_of_synapses_in_teritories_of_all_cells_organised_in_3D_array_with_appendix_of_synapses_to_muscles
                    [first_bit_index >> 3U],
                first_bit_index & 7U,
                m_static_state_of_neural_tissue->num_bits_per_synapse()
                );
}


void check_for_unsigned_wrap_errors(static_state_of_neural_tissue const& tissue_props)
{
    // The functions checked_* contains assumptions excluding wrap errors from their correct
    // executions. If all assertion of the functions are true, then it is safe to use operators
    // + and * directly for address computations in all instances of dynamic_state_of_neural_tissue
    // constructed from the passed instance of static_state_of_neural_tissue.

    checked_mul_64_bit(
        checked_add_64_bit(
            checked_mul_64_bit(
                checked_mul_64_bit(
                    static_cast<natural_64_bit>(tissue_props.num_tissue_cells_along_x_axis()),
                    static_cast<natural_64_bit>(tissue_props.num_tissue_cells_along_y_axis())
                    ),
                static_cast<natural_64_bit>(tissue_props.num_tissue_cells_along_columnar_axis())
                ),
            static_cast<natural_64_bit>(tissue_props.num_sensory_cells())
            ),
        static_cast<natural_64_bit>(tissue_props.num_bits_per_cell())
        );

    natural_64_bit num_sysnapses_in_column = 0U;
    for (natural_16_bit kind = 0; kind < tissue_props.num_kinds_of_cells_in_neural_tissue(); ++kind)
        num_sysnapses_in_column  =
                checked_add_64_bit(
                    num_sysnapses_in_column,
                    checked_mul_64_bit(
                        static_cast<natural_64_bit>(tissue_props.num_tissue_cells_in_column(kind)),
                        static_cast<natural_64_bit>(tissue_props.get_static_state_of_tissue_cell(kind)
                                                                .num_synapses_in_territory_of_cell())
                        )
                    );

    checked_mul_64_bit(
        checked_add_64_bit(
            checked_mul_64_bit(
                checked_mul_64_bit(
                    static_cast<natural_64_bit>(tissue_props.num_tissue_cells_along_x_axis()),
                    static_cast<natural_64_bit>(tissue_props.num_tissue_cells_along_y_axis())
                    ),
                num_sysnapses_in_column
                ),
            static_cast<natural_64_bit>(tissue_props.num_synapses_to_muscles())
            ),
        static_cast<natural_64_bit>(tissue_props.num_bits_per_synapse())
        );

}



}
