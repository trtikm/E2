#ifndef UTILITY_RANDOM_HPP_INCLUDED
#   define UTILITY_RANDOM_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <random>
#   include <vector>


//The classic Minimum Standard rand0 of Lewis, Goodman, and Miller.
using  random_generator_for_natural_32_bit = std::linear_congruential_engine<natural_32_bit, 16807UL, 0UL, 2147483647UL>;

// An alternative LCR (Lehmer Generator function).
//typedef std::linear_congruential_engine<natural_32_bit, 48271UL, 0UL, 2147483647UL> random_generator_for_natural_32_bit;

// The classic Mersenne Twister.
//typedef std::mersenne_twister_engine<natural_32_bit, 32, 624, 397, 31,
//                                                     0x9908b0dfUL, 11,
//                                                     0xffffffffUL, 7,
//                                                     0x9d2c5680UL, 15,
//                                                     0xefc60000UL, 18, 1812433253UL> random_generator_for_natural_32_bit;


random_generator_for_natural_32_bit&  default_random_generator();

natural_32_bit  get_random_natural_32_bit_in_range(
    natural_32_bit const min_value,
    natural_32_bit const max_value,
    random_generator_for_natural_32_bit&   generator = default_random_generator()
    );

float_32_bit  get_random_float_32_bit_in_range(
    float_32_bit const min_value,
    float_32_bit const max_value,
    random_generator_for_natural_32_bit&   generator
    );

void  reset(random_generator_for_natural_32_bit&  generator,
            natural_32_bit const  seed = random_generator_for_natural_32_bit::default_seed);


using  bar_random_distribution = std::vector<float_32_bit>;

bar_random_distribution  make_bar_random_distribution_from_count_bars(
        std::vector<natural_32_bit> const&  count_bars
        );
bar_random_distribution  make_bar_random_distribution_from_probability_bars(
        std::vector<float_32_bit> const&  probability_bars
        );

natural_32_bit  get_random_bar_index(
    bar_random_distribution const&  bar_distribution,
    random_generator_for_natural_32_bit&   generator
    );


#endif
