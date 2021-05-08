#ifndef NETLAB_UNIT_HPP_INCLUDED
#   define NETLAB_UNIT_HPP_INCLUDED

#   include <netlab/sockets.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>

namespace netlab {


struct  computation_unit
{
    float_32_bit  charge;

    std::vector<input_socket>  inputs;      // Is empty and not used, when the unit is in an input layer.
    std::vector<output_socket>  outputs;    // Is empty and not used, when the unit is in an output layer.
};


}


#endif
