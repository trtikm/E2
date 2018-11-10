#ifndef E2_SCENE_RECORDS_COLLIDER_COLLIDER_HPP_INCLUDED
#   define E2_SCENE_RECORDS_COLLIDER_COLLIDER_HPP_INCLUDED

#   include <angeo/collision_object_id.hpp>
#   include <vector>

namespace scn {


struct  collider final
{
    explicit collider(angeo::collision_object_id const  id, float_32_bit const  density_multiplier = 1.0f)
        : m_ids{id}
        , m_density_multiplier(density_multiplier)
    {}

    explicit collider(std::vector<angeo::collision_object_id> const&  ids, float_32_bit const  density_multiplier = 1.0f)
        : m_ids(ids)
        , m_density_multiplier(density_multiplier)
    {}

    angeo::collision_object_id  id() const { return m_ids.front(); }
    void  set_id(angeo::collision_object_id const  new_id) { m_ids.front() = new_id; }

    std::vector<angeo::collision_object_id> const&  ids() const { return m_ids; }
    std::vector<angeo::collision_object_id>&  ids() { return m_ids; }

    void  set_density_multiplier(float_32_bit const  density_multiplier) { m_density_multiplier = density_multiplier; }
    float_32_bit  get_density_multiplier() const { return m_density_multiplier; }

private:
    std::vector<angeo::collision_object_id>  m_ids;
    float_32_bit  m_density_multiplier;
};


}

#endif
