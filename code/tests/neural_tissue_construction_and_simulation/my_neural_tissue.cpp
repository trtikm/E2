#include "./my_neural_tissue.hpp"
#include <utility/development.hpp>
#include <utility/test.hpp>
#include <utility/random.hpp>


natural_16_bit  num_kinds_of_tissue_cells() noexcept { return 3U; }
natural_16_bit  num_kinds_of_sensory_cells() noexcept { return 2U; }

natural_16_bit  num_bits_per_cell() noexcept { return 8U * sizeof(my_cell); }
natural_16_bit  num_bits_per_synapse() noexcept { return 8U * sizeof(my_synapse); }
natural_16_bit  num_bits_per_signalling() noexcept { return 8U * sizeof(my_signalling); }

//natural_32_bit  num_cells_along_x_axis() noexcept { return 101U; }
//natural_32_bit  num_cells_along_y_axis() noexcept { return 51U; }
natural_32_bit  num_cells_along_x_axis() noexcept { return 21U; }
natural_32_bit  num_cells_along_y_axis() noexcept { return 31U; }

std::vector<natural_32_bit> const&  num_tissue_cells_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {8U,6U,4U};
    return v;
    }
std::vector<natural_32_bit> const&  num_synapses_in_territory_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {21U,11U,7U};
    return v;
    }
std::vector<natural_32_bit> const&  num_sensory_cells_of_cell_kind() noexcept {
    static std::vector<natural_32_bit> v = {51U,31U};
    return v;
    }

natural_32_bit  num_synapses_to_muscles() noexcept { return 111U; }

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

my_neural_tissue::my_neural_tissue(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> static_state_of_tissue
        )
    : cellab::neural_tissue(static_state_of_tissue,get_automated_binding_of_transition_functions())
{}

my_neural_tissue::my_neural_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_state_of_tissue
        )
    : cellab::neural_tissue(dynamic_state_of_tissue,get_automated_binding_of_transition_functions())
{}

my_neural_tissue::my_neural_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_state_of_tissue,
        bool
        )
    : cellab::neural_tissue(
            dynamic_state_of_tissue,
            transition_function_of_packed_synapse_to_muscle,
            transition_function_of_packed_synapse_inside_tissue,
            transition_function_of_packed_signalling,
            transition_function_of_packed_cell
            )
{}

void  my_neural_tissue::transition_function_of_synapse_to_muscle(
        my_synapse& synapse_to_be_updated,
        cellab::kind_of_cell const,
        my_cell const& source_cell
        )
{
    synapse_to_be_updated.increment();
    TEST_SUCCESS(synapse_to_be_updated.count() == source_cell.count() + 1U);
}

void  my_neural_tissue::transition_function_of_packed_synapse_to_muscle(
        bits_reference& bits_of_synapse_to_be_updated,
        cellab::kind_of_cell,
        bits_const_reference const& bits_of_source_cell
        )
{
    my_synapse  synapse_to_be_updated(bits_of_synapse_to_be_updated);
    synapse_to_be_updated.increment();
    synapse_to_be_updated >> bits_of_synapse_to_be_updated;

    my_cell const  source_cell(bits_of_source_cell);
    TEST_SUCCESS(synapse_to_be_updated.count() == source_cell.count() + 1U);
}

cellab::territorial_state_of_synapse  my_neural_tissue::transition_function_of_synapse_inside_tissue(
        my_synapse& synapse_to_be_updated,
        cellab::kind_of_cell const,
        my_cell const& source_cell,
        cellab::kind_of_cell const,
        my_cell const& territory_cell,
        cellab::territorial_state_of_synapse const,
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

cellab::territorial_state_of_synapse  my_neural_tissue::transition_function_of_packed_synapse_inside_tissue(
        bits_reference& bits_of_synapse_to_be_updated,
        cellab::kind_of_cell,
        bits_const_reference const& bits_of_source_cell,
        cellab::kind_of_cell,
        bits_const_reference const& bits_of_territory_cell,
        cellab::territorial_state_of_synapse,
        cellab::shift_in_coordinates const& shift_to_low_corner,
        cellab::shift_in_coordinates const& shift_to_high_corner,
        std::function<std::pair<bits_const_reference,cellab::kind_of_cell>(
                                        cellab::shift_in_coordinates const&)> const&
            get_signalling
        )
{
    my_synapse  synapse_to_be_updated(bits_of_synapse_to_be_updated);
    synapse_to_be_updated.increment();
    synapse_to_be_updated >> bits_of_synapse_to_be_updated;

    my_cell const  src_cell(bits_of_source_cell);
    TEST_SUCCESS(synapse_to_be_updated.count() == src_cell.count() + 1U);

    my_cell const  territory_cell(bits_of_territory_cell);
    TEST_SUCCESS(synapse_to_be_updated.count() == territory_cell.count() + 1U);

    my_signalling const  territory_low_corner(get_signalling(shift_to_low_corner).first);
    TEST_SUCCESS(synapse_to_be_updated.count() == territory_low_corner.count() + 1U);

    my_signalling const  territory_high_corner(get_signalling(shift_to_high_corner).first);
    TEST_SUCCESS(synapse_to_be_updated.count() == territory_high_corner.count() + 1U);

    return (cellab::territorial_state_of_synapse)get_random_natural_32_bit_in_range(
                    cellab::territorial_state_of_synapse::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY,
                    cellab::territorial_state_of_synapse::MIGRATION_ALONG_NEGATIVE_COLUMNAR_AXIS
                    );
}

void  my_neural_tissue::transition_function_of_signalling(
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

void  my_neural_tissue::transition_function_of_packed_signalling(
        bits_reference& bits_of_signalling_data_to_be_updated,
        cellab::kind_of_cell,
        cellab::shift_in_coordinates const& shift_to_low_corner,
        cellab::shift_in_coordinates const& shift_to_high_corner,
        std::function<std::pair<bits_const_reference,cellab::kind_of_cell>(
                                        cellab::shift_in_coordinates const&)> const&
            get_cell
        )
{
    my_signalling  signalling_to_be_updated(bits_of_signalling_data_to_be_updated);
    signalling_to_be_updated.increment();
    signalling_to_be_updated >> bits_of_signalling_data_to_be_updated;

    my_cell const  low_corner_cell(get_cell(shift_to_low_corner).first);
    TEST_SUCCESS(signalling_to_be_updated.count() == low_corner_cell.count() + 1U);

    my_cell const  high_corner_cell(get_cell(shift_to_high_corner).first);
    TEST_SUCCESS(signalling_to_be_updated.count() == high_corner_cell.count() + 1U);
}

void  my_neural_tissue::transition_function_of_cell(
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

void  my_neural_tissue::transition_function_of_packed_cell(
        bits_reference& bits_of_cell_to_be_updated,
        cellab::kind_of_cell,
        natural_32_bit num_of_synapses_connected_to_the_cell,
        std::function<std::tuple<bits_const_reference,cellab::kind_of_cell,cellab::kind_of_cell>(
                                natural_32_bit)> const&
            get_connected_synapse_at_index,
        cellab::shift_in_coordinates const& shift_to_low_corner,
        cellab::shift_in_coordinates const& shift_to_high_corner,
        std::function<std::pair<bits_const_reference,cellab::kind_of_cell>(
                                cellab::shift_in_coordinates const&)> const&
            get_signalling
        )
{
    my_cell  cell_to_be_updated(bits_of_cell_to_be_updated);
    cell_to_be_updated.increment();
    cell_to_be_updated >> bits_of_cell_to_be_updated;

    for (natural_32_bit i = 0U; i < num_of_synapses_connected_to_the_cell; ++i)
    {
        my_synapse const  synapse(std::get<0>(get_connected_synapse_at_index(i)));
        TEST_SUCCESS(cell_to_be_updated.count() == synapse.count());
    }

    my_signalling const  low_corner(get_signalling(shift_to_low_corner).first);
    TEST_SUCCESS(cell_to_be_updated.count() == low_corner.count());

    my_signalling const  high_corner(get_signalling(shift_to_high_corner).first);
    TEST_SUCCESS(cell_to_be_updated.count() == high_corner.count());
}
