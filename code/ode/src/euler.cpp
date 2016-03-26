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

void  euler(float_64_bit const  h,
            std::vector<derivation_function_type> const&  S,
            std::vector<float_64_bit> const& V,
            std::vector<float_64_bit>& output_V)
{
    ASSUMPTION(h >= 0.0);
    ASSUMPTION(!S.empty());
    ASSUMPTION(S.size() == V.size());
    ASSUMPTION(output_V.empty() || output_V.size() == S.size());

    output_V.resize(S.size());
    for (natural_64_bit  i = 0ULL; i < S.size(); ++i)
        output_V[i] = V[i] + h * S[i](V);
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
    ASSUMPTION(h >= 0.0);
    ASSUMPTION(!S.empty());
    ASSUMPTION(S.size() == V.size());
    ASSUMPTION(output_V.empty() || output_V.size() == S.size());

    std::vector<float_64_bit> dF;
    euler(h * 0.5,S,V,dF);
    euler(h,constant_system(dF),V,output_V);
}

void  midpoint(float_64_bit const  h,
               std::vector<derivation_function_type> const&  S,
               std::vector<float_64_bit>& V)
{
    std::vector<float_64_bit> const  temp = V;
    midpoint(h,S,temp,V);
}


}


//enum
//{
//    U = 0U,
//    V = 1U,
//};

//static float_64_bit  dU(std::vector<float_64_bit> const&  values)
//{
//    return values[V] - 0.5*values[U];
//}

//static float_64_bit  dV(std::vector<float_64_bit> const&  values)
//{
//    return values[V] + values[U]/5.0;
//}


//static void foo()
//{
//    std::vector<float_64_bit>  values{ 1.0, 2.0 };
//    ode::midpoint(0.1,{dU,dV},values);
//}
