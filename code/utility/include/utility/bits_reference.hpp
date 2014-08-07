#ifndef UTILITY_BITS_REFERENCE_HPP_INCLUDED
#   define UTILITY_BITS_REFERENCE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>


struct bits_const_reference;


struct bits_reference
{
    bits_reference(
        natural_8_bit* const first_byte_ptr,
        natural_8_bit const seek_in_the_first_byte,
        natural_16_bit const num_bits
        );

    operator bits_const_reference() const;

    natural_8_bit* first_byte_ptr();
    natural_8_bit const* first_byte_ptr() const;

    natural_8_bit seek_in_the_first_byte() const;
    natural_16_bit num_bits() const;

private:
    natural_8_bit* m_first_byte_ptr;
    natural_16_bit m_num_bits;
    natural_8_bit m_seek_in_the_first_byte;
};

bool get_bit(bits_reference const& bit_range, natural_16_bit const bit_index);
void set_bit(bits_reference& bit_range, natural_16_bit const bit_index, bool const value);

void bits_to_value(
    bits_reference const& source_of_bits,
    natural_8_bit index_of_the_first_bit,
    natural_8_bit how_many_bits,
    natural_32_bit& variable_where_the_value_will_be_stored
    );

void value_to_bits(
    natural_32_bit const& variable_where_the_value_is_stored,
    natural_8_bit how_many_bits_to_transfer,
    bits_reference& target_bits,
    natural_8_bit index_of_the_first_target_bit
    );



struct bits_const_reference
{
    bits_const_reference(
        natural_8_bit const* first_byte_ptr,
        natural_8_bit const seek_in_the_first_byte,
        natural_16_bit const num_bits
        );

    bits_const_reference(bits_reference const& bits);

    natural_8_bit const* first_byte_ptr() const;

    natural_8_bit seek_in_the_first_byte() const;
    natural_16_bit num_bits() const;

private:
    natural_8_bit const* m_first_byte_ptr;
    natural_16_bit m_num_bits;
    natural_8_bit m_seek_in_the_first_byte;
};

bool get_bit(bits_const_reference const& bit_range, natural_16_bit const bit_index);

template<typename target_variable_type>
void bits_to_value(
    bits_const_reference const& source_of_bits,
    natural_8_bit index_of_the_first_bit,
    natural_8_bit how_many_bits,
    target_variable_type& variable_where_the_value_will_be_stored
    )
{
    bits_to_value(
        bits_reference(
            const_cast<natural_8_bit*>(source_of_bits.first_byte_ptr()),
            source_of_bits.seek_in_the_first_byte(),
            source_of_bits.num_bits()
            ),
        index_of_the_first_bit,
        how_many_bits,
        variable_where_the_value_will_be_stored
        );
}


#endif
