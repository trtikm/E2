#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/basic_numeric_types.hpp>
#include <utility/array_of_bit_units.hpp>
#include <utility/bits_reference.hpp>
#include <utility/bit_count.hpp>
#include <utility/random.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>

static natural_64_bit  compute_total_num_bytes(natural_16_bit const num_bits_per_unit, natural_64_bit const num_units)
{
    natural_64_bit const  total_num_bits =
            compute_num_bits_of_all_array_units_with_checked_operations(num_bits_per_unit,num_units);

    natural_64_bit const total_num_bytes =
            num_bytes_to_store_bits(total_num_bits);

    return total_num_bytes;
}

static bool can_be_allocated(natural_16_bit const num_bits_per_unit, natural_64_bit const num_units)
{
    logging_severity_level const  original_severity_level = get_minimal_severity_level();
    try
    {
        set_minimal_severity_level(logging_severity_level::testing);
        std::shared_ptr<natural_8_bit> const ptr(new natural_8_bit[compute_total_num_bytes(num_bits_per_unit,num_units)]);
        set_minimal_severity_level(original_severity_level);
        return ptr.operator bool();
    }
    catch(...)
    {
        set_minimal_severity_level(original_severity_level);
        return false;
    }
}

static void test_num_bytes_to_store_bits()
{
    for (natural_32_bit nbits = 0U; nbits <= std::numeric_limits<natural_16_bit>::max(); ++nbits)
    {
        natural_64_bit nbytes = nbits / 8U;
        if (nbytes * 8U < nbits)
            ++nbytes;

        TEST_SUCCESS(num_bytes_to_store_bits(nbits) == nbytes);

        if (nbits % 1000U == 0U)
            TEST_PROGRESS_UPDATE();
    }
}

static bool are_adjacent(bits_const_reference const& first, bits_const_reference const& second)
{
    natural_16_bit const num_bits = first.shift_in_the_first_byte() + first.num_bits();
    natural_8_bit const shift = num_bits % 8U;
    natural_8_bit const* ptr = first.first_byte_ptr() + num_bytes_to_store_bits(num_bits);
    if (shift != 0U)
        --ptr;
    return (ptr == second.first_byte_ptr() && shift == second.shift_in_the_first_byte()) &&
           (first.num_bits() == second.num_bits())
           ;
}

static void test_accesses(array_of_bit_units& units, natural_64_bit const  index)
{
    bits_const_reference const bits0 = units.find_bits_of_unit(0ULL);
    bits_const_reference const bits = units.find_bits_of_unit(index);

    TEST_SUCCESS(bits.num_bits() == bits0.num_bits());
    TEST_SUCCESS(bits.first_byte_ptr() >= bits0.first_byte_ptr());
    TEST_SUCCESS(
        bits.first_byte_ptr() + num_bytes_to_store_bits(bits.shift_in_the_first_byte() + bits.num_bits())
        <=
        bits0.first_byte_ptr() + compute_total_num_bytes(units.num_bits_per_unit(),units.num_units())
        );

    if (index > 0ULL)
        TEST_SUCCESS(are_adjacent(units.find_bits_of_unit(index - 1ULL),bits));

    if (index < units.num_units() - 1ULL)
        TEST_SUCCESS(are_adjacent(bits,units.find_bits_of_unit(index + 1ULL)));
}

static void test_accesses(array_of_bit_units& units)
{
    //LOG(testing,"num_bits_per_unit = " << units.num_bits_per_unit() << ",   num_units = " << units.num_units() << "\n");

    test_accesses(units,0ULL);
    test_accesses(units,units.num_units() - 1ULL);

    natural_32_bit const  max_num_accesses = (natural_32_bit)std::min(1024ULL,units.num_units());
    natural_64_bit const  step = units.num_units() / max_num_accesses;
    natural_64_bit index = 0ULL;
    for (natural_32_bit  i = 0U; i < max_num_accesses; ++i, index += step)
        test_accesses(units,index % units.num_units());

    natural_32_bit const  max_index =
            (natural_32_bit)std::min((natural_64_bit)std::numeric_limits<natural_32_bit>::max(),units.num_units()-1ULL);
    for (natural_8_bit i = 0U; i < 100U; ++i)
    {
        natural_64_bit const  index = get_random_natural_32_bit_in_range(0U,max_index);
        test_accesses(units,index);
    }
}

static void test_array_of_units(natural_16_bit const  num_bits_per_unit, natural_64_bit const  num_units)
{
    try
    {
        array_of_bit_units  units(num_bits_per_unit, num_units);
        test_accesses(units);
    }
    catch(std::bad_alloc e)
    {
        TEST_FAILURE(can_be_allocated(num_bits_per_unit,num_units));
        LOG(testing,e.what() << " (but it is coverred by the test) : " << "num_bits_per_unit = " << num_bits_per_unit << ",   num_units = " << num_units);
    }
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    test_num_bytes_to_store_bits();

    for (natural_8_bit bit_shift_for_num_units = 1U; bit_shift_for_num_units < 64U; ++bit_shift_for_num_units)
    {
        natural_64_bit const  num_units = 1ULL << bit_shift_for_num_units;
        for (natural_8_bit bit_shift_for_unit = 1U; bit_shift_for_unit < 16U; ++bit_shift_for_unit)
        {
            natural_16_bit const  num_bits_per_unit = (natural_16_bit)(1U << bit_shift_for_unit);

            if (can_be_allocated(num_bits_per_unit, num_units))
                test_array_of_units(num_bits_per_unit, num_units);
            if (can_be_allocated(num_bits_per_unit-1U, num_units))
                test_array_of_units(num_bits_per_unit-1U, num_units);
            if (can_be_allocated(num_bits_per_unit, num_units-1ULL))
                test_array_of_units(num_bits_per_unit, num_units-1ULL);
            if (can_be_allocated(num_bits_per_unit-1U, num_units-1ULL))
                test_array_of_units(num_bits_per_unit-1U, num_units-1ULL);
        }
        TEST_PROGRESS_UPDATE();
    }

    for (natural_8_bit i = 0U; i < 255U; ++i)
    {
        natural_16_bit const  num_bits_per_unit = get_random_natural_32_bit_in_range(1U,1U << 8U);
        natural_64_bit const  num_units = get_random_natural_32_bit_in_range(1U,1U << 30U);
        if (can_be_allocated(num_bits_per_unit, num_units))
            test_array_of_units(num_bits_per_unit, num_units);
        TEST_PROGRESS_UPDATE();
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
