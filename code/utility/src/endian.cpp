#include <utility/endian.hpp>
#include <utility/basic_numeric_types.hpp>

bool is_this_big_endian_machine()
{
    natural_32_bit const value = 1U;
    return *reinterpret_cast<natural_8_bit const*>(&value) == 1U;
}
