#ifndef ANGEO_TENSOR_HASH_HPP_INCLUDED
#   define ANGEO_TENSOR_HASH_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <functional>
#   include <utility/hash_combine.hpp>

namespace angeo
{


template<typename TENSOR_TYPE, natural_32_bit  EPSILON_INVERTED>
struct tensor_hash;


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<vector2, EPSILON_INVERTED>
{
    typedef vector2 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& v) const
    {
        result_type seed = 0;
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(0)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(1)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<vector3, EPSILON_INVERTED>
{
    typedef vector3 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& v) const
    {
        result_type seed = 0;
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(0)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(1)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(2)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<vector4, EPSILON_INVERTED>
{
    typedef vector4 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& v) const
    {
        result_type seed = 0;
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(0)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(1)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(2)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(3)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<vector6, EPSILON_INVERTED>
{
    typedef vector6 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& v) const
    {
        result_type seed = 0;
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(0)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(1)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(2)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(3)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(4)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * v(5)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<quaternion, EPSILON_INVERTED>
{
    typedef quaternion argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& q) const
    {
        result_type seed = 0;
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * q.coeffs()(0)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * q.coeffs()(1)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * q.coeffs()(2)));
        ::hash_combine(seed, (result_type)(EPSILON_INVERTED * q.coeffs()(3)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<matrix22, EPSILON_INVERTED>
{
    typedef matrix22 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& M) const
    {
        result_type seed = 0;
        for (int i = 0; i != 2; ++i)
            for (int j = 0; j != 2; ++j)
                ::hash_combine(seed, (result_type)(EPSILON_INVERTED * M(i, j)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<matrix32, EPSILON_INVERTED>
{
    typedef matrix32 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& M) const
    {
        result_type seed = 0;
        for (int i = 0; i != 3; ++i)
            for (int j = 0; j != 2; ++j)
                ::hash_combine(seed, (result_type)(EPSILON_INVERTED * M(i, j)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<matrix33, EPSILON_INVERTED>
{
    typedef matrix33 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& M) const
    {
        result_type seed = 0;
        for (int i = 0; i != 3; ++i)
            for (int j = 0; j != 3; ++j)
                ::hash_combine(seed, (result_type)(EPSILON_INVERTED * M(i,j)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<matrix43, EPSILON_INVERTED>
{
    typedef matrix43 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& M) const
    {
        result_type seed = 0;
        for (int i = 0; i != 4; ++i)
            for (int j = 0; j != 3; ++j)
                ::hash_combine(seed, (result_type)(EPSILON_INVERTED * M(i, j)));
        return seed;
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_hash<matrix44, EPSILON_INVERTED>
{
    typedef matrix44 argument_type;
    typedef size_t result_type;

    inline result_type operator()(argument_type const& M) const
    {
        result_type seed = 0;
        for (int i = 0; i != 4; ++i)
            for (int j = 0; j != 4; ++j)
                ::hash_combine(seed, (result_type)(EPSILON_INVERTED * M(i, j)));
        return seed;
    }
};


}

#endif
