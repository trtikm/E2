#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/transition_algorithms.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/random.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>


struct tissue_element
{
    typedef natural_32_bit counter_type;
    tissue_element() : m_counter(0U) {}
    explicit tissue_element(counter_type const initval) : m_counter(initval) {}
    explicit tissue_element(bits_const_reference const& bits)
        : m_counter(bits_to_value<natural_32_bit>(bits))
    {}
    void operator>>(bits_reference const& bits) const
    { value_to_bits(count(),bits); }
    counter_type count() const { return m_counter; }
    tissue_element operator++() { ++m_counter; return *this; }
    static natural_32_bit num_bits() { return 8U * sizeof(counter_type); }
private:
    counter_type m_counter;
};

bool operator==(tissue_element const e1, tissue_element const e2) { return e1.count() == e2.count(); }



static void test_tissue(std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue,
                        tissue_element const cell_counter,
                        tissue_element const synapse_counter,
                        cellab::territorial_state_of_synapse const territorial_state,
                        tissue_element const signalling_counter,
                        tissue_element const sensory_cell_counter,
                        tissue_element const synapse_to_muscle_counter)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
        for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
            for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
            {
                TEST_SUCCESS(cell_counter == tissue_element(dynamic_tissue->find_bits_of_cell_in_tissue(x,y,c)));
                TEST_SUCCESS(signalling_counter == tissue_element(dynamic_tissue->find_bits_of_signalling(x,y,c)));
                natural_32_bit const num_synapses =
                        static_tissue->num_synapses_in_territory_of_cell_kind(
                                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                );
                for (natural_32_bit s = 0U; s < num_synapses; ++s)
                {
                    TEST_SUCCESS(synapse_counter == tissue_element(dynamic_tissue->find_bits_of_synapse_in_tissue(x,y,c,s)));
                    TEST_SUCCESS(territorial_state == bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(x,y,c,s)));
                    natural_8_bit const d0 =
                        cellab::convert_territorial_state_of_synapse_to_territorial_list_index(territorial_state);
                    TEST_SUCCESS(num_synapses == bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_delimiter_between_teritorial_lists(x,y,c,d0)));
                    for (natural_8_bit d = 0U; d < cellab::num_delimiters(); ++d)
                    {
                        natural_32_bit const  index =
                                bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_delimiter_between_teritorial_lists(x,y,c,d));
                        TEST_SUCCESS(d >= d0 || index == 0U);
                        TEST_SUCCESS(d < d0 || index == num_synapses);
                    }
                }
            }
    for (natural_32_bit i = 0U; i < static_tissue->num_sensory_cells(); ++i)
        TEST_SUCCESS(sensory_cell_counter == tissue_element(dynamic_tissue->find_bits_of_sensory_cell(i)));
    for (natural_32_bit i = 0U; i < static_tissue->num_synapses_to_muscles(); ++i)
        TEST_SUCCESS(synapse_to_muscle_counter == tissue_element(dynamic_tissue->find_bits_of_synapse_to_muscle(i)));
}



static void initialse_tissue(std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    tissue_element const element0;
    cellab::territorial_state_of_synapse const territorial_state =
            cellab::territorial_state_of_synapse::MIGRATION_ALONG_POSITIVE_X_AXIS;
    for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
        for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
            for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
            {
                element0 >> dynamic_tissue->find_bits_of_cell_in_tissue(x,y,c);
                element0 >> dynamic_tissue->find_bits_of_signalling(x,y,c);

                natural_32_bit const num_synapses =
                        static_tissue->num_synapses_in_territory_of_cell_kind(
                                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                );
                for (natural_32_bit s = 0U; s < num_synapses; ++s)
                {
                    element0 >> dynamic_tissue->find_bits_of_synapse_in_tissue(x,y,c,s);
                    value_to_bits(territorial_state,dynamic_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(x,y,c,s));

                    bits_reference bits_of_coords =
                            dynamic_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(x,y,c,s);
                    natural_16_bit const num_bits = dynamic_tissue->num_bits_per_source_cell_coordinate();
                    value_to_bits(
                            get_random_natural_32_bit_in_range(0U,static_tissue->num_cells_along_x_axis()-1U),
                            bits_of_coords,0U,num_bits);
                    value_to_bits(
                            get_random_natural_32_bit_in_range(0U,static_tissue->num_cells_along_y_axis()-1U),
                            bits_of_coords,num_bits,num_bits);
                    value_to_bits(
                            get_random_natural_32_bit_in_range(0U,static_tissue->num_cells_along_columnar_axis()-1U),
                            bits_of_coords,num_bits+num_bits,num_bits);

                    natural_8_bit const d0 =
                        cellab::convert_territorial_state_of_synapse_to_territorial_list_index(territorial_state);
                    for (natural_8_bit d = 0U; d < cellab::num_delimiters(); ++d)
                        if (d < d0)
                            value_to_bits(0U,dynamic_tissue->find_bits_of_delimiter_between_teritorial_lists(x,y,c,d));
                        else
                            value_to_bits(num_synapses,dynamic_tissue->find_bits_of_delimiter_between_teritorial_lists(x,y,c,d));
                }
            }

    for (natural_32_bit i = 0U; i < static_tissue->num_sensory_cells(); ++i)
        element0 >> dynamic_tissue->find_bits_of_sensory_cell(i);

    for (natural_32_bit i = 0U; i < static_tissue->num_synapses_to_muscles(); ++i)
    {
        element0 >> dynamic_tissue->find_bits_of_synapse_to_muscle(i);

        bits_reference bits_of_coords =
                dynamic_tissue->find_bits_of_coords_of_source_cell_of_synapse_to_muscle(i);
        natural_16_bit const num_bits = dynamic_tissue->num_bits_per_source_cell_coordinate();
        value_to_bits(
                get_random_natural_32_bit_in_range(0U,static_tissue->num_cells_along_x_axis()-1U),
                bits_of_coords,0U,num_bits);
        value_to_bits(
                get_random_natural_32_bit_in_range(0U,static_tissue->num_cells_along_y_axis()-1U),
                bits_of_coords,num_bits,num_bits);
        value_to_bits(
                get_random_natural_32_bit_in_range(0U,static_tissue->num_cells_along_columnar_axis()-1U),
                bits_of_coords,num_bits+num_bits,num_bits);
    }

    test_tissue(dynamic_tissue,
                element0,element0,
                territorial_state,
                element0,element0,element0
                );
}



void callback_transition_of_synapses_to_muscles(
            bits_reference& bits_of_synapse_to_be_updated,
            cellab::kind_of_cell,
            bits_const_reference const& bits_of_source_cell)
{
    tissue_element elemement(bits_of_synapse_to_be_updated);
    ++elemement;
    elemement >> bits_of_synapse_to_be_updated;

    tissue_element src_cell_elemement(bits_of_source_cell);
    TEST_SUCCESS(elemement.count() == src_cell_elemement.count() + 1U);
}

cellab::territorial_state_of_synapse callback_transition_of_synapses_of_tissue(
    bits_reference& bits_of_synapse_to_be_updated,
    cellab::kind_of_cell,
    bits_const_reference const& bits_of_source_cell,
    cellab::kind_of_cell,
    bits_const_reference const& bits_of_territory_cell,
    cellab::territorial_state_of_synapse territorial_state,
    cellab::shift_in_coordinates const& shift_to_low_corner,
    cellab::shift_in_coordinates const& shift_to_high_corner,
    std::function<std::pair<bits_const_reference,cellab::kind_of_cell>(cellab::shift_in_coordinates const&)> const&
        get_signalling
    )
{
    tissue_element elemement(bits_of_synapse_to_be_updated);
    ++elemement;
    elemement >> bits_of_synapse_to_be_updated;

    tissue_element src_cell_elemement(bits_of_source_cell);
    TEST_SUCCESS(elemement.count() == src_cell_elemement.count() + 1U);

    tissue_element territory_cell_element(bits_of_source_cell);
    TEST_SUCCESS(elemement.count() == territory_cell_element.count() + 1U);

    tissue_element territory_low_corner_elemement(get_signalling(shift_to_low_corner).first);
    TEST_SUCCESS(elemement.count() == territory_low_corner_elemement.count() + 1U);

    tissue_element territory_high_corner_elemement(get_signalling(shift_to_high_corner).first);
    TEST_SUCCESS(elemement.count() == territory_high_corner_elemement.count() + 1U);

    return territorial_state;
}

void callback_transition_function_of_signalling(
    bits_reference& bits_of_signalling_data_to_be_updated,
    cellab::kind_of_cell,
    cellab::shift_in_coordinates const& shift_to_low_corner,
    cellab::shift_in_coordinates const& shift_to_high_corner,
    std::function<std::pair<bits_const_reference,cellab::kind_of_cell>(cellab::shift_in_coordinates const&)> const&
        get_cell
    )
{
    tissue_element elemement(bits_of_signalling_data_to_be_updated);
    ++elemement;
    elemement >> bits_of_signalling_data_to_be_updated;

    tissue_element low_corner_cell_elemement(get_cell(shift_to_low_corner).first);
    TEST_SUCCESS(elemement.count() == low_corner_cell_elemement.count() + 1U);

    tissue_element high_corner_cell_elemement(get_cell(shift_to_high_corner).first);
    TEST_SUCCESS(elemement.count() == high_corner_cell_elemement.count() + 1U);
}

void callback_transition_function_of_cell(
    bits_reference& bits_of_cell_to_be_updated,
    cellab::kind_of_cell,
    natural_32_bit num_of_synapses_connected_to_the_cell,
    std::function<std::tuple<bits_const_reference,cellab::kind_of_cell,cellab::kind_of_cell>(natural_32_bit)> const&,
    cellab::shift_in_coordinates const& shift_to_low_corner,
    cellab::shift_in_coordinates const& shift_to_high_corner,
    std::function<std::pair<bits_const_reference,cellab::kind_of_cell>(cellab::shift_in_coordinates const&)> const&
        get_signalling
    )
{
    tissue_element elemement(bits_of_cell_to_be_updated);
    ++elemement;
    elemement >> bits_of_cell_to_be_updated;

    TEST_SUCCESS(num_of_synapses_connected_to_the_cell == 0U);

    tissue_element low_corner_elemement(get_signalling(shift_to_low_corner).first);
    TEST_SUCCESS(elemement.count() == low_corner_elemement.count());

    tissue_element high_corner_elemement(get_signalling(shift_to_high_corner).first);
    TEST_SUCCESS(elemement.count() == high_corner_elemement.count());
}

static void test_algorithms(std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue,
                            natural_32_bit const  num_avalilable_threads,
                            tissue_element& cell_counter,
                            tissue_element& synapse_counter,
                            tissue_element& signalling_counter,
                            tissue_element& sensory_cell_counter,
                            tissue_element& synapse_to_muscle_counter
                            )
{
    cellab::territorial_state_of_synapse const territorial_state =
            cellab::territorial_state_of_synapse::MIGRATION_ALONG_POSITIVE_X_AXIS;

    for (natural_32_bit i = 0U; i < 10U; ++i)
    {
        cellab::apply_transition_of_synapses_to_muscles(
                    dynamic_tissue,
                    &callback_transition_of_synapses_to_muscles,
                    num_avalilable_threads);

        test_tissue(dynamic_tissue,
                    cell_counter,
                    synapse_counter,
                    territorial_state,
                    signalling_counter,
                    sensory_cell_counter,
                    ++synapse_to_muscle_counter);

        cellab::apply_transition_of_synapses_of_tissue(
                    dynamic_tissue,
                    &callback_transition_of_synapses_of_tissue,
                    num_avalilable_threads);

        test_tissue(dynamic_tissue,
                    cell_counter,
                    ++synapse_counter,
                    territorial_state,
                    signalling_counter,
                    sensory_cell_counter,
                    synapse_to_muscle_counter);

        cellab::apply_transition_of_territorial_lists_of_synapses(
                    dynamic_tissue,
                    num_avalilable_threads);
        cellab::apply_transition_of_synaptic_migration_in_tissue(
                    dynamic_tissue,
                    num_avalilable_threads);

        test_tissue(dynamic_tissue,
                    cell_counter,
                    synapse_counter,
                    territorial_state,
                    signalling_counter,
                    sensory_cell_counter,
                    synapse_to_muscle_counter);

        cellab::apply_transition_of_signalling_in_tissue(
                    dynamic_tissue,
                    &callback_transition_function_of_signalling,
                    num_avalilable_threads);

        test_tissue(dynamic_tissue,
                    cell_counter,
                    synapse_counter,
                    territorial_state,
                    ++signalling_counter,
                    sensory_cell_counter,
                    synapse_to_muscle_counter);

        cellab::apply_transition_of_cells_of_tissue(
                    dynamic_tissue,
                    &callback_transition_function_of_cell,
                    num_avalilable_threads);

        test_tissue(dynamic_tissue,
                    ++cell_counter,
                    synapse_counter,
                    territorial_state,
                    signalling_counter,
                    sensory_cell_counter,
                    synapse_to_muscle_counter);

        TEST_PROGRESS_UPDATE();
    }
}

static void test_algorithms(std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue)
{
    tissue_element cell_counter;
    tissue_element synapse_counter;
    tissue_element signalling_counter;
    tissue_element sensory_cell_counter;
    tissue_element synapse_to_muscle_counter;
    test_algorithms(dynamic_tissue,1U,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter);
    test_algorithms(dynamic_tissue,2U,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter);
    test_algorithms(dynamic_tissue,4U,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter);
    for (natural_32_bit threads = 8U; threads <= 64U; threads += 8U)
        test_algorithms(dynamic_tissue,threads,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter);
}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    for (natural_16_bit tissue_cell_kinds = 1U; tissue_cell_kinds < 6U; ++tissue_cell_kinds)
    {
        for (natural_16_bit sensory_cell_kinds = 1U; sensory_cell_kinds < 3U; ++sensory_cell_kinds)
        {
            natural_16_bit const num_bits_per_cell = tissue_element::num_bits();
            natural_16_bit const num_bits_per_synapse = tissue_element::num_bits();
            natural_16_bit const num_bits_per_signalling = tissue_element::num_bits();

            for (natural_32_bit cells_x = 1U; cells_x < 302U; cells_x += 100U)
            {
                for (natural_32_bit cells_y = 1U; cells_y < 302U; cells_y += 100U)
                {
                    natural_32_bit cells_c = 0U;
                    std::vector<natural_32_bit> num_tissue_cells_of_cell_kind;
                    std::vector<natural_32_bit> num_synapses_in_territory_of_cell_kind;
                    for (natural_16_bit i = 1U; i <= tissue_cell_kinds; ++i)
                    {
                        cells_c += i;
                        num_tissue_cells_of_cell_kind.push_back(i);
                        num_synapses_in_territory_of_cell_kind.push_back(10U+i);
                    }

                    std::vector<natural_32_bit> num_sensory_cells_of_cell_kind;
                    for (natural_16_bit i = 0U; i < sensory_cell_kinds; ++i)
                        num_sensory_cells_of_cell_kind.push_back((i%2 == 0) ? 1U : 150U);

                    natural_32_bit const num_synapses_to_muscles = 200U;

                    for (natural_8_bit torus_x = 0U; torus_x < 2U; ++torus_x)
                        for (natural_8_bit torus_y = 0U; torus_y < 2U; ++torus_y)
                            for (natural_8_bit torus_c = 0U; torus_c < 2U; ++torus_c)
                            {
                                bool const is_x_axis_torus_axis = torus_x == 1;
                                bool const is_y_axis_torus_axis = torus_y == 1;
                                bool const is_c_axis_torus_axis = torus_c == 1;

                                for (natural_8_bit radius = 1U; radius < 3U; ++radius)
                                {
                                    natural_8_bit radius_x = std::min((natural_32_bit)radius,cells_x);
                                    natural_8_bit radius_y = std::min((natural_32_bit)radius,cells_y);
                                    natural_8_bit radius_c = std::min((natural_32_bit)radius,cells_c);
                                    std::vector<integer_8_bit> x_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> y_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> columnar_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> x_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> y_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> columnar_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> x_radius_of_cellular_neighbourhood_of_signalling;
                                    std::vector<integer_8_bit> y_radius_of_cellular_neighbourhood_of_signalling;
                                    std::vector<integer_8_bit> columnar_radius_of_cellular_neighbourhood_of_signalling;
                                    for (natural_16_bit i = 0U; i < tissue_cell_kinds; ++i)
                                    {
                                        x_radius_of_signalling_neighbourhood_of_cell.push_back(radius_x);
                                        y_radius_of_signalling_neighbourhood_of_cell.push_back(radius_y);
                                        columnar_radius_of_signalling_neighbourhood_of_cell.push_back(radius_c);
                                        x_radius_of_signalling_neighbourhood_of_synapse.push_back(radius_x);
                                        y_radius_of_signalling_neighbourhood_of_synapse.push_back(radius_y);
                                        columnar_radius_of_signalling_neighbourhood_of_synapse.push_back(radius_c);
                                        x_radius_of_cellular_neighbourhood_of_signalling.push_back(radius_x);
                                        y_radius_of_cellular_neighbourhood_of_signalling.push_back(radius_y);
                                        columnar_radius_of_cellular_neighbourhood_of_signalling.push_back(radius_c);
                                    }

                                    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_tissue;
                                    try
                                    {
                                        static_tissue = std::shared_ptr<cellab::static_state_of_neural_tissue const>(
                                                    new cellab::static_state_of_neural_tissue(
                                                        tissue_cell_kinds,
                                                        sensory_cell_kinds,
                                                        num_bits_per_cell,
                                                        num_bits_per_synapse,
                                                        num_bits_per_signalling,
                                                        cells_x,
                                                        cells_y,
                                                        num_tissue_cells_of_cell_kind,
                                                        num_synapses_in_territory_of_cell_kind,
                                                        num_sensory_cells_of_cell_kind,
                                                        num_synapses_to_muscles,
                                                        is_x_axis_torus_axis,
                                                        is_y_axis_torus_axis,
                                                        is_c_axis_torus_axis,
                                                        x_radius_of_signalling_neighbourhood_of_cell,
                                                        y_radius_of_signalling_neighbourhood_of_cell,
                                                        columnar_radius_of_signalling_neighbourhood_of_cell,
                                                        x_radius_of_signalling_neighbourhood_of_synapse,
                                                        y_radius_of_signalling_neighbourhood_of_synapse,
                                                        columnar_radius_of_signalling_neighbourhood_of_synapse,
                                                        x_radius_of_cellular_neighbourhood_of_signalling,
                                                        y_radius_of_cellular_neighbourhood_of_signalling,
                                                        columnar_radius_of_cellular_neighbourhood_of_signalling
                                                        ));
                                    }
                                    catch(...)
                                    {
//                                        LOG(testing,"Cannot create static state of tissue : "
//                                                    "num_kinds_of_tissue_cells = " << tissue_cell_kinds << ",   " <<
//                                                    "num_kinds_of_sensory_cells = " << sensory_cell_kinds << ",   " <<
//                                                    "num_cells_along_x_axis = " << cells_x << ",   " <<
//                                                    "num_cells_along_y_axis = " << cells_y << ",   " <<
//                                                    "radius = " << (natural_32_bit)radius
//                                                    );
                                        continue;
                                    }
                                    std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue;
                                    try
                                    {
                                        dynamic_tissue = std::shared_ptr<cellab::dynamic_state_of_neural_tissue>(
                                                    new cellab::dynamic_state_of_neural_tissue(static_tissue)
                                                    );
                                    }
                                    catch(...)
                                    {
//                                        LOG(testing,"Cannot create dynamic state of tissue : "
//                                                    "num_kinds_of_tissue_cells = " << tissue_cell_kinds << ",   " <<
//                                                    "num_kinds_of_sensory_cells = " << sensory_cell_kinds << ",   " <<
//                                                    "num_cells_along_x_axis = " << cells_x << ",   " <<
//                                                    "num_cells_along_y_axis = " << cells_y << ",   " <<
//                                                    "radius = " << (natural_32_bit)radius
//                                                    );
                                        continue;
                                    }

                                    initialse_tissue(dynamic_tissue);
                                    test_algorithms(dynamic_tissue);
                                }
                            }
                }
            }
        }
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
