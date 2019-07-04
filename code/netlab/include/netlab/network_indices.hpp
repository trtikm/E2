#ifndef NETLAB_NETWORK_INDICES_HPP_INCLUDED
#   define NETLAB_NETWORK_INDICES_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <functional>
#   include <limits>

namespace netlab {


/**
 * Each object in the network is be uniquely identified by:
 *      - index of a network layer in which the object resides,
 *      - index of the object inside that network layer.
 * Such pair thus uniquely identifies an access path to
 * to object in the network. We introduce types
 *      layer_index_type, and
 *      object_index_type
 * to describe type of both indices of the pair. We further
 * introduce a type
 *      compressed_layer_and_object_indices
 * for storing a pair of those indices in a memory-efficient
 * way.
 */


using  layer_index_type = natural_8_bit;

inline constexpr layer_index_type  max_layer_index() noexcept { return std::numeric_limits<layer_index_type>::max(); }
inline constexpr natural_64_bit  max_number_of_layers() noexcept { return max_layer_index() + 1UL; }


using  object_index_type = natural_64_bit;

inline constexpr object_index_type  max_object_index() noexcept { return 0x0000ffffffffffffUL; }
inline constexpr natural_64_bit  max_number_of_objects_in_layer() noexcept { return max_object_index() + 1UL; }


/**
 * Each object in the network is be uniquely identified by:
 *      - index of a network layer in which the object resides,
 *      - index of the object inside that network layer.
 * Such pair thus uniquely identifies an access path to
 * to object in the network. This data structure defines
 * the triple in efficient compressed form.
 * It is a compressed representation of a pair of indices
 * of an object in the network. The pair is encoded as a
 * 64-bit long unsigned integer. The most significant byte
 * stores the network layer index. The second most significant
 * byte is not used, so it is always zero. And the remaining
 * 48-bits is the index into the network.
 *
 * Note: Since memory layout of integers differ on little-
 * and big-endian machines, we use bit-shifts rather than
 * pointers in accessing individual indices (i.e. it might
 * be a bit slower, but fully portable).
 */
struct  compressed_layer_and_object_indices
{
    using  data_holder_type = natural_64_bit;

    compressed_layer_and_object_indices(layer_index_type const  layer_index, object_index_type const  object_index);

    layer_index_type   layer_index() const;
    object_index_type  object_index() const;

    bool  operator==(compressed_layer_and_object_indices const  other) const
    { return m_compressed_indices == other.m_compressed_indices; }

    bool  operator<(compressed_layer_and_object_indices const  other) const
    { return m_compressed_indices < other.m_compressed_indices; }

    data_holder_type  get_raw_data() const { return m_compressed_indices; }

private:
    data_holder_type  m_compressed_indices;
};


inline bool  operator!=(compressed_layer_and_object_indices const  a, compressed_layer_and_object_indices const  b) noexcept
{ return !(a == b); }


}

namespace std {


template<>
struct hash<netlab::compressed_layer_and_object_indices>
{
	size_t operator()(netlab::compressed_layer_and_object_indices const&  value) const
    {
        return hash<netlab::compressed_layer_and_object_indices::data_holder_type>()(value.get_raw_data());
    }
};


}

#endif
