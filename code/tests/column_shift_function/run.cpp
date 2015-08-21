#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellconnect/column_shift_function.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>
#include <functional>
#include <vector>
#include <string>
#include <sstream>


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();


    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
