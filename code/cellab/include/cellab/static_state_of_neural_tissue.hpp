#ifndef CELLAB_STATIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_STATIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_cell.hpp>
#   include <cellab/static_state_of_synapse.hpp>
#   include <boost/noncopyable.hpp>
#   include <boost/scoped_array.hpp>

struct static_state_of_neural_tissue : private boost::noncopyable
{
    static_state_of_neural_tissue(
        unsigned char const num_kinds_of_cells,
        unsigned char const num_bits_per_cell,
        unsigned char const num_bits_per_synapse,
        unsigned int const num_cells_along_x_axis,
        unsigned int const num_cells_along_y_axis,
        unsigned int const num_cells_along_columnar_axis,
        unsigned short const* const
            array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column,
        static_state_of_cell const* const
            array_where_indices_are_kinds_of_cells_and_values_are_static_states_of_cells,
        static_state_of_synapse const* const
            row_major_array2D_where_indices_are_pairs_of_kinds_of_cells_and_values_are_static_states_of_synapses
        );

    unsigned char num_kinds_of_cells() const;

    unsigned char num_bits_per_cell() const;
    unsigned char num_bits_per_synapse() const;

    unsigned int num_cells_along_x_axis() const;
    unsigned int num_cells_along_y_axis() const;
    unsigned int num_cells_along_columnar_axis() const;
    unsigned short num_cells_in_column(unsigned char const kind_of_cell) const;

    unsigned char compute_kind_of_cell_from_its_position_along_columnar_axis(
                        unsigned int const position_of_cell_in_column) const;

    static_state_of_cell const& get_static_state_of_cell(unsigned char const kind_of_cell) const;
    static_state_of_synapse const& get_static_state_of_synapse(unsigned char const kind_of_source_cell,
                                                               unsigned char const kind_of_target_cell) const;

private:
    unsigned int m_num_cells_along_x_axis;
    unsigned int m_num_cells_along_y_axis;
    unsigned int m_num_cells_along_columnar_axis;

    unsigned char m_num_kinds_of_cells;

    unsigned char m_num_bits_per_cell;
    unsigned char m_num_bits_per_synapse;

    boost::scoped_array<unsigned short> m_num_cells_in_column_per_kind_of_cell;
    boost::scoped_array<static_state_of_cell> m_static_state_of_cell_per_kind_of_cell;
    boost::scoped_array<static_state_of_synapse> m_static_state_of_synapse_per_pair_of_kinds_of_cells;
};

#endif
