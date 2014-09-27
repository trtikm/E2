#include "./program_info.hpp"
#include "./program_options.hpp"
#include "./my_neural_tissue.hpp"
#include "./my_environment.hpp"
#include <efloop/external_feedback_loop.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>


void run()
{
    TMPROF_BLOCK();

    std::shared_ptr<efloop::external_feedback_loop>  loop =
            std::shared_ptr<efloop::external_feedback_loop>(new efloop::external_feedback_loop(
                    std::shared_ptr<cellab::neural_tissue>(new my_neural_tissue()),
                    std::shared_ptr<envlab::rules_and_logic_of_environment>(new my_environment())
                    ));

    TEST_PRINT_STATISTICS();
}
