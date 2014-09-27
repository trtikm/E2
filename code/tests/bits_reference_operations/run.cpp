#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/bits_reference.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <array>

static void test_get_set_bit()
{
    TMPROF_BLOCK();

    natural_8_bit const  num_bytes = 16U;
    natural_8_bit const  num_bits = num_bytes * 8U;
    std::array<natural_8_bit,num_bytes+1U>  bits_field;
    std::array<bool,num_bits>  correct_values;

    for (natural_8_bit  i = 0U; i < num_bytes+1U; ++i)
        bits_field.at(i) = 0U;
    for (natural_8_bit  i = 0U; i < num_bits; ++i)
        correct_values.at(i) = false;

    for (natural_8_bit  shift = 0U; shift < 8U; ++shift)
    {
        for (natural_16_bit  nbits = 1U; nbits <= num_bits; ++nbits)
        {
            bits_reference  bits(&bits_field.at(0),shift,nbits);
            for (natural_16_bit  i = 0U; i < nbits; ++i)
            {
                set_bit(bits,i,true);
                correct_values.at(i) = true;
                for (natural_16_bit  j = 0U; j < nbits; ++j)
                    TEST_SUCCESS(get_bit(bits,j) == correct_values.at(j))

                set_bit(bits,i,false);
                correct_values.at(i) = false;
                for (natural_16_bit  j = 0U; j < nbits; ++j)
                    TEST_SUCCESS(get_bit(bits,j) == correct_values.at(j))
            }
        }
        TEST_PROGRESS_UPDATE();
    }
}

static void test_swap_bits()
{
    TMPROF_BLOCK();

    natural_8_bit const  num_bytes = 16U;
    natural_8_bit const  num_bits = num_bytes * 8U;
    std::array<natural_8_bit,num_bytes+1U>  bits_field_1 = {
        0xCD, 0xAB, 0xBE, 0xED,
        0xEB, 0xAC, 0xED, 0xBA,
        0xBE, 0xCD, 0xCE, 0xBD,
        0xED, 0xAB, 0xCE, 0xCA, 0xCD
    };
    std::array<natural_8_bit,num_bytes+1U>  bits_field_2 = {
        0xAE, 0xBD, 0xCB, 0xAC,
        0xDC, 0xBE, 0xCA, 0xDB,
        0xBE, 0xCD, 0xAB, 0xBA,
        0xAC, 0xED, 0xDA, 0xCE, 0xEB
    };
    std::array<bool,num_bits+8U>  correct_values_1;
    std::array<bool,num_bits+8U>  correct_values_2;
    for (natural_8_bit  i = 0U; i < num_bytes+1U; ++i)
        for (natural_8_bit  j = 0U; j < 8U; ++j)
        {
            correct_values_1.at(8U * i + j) = (bits_field_1.at(i) & (1U << (7U - j))) != 0U;
            correct_values_2.at(8U * i + j) = (bits_field_2.at(i) & (1U << (7U - j))) != 0U;
        }

    for (natural_8_bit  shift_1 = 0U; shift_1 < 8U; ++shift_1)
        for (natural_8_bit  shift_2 = 0U; shift_2 < 8U; ++shift_2)
            for (natural_16_bit  nbits = 1U; nbits <= num_bits; ++nbits)
            {
                bits_reference  bits_1(&bits_field_1.at(0),shift_1,nbits);
                bits_reference  bits_2(&bits_field_2.at(0),shift_2,nbits);
                for (natural_16_bit  i = 0U; i < nbits; ++i)
                {
                    swap_referenced_bits(bits_1,bits_2);
                    for (natural_16_bit  j = 0U; j < nbits; ++j)
                    {
                        TEST_SUCCESS(get_bit(bits_1,j) == correct_values_2.at(j + shift_2))
                        TEST_SUCCESS(get_bit(bits_2,j) == correct_values_1.at(j + shift_1))
                    }

                    swap_referenced_bits(bits_1,bits_2);
                    for (natural_16_bit  j = 0U; j < nbits; ++j)
                    {
                        TEST_SUCCESS(get_bit(bits_1,j) == correct_values_1.at(j + shift_1))
                        TEST_SUCCESS(get_bit(bits_2,j) == correct_values_2.at(j + shift_2))
                    }
                }
                TEST_PROGRESS_UPDATE();
            }
}

static void test_conversions_bits_and_32_bit_values()
{
    TMPROF_BLOCK();

    // TODO!!
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();
    test_get_set_bit();
    test_swap_bits();
    test_conversions_bits_and_32_bit_values();
    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
