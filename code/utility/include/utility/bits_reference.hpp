#ifndef UTILITY_BITS_REFERENCE_HPP_INCLUDED
#   define UTILITY_BITS_REFERENCE_HPP_INCLUDED


struct bits_const_reference;


struct bits_reference
{
    bits_reference(
        unsigned char* const first_byte_ptr,
        unsigned char const seek_in_the_first_byte,
        unsigned char const num_bits
        );

    operator bits_const_reference() const;

    unsigned char* first_byte_ptr();
    unsigned char const* first_byte_ptr() const;

    unsigned char seek_in_the_first_byte() const;
    unsigned char num_bits() const;

private:
    unsigned char* m_first_byte_ptr;
    unsigned char m_seek_in_the_first_byte;
    unsigned char m_num_bits;
};

bool get_bit(bits_reference const& bit_range, unsigned char const bit_index);
void set_bit(bits_reference& bit_range, unsigned char const bit_index, bool const value);

void bits_to_value(
    bits_reference const& source_of_bits,
    unsigned char index_of_the_first_bit,
    unsigned char how_many_bits,
    unsigned int& variable_where_the_value_will_be_stored
    );

void value_to_bits(
    unsigned int const& variable_where_the_value_is_stored,
    unsigned char how_many_bits_to_transfer,
    bits_reference& target_bits,
    unsigned char index_of_the_first_target_bit
    );



struct bits_const_reference
{
    bits_const_reference(
        unsigned char const* first_byte_ptr,
        unsigned char const seek_in_the_first_byte,
        unsigned char const num_bits
        );

    bits_const_reference(bits_reference const& bits);

    unsigned char const* first_byte_ptr() const;

    unsigned char seek_in_the_first_byte() const;
    unsigned char num_bits() const;

private:
    unsigned char const* m_first_byte_ptr;
    unsigned char m_seek_in_the_first_byte;
    unsigned char m_num_bits;
};

bool get_bit(bits_const_reference const& bit_range, unsigned char const bit_index);

template<typename target_variable_type>
void bits_to_value(
    bits_const_reference const& source_of_bits,
    unsigned char index_of_the_first_bit,
    unsigned char how_many_bits,
    target_variable_type& variable_where_the_value_will_be_stored
    )
{
    bits_to_value(
        bits_reference(
            const_cast<unsigned char*>(source_of_bits.first_byte_ptr()),
            source_of_bits.seek_in_the_first_byte(),
            source_of_bits.num_bits()
            ),
        index_of_the_first_bit,
        how_many_bits,
        variable_where_the_value_will_be_stored
        );
}


#endif
