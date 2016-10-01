#ifndef NETLAB_NETWORK_OBJECT_ID_HPP_INCLUDED
#   define NETLAB_NETWORK_OBJECT_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace netlab {


/**
 * Each object in the network can be uniquely identified by:
 *      - kind of the object (dock, ship, or spiker);
 *      - index of a network layer in which the object resides;
 *      - index into that layer where the object is located;
 * Such triple also uniquely identifies an access path to
 * to object in the network. This data structure defines
 * the triple in efficient compressed form.
 *
 * The triple is encoded in a single 64-bit long unsigned
 * integer. The most significant byte is the object kind.
 * The second most significant is the index of network.
 * And remaining 48-bits is the index into the network.
 *
 * Note that mamory layout of the data differs on little-
 * and big-endian machines. Therefore, we use bit-shifts
 * rather than pointers in accessing individual components
 * in the implementation.
 */
struct  network_object_id
{
    network_object_id(natural_8_bit const  object_kind,
                      natural_8_bit const  index_of_network_layer,
                      natural_64_bit const  index_in_network_layer);

    natural_8_bit   object_kind() const noexcept;
    natural_8_bit   index_of_network_layer() const noexcept;
    natural_64_bit  index_into_network_layer() const noexcept;

    bool  operator==(network_object_id const  other) const noexcept { return m_compressed_data == other.m_compressed_data; }

private:
    natural_64_bit  m_compressed_data;
};

inline constexpr natural_8_bit  object_kind_dock() noexcept { return 1U; }
inline constexpr natural_8_bit  object_kind_ship() noexcept { return 2U; }
inline constexpr natural_8_bit  object_kind_spiker() noexcept { return 3U; }

inline bool  operator!=(network_object_id const  a, network_object_id const  b) { return !(a == b); }


}

#endif
