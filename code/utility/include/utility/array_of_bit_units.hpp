#ifndef UTILITY_ARRAY_OF_BIT_UNITS_HPP_INCLUDED
#   define UTILITY_ARRAY_OF_BIT_UNITS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/bits_reference.hpp>
#   include <boost/noncopyable.hpp>
#   include <boost/scoped_array.hpp>


struct array_of_bit_units : private boost::noncopyable
{
    array_of_bit_units(natural_16_bit const num_bits_per_unit, natural_64_bit const num_units);
    bits_reference find_bits_of_unit(natural_64_bit const index_of_unit);
    natural_16_bit num_bits_per_unit() const;
    natural_64_bit num_units() const;
private:
    natural_64_bit m_num_bits_per_unit;
    natural_64_bit m_num_units;
    boost::scoped_array<natural_8_bit> m_bits_of_all_units;
};


natural_64_bit num_bytes_to_store_bits(natural_64_bit const num_bits_to_store);

natural_64_bit compute_num_bits_of_all_array_units_with_checked_operations(natural_16_bit const num_bits_per_unit,
                                                                           natural_64_bit const num_units);


#endif
