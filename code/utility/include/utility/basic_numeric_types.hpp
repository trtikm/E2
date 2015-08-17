#ifndef UTILITY_BASIC_NUMERIC_TYPES_HPP_INCLUDED
#   define UTILITY_BASIC_NUMERIC_TYPES_HPP_INCLUDED

#   include <utility/typefn_if_then_else.hpp>


typedef typefn_if_then_else<sizeof(char)*8U == 8U,char,void>::result
        integer_8_bit;

typedef typefn_if_then_else<sizeof(short)*8U == 16U,short,void>::result
        integer_16_bit;

typedef typefn_if_then_else<sizeof(int)*8U == 32U,int,void>::result
        integer_32_bit;

typedef typefn_if_then_else<sizeof(long long)*8U == 64U,long long,void>::result
        integer_64_bit;

typedef typefn_if_then_else<sizeof(unsigned char)*8U == 8U,unsigned char,void>::result
        natural_8_bit;

typedef typefn_if_then_else<sizeof(unsigned short)*8U == 16U,unsigned short,void>::result
        natural_16_bit;

typedef typefn_if_then_else<sizeof(unsigned int)*8U == 32U,unsigned int,void>::result
        natural_32_bit;

typedef typefn_if_then_else<sizeof(unsigned long long)*8U == 64U,unsigned long long,void>::result
        natural_64_bit;

typedef typefn_if_then_else<sizeof(float)*8U == 32U,float,void>::result
        float_32_bit;

typedef typefn_if_then_else<sizeof(double)*8U == 64U,double,void>::result
        float_64_bit;

#endif
