#ifndef NETLAB_EXTRA_DATA_FOR_SPIKERS_HPP_INCLUDED
#   define NETLAB_EXTRA_DATA_FOR_SPIKERS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <vector>

/**
 * These extra data for spikers are only provided to some network initialisation algorithms in order to
 * allow them an effective implementation. The network itself ensures that memory footprint of the constructed
 * network together with the extra data does not exceeds the memory footprint when the network is fully constructed.
 */

namespace netlab {


using  extra_data_for_spikers_in_one_layer = std::vector<float_32_bit>; //!< One float for for each spiker (index of spiker in
                                                                        //!< the layer corresponds to the float at the same index
                                                                        //!< in this vector). Interpretation of this float may be
                                                                        //!< different. But typically it is used for storing density
                                                                        //!< of ships in the sector of a spiker and used in algorithms
                                                                        //!< improving these values by shifting movement area centers
                                                                        //!< to (pseudo-)optimal locations. Then it may also serve as
                                                                        //!< data for computation of statistics of densities of ships
                                                                        //!< which is requested in construction of the network.
using  extra_data_for_spikers_in_layers = std::vector<extra_data_for_spikers_in_one_layer>; //!< For each layer one vector of extra data.


}

#endif
