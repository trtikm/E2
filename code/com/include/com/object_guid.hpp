#ifndef COM_OBJECT_GUID_HPP_INCLUDED
#   define COM_OBJECT_GUID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/hash_combine.hpp>
#   include <string>
#   include <limits>

namespace com {


enum struct  OBJECT_KIND : natural_8_bit
{
    FOLDER          = 0U,
    FRAME           = 1U,
    BATCH           = 2U,
    COLLIDER        = 3U,
    RIGID_BODY      = 4U,
    TIMER           = 5U,
    SENSOR          = 6U,
    AGENT           = 7U,
    NONE            = 8U,
};


constexpr natural_8_bit  get_num_object_kinds()
{
    return (natural_8_bit)OBJECT_KIND::NONE + (natural_8_bit)1;
}

inline natural_8_bit  as_number(OBJECT_KIND const  okind) { return (natural_8_bit)okind; }
OBJECT_KIND  as_object_kind(natural_8_bit  const  okind);
std::string  to_string(OBJECT_KIND const  okind);
OBJECT_KIND  read_object_kind_from_string(std::string const&  name);


struct  object_guid
{
    using  index_type = natural_16_bit;

    object_guid() : kind(OBJECT_KIND::NONE), index(std::numeric_limits<object_guid::index_type>::max()) {}
    object_guid(OBJECT_KIND const kind_, index_type const  index_) : kind(kind_), index(index_) {}

    OBJECT_KIND  kind;
    index_type  index;
};


inline object_guid  invalid_object_guid() { return {}; }


inline bool operator==(object_guid const  left, object_guid const  right) noexcept
{
    return left.kind == right.kind && left.index == right.index;
}


inline bool operator!=(object_guid const  left, object_guid const  right) noexcept
{
    return !(left == right);
}


inline bool operator<(object_guid const  left, object_guid const  right) noexcept
{
    return as_number(left.kind) < as_number(right.kind) ||
            (as_number(left.kind) == as_number(right.kind) && left.index < right.index);
}


inline bool operator>(object_guid const  left, object_guid const  right) noexcept
{
    return right < left;
}


inline bool operator<=(object_guid const  left, object_guid const  right) noexcept
{
    return left == right || left < right;
}


inline bool operator>=(object_guid const  left, object_guid const  right) noexcept
{
    return left == right || left > right;
}


}

namespace std {


template<>
struct hash<com::object_guid>
{
    size_t operator()(com::object_guid const&  id) const
    {
        std::size_t seed = 0;
        ::hash_combine(seed, com::as_number(id.kind));
        ::hash_combine(seed, id.index);
        return seed;
    }
};


}

#endif
