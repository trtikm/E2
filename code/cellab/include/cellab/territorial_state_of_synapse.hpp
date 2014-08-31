#ifndef CELLAB_TERRITORIAL_STATE_OF_SYNAPSE_HPP_INCLUDED
#   define CELLAB_TERRITORIAL_STATE_OF_SYNAPSE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace cellab {


enum territorial_state_of_synapse
{
    SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY = 0,
    MIGRATION_ALONG_POSITIVE_X_AXIS = 1,
    MIGRATION_ALONG_NEGATIVE_X_AXIS = 2,
    MIGRATION_ALONG_POSITIVE_Y_AXIS = 3,
    MIGRATION_ALONG_NEGATIVE_Y_AXIS = 4,
    MIGRATION_ALONG_POSITIVE_COLUMNAR_AXIS = 5,
    MIGRATION_ALONG_NEGATIVE_COLUMNAR_AXIS = 6
};


natural_32_bit  convert_territorial_state_of_synapse_to_territorial_list_index(
        territorial_state_of_synapse const territorial_state
        );


}

#endif
