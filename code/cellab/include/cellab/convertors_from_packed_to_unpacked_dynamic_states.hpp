#ifndef CELLAB_CONVERTORS_FROM_PACKED_TO_UNPACKED_DYNAMIC_STATE_HPP_INCLUDED
#   define CELLAB_CONVERTORS_FROM_PACKED_TO_UNPACKED_DYNAMIC_STATE_HPP_INCLUDED

#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <functional>

namespace cellab { namespace private_internal_implementation_details {


template<typename T>
T get_instance_at_index(
        std::function<bits_const_reference(natural_32_bit)> const& get_bits_function,
        natural_32_bit const index)
{
    bits_const_reference const bits = get_bits_function(index);
    ASSUMPTION(T::num_packed_bits() == bits.num_bits());
    return T(bits);
    //return T(get_bits_function(index));
}

template<typename T>
T get_instance_at_coordinates(
        std::function<bits_const_reference(natural_8_bit,natural_8_bit,natural_8_bit)> const& get_bits_function,
        natural_8_bit const x_axis_coordinate,
        natural_8_bit const y_axis_coordinate,
        natural_8_bit const columnar_axis_coordinate
        )
{
    bits_const_reference const bits =
            get_bits_function(x_axis_coordinate,y_axis_coordinate,columnar_axis_coordinate);
    ASSUMPTION(T::num_packed_bits() == bits.num_bits());
    return T(bits);
    //return T(get_bits_function(x_axis_coordinate,y_axis_coordinate,columnar_axis_coordinate));
}


}}

#endif
