#include "./program_info.hpp"
#include "./program_options.hpp"
#include "./my_neural_tissue.hpp"
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/random.hpp>
#include <array>


static void test_tissue(std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue,
                        natural_32_bit const cell_counter,
                        natural_32_bit const synapse_counter,
                        natural_32_bit const signalling_counter,
                        natural_32_bit const sensory_cell_counter,
                        natural_32_bit const synapse_to_muscle_counter,
                        bool const test_also_migration_to_proper_territorial_lists)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
        for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
            for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
            {
                TEST_SUCCESS(cell_counter == my_cell(dynamic_tissue->find_bits_of_cell_in_tissue(x,y,c)).count());
                TEST_SUCCESS(signalling_counter == my_signalling(dynamic_tissue->find_bits_of_signalling(x,y,c)).count());
                natural_32_bit const num_synapses =
                        static_tissue->num_synapses_in_territory_of_cell_kind(
                                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                );
                std::array<natural_32_bit,8U> boundaries_of_lists;
                if (test_also_migration_to_proper_territorial_lists)
                {
                    boundaries_of_lists.at(0U) = 0U;
                    for (natural_32_bit w = 1U; w < 7U; ++w)
                    {
                        boundaries_of_lists.at(w) =
                                bits_to_value<natural_32_bit>(
                                        dynamic_tissue->find_bits_of_delimiter_between_teritorial_lists(x,y,c,w - 1U)
                                        );
                        TEST_SUCCESS(boundaries_of_lists.at(w) >= boundaries_of_lists.at(w - 1U));
                    }
                    boundaries_of_lists.at(7U) = num_synapses;
                    TEST_SUCCESS(boundaries_of_lists.at(7U) >= boundaries_of_lists.at(6U));
                }
                for (natural_32_bit s = 0U; s < num_synapses; ++s)
                {
                    TEST_SUCCESS(synapse_counter == my_synapse(dynamic_tissue->find_bits_of_synapse_in_tissue(x,y,c,s)).count());
                    natural_32_bit const territorial_state =
                            bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(x,y,c,s));
                    TEST_SUCCESS(territorial_state < 7U);
                    if (test_also_migration_to_proper_territorial_lists)
                    {
                        natural_32_bit const territorial_list_index =
                                cellab::convert_territorial_state_of_synapse_to_territorial_list_index(
                                        static_cast<cellab::territorial_state_of_synapse>(territorial_state)
                                        );
                        TEST_SUCCESS(territorial_list_index < 7U);
                        TEST_SUCCESS(boundaries_of_lists.at(territorial_list_index) <= s &&
                                     s < boundaries_of_lists.at(territorial_list_index + 1U));
                    }
                }
            }
    for (natural_32_bit i = 0U; i < static_tissue->num_synapses_to_muscles(); ++i)
        TEST_SUCCESS(synapse_to_muscle_counter == my_synapse(dynamic_tissue->find_bits_of_synapse_to_muscle(i)).count());
    for (natural_32_bit i = 0U; i < static_tissue->num_sensory_cells(); ++i)
        TEST_SUCCESS(sensory_cell_counter == my_cell(dynamic_tissue->find_bits_of_sensory_cell(i)).count());
}



static void initialse_tissue_and_sensory_cells(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue
        )
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const>  static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    cellab::territorial_state_of_synapse const  territorial_state =
            cellab::territorial_state_of_synapse::MIGRATION_ALONG_POSITIVE_X_AXIS;
    for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
        for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
            for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
            {
                value_to_bits(0U,dynamic_tissue->find_bits_of_cell_in_tissue(x,y,c));
                value_to_bits(0U,dynamic_tissue->find_bits_of_signalling(x,y,c));

                natural_32_bit const num_synapses =
                        static_tissue->num_synapses_in_territory_of_cell_kind(
                                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                );
                for (natural_32_bit s = 0U; s < num_synapses; ++s)
                {
                    value_to_bits(0U,dynamic_tissue->find_bits_of_synapse_in_tissue(x,y,c,s));
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
                            get_random_natural_32_bit_in_range(0U,static_tissue->num_cells_along_columnar_axis()+
                                                                  static_tissue->num_sensory_cells()-1U),
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

    for (natural_32_bit i = 0U; i < static_tissue->num_synapses_to_muscles(); ++i)
    {
        value_to_bits(0U,dynamic_tissue->find_bits_of_synapse_to_muscle(i));

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

    for (natural_32_bit i = 0U; i < static_tissue->num_sensory_cells(); ++i)
        value_to_bits(0U,dynamic_tissue->find_bits_of_sensory_cell(i));

    test_tissue(dynamic_tissue,0U,0U,0U,0U,0U,false);
}

static void test_algorithms(std::shared_ptr<cellab::neural_tissue> const neural_tissue_ptr,
                            natural_32_bit const  num_avalilable_threads,
                            natural_32_bit& cell_counter,
                            natural_32_bit& synapse_counter,
                            natural_32_bit& signalling_counter,
                            natural_32_bit& sensory_cell_counter,
                            natural_32_bit& synapse_to_muscle_counter
                            )
{
    std::shared_ptr<cellab::dynamic_state_of_neural_tissue>  dynamic_tissue =
            neural_tissue_ptr->get_dynamic_state_of_neural_tissue();
    for (natural_32_bit i = 0U; i < 5U; ++i)
    {
        neural_tissue_ptr->apply_transition_of_synapses_to_muscles(num_avalilable_threads);
        TEST_PROGRESS_UPDATE();

        test_tissue(dynamic_tissue,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,++synapse_to_muscle_counter,false);
        TEST_PROGRESS_UPDATE();


        neural_tissue_ptr->apply_transition_of_synapses_of_tissue(num_avalilable_threads);
        TEST_PROGRESS_UPDATE();

        test_tissue(dynamic_tissue,cell_counter,++synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter,false);
        TEST_PROGRESS_UPDATE();


        neural_tissue_ptr->apply_transition_of_territorial_lists_of_synapses(num_avalilable_threads);
        TEST_PROGRESS_UPDATE();

        test_tissue(dynamic_tissue,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter,true);
        TEST_PROGRESS_UPDATE();


        neural_tissue_ptr->apply_transition_of_synaptic_migration_in_tissue(num_avalilable_threads);
        TEST_PROGRESS_UPDATE();

        test_tissue(dynamic_tissue,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter,false);
        TEST_PROGRESS_UPDATE();


        neural_tissue_ptr->apply_transition_of_signalling_in_tissue(num_avalilable_threads);
        TEST_PROGRESS_UPDATE();

        test_tissue(dynamic_tissue,cell_counter,synapse_counter,++signalling_counter,sensory_cell_counter,synapse_to_muscle_counter,false);
        TEST_PROGRESS_UPDATE();


        neural_tissue_ptr->apply_transition_of_cells_of_tissue(num_avalilable_threads);
        TEST_PROGRESS_UPDATE();

        test_tissue(dynamic_tissue,++cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter,false);
        TEST_PROGRESS_UPDATE();


        for (natural_32_bit s = 0U;
             s < dynamic_tissue->get_static_state_of_neural_tissue()->num_sensory_cells();
             ++s)
        {
            bits_reference  bits_of_sensory_cell = dynamic_tissue->find_bits_of_sensory_cell(s);
            my_cell  sensory_cell(bits_of_sensory_cell);
            sensory_cell.increment();
            sensory_cell >> bits_of_sensory_cell;
        }
        TEST_PROGRESS_UPDATE();

        test_tissue(dynamic_tissue,cell_counter,synapse_counter,signalling_counter,++sensory_cell_counter,synapse_to_muscle_counter,false);
        TEST_PROGRESS_UPDATE();
    }
}


static void  test_tissue(std::shared_ptr<cellab::neural_tissue> const neural_tissue_ptr)
{
    initialse_tissue_and_sensory_cells(neural_tissue_ptr->get_dynamic_state_of_neural_tissue() );
    natural_32_bit cell_counter = 0U;
    natural_32_bit synapse_counter = 0U;
    natural_32_bit signalling_counter = 0U;
    natural_32_bit sensory_cell_counter = 0U;
    natural_32_bit synapse_to_muscle_counter = 0U;
    test_algorithms(neural_tissue_ptr,64U,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter);
    test_algorithms(neural_tissue_ptr,128U,cell_counter,synapse_counter,signalling_counter,sensory_cell_counter,synapse_to_muscle_counter);
}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    test_tissue( std::shared_ptr<cellab::neural_tissue>(new my_neural_tissue()) );

    {
        std::shared_ptr<cellab::static_state_of_neural_tissue const> static_tissue =
                std::shared_ptr<cellab::static_state_of_neural_tissue const>(
                    new cellab::static_state_of_neural_tissue(
                          num_kinds_of_tissue_cells(),
                          num_kinds_of_sensory_cells(),
                          num_kinds_of_synapses_to_muscles(),
                          num_bits_per_cell(),
                          num_bits_per_synapse(),
                          num_bits_per_signalling(),
                          num_cells_along_x_axis(),
                          num_cells_along_y_axis(),
                          num_tissue_cells_of_cell_kind(),
                          num_synapses_in_territory_of_cell_kind(),
                          num_sensory_cells_of_cell_kind(),
                          num_synapses_to_muscles_of_kind(),
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
                          columnar_radius_of_cellular_neighbourhood_of_signalling()
                          )
                    );

        test_tissue( std::shared_ptr<cellab::neural_tissue>(new my_neural_tissue(static_tissue)) );

        {
            std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue =
                    std::shared_ptr<cellab::dynamic_state_of_neural_tissue>(
                        new cellab::dynamic_state_of_neural_tissue(static_tissue)
                        );

            test_tissue( std::shared_ptr<cellab::neural_tissue>(new my_neural_tissue(dynamic_tissue)) );
            test_tissue( std::shared_ptr<cellab::neural_tissue>(new my_neural_tissue(dynamic_tissue,true)) );
        }
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
