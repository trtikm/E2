#ifndef E2_SCENE_RECORDS_RIGID_BODY_RIGID_BODY_HPP_INCLUDED
#   define E2_SCENE_RECORDS_RIGID_BODY_RIGID_BODY_HPP_INCLUDED

#   include <angeo/rigid_body.hpp>
#   include <angeo/tensor_math.hpp>

namespace scn {


struct  rigid_body final
{
    rigid_body(angeo::rigid_body_id const&  id)
        : m_id(id)
    {}

    angeo::rigid_body_id  id() const { return m_id; }
    void  set_id(angeo::rigid_body_id const  new_id) { m_id = new_id; }

private:
    angeo::rigid_body_id  m_id;
};


struct  rigid_body_props
{
    vector3  m_linear_velocity;
    vector3  m_angular_velocity;
    vector3  m_external_linear_acceleration;
    vector3  m_external_angular_acceleration;
};


}

#endif
