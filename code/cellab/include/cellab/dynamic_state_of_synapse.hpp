#ifndef CELLAB_DYNAMIC_STATE_OF_SYNAPSE_HPP_INCLUDED
#   define CELLAB_DYNAMIC_STATE_OF_SYNAPSE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>


namespace cellab {


struct bits_reference;
struct bits_const_reference;

struct dynamic_state_of_synapse
{
    dynamic_state_of_synapse& operator<<(bits_reference const& bits);
    dynamic_state_of_synapse& operator<<(bits_const_reference const& bits);
};

bits_reference& operator>>(dynamic_state_of_synapse const& dynamic_data_of_synapse, bits_reference& bits);


}

#endif
