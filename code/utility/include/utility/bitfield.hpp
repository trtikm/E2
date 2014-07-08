#ifndef BITFIELD_HPP_INCLUDED
#   define BITFIELD_HPP_INCLUDED

#   include <utility/endian.hpp>
#   include <utility/assumptions.hpp>
#   include <boost/noncopyable.hpp>

namespace e2_details
{
    bool read_from_bitfield(unsigned char const* field_begin, unsigned long bit_index);
    void write_to_bitfield(unsigned char* field_begin, unsigned long bit_index,
                           bool const value);
}

template<typename BASE_TYPE>
struct bitbuffer
{
    typedef BASE_TYPE base_type;
    typedef unsigned int uint;
    typedef unsigned char byte;

    bitbuffer(base_type const value = 0)
        : buffer()
    {
        if (is_this_big_endian_machine())
            buffer = value;
        else
            copy_with_other_endian(&value,&buffer);
    }

    base_type value() const
    {
        if (is_this_big_endian_machine())
            return buffer;
        base_type result;
        copy_with_other_endian(&buffer,&result);
        return result;
    }

    operator base_type() const { return value(); }

    static uint numbits() { return 8*sizeof(base_type); }

    bool getbit(uint const bit_index) const
    {
        ASSUMPTION(bit_index < numbits());
        return e2_details::read_from_bitfield(reinterpret_cast<unsigned char const*>(&buffer),
                                              numbits() - 1 - bit_index);
    }

    void setbit(uint const bit_index, bool const value)
    {
        ASSUMPTION(bit_index < numbits());
        return e2_details::write_to_bitfield(reinterpret_cast<unsigned char*>(&buffer),
                                             numbits() - 1 - bit_index,value);
    }

private:
    base_type buffer;
};

struct bitfield : private boost::noncopyable
{
    typedef unsigned long ulong;
    typedef unsigned char byte;

    bitfield(ulong const numbits)
        : byte_field(new byte[(8*(numbits/8)==numbits) ? numbits : numbits + 1])
        , num_bits(numbits)
    {}

    ~bitfield()
    {
        delete [] byte_field;
    }

    ulong numbits() const { return num_bits; }

    bool getbit(ulong const bit_index) const
    {
        ASSUMPTION(bit_index < numbits());
        return e2_details::read_from_bitfield(byte_field,bit_index);
    }

    void setbit(ulong const bit_index, bool const value)
    {
        ASSUMPTION(bit_index < numbits());
        return e2_details::write_to_bitfield(byte_field,bit_index,value);
    }

private:
    byte* byte_field;
    ulong num_bits;
};

template<typename BUFFER_BASE_TYPE>
bitbuffer<BUFFER_BASE_TYPE>
getbits(bitfield const& field, bitfield::ulong const start_bit_index,
        typename bitbuffer<BUFFER_BASE_TYPE>::uint num_bits
            //= sizeof(BUFFER_BASE_TYPE)
        )
{
    ASSUMPTION(num_bits <= bitbuffer<BUFFER_BASE_TYPE>::numbits());
    ASSUMPTION(start_bit_index + num_bits <= field.numbits());
    bitbuffer<BUFFER_BASE_TYPE> buffer;
    for (typename bitbuffer<BUFFER_BASE_TYPE>::uint i = 0; i < num_bits; ++i)
        buffer.setbit(i,field.getbit(start_bit_index + i));
    return buffer;
}

template<typename BUFFER_BASE_TYPE>
void setbits(bitfield& field, bitfield::ulong const start_bit_index,
             typename bitbuffer<BUFFER_BASE_TYPE>::uint num_bits,
             bitbuffer<BUFFER_BASE_TYPE> buffer)
{
    ASSUMPTION(num_bits <= bitbuffer<BUFFER_BASE_TYPE>::numbits());
    ASSUMPTION(start_bit_index + num_bits <= field.numbits());
    for (typename bitbuffer<BUFFER_BASE_TYPE>::uint i = 0; i < num_bits; ++i)
        field.setbit(start_bit_index + i,buffer.getbit(i));
}

#endif
