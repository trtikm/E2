#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/basic_numeric_types.hpp>
#include <utility/array_of_bit_units.hpp>
#include <utility/random.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <limits>
#include <memory>

static bool can_be_allocated(natural_16_bit const num_bits_per_unit, natural_64_bit const num_units)
{
    try
    {
        natural_64_bit const  total_num_bits =
                compute_num_bits_of_all_array_units_with_checked_operations(num_bits_per_unit,num_units);

        natural_64_bit const total_num_bytes =
                num_bytes_to_store_bits(total_num_bits);

        std::shared_ptr<natural_8_bit> const ptr(new natural_8_bit[total_num_bytes]);
        return ptr.operator bool();
    }
    catch(...)
    {
        return false;
    }
}

static void log_successfull_allocation(array_of_bit_units& units)
{
//    std::cout << "num_bits_per_unit = " << units.num_bits_per_unit()
//              << ",   num_units = " << units.num_units()
//              << "\n";
    LOG(debug,"num_bits_per_unit = " << units.num_bits_per_unit() << ",   num_units = " << units.num_units() << "\n");
}

static void test_accesses(array_of_bit_units& units)
{
    log_successfull_allocation(units);

    //TEST_SUCCESS(true);
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    for (natural_8_bit bit_shift_for_num_units = 1U; bit_shift_for_num_units < 64U; ++bit_shift_for_num_units)
    {
        natural_64_bit const  num_units = 1ULL << bit_shift_for_num_units;
        for (natural_8_bit bit_shift_for_unit = 1U; bit_shift_for_unit < 16U; ++bit_shift_for_unit)
        {
            natural_64_bit const  num_bits_per_unit = 1U << bit_shift_for_unit;

            if (can_be_allocated(num_bits_per_unit, num_units))
            {
                array_of_bit_units  units(num_bits_per_unit, num_units);
                test_accesses(units);
            }
            if (can_be_allocated(num_bits_per_unit-1U, num_units))
            {
                array_of_bit_units  units(num_bits_per_unit-1U, num_units);
                test_accesses(units);
            }
            if (can_be_allocated(num_bits_per_unit, num_units-1ULL))
            {
                array_of_bit_units  units(num_bits_per_unit, num_units-1ULL);
                test_accesses(units);
            }
            if (can_be_allocated(num_bits_per_unit-1U, num_units-1ULL))
            {
                array_of_bit_units  units(num_bits_per_unit-1U, num_units-1ULL);
                test_accesses(units);
            }
        }
    }

    for (natural_8_bit i = 0U; i < 255U; ++i)
    {
        natural_16_bit const  num_bits_per_unit = get_random_natural_32_bit_in_range(1U,1U << 8U);
        natural_64_bit const  num_units = get_random_natural_32_bit_in_range(1U,1U << 30U);
        if (can_be_allocated(num_bits_per_unit, num_units))
        {
            array_of_bit_units  units(num_bits_per_unit, num_units);
            test_accesses(units);
        }
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
