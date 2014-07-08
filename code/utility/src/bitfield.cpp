#include <utility/bitfield.hpp>

namespace e2_details
{

    bool read_from_bitfield(unsigned char const* field_begin, unsigned long bit_index)
    {
        unsigned long const byte_index = bit_index >> 3U;
        unsigned char const bit_mask = (unsigned char)0x80>>(unsigned char)(bit_index & 7U);
        return (field_begin[byte_index] & bit_mask) != 0;
    }

    void write_to_bitfield(unsigned char* field_begin, unsigned long bit_index,
                           bool const value)
    {
        unsigned long const byte_index = bit_index >> 3U;
        unsigned char const bit_mask = (unsigned char)0x80>>(unsigned char)(bit_index & 7U);
        if (value)
            field_begin[byte_index] |= bit_mask;
        else
            field_begin[byte_index] &= ~bit_mask;
    }

}
