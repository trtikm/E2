#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <random>

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

static random_generator_for_natural_32_bit&  generator()
{
    static random_generator_for_natural_32_bit the_generator;
    return the_generator;
}

natural_32_bit  get_random_natural_32_bit_in_range(
    natural_32_bit const min_value,
    natural_32_bit const max_value
    )
{
    ASSUMPTION(min_value <= max_value);
    return std::uniform_int_distribution<natural_32_bit>(min_value,max_value)(generator());
}



//#include <utility/invariants.hpp>
//#include <vector>
//#include <limits>
//#include <cmath>
//#include <algorithm>




//struct probability_mass_function
//{
//    probability_mass_function(
//            std::vector<natural_32_bit> const& values_of_random_variable, // must be sorted!
//            std::vector<natural_32_bit> const& frequences_of_values_of_random_variable
//            )
//        : m_values_of_random_variable(values_of_random_variable)
//        , m_cumulative_distribution_function(frequences_of_values_of_random_variable.size(),0U)
//    {
//        ASSUMPTION(values_of_random_variable.size() > 0);
//        ASSUMPTION(values_of_random_variable.size() == frequences_of_values_of_random_variable.size());
//        ASSUMPTION(m_cumulative_distribution_function.size() == frequences_of_values_of_random_variable.size());
//        // TODO: Check for sortedness of values_of_random_variable!


//        // TODO: the following code is wrong: It does not add values appearing in between two adjacent
//        //       values in the array frequences_of_values_of_random_variable. Linear interpolation
//        //       should be used to compute summary value of all such inner values.


//        natural_64_bit  raw_sum = 0ULL;
//        for (auto frequency : frequences_of_values_of_random_variable)
//            raw_sum += frequency;
//        ASSUMPTION(raw_sum > 0ULL);

//        natural_32_bit const  max_value = std::numeric_limits<natural_32_bit>::max();

//        float_64_bit const  scale = static_cast<float_64_bit>(max_value) / static_cast<float_64_bit>(raw_sum);

//        m_cumulative_distribution_function.at(0) =
//                static_cast<natural_32_bit>(
//                    std::round( scale * frequences_of_values_of_random_variable.at(0) )
//                    );
//        for (natural_32_bit i = 1U; i < m_cumulative_distribution_function.size() - 1U; ++i)
//            m_cumulative_distribution_function.at(i) = m_cumulative_distribution_function.at(i-1) +
//                    static_cast<natural_32_bit>(
//                        std::round( scale * frequences_of_values_of_random_variable.at(i) )
//                        );
//        m_cumulative_distribution_function.at(m_cumulative_distribution_function.size() - 1U) = max_value;
//    }

//    std::vector<natural_32_bit> const&  values_of_random_variable() const { return m_values_of_random_variable; }
//    std::vector<natural_32_bit> const&  cumulative_distribution_function() const { return m_cumulative_distribution_function; }
//    //natural_32_bit  operator ()() const;

//private:
//    std::vector<natural_32_bit> m_values_of_random_variable;
//    std::vector<natural_32_bit> m_cumulative_distribution_function;
//};


//natural_32_bit  get_random_natural_32_bit(probability_mass_function const& mass_function)
//{
//    natural_32_bit const key = get_random_natural_32_bit_in_range(0U,std::numeric_limits<natural_32_bit>::max());

//    std::vector<natural_32_bit>::const_iterator const it =
//            std::lower_bound(
//                    mass_function.cumulative_distribution_function().begin(),
//                    mass_function.cumulative_distribution_function().end(),
//                    key
//                    );
//    INVARIANT(it != mass_function.cumulative_distribution_function().end());
//    if (it == mass_function.cumulative_distribution_function().begin())
//        return mass_function.values_of_random_variable().at(0);

//    natural_32_bit const x1 = std::distance(it,mass_function.cumulative_distribution_function().begin());
//    natural_32_bit const x0 = x1 - 1U;

//    natural_32_bit const y1 = mass_function.cumulative_distribution_function().at(x1);
//    natural_32_bit const y0 = mass_function.cumulative_distribution_function().at(x0);

//    (void)y1;
//    (void)y0;

//    // TODO: not done yet! An index in between [x0,x1] should be computed and returned. Use values
//    //       key, y0, and y1 for the computation. Here key represents a "scale" between y0 and y1.
//    //       But it is NOT a linear interpolation!

//    return x1; // This is not correct (it solves compile error)
//}



////void foo()
////{
////    probability_mass_function  pmf({1,2},{3,4});
////    natural_32_bit  rndval =  get_random_natural_32_bit(pmf);

////}

