#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellconnect/column_shift_function.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    // TODO: Implement your unit test here. Call the macro:
    //              TEST_PROGRESS_UPDATE();
    //       each time you want to update the progress bar of your test.

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
