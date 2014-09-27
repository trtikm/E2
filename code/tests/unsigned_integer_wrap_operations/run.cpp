#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/checked_number_operations.hpp>
#include <utility/test.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <limits>

template<typename unsigned_number_type>
static unsigned_number_type  to_checked_number(natural_64_bit const i)
{
    ASSUMPTION(i <= natural_64_bit(0xFFU));
    natural_8_bit const NBYTES = sizeof(unsigned_number_type);
    natural_8_bit const SHIFT = (NBYTES - 1U) * 8U;
    unsigned_number_type const MAXVALUE = std::numeric_limits<unsigned_number_type>::max();
    return (unsigned_number_type(i) << SHIFT) | (MAXVALUE ^ (unsigned_number_type(0xFFU) << SHIFT));
}

void run()
{
    TMPROF_BLOCK();

    TEST_SUCCESS(sizeof(natural_8_bit) == 1);
    TEST_SUCCESS(sizeof(natural_16_bit) == 2);
    TEST_SUCCESS(sizeof(natural_32_bit) == 4);
    TEST_SUCCESS(sizeof(natural_64_bit) == 8);

    TEST_SUCCESS(std::numeric_limits<natural_8_bit>::max()  == 0xFFU);
    TEST_SUCCESS(std::numeric_limits<natural_16_bit>::max() == 0xFFFFU);
    TEST_SUCCESS(std::numeric_limits<natural_32_bit>::max() == 0xFFFFFFFFU);
    TEST_SUCCESS(std::numeric_limits<natural_64_bit>::max() == 0xFFFFFFFFFFFFFFFFU);

    TEST_SUCCESS(to_checked_number<natural_8_bit>(0xCDU)  == 0xCDU);
    TEST_SUCCESS(to_checked_number<natural_16_bit>(0xCDU) == 0xCDFFU);
    TEST_SUCCESS(to_checked_number<natural_32_bit>(0xCDU) == 0xCDFFFFFFU);
    TEST_SUCCESS(to_checked_number<natural_64_bit>(0xCDU) == 0xCDFFFFFFFFFFFFFFU);

    std::cout << "Testing detection of 8,16,and 32-bit unsigned wrap errors ... "; std::cout.flush();
    for (natural_64_bit i = 0; i <= natural_64_bit(0xFFU); ++i)
        for (natural_64_bit j = 0; j <= natural_64_bit(0xFFU); ++j)
        {
            natural_8_bit const i8 = natural_8_bit(i);
            natural_8_bit const j8 = natural_8_bit(j);

            if (i + j <= std::numeric_limits<natural_8_bit>::max())
                TEST_SUCCESS_EX(checked_add_8_bit(i8,j8) == i8 + j8,false,false)
            else
                TEST_FAILURE_EX(checked_add_8_bit(i8,j8) == i8 + j8,false,false)
            if (i * j <= std::numeric_limits<natural_8_bit>::max())
                TEST_SUCCESS_EX(checked_mul_8_bit(i8,j8) == i8 * j8,false,false)
            else
                TEST_FAILURE_EX(checked_mul_8_bit(i8,j8) == i8 * j8,false,false)

            natural_16_bit const i16 = to_checked_number<natural_16_bit>(i);
            natural_16_bit const j16 = to_checked_number<natural_16_bit>(j);

            if (natural_64_bit(i16) + natural_64_bit(j16) <= std::numeric_limits<natural_16_bit>::max())
                TEST_SUCCESS_EX(checked_add_16_bit(i16,j16) == i16 + j16,false,false)
            else
                TEST_FAILURE_EX(checked_add_16_bit(i16,j16) == i16 + j16,false,false)
            if (natural_64_bit(i16) * natural_64_bit(j16) <= std::numeric_limits<natural_16_bit>::max())
                TEST_SUCCESS_EX(checked_mul_16_bit(i16,j16) == i16 * j16,false,false)
            else
                TEST_FAILURE_EX(checked_mul_16_bit(i16,j16) == i16 * j16,false,false)

            natural_32_bit const i32 = to_checked_number<natural_32_bit>(i);
            natural_32_bit const j32 = to_checked_number<natural_32_bit>(j);

            if (natural_64_bit(i32) + natural_64_bit(j32) <= std::numeric_limits<natural_32_bit>::max())
                TEST_SUCCESS_EX(checked_add_32_bit(i32,j32) == i32 + j32,false,false)
            else
                TEST_FAILURE_EX(checked_add_32_bit(i32,j32) == i32 + j32,false,false)
            if (natural_64_bit(i32) * natural_64_bit(j32) <= std::numeric_limits<natural_32_bit>::max())
                TEST_SUCCESS_EX(checked_mul_32_bit(i32,j32) == i32 * j32,false,false)
            else
                TEST_FAILURE_EX(checked_mul_32_bit(i32,j32) == i32 * j32,false,false)
        }
    std::cout << "Done.\n";

    TEST_PRINT_STATISTICS();
}
