#ifndef COM_COLLISION_CONTACT_HPP_INCLUDED
#   define COM_COLLISION_CONTACT_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <angeo/tensor_math.hpp>

namespace com {


struct  collision_contact
{
    collision_contact()
        : m_first_collider(invalid_object_guid())
        , m_second_collider(invalid_object_guid())
        , m_contact_point(vector3_zero())
        , m_unit_normal(vector3_unit_z())
        , m_penetration_depth(0.0f)
        , m_scene_index(std::numeric_limits<natural_8_bit>::max())
    {}
    collision_contact(natural_8_bit const  scene_index_, object_guid const  first_collider_, object_guid  const  second_collider_,
                      vector3 const&  contact_point_, vector3 const&  unit_normal_,
                      float_32_bit const penetration_depth_)
        : m_first_collider(first_collider_)
        , m_second_collider(second_collider_)
        , m_contact_point(contact_point_)
        , m_unit_normal(unit_normal_)
        , m_penetration_depth(penetration_depth_)
        , m_scene_index(scene_index_)
    {}

    object_guid  first_collider() const { return m_first_collider; }
    object_guid  second_collider() const { return m_second_collider; }
    vector3 const&  contact_point() const { return m_contact_point; }
    vector3 const&  unit_normal() const { return m_unit_normal; }
    float_32_bit  penetration_depth() const { return m_penetration_depth; }
    natural_8_bit  scene_index() const { return m_scene_index; }

    object_guid  other_collider(object_guid const  collider_guid) const
    {
        return collider_guid == m_first_collider ? m_second_collider :
                (collider_guid == m_second_collider ? m_first_collider : invalid_object_guid());
    }

    vector3  unit_normal(object_guid const  collider_guid) const
    {
        return collider_guid == m_first_collider ? m_unit_normal : -m_unit_normal;
    }

private:

    object_guid  m_first_collider;
    object_guid  m_second_collider;
    vector3  m_contact_point;
    vector3  m_unit_normal;
    float_32_bit  m_penetration_depth;
    natural_8_bit  m_scene_index;
};


}

#endif
