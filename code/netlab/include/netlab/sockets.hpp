#ifndef NETLAB_SOCKETS_HPP_INCLUDED
#   define NETLAB_SOCKETS_HPP_INCLUDED

#   include <netlab/uid.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace netlab {


struct  input_socket
{
    uid  other;
    float_32_bit  weight;
};


struct  output_socket
{
    uid  other;
};


}


#endif
