#ifndef E2_SCENE_RECORDS_SENSOR_SENSOR_HPP_INCLUDED
#   define E2_SCENE_RECORDS_SENSOR_SENSOR_HPP_INCLUDED

#   include <scene/scene_node_id.hpp>
#   include <ai/sensor_id.hpp>
#   include <ai/sensor_kind.hpp>
#   include <ai/property_map.hpp>
#   include <memory>

namespace scn {


struct  sensor_props
{
    ai::SENSOR_KIND  m_sensor_kind;
    bool  m_enabled;
    ai::property_map  m_sensor_props;
};


struct  sensor  final
{
    sensor(ai::sensor_id const  id, sensor_props const&  props)
        : m_id(id)
        , m_props(props)
    {}

    ai::sensor_id  id() const { return m_id; }
    sensor_props const&  get_props() const { return m_props; }

private:
    ai::sensor_id  m_id;
    sensor_props  m_props;
};


}

#endif
