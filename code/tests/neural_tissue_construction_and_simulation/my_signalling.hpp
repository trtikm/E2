#ifndef E2_TEST_NEURAL_TISSUE_CONSTRUCTION_AND_SIMULATION_MY_SIGNALLING_HPP_INCLUDED
#   define E2_TEST_NEURAL_TISSUE_CONSTRUCTION_AND_SIMULATION_MY_SIGNALLING_HPP_INCLUDED

#   include <utility/bits_reference.hpp>


struct my_signalling
{
    my_signalling(bits_reference const& bits) : m_count(bits_to_value<natural_32_bit>(bits)) {}
    my_signalling(bits_const_reference const& bits) : m_count(bits_to_value<natural_32_bit>(bits)) {}
    void  operator>>(bits_reference const& bits) const { value_to_bits(count(),bits); }
    void  increment() noexcept { ++m_count; }
    natural_32_bit  count() const noexcept { return m_count; }
private:
    natural_32_bit  m_count;
};


#endif
