#include "./my_neural_tissue.hpp"
#include <utility/development.hpp>
#include <utility/random.hpp>
#include <utility/test.hpp>
#include <vector>


natural_16_bit  num_kinds_of_tissue_cells() noexcept { return 3U; }
natural_16_bit  num_kinds_of_sensory_cells() noexcept { return 5U; }

natural_16_bit  num_bits_per_cell() noexcept { return 8U * sizeof(my_cell); }
natural_16_bit  num_bits_per_synapse() noexcept { return 8U * sizeof(my_synapse); }
natural_16_bit  num_bits_per_signalling() noexcept { return 8U * sizeof(my_signalling); }

natural_32_bit  num_cells_along_x_axis() noexcept { return 31U; }
natural_32_bit  num_cells_along_y_axis() noexcept { return 32U; }

std::vector<natural_32_bit> const&  num_tissue_cells_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {7U,5U,3U};
    return v;
    }
std::vector<natural_32_bit> const&  num_synapses_in_territory_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {5U,4U,3U};
    return v;
    }
std::vector<natural_32_bit> const&  num_sensory_cells_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {500U,400U,100U,600U,1400U};
    return v;
    }

natural_32_bit  num_synapses_to_muscles() noexcept { return 3000U; }

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
{
    for (natural_32_bit x = 0U; x < get_static_state_of_neural_tissue()->num_cells_along_x_axis(); ++x)
        for (natural_32_bit y = 0U; y < get_static_state_of_neural_tissue()->num_cells_along_y_axis(); ++y)
            for (natural_32_bit c = 0U; c < get_static_state_of_neural_tissue()->num_cells_along_columnar_axis(); ++c)
            {
                value_to_bits(0U,get_dynamic_state_of_neural_tissue()->find_bits_of_cell_in_tissue(x,y,c));
                value_to_bits(0U,get_dynamic_state_of_neural_tissue()->find_bits_of_signalling(x,y,c));

                natural_32_bit const num_synapses =
                        get_static_state_of_neural_tissue()->num_synapses_in_territory_of_cell_kind(
                                get_static_state_of_neural_tissue()->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                );
                for (natural_32_bit s = 0U; s < num_synapses; ++s)
                {
                    value_to_bits(0U,get_dynamic_state_of_neural_tissue()->find_bits_of_synapse_in_tissue(x,y,c,s));

                    cellab::territorial_state_of_synapse const  territorial_state =
                            (cellab::territorial_state_of_synapse)get_random_natural_32_bit_in_range(
                                    cellab::territorial_state_of_synapse::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY,
                                    cellab::territorial_state_of_synapse::MIGRATION_ALONG_NEGATIVE_COLUMNAR_AXIS
                                    );
                    value_to_bits(territorial_state,get_dynamic_state_of_neural_tissue()->find_bits_of_territorial_state_of_synapse_in_tissue(x,y,c,s));

                    bits_reference bits_of_coords =
                            get_dynamic_state_of_neural_tissue()->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(x,y,c,s);
                    natural_16_bit const num_bits = get_dynamic_state_of_neural_tissue()->num_bits_per_source_cell_coordinate();
                    value_to_bits(
                            get_random_natural_32_bit_in_range(0U,get_static_state_of_neural_tissue()->num_cells_along_x_axis()-1U),
                            bits_of_coords,0U,num_bits);
                    value_to_bits(
                            get_random_natural_32_bit_in_range(0U,get_static_state_of_neural_tissue()->num_cells_along_y_axis()-1U),
                            bits_of_coords,num_bits,num_bits);
                    value_to_bits(
                            get_random_natural_32_bit_in_range(0U,get_static_state_of_neural_tissue()->num_cells_along_columnar_axis()+
                                                                  get_static_state_of_neural_tissue()->num_sensory_cells()-1U),
                            bits_of_coords,num_bits+num_bits,num_bits);

                    natural_8_bit const d0 =
                        cellab::convert_territorial_state_of_synapse_to_territorial_list_index(territorial_state);
                    for (natural_8_bit d = 0U; d < cellab::num_delimiters(); ++d)
                        if (d < d0)
                            value_to_bits(0U,get_dynamic_state_of_neural_tissue()->find_bits_of_delimiter_between_teritorial_lists(x,y,c,d));
                        else
                            value_to_bits(num_synapses,get_dynamic_state_of_neural_tissue()->find_bits_of_delimiter_between_teritorial_lists(x,y,c,d));
                }
            }

    for (natural_32_bit i = 0U; i < get_static_state_of_neural_tissue()->num_synapses_to_muscles(); ++i)
    {
        value_to_bits(0U,get_dynamic_state_of_neural_tissue()->find_bits_of_synapse_to_muscle(i));

        bits_reference bits_of_coords =
                get_dynamic_state_of_neural_tissue()->find_bits_of_coords_of_source_cell_of_synapse_to_muscle(i);
        natural_16_bit const num_bits = get_dynamic_state_of_neural_tissue()->num_bits_per_source_cell_coordinate();
        value_to_bits(
                get_random_natural_32_bit_in_range(0U,get_static_state_of_neural_tissue()->num_cells_along_x_axis()-1U),
                bits_of_coords,0U,num_bits);
        value_to_bits(
                get_random_natural_32_bit_in_range(0U,get_static_state_of_neural_tissue()->num_cells_along_y_axis()-1U),
                bits_of_coords,num_bits,num_bits);
        value_to_bits(
                get_random_natural_32_bit_in_range(0U,get_static_state_of_neural_tissue()->num_cells_along_columnar_axis()-1U),
                bits_of_coords,num_bits+num_bits,num_bits);
    }

    for (natural_32_bit i = 0U; i < get_static_state_of_neural_tissue()->num_sensory_cells(); ++i)
        value_to_bits(0U,get_dynamic_state_of_neural_tissue()->find_bits_of_sensory_cell(i));
}

void  my_neural_tissue::transition_function_of_synapse_to_muscle(
        my_synapse& synapse_to_be_updated,
        cellab::kind_of_cell,
        my_cell const& source_cell
        )
{
    synapse_to_be_updated.increment();
    TEST_SUCCESS(synapse_to_be_updated.count() == source_cell.count() + 1U);
}

cellab::territorial_state_of_synapse  my_neural_tissue::transition_function_of_synapse_inside_tissue(
        my_synapse& synapse_to_be_updated,
        cellab::kind_of_cell,
        my_cell const& source_cell,
        cellab::kind_of_cell,
        my_cell const& territory_cell,
        cellab::territorial_state_of_synapse,
        cellab::shift_in_coordinates const& shift_to_low_corner,
        cellab::shift_in_coordinates const& shift_to_high_corner,
        std::function<cellab::kind_of_cell(cellab::shift_in_coordinates const&,
                                           instance_wrapper<my_signalling const>&)> const&
            get_signalling
        )
{
    synapse_to_be_updated.increment();

    TEST_SUCCESS(synapse_to_be_updated.count() == source_cell.count() + 1U);
    TEST_SUCCESS(synapse_to_be_updated.count() == territory_cell.count() + 1U);

    instance_wrapper<my_signalling const> signalling;
    get_signalling(shift_to_low_corner,signalling);
    TEST_SUCCESS(synapse_to_be_updated.count() == signalling->count() + 1U);
    get_signalling(shift_to_high_corner,signalling);
    TEST_SUCCESS(synapse_to_be_updated.count() == signalling->count() + 1U);

    return (cellab::territorial_state_of_synapse)get_random_natural_32_bit_in_range(
                    cellab::territorial_state_of_synapse::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY,
                    cellab::territorial_state_of_synapse::MIGRATION_ALONG_NEGATIVE_COLUMNAR_AXIS
                    );
}

void my_neural_tissue::transition_function_of_signalling(
        my_signalling& signalling_to_be_updated,
        cellab::kind_of_cell,
        cellab::shift_in_coordinates const& shift_to_low_corner,
        cellab::shift_in_coordinates const& shift_to_high_corner,
        std::function<cellab::kind_of_cell(cellab::shift_in_coordinates const&,
                                           instance_wrapper<my_cell const>&)> const&
            get_cell
        )
{
    signalling_to_be_updated.increment();

    instance_wrapper<my_cell const> cell;
    get_cell(shift_to_low_corner,cell);
    TEST_SUCCESS(signalling_to_be_updated.count() == cell->count() + 1U);
    get_cell(shift_to_high_corner,cell);
    TEST_SUCCESS(signalling_to_be_updated.count() == cell->count() + 1U);
}

void my_neural_tissue::transition_function_of_cell(
        my_cell& cell_to_be_updated,
        cellab::kind_of_cell,
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
    cell_to_be_updated.increment();

    for (natural_32_bit i = 0U; i < num_of_synapses_connected_to_the_cell; ++i)
    {
        instance_wrapper<my_synapse const> synapse;
        get_connected_synapse_at_index(i,synapse);
        TEST_SUCCESS(cell_to_be_updated.count() == synapse->count());
    }

    instance_wrapper<my_signalling const> signalling;
    get_signalling(shift_to_low_corner,signalling);
    TEST_SUCCESS(cell_to_be_updated.count() == signalling->count());

    get_signalling(shift_to_high_corner,signalling);
    TEST_SUCCESS(cell_to_be_updated.count() == signalling->count());
}
