#include "./my_neural_tissue.hpp"
#include <utility/development.hpp>
#include <vector>


natural_16_bit  num_kinds_of_tissue_cells() noexcept { return 3U; }
natural_16_bit  num_kinds_of_sensory_cells() noexcept { return 2U; }

natural_16_bit  num_bits_per_cell() noexcept { return sizeof(my_cell); }
natural_16_bit  num_bits_per_synapse() noexcept { return sizeof(my_synapse); }
natural_16_bit  num_bits_per_signalling() noexcept { return sizeof(my_signalling); }

natural_32_bit  num_cells_along_x_axis() noexcept { return 100U; }
natural_32_bit  num_cells_along_y_axis() noexcept { return 50U; }

std::vector<natural_32_bit> const&  num_tissue_cells_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {7U,5U,3U};
    return v;
    }
std::vector<natural_32_bit> const&  num_synapses_in_territory_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {20U,10U,5U};
    return v;
    }
std::vector<natural_32_bit> const&  num_sensory_cells_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {50U,30U};
    return v;
    }

natural_32_bit  num_synapses_to_muscles() noexcept { return 100U; }

bool  is_x_axis_torus_axis() noexcept { return true; }
bool  is_y_axis_torus_axis() noexcept { return false; }
bool  is_columnar_axis_torus_axis() noexcept { return true; }

std::vector<integer_8_bit> const&  x_radius_of_signalling_neighbourhood_of_cell() noexcept {
    static std::vector<integer_8_bit> v = {2,1,1};
    return v;
    }
std::vector<integer_8_bit> const&  y_radius_of_signalling_neighbourhood_of_cell() noexcept {
    static std::vector<integer_8_bit> v = {1,2,1};
    return v;
    }
std::vector<integer_8_bit> const&  columnar_radius_of_signalling_neighbourhood_of_cell() noexcept {
    static std::vector<integer_8_bit> v = {1,1,2};
    return v;
    }

std::vector<integer_8_bit> const&  x_radius_of_signalling_neighbourhood_of_synapse() noexcept {
    static std::vector<integer_8_bit> v = {2,1,1};
    return v;
    }
std::vector<integer_8_bit> const&  y_radius_of_signalling_neighbourhood_of_synapse() noexcept {
    static std::vector<integer_8_bit> v = {1,2,1};
    return v;
    }
std::vector<integer_8_bit> const&  columnar_radius_of_signalling_neighbourhood_of_synapse() noexcept {
    static std::vector<integer_8_bit> v = {1,1,2};
    return v;
    }

std::vector<integer_8_bit> const&  x_radius_of_cellular_neighbourhood_of_signalling() noexcept {
    static std::vector<integer_8_bit> v = {2,1,1};
    return v;
    }
std::vector<integer_8_bit> const&  y_radius_of_cellular_neighbourhood_of_signalling() noexcept {
    static std::vector<integer_8_bit> v = {1,2,1};
    return v;
    }
std::vector<integer_8_bit> const&  columnar_radius_of_cellular_neighbourhood_of_signalling() noexcept {
    static std::vector<integer_8_bit> v = {1,1,2};
    return v;
    }

cellab::automated_binding_of_transition_functions<my_neural_tissue,my_cell,my_synapse,my_signalling>
get_automated_binding_of_transition_functions() noexcept {
    return cellab::automated_binding_of_transition_functions<my_neural_tissue,my_cell,my_synapse,my_signalling>();
    }


my_neural_tissue::my_neural_tissue()
    : cellab::neural_tissue(
          num_kinds_of_tissue_cells(),
          num_kinds_of_sensory_cells(),
          num_bits_per_cell(),
          num_bits_per_synapse(),
          num_bits_per_signalling(),
          num_cells_along_x_axis(),
          num_cells_along_y_axis(),
          num_tissue_cells_of_cell_kind(),
          num_synapses_in_territory_of_cell_kind(),
          num_sensory_cells_of_cell_kind(),
          num_synapses_to_muscles(),
          is_x_axis_torus_axis(),
          is_y_axis_torus_axis(),
          is_columnar_axis_torus_axis(),
          x_radius_of_signalling_neighbourhood_of_cell(),
          y_radius_of_signalling_neighbourhood_of_cell(),
          columnar_radius_of_signalling_neighbourhood_of_cell(),
          x_radius_of_signalling_neighbourhood_of_synapse(),
          y_radius_of_signalling_neighbourhood_of_synapse(),
          columnar_radius_of_signalling_neighbourhood_of_synapse(),
          x_radius_of_cellular_neighbourhood_of_signalling(),
          y_radius_of_cellular_neighbourhood_of_signalling(),
          columnar_radius_of_cellular_neighbourhood_of_signalling(),
          get_automated_binding_of_transition_functions()
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
