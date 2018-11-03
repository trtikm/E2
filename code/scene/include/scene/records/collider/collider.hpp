#ifndef E2_SCENE_RECORDS_COLLIDER_COLLIDER_HPP_INCLUDED
#   define E2_SCENE_RECORDS_COLLIDER_COLLIDER_HPP_INCLUDED

#   include <angeo/collision_object_id.hpp>

namespace scn {


struct  collider final
{
    explicit collider(angeo::collision_object_id const  id, float_32_bit const  density_multiplier = 1.0f)
        : m_id(id)
        , m_density_multiplier(density_multiplier)
    {}

    angeo::collision_object_id  id() const { return m_id; }
    void  set_id(angeo::collision_object_id const  new_id) { m_id = new_id; }

    void  set_density_multiplier(float_32_bit const  density_multiplier) { m_density_multiplier = density_multiplier; }
    float_32_bit  get_density_multiplier() const { return m_density_multiplier; }

private:
    angeo::collision_object_id  m_id;
    float_32_bit  m_density_multiplier;
};


}

#endif
