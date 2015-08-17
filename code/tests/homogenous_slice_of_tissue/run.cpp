#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/homogenous_slice_of_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/array_of_bit_units.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/bit_count.hpp>
#include <utility/random.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>

static natural_64_bit  compute_total_num_bytes(natural_16_bit const num_bits_per_unit,
                                               natural_32_bit const num_units_along_x_axis,
                                               natural_32_bit const num_units_along_y_axis,
                                               natural_64_bit const num_units_along_columnar_axis)
{
    natural_64_bit const  total_num_bits =
            cellab::compute_num_bits_of_slice_of_tissue_with_checked_operations(num_bits_per_unit,
                                                                                num_units_along_x_axis,
                                                                                num_units_along_y_axis,
                                                                                num_units_along_columnar_axis);

    natural_64_bit const total_num_bytes =
            num_bytes_to_store_bits(total_num_bits);

    return total_num_bytes;
}

static bool can_be_allocated(natural_16_bit const num_bits_per_unit,
                             natural_32_bit const num_units_along_x_axis,
                             natural_32_bit const num_units_along_y_axis,
                             natural_64_bit const num_units_along_columnar_axis)
{
    logging_severity_level const  original_severity_level = get_minimal_severity_level();
    try
    {
        set_minimal_severity_level(logging_severity_level::testing);
        std::shared_ptr<natural_8_bit> const ptr(
                new natural_8_bit[
                    compute_total_num_bytes(num_bits_per_unit,
                                            num_units_along_x_axis,
                                            num_units_along_y_axis,
                                            num_units_along_columnar_axis)
                    ]);
        set_minimal_severity_level(original_severity_level);
        return ptr.operator bool();
    }
    catch(...)
    {
        set_minimal_severity_level(original_severity_level);
        return false;
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

static void test_accesses(cellab::homogenous_slice_of_tissue&  slice, natural_32_bit const shift_X,
                          natural_32_bit const  shift_Y, natural_32_bit const  shift_C)
{
    bits_const_reference const bits0 = slice.find_bits_of_unit(0U,0U,0ULL);
    bits_const_reference const bits = slice.find_bits_of_unit(shift_X,shift_Y,shift_C);

    TEST_SUCCESS(bits.num_bits() == bits0.num_bits());
    TEST_SUCCESS(bits.first_byte_ptr() >= bits0.first_byte_ptr());
    TEST_SUCCESS(
        bits.first_byte_ptr() + num_bytes_to_store_bits(bits.shift_in_the_first_byte() + bits.num_bits())
        <=
        bits0.first_byte_ptr() + compute_total_num_bytes(slice.num_bits_per_unit(),
                                                         slice.num_units_along_x_axis(),
                                                         slice.num_units_along_y_axis(),
                                                         slice.num_units_along_columnar_axis())
        );

    if (shift_X < slice.num_units_along_x_axis() - 1U ||
        shift_Y < slice.num_units_along_y_axis() - 1U ||
        shift_C < slice.num_units_along_columnar_axis() - 1ULL)
    {
        natural_32_bit shift_X_next = shift_X;
        natural_32_bit shift_Y_next = shift_Y;
        natural_32_bit shift_C_next = shift_C;
        cellab::go_to_next_coordinates(
                    shift_X_next,shift_Y_next,shift_C_next,
                    1U,
                    slice.num_units_along_x_axis(),
                    slice.num_units_along_y_axis(),
                    (natural_32_bit)slice.num_units_along_columnar_axis()
                    );

        TEST_SUCCESS(shift_X_next < slice.num_units_along_x_axis());
        TEST_SUCCESS(shift_Y_next < slice.num_units_along_y_axis());
        TEST_SUCCESS(shift_C_next < (natural_32_bit)slice.num_units_along_columnar_axis());

        TEST_SUCCESS(are_adjacent(bits,slice.find_bits_of_unit(shift_X_next,shift_Y_next,shift_C_next)));
    }
}

static void test_accesses(cellab::homogenous_slice_of_tissue&  slice)
{
//    LOG(testing,"num_bits_per_unit = " << slice.num_bits_per_unit() << ",   " <<
//                "num_units_along_x_axis = " << slice.num_units_along_x_axis() << ",   " <<
//                "num_units_along_y_axis = " << slice.num_units_along_y_axis() << ",   " <<
//                "num_units_along_columnar_axis = " << slice.num_units_along_columnar_axis()
//                );

    test_accesses(slice,0U,0U,0U);
    test_accesses(slice,0U,
                        0U,
                        (natural_32_bit)(slice.num_units_along_columnar_axis() - 1U));
    test_accesses(slice,0U,
                        slice.num_units_along_y_axis() - 1U,
                        0U);
    test_accesses(slice,0U,
                        slice.num_units_along_y_axis() - 1U,
                        (natural_32_bit)(slice.num_units_along_columnar_axis() - 1ULL));
    test_accesses(slice,slice.num_units_along_x_axis() - 1U,
                        0U,
                        0U);
    test_accesses(slice,slice.num_units_along_x_axis() - 1U,
                        0U,
                        (natural_32_bit)(slice.num_units_along_columnar_axis() - 1ULL));
    test_accesses(slice,slice.num_units_along_x_axis() - 1U,
                        slice.num_units_along_y_axis() - 1U,
                        0U);
    test_accesses(slice,slice.num_units_along_x_axis() - 1U,
                        slice.num_units_along_y_axis() - 1U,
                        (natural_32_bit)(slice.num_units_along_columnar_axis() - 1ULL));

    for (natural_8_bit i = 0U; i < 10U; ++i)
    {
        natural_32_bit const  shift_X =
                get_random_natural_32_bit_in_range(0U,slice.num_units_along_x_axis()-1U);
        natural_32_bit const  shift_Y =
                get_random_natural_32_bit_in_range(0U,slice.num_units_along_y_axis()-1U);
        natural_32_bit const  shift_C =
                get_random_natural_32_bit_in_range(0U,(natural_32_bit)(slice.num_units_along_columnar_axis()-1ULL));
        test_accesses(slice,shift_X,shift_Y,shift_C);
    }
}

static void test_3Darray_of_units(natural_16_bit const num_bits_per_unit,
                                  natural_32_bit const num_units_along_x_axis,
                                  natural_32_bit const num_units_along_y_axis,
                                  natural_32_bit const num_units_along_columnar_axis)
{
    try
    {
        cellab::homogenous_slice_of_tissue  slice(
                    num_bits_per_unit,
                    num_units_along_x_axis,
                    num_units_along_y_axis,
                    num_units_along_columnar_axis
                    );
        test_accesses(slice);
    }
    catch(std::bad_alloc e)
    {
        TEST_FAILURE(can_be_allocated(
                         num_bits_per_unit,
                         num_units_along_x_axis,
                         num_units_along_y_axis,
                         num_units_along_columnar_axis
                         ));
        LOG(testing,e.what() << " (but it is coverred by the test) : " <<
                    "num_bits_per_unit = " << num_bits_per_unit << ",   " <<
                    "num_units_along_x_axis = " << num_units_along_x_axis << ",   " <<
                    "num_units_along_y_axis = " << num_units_along_y_axis << ",   " <<
                    "num_units_along_columnar_axis = " << num_units_along_columnar_axis
                    );
    }
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    for (natural_8_bit shift_for_X_units = 1U; shift_for_X_units < 32U; ++shift_for_X_units)
    {
        natural_32_bit const  num_units_along_x_axis = 1U << shift_for_X_units;
        for (natural_8_bit shift_for_Y_units = 1U; shift_for_Y_units < 32U; ++shift_for_Y_units)
        {
            natural_32_bit const  num_units_along_y_axis = 1U << shift_for_Y_units;
            for (natural_8_bit shift_for_C_units = 1U; shift_for_C_units < 32U; ++shift_for_C_units)
            {
                natural_32_bit const  num_units_along_columnar_axis = 1U << shift_for_C_units;
                for (natural_8_bit shift_for_unit = 1U; shift_for_unit < 16U; ++shift_for_unit)
                {
                    natural_16_bit const  num_bits_per_unit = 1U << shift_for_unit;

                    for (natural_8_bit dx = 0U; dx < 2U; ++dx)
                        for (natural_8_bit dy = 0U; dy < 2U; ++dy)
                            for (natural_8_bit dc = 0U; dc < 2U; ++dc)
                                for (natural_8_bit dbits = 0U; dbits < 2U; ++dbits)
                                    if (can_be_allocated(num_bits_per_unit - dbits,
                                                         num_units_along_x_axis - dx,
                                                         num_units_along_y_axis - dy,
                                                         num_units_along_columnar_axis - dc))
                                        test_3Darray_of_units(num_bits_per_unit - dbits,
                                                              num_units_along_x_axis - dx,
                                                              num_units_along_y_axis - dy,
                                                              num_units_along_columnar_axis - dc);
                }
                TEST_PROGRESS_UPDATE();
            }
        }
    }

    for (natural_8_bit i = 0U; i < 100U; ++i)
    {
        natural_16_bit const  num_bits_per_unit = get_random_natural_32_bit_in_range(1U,1U << 8U);
        natural_32_bit const  num_units_along_x_axis = get_random_natural_32_bit_in_range(1U,100000U);
        natural_32_bit const  num_units_along_y_axis = get_random_natural_32_bit_in_range(1U,100000U);
        natural_32_bit const  num_units_along_columnar_axis = get_random_natural_32_bit_in_range(1U,100U);
        if (can_be_allocated(num_bits_per_unit,
                             num_units_along_x_axis,
                             num_units_along_y_axis,
                             num_units_along_columnar_axis))
            test_3Darray_of_units(num_bits_per_unit,
                                  num_units_along_x_axis,
                                  num_units_along_y_axis,
                                  num_units_along_columnar_axis);
        TEST_PROGRESS_UPDATE();
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
