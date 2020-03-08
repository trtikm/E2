#ifndef ANGEO_COLLISION_SHAPE_FEATURE_ID_HPP_INCLUDED
#   define ANGEO_COLLISION_SHAPE_FEATURE_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/hash_combine.hpp>
#   include <utility/assumptions.hpp>

namespace angeo {


enum struct  COLLISION_SHAPE_FEATURE_TYPE : natural_8_bit
{
    VERTEX      = 0,
    EDGE        = 1,
    FACE        = 2,
    VOLUME      = 3
};


inline natural_8_bit  as_number(COLLISION_SHAPE_FEATURE_TYPE const  type) noexcept
{
    return (natural_8_bit)type;
}


inline COLLISION_SHAPE_FEATURE_TYPE  as_collision_shape_feature_type(natural_8_bit const  index)
{
    ASSUMPTION(index <= as_number(COLLISION_SHAPE_FEATURE_TYPE::VOLUME));
    return (COLLISION_SHAPE_FEATURE_TYPE) index;
}


struct  collision_shape_feature_id
{
    natural_32_bit   m_feature_type : 2,
                     m_feature_index : 32 - 2;
};


static_assert(sizeof(collision_shape_feature_id) == sizeof(natural_32_bit), "The id must exactly fit to 32 bits.");


inline collision_shape_feature_id  make_collision_shape_feature_id(
        COLLISION_SHAPE_FEATURE_TYPE const  feature_type,
        natural_32_bit  feature_index
        ) noexcept
{
    collision_shape_feature_id  cfid;
    cfid.m_feature_type = as_number(feature_type);
    cfid.m_feature_index = feature_index;
    return cfid;
}


inline COLLISION_SHAPE_FEATURE_TYPE  get_feature_type(collision_shape_feature_id const  cfid) noexcept
{
    return static_cast<COLLISION_SHAPE_FEATURE_TYPE>(cfid.m_feature_type);
}


inline natural_32_bit  get_feature_index(collision_shape_feature_id const  cfid) noexcept
{
    return cfid.m_feature_index;
}


inline natural_32_bit  as_number(collision_shape_feature_id const  cfid) noexcept
{
    return *reinterpret_cast<natural_32_bit const*>(&cfid);
}


inline bool operator==(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) == as_number(right);
}


inline bool operator!=(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) != as_number(right);
}


inline bool operator<(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) < as_number(right);
}


inline bool operator>(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) > as_number(right);
}


inline bool operator<=(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) <= as_number(right);
}


inline bool operator>=(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) >= as_number(right);
}


}

namespace std
{


template<>
struct hash<angeo::collision_shape_feature_id>
{
    size_t operator()(angeo::collision_shape_feature_id const&  cfid) const
    {
        return std::hash<natural_32_bit>()(as_number(cfid));
    }
};


}

#endif
