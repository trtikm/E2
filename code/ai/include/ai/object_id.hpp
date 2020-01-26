#ifndef AI_OBJECT_ID_HPP_INCLUDED
#   define AI_OBJECT_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
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

    static inline object_id  make_invalid(OBJECT_KIND const  kind_ = OBJECT_KIND::AGENT) { return object_id(kind_, std::numeric_limits<index_type>::max()); }

    bool  valid() const { return index != std::numeric_limits<index_type>::max(); }

    OBJECT_KIND  kind;
    index_type  index;
};


}

#endif
