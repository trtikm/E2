#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <utility/endian.hpp>
#include <utility/checked_number_operations.hpp>
#include <algorithm>

#include <utility/development.hpp>


static bool read_bit(natural_8_bit const* first_byte_ptr, natural_16_bit bit_index)
{
    natural_16_bit const byte_index = bit_index >> 3U;
    natural_8_bit const bit_mask = (natural_8_bit)0x80>>(natural_8_bit)(bit_index & 7U);
    return (first_byte_ptr[byte_index] & bit_mask) != 0;
}

static void write_bit(natural_8_bit* first_byte_ptr, natural_16_bit bit_index, bool const value)
{
    natural_16_bit const byte_index = bit_index >> 3U;
    natural_8_bit const bit_mask = (natural_8_bit)0x80>>(natural_8_bit)(bit_index & 7U);
    if (value)
        first_byte_ptr[byte_index] |= bit_mask;
    else
        first_byte_ptr[byte_index] &= ~bit_mask;
}

bits_reference::bits_reference(
    natural_8_bit* const first_byte_ptr,
    natural_8_bit const seek_in_the_first_byte,
    natural_16_bit const num_bits
    )
    : m_first_byte_ptr(first_byte_ptr)
    , m_num_bits(num_bits)
    , m_seek_in_the_first_byte(seek_in_the_first_byte)
{
    ASSUMPTION(m_first_byte_ptr != nullptr);
    ASSUMPTION(m_seek_in_the_first_byte < 8U);
    ASSUMPTION(m_num_bits > 0U);
    checked_add_16_bit(static_cast<natural_16_bit>(m_seek_in_the_first_byte),m_num_bits);
}

//bits_reference::operator bits_const_reference() const
//{
//    return bits_const_reference(*this);
//}

natural_8_bit* bits_reference::first_byte_ptr()
{
    return m_first_byte_ptr;
}

natural_8_bit const* bits_reference::first_byte_ptr() const
{
    return m_first_byte_ptr;
}

natural_8_bit bits_reference::seek_in_the_first_byte() const
{
    return m_seek_in_the_first_byte;
}

natural_16_bit bits_reference::num_bits() const
{
    return m_num_bits;
}

bits_const_reference::bits_const_reference(
    natural_8_bit const* first_byte_ptr,
    natural_8_bit const seek_in_the_first_byte,
    natural_16_bit const num_bits
    )
    : m_first_byte_ptr(first_byte_ptr)
    , m_num_bits(num_bits)
    , m_seek_in_the_first_byte(seek_in_the_first_byte)
{
    ASSUMPTION(m_first_byte_ptr != nullptr);
    ASSUMPTION(m_seek_in_the_first_byte < 8U);
    ASSUMPTION(m_num_bits > 0U);
    checked_add_16_bit(static_cast<natural_16_bit>(m_seek_in_the_first_byte),m_num_bits);
}

bits_const_reference::bits_const_reference(bits_reference const& bits)
    : m_first_byte_ptr(bits.first_byte_ptr())
    , m_num_bits(bits.num_bits())
    , m_seek_in_the_first_byte(bits.seek_in_the_first_byte())
{}

natural_8_bit const* bits_const_reference::first_byte_ptr() const
{
    return m_first_byte_ptr;
}

natural_8_bit bits_const_reference::seek_in_the_first_byte() const
{
    return m_seek_in_the_first_byte;
}

natural_16_bit bits_const_reference::num_bits() const
{
    return m_num_bits;
}

bool get_bit(bits_const_reference const& bit_range, natural_16_bit const bit_index)
{
    ASSUMPTION(
        static_cast<natural_32_bit>(bit_range.seek_in_the_first_byte()) +  static_cast<natural_32_bit>(bit_index)
        < bit_range.num_bits()
        );
    return read_bit(bit_range.first_byte_ptr(),bit_range.seek_in_the_first_byte() + bit_index);
}

bool get_bit(bits_reference const& bit_range, natural_16_bit const bit_index)
{
    ASSUMPTION(
        static_cast<natural_32_bit>(bit_range.seek_in_the_first_byte()) +  static_cast<natural_32_bit>(bit_index)
        < bit_range.num_bits()
        );
    return read_bit(bit_range.first_byte_ptr(),bit_range.seek_in_the_first_byte() + bit_index);
}

void set_bit(bits_reference& bit_range, natural_16_bit const bit_index, bool const value)
{
    ASSUMPTION(
        static_cast<natural_32_bit>(bit_range.seek_in_the_first_byte()) +  static_cast<natural_32_bit>(bit_index)
        < bit_range.num_bits()
        );
    write_bit(bit_range.first_byte_ptr(),bit_range.seek_in_the_first_byte() + bit_index,value);
}

void bits_to_value(
    bits_reference const& source_of_bits,
    natural_8_bit index_of_the_first_bit,
    natural_8_bit how_many_bits,
    natural_32_bit& variable_where_the_value_will_be_stored
    )
{
    ASSUMPTION(
        static_cast<natural_16_bit>(index_of_the_first_bit) +  static_cast<natural_16_bit>(how_many_bits)
        < source_of_bits.num_bits()
        );
    ASSUMPTION(static_cast<natural_32_bit>(how_many_bits) <= sizeof(variable_where_the_value_will_be_stored) * 8U);

    variable_where_the_value_will_be_stored = 0U;

    natural_8_bit* target_memory = reinterpret_cast<natural_8_bit*>(
        &variable_where_the_value_will_be_stored
        );

    bits_reference target_bits(
        target_memory,
        0,
        sizeof(variable_where_the_value_will_be_stored) * 8U
        );

    for (natural_8_bit i = 0; i < how_many_bits; ++i)
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
    natural_32_bit const& variable_where_the_value_is_stored,
    natural_8_bit how_many_bits_to_transfer,
    bits_reference& target_bits,
    natural_8_bit index_of_the_first_target_bit
    )
{
    NOT_IMPLEMENTED_YET();
}
