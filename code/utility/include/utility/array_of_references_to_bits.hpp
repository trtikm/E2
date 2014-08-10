#ifndef UTILITY_ARRAY_OF_REFERENCES_TO_BITS_HPP_INCLUDED
#   define UTILITY_ARRAY_OF_REFERENCES_TO_BITS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/bits_reference.hpp>


struct array_of_references_to_bits
{
    array_of_references_to_bits(
            natural_8_bit* const pointer_to_first_byte_of_the_array,
            natural_8_bit const seek_in_the_first_byte_to_the_first_bit_of_the_array,
            natural_16_bit const num_bits_per_array_element,
            natural_64_bit const num_elements_in_array
            );

    bits_reference get_element(natural_64_bit const index);
    bits_const_reference get_element(natural_64_bit const index) const;

private:


};


#endif
