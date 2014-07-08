#ifndef CELLAB_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/cell_kinds.hpp>
#   include <cellab/bits_reference.hpp>
#   include <boost/noncopyable.hpp>
#   include <boost/scoped_array.hpp>

struct neural_tissue : private boost::noncopyable
{
    neural_tissue(
        unsigned int const num_columns_along_x_axis,
        unsigned int const num_columns_along_y_axis,
        unsigned char const* const num_bits_per_cell_kind,
        unsigned short const* const num_cells_of_kind_in_column,
        unsigned char const num_bits_per_synapse,
        unsigned char const* const max_num_synapses_per_cell_kind
        );

    unsigned int num_columns_along_x_axis() const;
    unsigned int num_columns_along_y_axis() const;

    unsigned char num_bits_per_cell(CELL_KIND const cell_kind) const;

    unsigned char num_bits_per_synapse() const;

    unsigned short num_cells_in_column(CELL_KIND const cell_kind) const;
    unsigned short max_num_synapses_per_cell(CELL_KIND const cell_kind) const;

    bits_reference cell_bits_reference(
        unsigned int const column_index_x,
        unsigned int const column_index_y,
        CELL_KIND const cell_kind,
        unsigned short const cell_index
        );

    bits_reference synapse_bits_reference(
        unsigned int const column_index_x,
        unsigned int const column_index_y,
        CELL_KIND const target_cell_kind,
        unsigned short const target_cell_index,
        unsigned short const synapse_index
        );

private:
    unsigned int m_num_columns_along_x_axis;
    unsigned int m_num_columns_along_y_axis;
    boost::scoped_array<unsigned char> m_num_bits_per_cell_kind;
    boost::scoped_array<unsigned short> m_num_cells_of_kind_in_column;
    unsigned char m_num_bits_per_synapse;
    boost::scoped_array<unsigned char> m_max_num_synapses_per_cell_kind;
    boost::scoped_array< boost::scoped_array<unsigned char> > m_cell_layers;
    boost::scoped_array< boost::scoped_array<unsigned char> > m_synapse_layers;
};

#endif
