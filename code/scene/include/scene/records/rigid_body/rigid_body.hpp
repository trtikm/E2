#ifndef E2_SCENE_RECORDS_RIGID_BODY_RIGID_BODY_HPP_INCLUDED
#   define E2_SCENE_RECORDS_RIGID_BODY_RIGID_BODY_HPP_INCLUDED

#   include <angeo/rigid_body.hpp>
#   include <angeo/tensor_math.hpp>

namespace scn {


struct  rigid_body final
{
    rigid_body(angeo::rigid_body_id const&  id, bool const  auto_compute_mass_and_inertia_tensor_)
        : m_id(id)
        , m_auto_compute_mass_and_inertia_tensor(auto_compute_mass_and_inertia_tensor_)
    {}

    angeo::rigid_body_id  id() const { return m_id; }
    void  set_id(angeo::rigid_body_id const  new_id) { m_id = new_id; }

    bool auto_compute_mass_and_inertia_tensor() const { return m_auto_compute_mass_and_inertia_tensor; }

private:
    angeo::rigid_body_id  m_id;
    bool  m_auto_compute_mass_and_inertia_tensor;
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
