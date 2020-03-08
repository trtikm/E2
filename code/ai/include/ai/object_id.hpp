#ifndef AI_OBJECT_ID_HPP_INCLUDED
#   define AI_OBJECT_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/hash_combine.hpp>
#   include <limits>

namespace ai {


enum struct OBJECT_KIND : natural_8_bit
{
    AGENT       = 0,
    DEVICE      = 1,
    SENSOR      = 2,
};


struct  object_id
{
    using  index_type = natural_32_bit;

    object_id(OBJECT_KIND const  kind_, index_type const  index_)
        : kind(kind_)
        , index(index_)
    {}

    // Makes an invalid id of the passed kind
    object_id(OBJECT_KIND const  kind_)
        : object_id(kind_, std::numeric_limits<index_type>::max())
    {}

    // Makes an invalid id of the agent kind
    object_id()
        : object_id(OBJECT_KIND::AGENT)
    {}

    static inline object_id  make_invalid() { return {}; }

    bool  valid() const { return index != std::numeric_limits<index_type>::max(); }

    OBJECT_KIND  kind;
    index_type  index;
};


inline natural_8_bit  as_number(OBJECT_KIND const  kind) noexcept
{
    return (natural_8_bit)kind;
}


inline bool operator==(object_id const&  left, object_id const&  right) noexcept
{
    return left.kind == right.kind && left.index == right.index;
}


inline bool operator!=(object_id const&  left, object_id const&  right) noexcept
{
    return !(left == right);
}


inline bool operator<(object_id const&  left, object_id const&  right) noexcept
{
    return as_number(left.kind) < as_number(right.kind) ||
            (as_number(left.kind) == as_number(right.kind) && left.index < right.index);
}


inline bool operator>(object_id const&  left, object_id const&  right) noexcept
{
    return right < left;
}


inline bool operator<=(object_id const&  left, object_id const&  right) noexcept
{
    return left == right || left < right;
}


inline bool operator>=(object_id const&  left, object_id const&  right) noexcept
{
    return left == right || left > right;
}


}

namespace std {


template<>
struct hash<ai::object_id>
{
    size_t operator()(ai::object_id const&  id) const
    {
        std::size_t seed = 0;
        ::hash_combine(seed, ai::as_number(id.kind));
        ::hash_combine(seed, id.index);
        return seed;
    }
};


}

#endif
