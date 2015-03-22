#include "./program_info.hpp"
#include "./program_options.hpp"
#include "./my_neural_tissue.hpp"
#include "./my_environment.hpp"
#include <efloop/external_feedback_loop.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>


static void test_tissue(std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue,
                        natural_32_bit const round_counter)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
        for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
            for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
            {
                TEST_SUCCESS(round_counter == my_cell(dynamic_tissue->find_bits_of_cell_in_tissue(x,y,c)).count());
                TEST_SUCCESS(round_counter == my_signalling(dynamic_tissue->find_bits_of_signalling(x,y,c)).count());
                natural_32_bit const num_synapses =
                        static_tissue->num_synapses_in_territory_of_cell_kind(
                                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                );
                for (natural_32_bit s = 0U; s < num_synapses; ++s)
                {
                    TEST_SUCCESS(round_counter == my_synapse(dynamic_tissue->find_bits_of_synapse_in_tissue(x,y,c,s)).count());
                    TEST_SUCCESS(bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(x,y,c,s)) < 7U);
                }
            }

    TEST_PROGRESS_UPDATE();

    for (natural_32_bit i = 0U; i < static_tissue->num_synapses_to_muscles(); ++i)
        TEST_SUCCESS(round_counter == my_synapse(dynamic_tissue->find_bits_of_synapse_to_muscle(i)).count());
    for (natural_32_bit i = 0U; i < static_tissue->num_sensory_cells(); ++i)
        TEST_SUCCESS(round_counter == my_cell(dynamic_tissue->find_bits_of_sensory_cell(i)).count());
}

static void test_round(std::shared_ptr<efloop::external_feedback_loop> const  loop,
                       std::vector<natural_32_bit> const&  num_avalilable_tissue_threads,
                       natural_32_bit const  num_avalilable_envirinment_threads,
                       natural_32_bit&  round_counter
                       )
{
    ASSUMPTION(num_avalilable_tissue_threads.size() == loop->num_neural_tissues());

    loop->compute_next_state_of_neural_tissues_and_environment(num_avalilable_tissue_threads,
                                                               num_avalilable_envirinment_threads);
    TEST_PROGRESS_UPDATE();

    ++round_counter;
    for (natural_32_bit i = 0U; i < loop->num_neural_tissues(); ++i)
    {
        test_tissue(loop->get_neural_tissue(i)->get_dynamic_state_of_neural_tissue(),round_counter);
        TEST_PROGRESS_UPDATE();
    }
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    std::shared_ptr<efloop::external_feedback_loop>  loop =
            std::shared_ptr<efloop::external_feedback_loop>(
                    new efloop::external_feedback_loop(
                            std::vector< std::shared_ptr<cellab::neural_tissue> >{
                                    std::shared_ptr<cellab::neural_tissue>(new my_neural_tissue()),
                                    std::shared_ptr<cellab::neural_tissue>(new my_neural_tissue())
                                    },
                            std::shared_ptr<envlab::rules_and_logic_of_environment>(new my_environment())
                            )
                    );

    natural_32_bit counter = 0U;
    for (natural_32_bit i = 0U; i < 5U; ++i)
    {
        test_round(loop,std::vector<natural_32_bit>{0U,0U},1U,counter);
        test_round(loop,std::vector<natural_32_bit>{0U,0U},2U,counter);
        test_round(loop,std::vector<natural_32_bit>{1U,2U},2U,counter);
        test_round(loop,std::vector<natural_32_bit>{0U,1U},1U,counter);
        test_round(loop,std::vector<natural_32_bit>{2U,1U},2U,counter);
        test_round(loop,std::vector<natural_32_bit>{11U,20U},12U,counter);
        test_round(loop,std::vector<natural_32_bit>{22U,41U},23U,counter);
        test_round(loop,std::vector<natural_32_bit>{33U,64U},64U,counter);
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
