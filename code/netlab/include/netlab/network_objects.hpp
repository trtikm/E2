#ifndef NETLAB_NETWORK_OBJECTS_HPP_INCLUDED
#   define NETLAB_NETWORK_OBJECTS_HPP_INCLUDED

#   include <utility/tensor_math.hpp>

namespace netlab {


/**
 *
 *
 *
 */
struct  spiker
{
private:
//    std::vector<network_data_id>  m_dock_ids;
//    std::vector<network_data_id>  m_ship_ids;
//    vector3  m_center_of_movement_area_for_ships;
    natural_64_bit  m_last_update_id;
};


/**
 *
 *
 *
 */
struct  dock
{
private:
//    network_data_id  m_spiker_id;
};


/**
 *
 *
 *
 */
struct  ship
{
    vector3 const&  position() const noexcept { return m_position; }
    void  set_position(vector3 const&  pos) { m_position = pos; }

    vector3 const&  velocity() const noexcept { return m_velocity; }
    void  set_velocity(vector3 const&  v) { m_velocity = v; }

private:
//    network_data_id  m_spiker_id;
    vector3  m_position;
    vector3  m_velocity;
};


}

#endif
