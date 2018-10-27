#ifndef E2_SCENE_RECORDS_COLLIDER_COLLIDER_HPP_INCLUDED
#   define E2_SCENE_RECORDS_COLLIDER_COLLIDER_HPP_INCLUDED

#   include <angeo/collision_object_id.hpp>

namespace scn {


struct  collider final
{
    explicit collider(angeo::collision_object_id const  id)
        : m_id(id)
    {}

    angeo::collision_object_id  id() const { return m_id; }
    void  set_id(angeo::collision_object_id const  new_id) { m_id = new_id; }

private:
    angeo::collision_object_id  m_id;
};


}

#endif
