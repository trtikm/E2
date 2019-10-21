#ifndef ANGEO_TENSOR_EQUAL_TO_HPP_INCLUDED
#   define ANGEO_TENSOR_EQUAL_TO_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <functional>

namespace angeo
{

    
template<typename TENSOR_TYPE, natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to;


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<vector2, EPSILON_INVERTED>
{
    typedef vector2 first_argument_type;
    typedef vector2 second_argument_type;
    typedef bool result_type;
    
    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_2d(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<vector3, EPSILON_INVERTED>
{
    typedef vector3 first_argument_type;
    typedef vector3 second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_3d(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<vector4, EPSILON_INVERTED>
{
    typedef vector4 first_argument_type;
    typedef vector4 second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_4d(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<vector6, EPSILON_INVERTED>
{
    typedef vector6 first_argument_type;
    typedef vector6 second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_6d(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<quaternion, EPSILON_INVERTED>
{
    typedef quaternion first_argument_type;
    typedef quaternion second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<matrix22, EPSILON_INVERTED>
{
    typedef matrix22 first_argument_type;
    typedef matrix22 second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_22(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<matrix32, EPSILON_INVERTED>
{
    typedef matrix32 first_argument_type;
    typedef matrix32 second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_32(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<matrix33, EPSILON_INVERTED>
{
    typedef matrix33 first_argument_type;
    typedef matrix33 second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_33(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<matrix43, EPSILON_INVERTED>
{
    typedef matrix43 first_argument_type;
    typedef matrix43 second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_43(left, right, 1.0f / EPSILON_INVERTED);
    }
};


template<natural_32_bit  EPSILON_INVERTED>
struct tensor_equal_to<matrix44, EPSILON_INVERTED>
{
    typedef matrix44 first_argument_type;
    typedef matrix44 second_argument_type;
    typedef bool result_type;

    inline result_type operator()(first_argument_type const& left, second_argument_type const& right) const
    {
        return are_equal_44(left, right, 1.0f / EPSILON_INVERTED);
    }
};


}

#endif
