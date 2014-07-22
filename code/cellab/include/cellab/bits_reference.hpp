#ifndef CELLAB_BITS_REFERENCE_HPP_INCLUDED
#   define CELLAB_BITS_REFERENCE_HPP_INCLUDED

namespace cellab {


struct bits_reference
{
    bits_reference(
        unsigned char* first_byte_ptr,
        unsigned char seek_in_the_first_byte,
        unsigned char num_bits
        );

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


}

#endif
