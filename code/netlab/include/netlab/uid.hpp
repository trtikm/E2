#ifndef NETLAB_UID_HPP_INCLUDED
#   define NETLAB_UID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <functional>

namespace netlab {


struct uid
{
    static constexpr natural_8_bit  NUM_LAYER_BITS  = 6;
    static constexpr natural_8_bit  NUM_UNIT_BITS   = 14;
    static constexpr natural_8_bit  NUM_SOCKET_BITS = 12;

    static constexpr natural_8_bit   MAX_NUM_LAYERS             = 1 << NUM_LAYER_BITS;
    static constexpr natural_16_bit  MAX_NUM_UNITS_PER_LAYER    = 1 << NUM_UNIT_BITS;
    static constexpr natural_16_bit  MAX_NUM_SOCKETS_PER_UNIT   = 1 << NUM_SOCKET_BITS;

    static_assert(uid::MAX_NUM_LAYERS           == 64, "");
    static_assert(uid::MAX_NUM_UNITS_PER_LAYER  == 16384, "");
    static_assert(uid::MAX_NUM_SOCKETS_PER_UNIT == 4096, "");

    static inline uid  as_layer(uid const  id) { return { id.layer, 0U, 0U }; }
    static inline uid  as_unit(uid const  id) { return { id.layer, id.unit, 0U }; }
    static inline uid  as_socket(uid  unit_id, natural_16_bit const  socket_idx) { unit_id.socket = socket_idx; return unit_id; }

    static inline natural_32_bit  as_number(uid const  id) noexcept { return *reinterpret_cast<natural_32_bit const*>(&id); }

    natural_32_bit  layer : NUM_LAYER_BITS,
                    unit  : NUM_UNIT_BITS,
                    socket: NUM_SOCKET_BITS;
};
static_assert(sizeof(uid) == sizeof(natural_32_bit), "");


inline bool operator==(uid const  left, uid const  right) noexcept
{
    return uid::as_number(left) == uid::as_number(right);
}


inline bool operator<(uid const  left, uid const  right) noexcept
{
    return uid::as_number(left) < uid::as_number(right);
}


}

namespace std {


template<>
struct hash<netlab::uid>
{
    size_t  operator()(netlab::uid const  id) const
    {
        return std::hash<natural_32_bit>()(netlab::uid::as_number(id));
    }
};


}

#endif
