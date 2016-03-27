#include <ode/euler.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility>

namespace ode {


derivation_function_type  constant_derivative(float_64_bit const  value)
{
    return std::bind([](std::vector<float_64_bit> const&, float_64_bit const data){ return data; },
                     std::placeholders::_1,value);
}

std::vector<derivation_function_type>  constant_system(std::vector<float_64_bit> const&  data)
{
    std::vector<derivation_function_type>  S;
    for (float_64_bit  value : data)
        S.push_back(constant_derivative(value));
    return std::move(S);
}

void  evaluate(std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit> const& V,
               std::vector<float_64_bit>& output_V)
{
    ASSUMPTION(!S.empty());
    ASSUMPTION(S.size() == V.size());
    ASSUMPTION(output_V.empty() || output_V.size() == S.size());

    output_V.resize(S.size());
    for (natural_64_bit  i = 0ULL; i < S.size(); ++i)
        output_V[i] = S[i](V);
}

void  evaluate(std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit>& V)
{
    std::vector<float_64_bit> const  temp = V;
    evaluate(S,temp,V);
}

void  euler(float_64_bit const  h,
            std::vector<derivation_function_type> const&  S,
            std::vector<float_64_bit> const& V,
            std::vector<float_64_bit>& output_V)
{
    evaluate(S,V,output_V);
    for (natural_64_bit  i = 0ULL; i < S.size(); ++i)
        output_V[i] = V[i] + h * output_V[i];
}

void  euler(float_64_bit const  h,
            std::vector<derivation_function_type> const&  S,
            std::vector<float_64_bit>& V)
{
    std::vector<float_64_bit> const  temp = V;
    euler(h,S,temp,V);
}

void  midpoint(float_64_bit const  h,
               std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit> const& V,
               std::vector<float_64_bit>& output_V)
{
    std::vector<float_64_bit> middle;
    euler(h * 0.5,S,V,middle);
    evaluate(S,middle);
    euler(h,constant_system(middle),V,output_V);
}

void  midpoint(float_64_bit const  h,
               std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit>& V)
{
    std::vector<float_64_bit> const  temp = V;
    midpoint(h,S,temp,V);
}

void  runge_kutta_4(float_64_bit const  h,
                    std::vector<derivation_function_type> const&  S,
                    std::vector<float_64_bit> const& V,
                    std::vector<float_64_bit>& output_V)
{
    std::vector<float_64_bit> temp;

    std::vector<float_64_bit> k1;
    evaluate(S,V,k1);
    std::vector<float_64_bit> k2;
    euler(h * 0.5,constant_system(k1),V,temp);
    evaluate(S,temp,k2);
    std::vector<float_64_bit> k3;
    euler(h * 0.5,constant_system(k2),V,temp);
    evaluate(S,temp,k3);
    std::vector<float_64_bit> k4;
    euler(h,constant_system(k3),V,temp);
    evaluate(S,temp,k4);

    for (natural_64_bit  i = 0ULL; i < S.size(); ++i)
        output_V[i] = V[i] + h * (k1[i] + 2.0 * (k2[i] + k3[i]) + k4[i]) / 6.0;
}

void  runge_kutta_4(float_64_bit const  h,
                    std::vector<derivation_function_type> const&  S,
                    std::vector<float_64_bit>& V)
{
    std::vector<float_64_bit> const  temp = V;
    runge_kutta_4(h,S,temp,V);
}


}
