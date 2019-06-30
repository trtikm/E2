#ifndef ANGEO_TENSOR_STD_SPECIALISATIONS_HPP_INCLUDED
#   define ANGEO_TENSOR_STD_SPECIALISATIONS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <utility/hash_combine.hpp>

namespace std
{


template<>
struct hash<vector3>
{
    inline size_t operator()(vector3 const& v) const
    {
        size_t seed = 0;
        ::hash_combine(seed, v(0));
        ::hash_combine(seed, v(1));
        ::hash_combine(seed, v(2));
        return seed;
    }
};

template<>
struct equal_to<vector3>
{
    typedef vector3  first_argument_type;
    typedef vector3  second_argument_type;
    typedef bool result_type;

    inline bool operator()(vector3 const&  left, vector3 const&  right) const
    {
        return are_equal_3d(left, right, 0.0001f);
    }
};


}

#endif
