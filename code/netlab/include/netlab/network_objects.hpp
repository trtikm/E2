#ifndef NETLAB_NETWORK_OBJECTS_HPP_INCLUDED
#   define NETLAB_NETWORK_OBJECTS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace netlab {


/**
 *
 *
 *
 */
struct  spiker
{
    spiker();
    virtual ~spiker() {}

    natural_64_bit  last_update_id() const noexcept { return m_last_update_id; }
    void  set_last_update_id(natural_64_bit const  value) { m_last_update_id = value; }

private:
    natural_64_bit  m_last_update_id;
};


/**
 *
 *
 *
 */
struct  dock
{
    virtual ~dock() {}
};


/**
 *
 *
 *
 */
struct  ship
{
    ship();
    virtual ~ship() {}

    vector3 const&  position() const noexcept { return m_position; }
    void  set_position(vector3 const&  pos) { m_position = pos; }
    vector3&  get_position_nonconst_reference() noexcept { return m_position; }

    vector3 const&  velocity() const noexcept { return m_velocity; }
    void  set_velocity(vector3 const&  v) { m_velocity = v; }
    vector3&  get_velocity_nonconst_reference() noexcept { return m_velocity; }

private:
    vector3  m_position;
    vector3  m_velocity;
};


}

#endif
