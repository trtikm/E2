#ifndef NETLAB_EXTRA_DATA_FOR_SPIKERS_HPP_INCLUDED
#   define NETLAB_EXTRA_DATA_FOR_SPIKERS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/assumptions.hpp>
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


struct  accessor_to_extra_data_for_spikers_in_layers
{
    explicit accessor_to_extra_data_for_spikers_in_layers(extra_data_for_spikers_in_layers* const  data)
        : m_data(data)
    {
        ASSUMPTION(m_data != nullptr);
    }

    extra_data_for_spikers_in_one_layer::value_type  get_extra_data_of_spiker(
            layer_index_type const  layer_index,
            object_index_type const  object_index
            ) const
    {
        ASSUMPTION(layer_index < m_data->size() && object_index < m_data->at(layer_index).size());
        return m_data->at(layer_index).at(object_index);
    }

    void  set_extra_data_of_spiker(
        layer_index_type const  layer_index,
        object_index_type const  object_index,
        extra_data_for_spikers_in_one_layer::value_type const  value
        )
    {
        ASSUMPTION(layer_index < m_data->size() && object_index < m_data->at(layer_index).size());
        m_data->at(layer_index).at(object_index) = value;
    }

    void  add_value_to_extra_data_of_spiker(
        layer_index_type const  layer_index,
        object_index_type const  object_index,
        extra_data_for_spikers_in_one_layer::value_type const  value_to_add
        )
    {
        ASSUMPTION(layer_index < m_data->size() && object_index < m_data->at(layer_index).size());
        m_data->at(layer_index).at(object_index) += value_to_add;
    }

private:
    accessor_to_extra_data_for_spikers_in_layers() = delete;
    accessor_to_extra_data_for_spikers_in_layers(accessor_to_extra_data_for_spikers_in_layers const&) = delete;
    accessor_to_extra_data_for_spikers_in_layers& operator=(accessor_to_extra_data_for_spikers_in_layers const&) = delete;
    accessor_to_extra_data_for_spikers_in_layers(accessor_to_extra_data_for_spikers_in_layers&&) = delete;
    accessor_to_extra_data_for_spikers_in_layers& operator=(accessor_to_extra_data_for_spikers_in_layers&&) = delete;

    extra_data_for_spikers_in_layers*  m_data;
};


}

#endif
