#include "./my_neural_tissue.hpp"
#include <utility/development.hpp>


my_neural_tissue::my_neural_tissue()
    : cellab::neural_tissue(
          2, //natural_16_bit const num_kinds_of_tissue_cells
          1, //natural_16_bit const num_kinds_of_sensory_cells
          10, //natural_16_bit const num_bits_per_cell,
          15, //natural_16_bit const num_bits_per_synapse,
          5, //natural_16_bit const num_bits_per_signalling,
          10, //natural_32_bit const num_cells_along_x_axis,
          10, //natural_32_bit const num_cells_along_y_axis,
          std::vector<natural_32_bit>({3,2}), //std::vector<natural_32_bit> const& num_tissue_cells_of_cell_kind,
          std::vector<natural_32_bit>({10,6}), //std::vector<natural_32_bit> const& num_synapses_in_territory_of_cell_kind,
          std::vector<natural_32_bit>({2}), //std::vector<natural_32_bit> const& num_sensory_cells_of_cell_kind,
          3, //natural_32_bit const num_synapses_to_muscles,
          true, //bool const is_x_axis_torus_axis,
          true, //bool const is_y_axis_torus_axis,
          true, //bool const is_columnar_axis_torus_axis,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_cell,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_cell,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_cell,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_synapse,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_synapse,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_synapse,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& x_radius_of_cellular_neighbourhood_of_signalling,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& y_radius_of_cellular_neighbourhood_of_signalling,
          std::vector<integer_8_bit>({1,1}), //std::vector<integer_8_bit> const& columnar_radius_of_cellular_neighbourhood_of_signalling,
          cellab::automated_binding_of_transition_functions<my_neural_tissue,my_cell,my_synapse,my_signalling>()
          )
{}

void  my_neural_tissue::transition_function_of_synapse_to_muscle(
        my_synapse& synapse_to_be_updated,
        cellab::kind_of_cell const kind_of_source_cell,
        my_cell const& source_cell
        )
{
    NOT_IMPLEMENTED_YET();
}

cellab::territorial_state_of_synapse  my_neural_tissue::transition_function_of_synapse_inside_tissue(
        my_synapse& synapse_to_be_updated,
        cellab::kind_of_cell const kind_of_source_cell,
        my_cell const& source_cell,
        cellab::kind_of_cell const kind_of_territory_cell,
        my_cell const& territory_cell,
        cellab::territorial_state_of_synapse const current_territorial_state_of_synapse,
        cellab::shift_in_coordinates const& shift_to_low_corner,
        cellab::shift_in_coordinates const& shift_to_high_corner,
        std::function<cellab::kind_of_cell(cellab::shift_in_coordinates const&,
                                           instance_wrapper<my_signalling const>&)> const&
            get_signalling
        )
{
    NOT_IMPLEMENTED_YET();
    //return current_territorial_state_of_synapse;
}

void my_neural_tissue::transition_function_of_signalling(
        my_signalling& signalling_to_be_updated,
        cellab::kind_of_cell kind_of_territory_cell,
        cellab::shift_in_coordinates const& shift_to_low_corner,
        cellab::shift_in_coordinates const& shift_to_high_corner,
        std::function<cellab::kind_of_cell(cellab::shift_in_coordinates const&,
                                           instance_wrapper<my_cell const>&)> const&
            get_cell
        )
{
    NOT_IMPLEMENTED_YET();
}

void my_neural_tissue::transition_function_of_cell(
        my_cell& cell_to_be_updated,
        cellab::kind_of_cell kind_of_cell_to_be_updated,
        natural_32_bit num_of_synapses_connected_to_the_cell,
        std::function<std::pair<cellab::kind_of_cell,cellab::kind_of_cell>(
                            natural_32_bit const index_of_synapse,
                            instance_wrapper<my_synapse const>&)> const&
            get_connected_synapse_at_index,
        cellab::shift_in_coordinates const& shift_to_low_corner,
        cellab::shift_in_coordinates const& shift_to_high_corner,
        std::function<cellab::kind_of_cell(cellab::shift_in_coordinates const&,
                                           instance_wrapper<my_signalling const>&)> const&
            get_signalling
        )
{
    NOT_IMPLEMENTED_YET();
}
