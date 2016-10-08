#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/timeprof.hpp>
#include <utility/test.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


extern  void test_array_of_derived();


void run()
{
    TMPROF_BLOCK();
    TEST_PROGRESS_SHOW();

    test_array_of_derived();

    TEST_PROGRESS_HIDE();
    TEST_PRINT_STATISTICS();
}
