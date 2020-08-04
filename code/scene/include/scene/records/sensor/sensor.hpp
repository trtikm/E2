#ifndef E2_SCENE_RECORDS_SENSOR_SENSOR_HPP_INCLUDED
#   define E2_SCENE_RECORDS_SENSOR_SENSOR_HPP_INCLUDED

#   include <scene/scene_node_id.hpp>
#   include <aiold/sensor_id.hpp>
#   include <aiold/sensor_kind.hpp>
#   include <aiold/property_map.hpp>
#   include <memory>

namespace scn {


struct  sensor_props
{
    aiold::SENSOR_KIND  m_sensor_kind;
    bool  m_enabled;
    aiold::property_map  m_sensor_props;
};


struct  sensor  final
{
    sensor(aiold::sensor_id const  id, sensor_props const&  props)
        : m_id(id)
        , m_props(props)
    {}

    aiold::sensor_id  id() const { return m_id; }
    sensor_props const&  get_props() const { return m_props; }

private:
    aiold::sensor_id  m_id;
    sensor_props  m_props;
};


}

#endif
