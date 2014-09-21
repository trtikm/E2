#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/checked_number_operations.hpp>
#include <utility/test.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

void run()
{
    TMPROF_BLOCK();

    TEST_SUCCESS(sizeof(natural_8_bit) == 1);
    TEST_SUCCESS(sizeof(natural_16_bit) == 2);
    TEST_SUCCESS(sizeof(natural_32_bit) == 4);
    TEST_SUCCESS(sizeof(natural_64_bit) == 8);

    // TODO!!

    TEST_PRINT_STATISTICS();
}
