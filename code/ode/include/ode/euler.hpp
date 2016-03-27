#ifndef ODE_SOLVERS_HPP_INCLUDED
#   define ODE_SOLVERS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <functional>
#   include <vector>

namespace ode {


using  derivation_function_type = std::function<float_64_bit(std::vector<float_64_bit> const&)>;


derivation_function_type  constant_derivative(float_64_bit const  value);
std::vector<derivation_function_type>  constant_system(std::vector<float_64_bit> const&  data);


using  solver_function_type0 =
            void(*)(float_64_bit,std::vector<ode::derivation_function_type> const&,
                    std::vector<float_64_bit> const&, std::vector<float_64_bit>&);

using  solver_function_type1 =
            void(*)(float_64_bit,std::vector<ode::derivation_function_type> const&,
                    std::vector<float_64_bit>&);


void  evaluate(std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit> const& V,
               std::vector<float_64_bit>& output_V);

void  evaluate(std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit>& V);


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

void  runge_kutta_4(float_64_bit const  h,
                    std::vector<derivation_function_type> const&  S,
                    std::vector<float_64_bit> const& V,
                    std::vector<float_64_bit>& output_V);

void  runge_kutta_4(float_64_bit const  h,
                    std::vector<derivation_function_type> const&  S,
                    std::vector<float_64_bit>& V);



template<typename... arg_types>
float_64_bit  euler(float_64_bit const  h, float_64_bit const  value,
                    float_64_bit(*f)(arg_types...), arg_types... args)
{
    return value + h * f(args...);
}


}

#endif
