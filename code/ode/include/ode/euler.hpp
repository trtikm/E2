#ifndef ODE_EULER_HPP_INCLUDED
#   define ODE_EULER_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace ode {


template<typename... arg_types>
float_64_bit  euler(float_64_bit const  step, float_64_bit const  value,
                    float_64_bit(*f)(arg_types...), arg_types... args)
{
    return value + step * f(args...);
}


}

#endif
