#ifndef CELLAB_STATIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_STATIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <boost/noncopyable.hpp>
#   include <boost/scoped_array.hpp>
#   include <vector>

namespace cellab {


typedef natural_16_bit kind_of_cell;

struct static_state_of_neural_tissue : private boost::noncopyable
{
    static_state_of_neural_tissue(
        natural_16_bit const num_kinds_of_cells,
        natural_16_bit const num_bits_per_cell,
        natural_16_bit const num_bits_per_synapse,
        natural_16_bit const num_bits_per_signalling,
        natural_32_bit const num_tissue_cells_along_x_axis,
        natural_32_bit const num_tissue_cells_along_y_axis,
        natural_32_bit const num_sensory_cells,
        natural_32_bit const num_synapses_to_muscles,
        std::vector<natural_32_bit> const& num_cells_along_columnar_axis_of_cell_kind,
        std::vector<natural_32_bit> const& num_synapses_in_territory_of_each_cell_of_kind
        //TODO...
        );

    ~static_state_of_neural_tissue();

    natural_16_bit  num_kinds_of_cells() const;

    natural_16_bit  num_bits_per_cell() const;
    natural_16_bit  num_bits_per_synapse() const;
    natural_16_bit  num_bits_per_signalling() const;

    natural_32_bit  num_tissue_cells_along_x_axis() const;
    natural_32_bit  num_tissue_cells_along_y_axis() const;
    natural_32_bit  num_tissue_cells_along_columnar_axis() const;
    natural_32_bit  num_tissue_cells_along_columnar_axis_of_cell_kind(kind_of_cell const cell_kind) const;

    natural_32_bit  num_synapses_in_territory_of_cell(kind_of_cell const cell_kind) const;

    natural_32_bit  num_sensory_cells() const;
    natural_32_bit  num_synapses_to_muscles() const;

    kind_of_cell  compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                natural_32_bit position_of_tissue_cell_in_column) const;

    bool  is_x_axis_torus_axis() const;
    bool  is_y_axis_torus_axis() const;
    bool  is_columnar_axis_torus_axis() const;

    integer_8_bit  get_x_radius_of_signalling_neighbourhood_of_cell(kind_of_cell const cell_kind) const;
    integer_8_bit  get_y_radius_of_signalling_neighbourhood_of_cell(kind_of_cell const cell_kind) const;
    integer_8_bit  get_columnar_radius_of_signalling_neighbourhood_of_cell(kind_of_cell const cell_kind) const;

    integer_8_bit  get_x_radius_of_signalling_neighbourhood_of_synapse(kind_of_cell const cell_kind) const;
    integer_8_bit  get_y_radius_of_signalling_neighbourhood_of_synapse(kind_of_cell const cell_kind) const;
    integer_8_bit  get_columnar_radius_of_signalling_neighbourhood_of_synapse(kind_of_cell const cell_kind) const;

    integer_8_bit  get_x_radius_of_cellular_neighbourhood_of_signalling(kind_of_cell const cell_kind) const;
    integer_8_bit  get_y_radius_of_cellular_neighbourhood_of_signalling(kind_of_cell const cell_kind) const;
    integer_8_bit  get_columnar_radius_of_cellular_neighbourhood_of_signalling(kind_of_cell const cell_kind) const;

private:
    natural_16_bit m_num_kinds_of_tissue_cells;

    natural_16_bit m_num_bits_per_cell;
    natural_16_bit m_num_bits_per_synapse;
    natural_16_bit m_num_bits_per_signalling;

    natural_32_bit m_num_tissue_cells_along_x_axis;
    natural_32_bit m_num_tissue_cells_along_y_axis;
    natural_32_bit m_num_tissue_cells_along_columnar_axis;

    natural_32_bit m_num_sensory_cells;
    natural_32_bit m_num_synapses_to_muscles;

    std::vector<natural_32_bit> m_num_cells_along_columnar_axis_of_cell_kind;
    std::vector<natural_32_bit> m_num_synapses_in_territory_of_each_cell_of_kind;
};


}

#endif
