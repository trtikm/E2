#ifndef ODE_EULER_HPP_INCLUDED
#   define ODE_EULER_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <functional>
#   include <vector>

namespace ode {


using  derivation_function_type = std::function<float_64_bit(std::vector<float_64_bit> const&)>;


derivation_function_type  constant_derivative(float_64_bit const  value);
std::vector<derivation_function_type>  constant_system(std::vector<float_64_bit> const&  data);


void  euler(float_64_bit const  h,
            std::vector<derivation_function_type> const&  S,
            std::vector<float_64_bit> const&  V,
            std::vector<float_64_bit>&  output_V);

void  euler(float_64_bit const  h,
            std::vector<derivation_function_type> const&  S,
            std::vector<float_64_bit>&  V);

void  midpoint(float_64_bit const  h,
               std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit> const&  V,
               std::vector<float_64_bit>&  output_V);

void  midpoint(float_64_bit const  h,
               std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit>&  V);



template<typename... arg_types>
float_64_bit  euler(float_64_bit const  step, float_64_bit const  value,
                    float_64_bit(*f)(arg_types...), arg_types... args)
{
    return value + step * f(args...);
}


}

#endif
