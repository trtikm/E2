#ifndef CELLAB_TERRITORIAL_STATE_OF_SYNAPSE_HPP_INCLUDED
#   define CELLAB_TERRITORIAL_STATE_OF_SYNAPSE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace cellab {


/**
 * These numeric constants specify a desired behaviour of a synapse in a territory of the neural.
 * tissue. There are only 7 possible types of behaviour (territorial states) 0,&hellip;,6 for any
 * synapse. The value 0 means that the synapse is connected to the target cell appearing in the same
 * territory as the synapse, and the remaining 6 values say that the synapse only passes through
 * its current territory to another territory in the neighbourhood. The synapse can move along
 * one of three coordinate axes (x, y, or columnar) in either positive or negative direction,
 * which gives us 6 possible moves.
 */
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


/**
 * All synapses in any territory are stored in three aligned lists. These lists are internally
 * sorted such that the territory starts by all synapses with the territorial states 0, then follow
 * all synapses with the territorial state 1, and so on. Therefore, values of possible territorial states
 * (see the enum above) also direcly represent indices of territorial (sub-)lists. It means that
 * this function is the identity (only the type of numbers is changed, from signed to unsigned).
 */
natural_32_bit  convert_territorial_state_of_synapse_to_territorial_list_index(
        territorial_state_of_synapse const territorial_state
        );


}

#endif
