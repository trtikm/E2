#ifndef CELLAB_UTILITIES_FOR_TRANSITION_ALGORITHMS_HPP_INCLUDED
#   define CELLAB_UTILITIES_FOR_TRANSITION_ALGORITHMS_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <cellab/shift_in_coordinates.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/bits_reference.hpp>
#   include <memory>
#   include <tuple>

namespace cellab {


struct tissue_coordinates
{
    tissue_coordinates(
            natural_32_bit const coord_along_x_axis,
            natural_32_bit const coord_along_y_axis,
            natural_32_bit const coord_along_columnar_axis
            );
    natural_32_bit  get_coord_along_x_axis() const;
    natural_32_bit  get_coord_along_y_axis() const;
    natural_32_bit  get_coord_along_columnar_axis() const;
private:
    natural_32_bit  m_coord_along_x_axis;
    natural_32_bit  m_coord_along_y_axis;
    natural_32_bit  m_coord_along_columnar_axis;
};

struct spatial_neighbourhood
{
    spatial_neighbourhood(
            tissue_coordinates const& center,
            shift_in_coordinates const& shift_to_low_corner,
            shift_in_coordinates const& shift_to_high_corner
            );
    tissue_coordinates const& get_center_of_neighbourhood() const;
    shift_in_coordinates const& get_shift_to_low_corner() const;
    shift_in_coordinates const& get_shift_to_high_corner() const;
private:
    tissue_coordinates m_center_of_neighbourhood;
    shift_in_coordinates m_shift_to_low_corner;
    shift_in_coordinates m_shift_to_high_corner;
};


bool operator==(tissue_coordinates const& left, tissue_coordinates const& right);

tissue_coordinates  shift_coordinates(
        tissue_coordinates const& coords,
        shift_in_coordinates const& shift,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        natural_32_bit const num_cells_along_columnar_axis
        );

bool  go_to_next_coordinates(
        natural_32_bit& x_coord, natural_32_bit& y_coord, natural_32_bit& c_coord,
        natural_32_bit const extent,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        natural_32_bit const num_cells_along_columnar_axis
        );

bool  go_to_next_column(
        natural_32_bit& x_coord, natural_32_bit& y_coord,
        natural_32_bit const extent,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis
        );

bool  go_to_next_index(
        natural_32_bit& index,
        natural_32_bit const extent,
        natural_32_bit const size
        );

integer_8_bit  clip_shift(
        integer_8_bit const shift,
        natural_32_bit const origin,
        natural_32_bit const length_of_axis,
        bool const is_it_torus_axis
        );

void  write_tissue_coordinates_to_bits_of_coordinates(tissue_coordinates const& coords, bits_reference& bits_ref);
tissue_coordinates  convert_bits_of_coordinates_to_tissue_coordinates(bits_reference const& bits_ref);

tissue_coordinates  get_coordinates_of_source_cell_of_synapse_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& coords_of_territorial_cell_of_synapse,
        natural_32_bit const index_of_synapse_in_territory
        );

tissue_coordinates  get_coordinates_of_source_cell_of_synapse_to_muscle(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const index_of_synapse_to_muscle
        );

natural_32_bit  get_begin_index_of_territorial_list_of_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& coordinates_of_cell,
        natural_8_bit const index_of_territorial_list
        );
natural_32_bit  get_end_index_of_territorial_list_of_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        tissue_coordinates const& coordinates_of_cell,
        natural_8_bit const index_of_territorial_list
        );

void  swap_all_data_of_two_synapses(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& first_cell_coordinates,
        natural_32_bit const synapse_index_in_first_territory,
        tissue_coordinates const& second_cell_coordinates,
        natural_32_bit const synapse_index_in_second_territory
        );


std::pair<bits_const_reference,kind_of_cell>  get_signalling_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_coordinates const& shift
        );

std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>  get_synapse_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        tissue_coordinates const& target_cell,
        kind_of_cell const kind_of_target_cell,
        natural_32_bit const number_of_synapses_in_range,
        natural_32_bit const shift_to_start_index,
        natural_32_bit const shift_from_start_index
        );

std::pair<bits_const_reference,kind_of_cell>  get_cell_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_coordinates const& shift
        );


}

#endif
