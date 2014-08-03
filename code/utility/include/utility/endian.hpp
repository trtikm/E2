#ifndef UTILITY_ENDIAN_HPP_INCLUDED
#   define UTILITY_ENDIAN_HPP_INCLUDED

bool is_this_big_endian_machine();

//#include <utility/basic_numeric_types.hpp>
//template<typename T>
//void copy_with_other_endian(T const* src, T* dst)
//{
//    for (natural_32_bit i = 0U; i < sizeof(T); ++i)
//        *(reinterpret_cast<natural_8_bit*>(dst) + (sizeof(T) - 1 - i)) =
//            *(reinterpret_cast<natural_8_bit const*>(src) + i);
//}

#endif
