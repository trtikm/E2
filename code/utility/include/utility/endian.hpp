#ifndef UTILITY_ENDIAN_HPP_INCLUDED
#   define UTILITY_ENDIAN_HPP_INCLUDED

bool is_this_big_endian_machine();

template<typename T>
void copy_with_other_endian(T const* src, T* dst)
{
    for (unsigned int i = 0; i < sizeof(T); ++i)
        *(reinterpret_cast<unsigned char*>(dst) + (sizeof(T) - 1 - i)) =
            *(reinterpret_cast<unsigned char const*>(src) + i);
}

#endif
