#include <cellab/neural_tissue.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <algorithm>

neural_tissue::neural_tissue(
    unsigned int const num_columns_along_x_axis,
    unsigned int const num_columns_along_y_axis,
    unsigned char const* const num_bits_per_cell_kind,
    unsigned short const* const num_cells_of_kind_in_column,
    unsigned char const num_bits_per_synapse,
    unsigned char const* const max_num_synapses_per_cell_kind
    )
    : m_num_columns_along_x_axis(num_columns_along_x_axis)
    , m_num_columns_along_y_axis(num_columns_along_y_axis)
    , m_num_bits_per_cell_kind(new unsigned char[num_cell_kinds()])
    , m_num_cells_of_kind_in_column(new unsigned short[num_cell_kinds()])
    , m_num_bits_per_synapse(num_bits_per_synapse)
    , m_max_num_synapses_per_cell_kind(new unsigned char[num_cell_kinds()])
    , m_cell_layers(new boost::scoped_array<unsigned char>[num_cell_kinds()])
    , m_synapse_layers(new boost::scoped_array<unsigned char>[num_cell_kinds()])
{
    ASSUMPTION(m_num_columns_along_x_axis > 0);
    ASSUMPTION(m_num_columns_along_y_axis > 0);
    ASSUMPTION(m_num_bits_per_synapse > 0);

    std::copy(
        num_bits_per_cell_kind,
        num_bits_per_cell_kind + num_cell_kinds(),
        m_num_bits_per_cell_kind.get()
        );
    std::copy(
        num_cells_of_kind_in_column,
        num_cells_of_kind_in_column + num_cell_kinds(),
        m_num_cells_of_kind_in_column.get()
        );
    std::copy(
        max_num_synapses_per_cell_kind,
        max_num_synapses_per_cell_kind + num_cell_kinds(),
        m_max_num_synapses_per_cell_kind.get()
        );

    for (unsigned char i = 0; i < num_cell_kinds(); ++i)
    {
        ASSUMPTION(m_num_cells_of_kind_in_column[i] > 0);
        ASSUMPTION(m_num_bits_per_cell_kind[i] > 0);

        unsigned long const num_bits =
            m_num_columns_along_x_axis *
            m_num_columns_along_y_axis *
            m_num_cells_of_kind_in_column[i] *
            m_num_bits_per_cell_kind[i]
            ;
        unsigned long const num_bytes =
            num_bits / 8U +
            ((8U * (num_bits / 8U) == num_bits) ? 0U : 1U)
            ;
        INVARIANT(8U * num_bytes <= num_bits && 8U * num_bytes < num_bits + 8);

        m_cell_layers[i].reset(new unsigned char[num_bytes]);
    }

    for (unsigned char i = 0; i < num_cell_kinds(); ++i)
    {
        ASSUMPTION(m_num_bits_per_synapse > 0);

        unsigned long const num_bits =
            m_num_columns_along_x_axis *
            m_num_columns_along_y_axis *
            m_num_cells_of_kind_in_column[i] *
            m_max_num_synapses_per_cell_kind[i] *
            m_num_bits_per_synapse
            ;
        unsigned long const num_bytes =
            num_bits / 8U +
            ((8U * (num_bits / 8U) == num_bits) ? 0U : 1U)
            ;
        INVARIANT(8U * num_bytes <= num_bits && 8U * num_bytes < num_bits + 8);

        m_synapse_layers[i].reset(new unsigned char[num_bytes]);
    }
}

unsigned int neural_tissue::num_columns_along_x_axis() const
{
    return m_num_columns_along_x_axis;
}

unsigned int neural_tissue::num_columns_along_y_axis() const
{
    return m_num_columns_along_y_axis;
}

unsigned char neural_tissue::num_bits_per_cell(CELL_KIND const cell_kind) const
{
    return m_num_bits_per_cell_kind[cell_kind];
}

unsigned char neural_tissue::num_bits_per_synapse() const
{
    return m_num_bits_per_synapse;
}

unsigned short neural_tissue::num_cells_in_column(CELL_KIND const cell_kind) const
{
    return m_num_cells_of_kind_in_column[cell_kind];
}

unsigned short neural_tissue::max_num_synapses_per_cell(CELL_KIND const cell_kind) const
{
    return m_max_num_synapses_per_cell_kind[cell_kind];
}

bits_reference neural_tissue::cell_bits_reference(
    unsigned int const column_index_x,
    unsigned int const column_index_y,
    CELL_KIND const cell_kind,
    unsigned short const cell_index
    )
{
    ASSUMPTION(column_index_x < num_columns_along_x_axis());
    ASSUMPTION(column_index_y < num_columns_along_y_axis());
    ASSUMPTION(cell_kind < num_cell_kinds());
    ASSUMPTION(cell_index < num_cells_in_column(cell_kind));

    unsigned int const num_bits_per_column =
        num_cells_in_column(cell_kind) * num_bits_per_cell(cell_kind)
        ;

    unsigned long const index_of_the_first_bit_of_the_referenced_cell =
        column_index_y * (num_columns_along_x_axis() * num_bits_per_column) +
        column_index_x * num_bits_per_column +
        cell_index * num_bits_per_cell(cell_kind)
        ;

    unsigned long const index_of_the_first_byte_of_the_referenced_cell =
        index_of_the_first_bit_of_the_referenced_cell / 8U
        ;

    unsigned char const seek_to_the_first_bit_in_the_byte =
        index_of_the_first_bit_of_the_referenced_cell  -
        index_of_the_first_byte_of_the_referenced_cell * 8U
        ;
    INVARIANT(seek_to_the_first_bit_in_the_byte < 8U);

    return bits_reference(
                &m_cell_layers[cell_kind][index_of_the_first_byte_of_the_referenced_cell],
                seek_to_the_first_bit_in_the_byte,
                num_bits_per_cell(cell_kind)
                );
}

bits_reference neural_tissue::synapse_bits_reference(
    unsigned int const column_index_x,
    unsigned int const column_index_y,
    CELL_KIND const target_cell_kind,
    unsigned short const target_cell_index,
    unsigned short const synapse_index
    )
{
    ASSUMPTION(column_index_x < num_columns_along_x_axis());
    ASSUMPTION(column_index_y < num_columns_along_y_axis());
    ASSUMPTION(target_cell_kind < num_cell_kinds());
    ASSUMPTION(target_cell_index < num_cells_in_column(target_cell_kind));
    ASSUMPTION(synapse_index < max_num_synapses_per_cell(target_cell_kind));

    unsigned int const num_bits_of_all_synapses_per_cell =
        max_num_synapses_per_cell(target_cell_kind) *
        num_bits_per_synapse()
        ;

    unsigned int const num_bits_per_column =
        num_cells_in_column(target_cell_kind) *
        num_bits_of_all_synapses_per_cell;
        ;

    unsigned long const index_of_the_first_bit_of_the_referenced_synapse =
        column_index_y * (num_columns_along_x_axis() * num_bits_per_column) +
        column_index_x * num_bits_per_column +
        target_cell_index * num_bits_of_all_synapses_per_cell +
        synapse_index * num_bits_per_synapse()
        ;

    unsigned long const index_of_the_first_byte_of_the_referenced_synapse =
        index_of_the_first_bit_of_the_referenced_synapse / 8U
        ;

    unsigned char const seek_to_the_first_bit_in_the_byte =
        index_of_the_first_bit_of_the_referenced_synapse  -
        index_of_the_first_byte_of_the_referenced_synapse * 8U
        ;
    INVARIANT(seek_to_the_first_bit_in_the_byte < 8U);

    return bits_reference(
                &m_synapse_layers[target_cell_kind][index_of_the_first_byte_of_the_referenced_synapse],
                seek_to_the_first_bit_in_the_byte,
                num_bits_per_synapse()
                );
}
