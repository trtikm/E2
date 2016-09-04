#ifndef UTILITY_RANDOM_HPP_INCLUDED
#   define UTILITY_RANDOM_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <random>


//The classic Minimum Standard rand0 of Lewis, Goodman, and Miller.
typedef std::linear_congruential_engine<natural_32_bit, 16807UL, 0UL, 2147483647UL> random_generator_for_natural_32_bit;

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


#endif
