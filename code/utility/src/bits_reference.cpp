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


namespace private_internal_implementation_details {

bits_reference_impl::bits_reference_impl(
        natural_8_bit* const first_byte_ptr,
        natural_8_bit const shift_in_the_first_byte,
        natural_16_bit const num_bits
        )
    : m_first_byte_ptr(first_byte_ptr)
    , m_num_bits(num_bits)
    , m_shift_in_the_first_byte(shift_in_the_first_byte)
{
    checked_add_16_bit(m_num_bits,m_shift_in_the_first_byte);
}

natural_8_bit* bits_reference_impl::first_byte_ptr()
{
    return m_first_byte_ptr;
}

natural_8_bit const* bits_reference_impl::first_byte_ptr() const
{
    return m_first_byte_ptr;
}

natural_8_bit bits_reference_impl::shift_in_the_first_byte() const
{
    return m_shift_in_the_first_byte;
}

natural_16_bit bits_reference_impl::num_bits() const
{
    return m_num_bits;
}

bits_reference_impl& get_impl(bits_reference const& bits_ref)
{
    return const_cast<bits_reference_impl&>(bits_ref.m_data);
}

bits_reference_impl& get_impl(bits_const_reference const& bits_ref)
{
    return const_cast<bits_reference_impl&>(bits_ref.m_data);
}

static bool  get_bit(bits_reference_impl const& bits_ref, natural_16_bit const bit_index)
{
    ASSUMPTION( bit_index < bits_ref.num_bits() );
    return read_bit(bits_ref.first_byte_ptr(),bits_ref.shift_in_the_first_byte() + bit_index);
}

static void  set_bit(bits_reference_impl& bits_ref, natural_16_bit const bit_index, bool const value)
{
    ASSUMPTION( bit_index < bits_ref.num_bits() );
    write_bit(bits_ref.first_byte_ptr(),bits_ref.shift_in_the_first_byte() + bit_index,value);
}

static void  swap_referenced_bits( bits_reference_impl& left_bits, bits_reference_impl& right_bits)
{
    NOT_IMPLEMENTED_YET();
}

static void bits_to_value(
    bits_reference_impl const& source_bits,
    natural_8_bit index_of_start_bit,
    natural_8_bit how_many_bits,
    natural_32_bit& variable_where_the_value_will_be_stored
    )
{
    ASSUMPTION( index_of_start_bit +  how_many_bits < source_bits.num_bits() );
    ASSUMPTION( how_many_bits <= sizeof(variable_where_the_value_will_be_stored) * 8U );

    variable_where_the_value_will_be_stored = 0U;

    natural_8_bit* const target_memory = reinterpret_cast<natural_8_bit*>(
        &variable_where_the_value_will_be_stored
        );

    bits_reference_impl target_bits(
        target_memory,
        0,
        sizeof(variable_where_the_value_will_be_stored) * 8U
        );

    for (natural_8_bit i = 0; i < how_many_bits; ++i)
        set_bit(
            target_bits,
            target_bits.num_bits() - 1U - i,
            get_bit(
                source_bits,
                index_of_start_bit + i
                )
            );

    if (is_this_big_endian_machine())
        std::reverse(
            target_memory,
            target_memory + sizeof(variable_where_the_value_will_be_stored)
            );
}

void value_to_bits(
    natural_32_bit const variable_where_the_value_is_stored,
    bits_reference_impl& target_bits,
    natural_8_bit const how_many_bits_to_transfer
    )
{
    NOT_IMPLEMENTED_YET();
}

}

namespace details = private_internal_implementation_details;

bits_reference::operator  bits_const_reference() const { return bits_const_reference(*this); }

bits_const_reference::bits_const_reference(bits_reference const& bits)
    : m_data(const_cast<natural_8_bit*>(bits.first_byte_ptr()),bits.shift_in_the_first_byte(),bits.num_bits())
{}

bits_const_reference&  bits_const_reference::operator=(bits_reference const& bits)
{
    m_data = private_internal_implementation_details::bits_reference_impl(
                    const_cast<natural_8_bit*>(bits.first_byte_ptr()),
                    bits.shift_in_the_first_byte(),
                    bits.num_bits()
                    );
    return *this;
}


bool get_bit(bits_reference const& bits_ref, natural_16_bit const bit_index)
{
    return details::get_bit(details::get_impl(bits_ref),bit_index);
}

bool get_bit(bits_const_reference const& bits_ref, natural_16_bit const bit_index)
{
    return details::get_bit(details::get_impl(bits_ref),bit_index);
}

void set_bit(bits_reference& bits_ref, natural_16_bit const bit_index, bool const value)
{
    return details::set_bit(details::get_impl(bits_ref),bit_index,value);
}

void  swap_referenced_bits( bits_reference& left_bits, bits_reference& right_bits)
{
    details::swap_referenced_bits(details::get_impl(left_bits),details::get_impl(right_bits));
}

void bits_to_value(
    bits_reference const& source_bits,
    natural_8_bit index_of_the_first_bit,
    natural_8_bit how_many_bits,
    natural_32_bit& variable_where_the_value_will_be_stored
    )
{
    return details::bits_to_value(
                details::get_impl(source_bits),
                index_of_the_first_bit,
                how_many_bits,
                variable_where_the_value_will_be_stored
                );
}

void bits_to_value(
    bits_const_reference const& source_bits,
    natural_8_bit index_of_the_first_bit,
    natural_8_bit how_many_bits,
    natural_32_bit& variable_where_the_value_will_be_stored
    )
{
    return details::bits_to_value(
                details::get_impl(source_bits),
                index_of_the_first_bit,
                how_many_bits,
                variable_where_the_value_will_be_stored
                );
}

void value_to_bits(
    natural_32_bit const variable_where_the_value_is_stored,
    bits_reference& target_bits,
    natural_8_bit const how_many_bits_to_transfer
    )
{
    details::value_to_bits(
                variable_where_the_value_is_stored,
                details::get_impl(target_bits),
                how_many_bits_to_transfer
                );
}

void value_to_bits(
    natural_32_bit const variable_where_the_value_is_stored,
    bits_reference& target_bits
    )
{
    value_to_bits( variable_where_the_value_is_stored, target_bits, target_bits.num_bits() );
}
