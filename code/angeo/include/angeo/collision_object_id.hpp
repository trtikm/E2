#ifndef ANGEO_COLLISION_OBJECT_ID_HPP_INCLUDED
#   define ANGEO_COLLISION_OBJECT_ID_HPP_INCLUDED

#   include <angeo/collision_shape_id.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/hash_combine.hpp>

namespace angeo {


struct  collision_object_id
{
    natural_32_bit   m_shape_type : 4,
                     m_instance_index : 32 - 4;
};


static_assert(sizeof(collision_object_id) == sizeof(natural_32_bit), "The id must exactly fit to 32 bits.");


inline collision_object_id  make_collision_object_id(
        COLLISION_SHAPE_TYPE const  shape_type,
        natural_32_bit  instance_index
        ) noexcept
{
    collision_object_id  coid;
    coid.m_shape_type = static_cast<natural_8_bit>(shape_type);
    coid.m_instance_index = instance_index;
    return coid;
}


inline COLLISION_SHAPE_TYPE  get_shape_type(collision_object_id const  coid) noexcept
{
    return static_cast<COLLISION_SHAPE_TYPE>(coid.m_shape_type);
}


inline natural_32_bit  get_instance_index(collision_object_id const  coid) noexcept
{
    return coid.m_instance_index;
}


inline natural_32_bit  as_number(collision_object_id const  coid) noexcept
{
    return *reinterpret_cast<natural_32_bit const*>(&coid);
}


inline bool operator==(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) == as_number(right);
}


inline bool operator!=(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) != as_number(right);
}


inline bool operator<(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) < as_number(right);
}


inline bool operator>(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) > as_number(right);
}


inline bool operator<=(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) <= as_number(right);
}


inline bool operator>=(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) >= as_number(right);
}


}

namespace std {


template<>
struct hash<angeo::collision_object_id>
{
    size_t operator()(angeo::collision_object_id const&  coid) const
    {
        return std::hash<natural_32_bit>()(as_number(coid));
    }
};


}

#endif
