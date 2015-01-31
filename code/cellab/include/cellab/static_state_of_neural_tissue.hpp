#ifndef CELLAB_STATIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_STATIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <boost/noncopyable.hpp>
#   include <boost/scoped_array.hpp>
#   include <vector>
#   include <tuple>

namespace cellab {


typedef natural_16_bit kind_of_cell;

struct static_state_of_neural_tissue : private boost::noncopyable
{
    static_state_of_neural_tissue(
        natural_16_bit const num_kinds_of_tissue_cells,
        natural_16_bit const num_kinds_of_sensory_cells,
        natural_16_bit const num_bits_per_cell,
        natural_16_bit const num_bits_per_synapse,
        natural_16_bit const num_bits_per_signalling,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        std::vector<natural_32_bit> const& num_tissue_cells_of_cell_kind,
        std::vector<natural_32_bit> const& num_synapses_in_territory_of_cell_kind,
        std::vector<natural_32_bit> const& num_sensory_cells_of_cell_kind,
        natural_32_bit const num_synapses_to_muscles,
        bool const is_x_axis_torus_axis,
        bool const is_y_axis_torus_axis,
        bool const is_columnar_axis_torus_axis,
        std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& x_radius_of_cellular_neighbourhood_of_signalling,
        std::vector<integer_8_bit> const& y_radius_of_cellular_neighbourhood_of_signalling,
        std::vector<integer_8_bit> const& columnar_radius_of_cellular_neighbourhood_of_signalling
        );

    ~static_state_of_neural_tissue();

    natural_16_bit  num_kinds_of_cells() const;
    natural_16_bit  num_kinds_of_tissue_cells() const;
    natural_16_bit  num_kinds_of_sensory_cells() const;
    kind_of_cell  lowest_kind_of_sensory_cells() const;

    natural_16_bit  num_bits_per_cell() const;
    natural_16_bit  num_bits_per_synapse() const;
    natural_16_bit  num_bits_per_signalling() const;

    natural_32_bit  num_cells_along_x_axis() const;
    natural_32_bit  num_cells_along_y_axis() const;
    natural_32_bit  num_cells_along_columnar_axis() const;

    natural_32_bit  num_tissue_cells_of_cell_kind(kind_of_cell const cell_kind) const;
    natural_32_bit  num_synapses_in_territory_of_cell_kind(kind_of_cell const cell_kind) const;
    natural_32_bit  num_sensory_cells_of_cell_kind(kind_of_cell const cell_kind) const;

    natural_32_bit  num_sensory_cells() const;
    natural_32_bit  num_synapses_to_muscles() const;

    kind_of_cell  compute_kind_of_cell_from_its_position_along_columnar_axis(
                natural_32_bit position_of_cell_in_column) const;

    std::pair<kind_of_cell,natural_32_bit>
    compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
                natural_32_bit position_of_cell_in_column) const;

    kind_of_cell  compute_kind_of_sensory_cell_from_its_index(natural_32_bit index_of_sensory_cell) const;

    std::pair<kind_of_cell,natural_32_bit>
    compute_kind_of_sensory_cell_and_relative_index_from_its_index(natural_32_bit index_of_sensory_cell) const;

    natural_32_bit  compute_index_of_first_sensory_cell_of_kind(kind_of_cell const sensory_cell_kind) const;
    natural_32_bit  compute_columnar_coord_of_first_tissue_cell_of_kind(kind_of_cell const tissue_cell_kind) const;

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
    natural_16_bit m_num_kinds_of_cells;
    natural_16_bit m_num_kinds_of_tissue_cells;

    natural_16_bit m_num_bits_per_cell;
    natural_16_bit m_num_bits_per_synapse;
    natural_16_bit m_num_bits_per_signalling;

    natural_32_bit m_num_cells_along_x_axis;
    natural_32_bit m_num_cells_along_y_axis;
    natural_32_bit m_num_cells_along_columnar_axis;

    std::vector<natural_32_bit> m_num_tissue_cells_of_cell_kind;
    std::vector<natural_32_bit> m_num_synapses_in_territory_of_cell_kind;
    std::vector<natural_32_bit> m_num_sensory_cells_of_cell_kind;

    natural_32_bit m_num_sensory_cells;
    natural_32_bit m_num_synapses_to_muscles;

    std::vector<natural_32_bit> m_end_index_along_columnar_axis_of_cell_kind;

    bool m_is_x_axis_torus_axis;
    bool m_is_y_axis_torus_axis;
    bool m_is_columnar_axis_torus_axis;

    std::vector<integer_8_bit> m_x_radius_of_signalling_neighbourhood_of_cell;
    std::vector<integer_8_bit> m_y_radius_of_signalling_neighbourhood_of_cell;
    std::vector<integer_8_bit> m_columnar_radius_of_signalling_neighbourhood_of_cell;

    std::vector<integer_8_bit> m_x_radius_of_signalling_neighbourhood_of_synapse;
    std::vector<integer_8_bit> m_y_radius_of_signalling_neighbourhood_of_synapse;
    std::vector<integer_8_bit> m_columnar_radius_of_signalling_neighbourhood_of_synapse;

    std::vector<integer_8_bit> m_x_radius_of_cellular_neighbourhood_of_signalling;
    std::vector<integer_8_bit> m_y_radius_of_cellular_neighbourhood_of_signalling;
    std::vector<integer_8_bit> m_columnar_radius_of_cellular_neighbourhood_of_signalling;
};


}

#endif
