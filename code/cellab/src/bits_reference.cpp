#include <cellab/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <utility/endian.hpp>
#include <algorithm>

#include <utility/development.hpp>

namespace cellab {


static bool read_bit(unsigned char const* first_byte_ptr, unsigned long bit_index)
{
    unsigned long const byte_index = bit_index >> 3U;
    unsigned char const bit_mask = (unsigned char)0x80>>(unsigned char)(bit_index & 7U);
    return (first_byte_ptr[byte_index] & bit_mask) != 0;
}

static void write_bit(unsigned char* first_byte_ptr, unsigned long bit_index, bool const value)
{
    unsigned long const byte_index = bit_index >> 3U;
    unsigned char const bit_mask = (unsigned char)0x80>>(unsigned char)(bit_index & 7U);
    if (value)
        first_byte_ptr[byte_index] |= bit_mask;
    else
        first_byte_ptr[byte_index] &= ~bit_mask;
}

bits_reference::bits_reference(
    unsigned char* first_byte_ptr,
    unsigned char seek_in_the_first_byte,
    unsigned char num_bits
    )
    : m_first_byte_ptr(first_byte_ptr)
    , m_seek_in_the_first_byte(seek_in_the_first_byte)
    , m_num_bits(num_bits)
{
    ASSUMPTION(m_first_byte_ptr != nullptr);
    ASSUMPTION(m_seek_in_the_first_byte < 8U);
    ASSUMPTION(m_num_bits > 0U);
}

unsigned char* bits_reference::first_byte_ptr()
{
    return m_first_byte_ptr;
}

unsigned char const* bits_reference::first_byte_ptr() const
{
    return m_first_byte_ptr;
}

unsigned char bits_reference::seek_in_the_first_byte() const
{
    return m_seek_in_the_first_byte;
}

unsigned char bits_reference::num_bits() const
{
    return m_num_bits;
}

bool get_bit(bits_reference const& bit_range, unsigned char const bit_index)
{
    ASSUMPTION(bit_range.seek_in_the_first_byte() + bit_index < bit_range.num_bits());
    return read_bit(bit_range.first_byte_ptr(),bit_range.seek_in_the_first_byte() + bit_index);
}

void set_bit(bits_reference& bit_range, unsigned char const bit_index, bool const value)
{
    ASSUMPTION(bit_range.seek_in_the_first_byte() + bit_index < bit_range.num_bits());
    write_bit(bit_range.first_byte_ptr(),bit_range.seek_in_the_first_byte() + bit_index,value);
}

void bits_to_value(
    bits_reference const& source_of_bits,
    unsigned char index_of_the_first_bit,
    unsigned char how_many_bits,
    unsigned int& variable_where_the_value_will_be_stored
    )
{
    ASSUMPTION(index_of_the_first_bit + how_many_bits <= source_of_bits.num_bits());
    ASSUMPTION(how_many_bits <= sizeof(variable_where_the_value_will_be_stored) * 8U);

    variable_where_the_value_will_be_stored = 0U;

    unsigned char* target_memory = reinterpret_cast<unsigned char*>(
        &variable_where_the_value_will_be_stored
        );

    bits_reference target_bits(
        target_memory,
        0,
        sizeof(variable_where_the_value_will_be_stored) * 8U
        );

    for (unsigned char i = 0; i < how_many_bits; ++i)
        set_bit(
            target_bits,
            target_bits.num_bits() - (i + 1),
            get_bit(
                source_of_bits,
                index_of_the_first_bit + how_many_bits - (i + 1)
                )
            );

    if (is_this_big_endian_machine())
        std::reverse(
            target_memory,
            target_memory + sizeof(variable_where_the_value_will_be_stored)
            );
}

void value_to_bits(
    unsigned int const& variable_where_the_value_is_stored,
    unsigned char how_many_bits_to_transfer,
    bits_reference& target_bits,
    unsigned char index_of_the_first_target_bit
    )
{
    NOT_IMPLEMENTED_YET();
}


}
