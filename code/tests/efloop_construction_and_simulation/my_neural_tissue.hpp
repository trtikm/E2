#ifndef E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_NEURAL_TISSUE_HPP_INCLUDED
#   define E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_NEURAL_TISSUE_HPP_INCLUDED

#   include "./my_cell.hpp"
#   include "./my_synapse.hpp"
#   include "./my_signalling.hpp"
#   include <cellab/neural_tissue.hpp>
#   include <utility/instance_wrapper.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <functional>
#   include <tuple>


struct my_neural_tissue : public cellab::neural_tissue
{
    my_neural_tissue();

    void  transition_function_of_synapse_to_muscle(
            my_synapse& synapse_to_be_updated,
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


#endif
