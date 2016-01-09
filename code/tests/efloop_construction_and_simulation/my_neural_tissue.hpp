#ifndef E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_NEURAL_TISSUE_HPP_INCLUDED
#   define E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_NEURAL_TISSUE_HPP_INCLUDED

#   include "./my_cell.hpp"
#   include "./my_synapse.hpp"
#   include "./my_signalling.hpp"
#   include <cellab/neural_tissue.hpp>
#   include <utility/instance_wrapper.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <functional>
#   include <vector>
#   include <tuple>


struct my_neural_tissue : public cellab::neural_tissue
{
    my_neural_tissue();

    void  transition_function_of_synapse_to_muscle(
            my_synapse& synapse_to_be_updated,
            cellab::kind_of_synapse_to_muscle const kind_of_synapse_to_muscle_to_be_updated,
            cellab::kind_of_cell const kind_of_source_cell,
            my_cell const& source_cell
            );

    cellab::territorial_state_of_synapse  transition_function_of_synapse_inside_tissue(
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
            );

    void transition_function_of_signalling(
            my_signalling& signalling_to_be_updated,
            cellab::kind_of_cell kind_of_territory_cell,
            cellab::shift_in_coordinates const& shift_to_low_corner,
            cellab::shift_in_coordinates const& shift_to_high_corner,
            std::function<cellab::kind_of_cell(cellab::shift_in_coordinates const&,
                                               instance_wrapper<my_cell const>&)> const&
                get_cell
            );

    void transition_function_of_cell(
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
            );
};


natural_16_bit  num_kinds_of_tissue_cells() noexcept;
natural_16_bit  num_kinds_of_sensory_cells() noexcept;
natural_16_bit  num_kinds_of_synapses_to_myscles() noexcept;

natural_16_bit  num_bits_per_cell() noexcept;
natural_16_bit  num_bits_per_synapse() noexcept;
natural_16_bit  num_bits_per_signalling() noexcept;

natural_32_bit  num_cells_along_x_axis() noexcept;
natural_32_bit  num_cells_along_y_axis() noexcept;

std::vector<natural_32_bit> const&  num_tissue_cells_of_cell_kind() noexcept;
std::vector<natural_32_bit> const&  num_synapses_in_territory_of_cell_kind() noexcept;
std::vector<natural_32_bit> const&  num_sensory_cells_of_cell_kind() noexcept;
std::vector<natural_32_bit> const&  num_synapses_to_muscles_of_kind() noexcept;

bool  is_x_axis_torus_axis() noexcept;
bool  is_y_axis_torus_axis() noexcept;
bool  is_columnar_axis_torus_axis() noexcept;

std::vector<integer_8_bit> const&  x_radius_of_signalling_neighbourhood_of_cell() noexcept;
std::vector<integer_8_bit> const&  y_radius_of_signalling_neighbourhood_of_cell() noexcept;
std::vector<integer_8_bit> const&  columnar_radius_of_signalling_neighbourhood_of_cell() noexcept;

std::vector<integer_8_bit> const&  x_radius_of_signalling_neighbourhood_of_synapse() noexcept;
std::vector<integer_8_bit> const&  y_radius_of_signalling_neighbourhood_of_synapse() noexcept;
std::vector<integer_8_bit> const&  columnar_radius_of_signalling_neighbourhood_of_synapse() noexcept;

std::vector<integer_8_bit> const&  x_radius_of_cellular_neighbourhood_of_signalling() noexcept;
std::vector<integer_8_bit> const&  y_radius_of_cellular_neighbourhood_of_signalling() noexcept;
std::vector<integer_8_bit> const&  columnar_radius_of_cellular_neighbourhood_of_signalling() noexcept;

cellab::automated_binding_of_transition_functions<my_neural_tissue,my_cell,my_synapse,my_signalling>
get_automated_binding_of_transition_functions() noexcept;


#endif
