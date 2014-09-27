#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>
#include <memory>

typedef std::vector<std::shared_ptr<cellab::static_state_of_neural_tissue> > static_states_vector;

static_states_vector const&  get_static_states()
{
    static static_states_vector static_states = {
        std::shared_ptr<cellab::static_state_of_neural_tissue>(new cellab::static_state_of_neural_tissue(
                2U, //natural_16_bit const num_kinds_of_tissue_cells
                1U, //natural_16_bit const num_kinds_of_sensory_cells
                10U, //natural_16_bit const num_bits_per_cell,
                15U, //natural_16_bit const num_bits_per_synapse,
                5U, //natural_16_bit const num_bits_per_signalling,
                10U, //natural_32_bit const num_cells_along_x_axis,
                10U, //natural_32_bit const num_cells_along_y_axis,
                std::vector<natural_32_bit>({3U,2U}), //std::vector<natural_32_bit> const& num_tissue_cells_of_cell_kind,
                std::vector<natural_32_bit>({10U,6U}), //std::vector<natural_32_bit> const& num_synapses_in_territory_of_cell_kind,
                std::vector<natural_32_bit>({2U}), //std::vector<natural_32_bit> const& num_sensory_cells_of_cell_kind,
                3U, //natural_32_bit const num_synapses_to_muscles,
                true, //bool const is_x_axis_torus_axis,
                true, //bool const is_y_axis_torus_axis,
                true, //bool const is_columnar_axis_torus_axis,
                std::vector<integer_8_bit>({1U,1U}), //std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_cell,
                std::vector<integer_8_bit>({1U,1U}), //std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_cell,
                std::vector<integer_8_bit>({1U,1U}), //std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_cell,
                std::vector<integer_8_bit>({1U,1U}), //std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_synapse,
                std::vector<integer_8_bit>({1U,1U}), //std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_synapse,
                std::vector<integer_8_bit>({1U,1U}), //std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_synapse,
                std::vector<integer_8_bit>({1U,1U}), //std::vector<integer_8_bit> const& x_radius_of_cellular_neighbourhood_of_signalling,
                std::vector<integer_8_bit>({1U,1U}), //std::vector<integer_8_bit> const& y_radius_of_cellular_neighbourhood_of_signalling,
                std::vector<integer_8_bit>({1U,1U})  //std::vector<integer_8_bit> const& columnar_radius_of_cellular_neighbourhood_of_signalling,
                )),
    };
    return static_states;
}

void run()
{
    TMPROF_BLOCK();

    for (static_states_vector::const_iterator it = get_static_states().begin();
            it != get_static_states().end(); ++it)
    {
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_state =
                std::shared_ptr<cellab::dynamic_state_of_neural_tissue>(
                        new cellab::dynamic_state_of_neural_tissue(*it)
                        );

    }

    TEST_PRINT_STATISTICS();
}
