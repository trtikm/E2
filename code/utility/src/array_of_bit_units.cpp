#include <utility/array_of_bit_units.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/assumptions.hpp>

natural_64_bit num_bytes_to_store_bits(natural_64_bit const num_bits_to_store)
{
    return (num_bits_to_store >> 3U) + (((num_bits_to_store & 7U) == 0U) ? 0U : 1U);
}

natural_64_bit compute_num_bits_of_all_array_units_with_checked_operations(natural_16_bit const num_bits_per_unit,
                                                                           natural_64_bit const num_units)
{
    ASSUMPTION(num_bits_per_unit > 0U);
    ASSUMPTION(num_units > 0U);
    return checked_mul_64_bit(num_bits_per_unit,num_units);
}


array_of_bit_units::array_of_bit_units(natural_16_bit const num_bits_per_unit,natural_64_bit const num_units)
    : m_num_bits_per_unit(num_bits_per_unit)
    , m_num_units(num_units)
    , m_bits_of_all_units(
        new natural_8_bit[
            num_bytes_to_store_bits(
                compute_num_bits_of_all_array_units_with_checked_operations(m_num_bits_per_unit,m_num_units))
            ]
        )
{
    ASSUMPTION(m_num_bits_per_unit > 0U);
    ASSUMPTION(m_num_units > 0U);
}

bits_reference array_of_bit_units::find_bits_of_unit(natural_64_bit const index_of_unit)
{
    ASSUMPTION(index_of_unit < m_num_units);
    natural_64_bit const first_bit_index = index_of_unit * m_num_bits_per_unit;
    return bits_reference(&m_bits_of_all_units[first_bit_index >> 3U],
                          first_bit_index & 7U,
                          m_num_bits_per_unit);
}

natural_16_bit  array_of_bit_units::num_bits_per_unit() const
{
    return m_num_bits_per_unit;
}

natural_64_bit  array_of_bit_units::num_units() const
{
    return m_num_units;
}
