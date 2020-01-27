#ifndef E2_SCENE_RECORDS_SENSOR_UTILS_HPP_INCLUDED
#   define E2_SCENE_RECORDS_SENSOR_UTILS_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_utils.hpp>
#   include <scene/records/sensor/sensor.hpp>

namespace scn {


inline scene_node::folder_name  get_sensors_folder_name() { return "sensors"; }

inline scene_record_id  make_sensor_record_id(scene_node_id const&  id, scene_node::record_name const&  sensor_name)
{
    return { id, get_sensors_folder_name(), sensor_name };
}

inline scene_node_record_id  make_sensor_node_record_id(scene_node::record_name const&  sensor_name)
{
    return { get_sensors_folder_name(), sensor_name };
}

inline scene_node::folder_content::records_map const&  get_sensor_holders(scene_node const&  node)
{
    return get_folder_records_map(node, get_sensors_folder_name());
}

inline sensor const*  as_sensor(scene_node::record_holder const&  holder)
{
    return &record_cast<sensor>(holder);
}

inline sensor const*  get_sensor(scene_node const&  node, scene_node::record_name const&  sensor_name)
{
    return &get_record<sensor>(node, make_sensor_node_record_id(sensor_name));
}

inline sensor*  get_sensor(scene_node&  node, scene_node::record_name const&  sensor_name)
{
    return const_cast<sensor*>(&get_record<sensor>(node, make_sensor_node_record_id(sensor_name)));
}

inline bool  has_sensor(scene_node const&  n, scene_node::record_name const&  sensor_name)
{
    return has_record(n, make_sensor_node_record_id(sensor_name));
}

inline sensor const* get_sensor(scene const&  s, scene_record_id const&  id)
{
    ASSUMPTION(id.get_folder_name() == get_sensors_folder_name());
    return &get_record<sensor>(s, id);
}

inline sensor*  get_sensor(scene&  s, scene_record_id const&  id)
{
    ASSUMPTION(id.get_folder_name() == get_sensors_folder_name());
    return const_cast<sensor*>(&get_record<sensor>(s, id));
}

inline bool  has_sensor(scene const&  s, scene_record_id const&  id)
{
    ASSUMPTION(id.get_folder_name() == get_sensors_folder_name());
    return has_record(s, id);
}

inline void  insert_sensor(scene_node&  n, scene_node::record_name const&  sensor_name, ai::sensor_id const  sensor_id, sensor_props const&  props)
{
    insert_record<sensor>(n, make_sensor_node_record_id(sensor_name), sensor(sensor_id, props));
}

inline void  insert_sensor(scene&  s, scene_record_id const&  id, ai::sensor_id const  sensor_id, sensor_props const&  props)
{
    insert_record<sensor>(s, id, sensor(sensor_id, props));
}

inline void  erase_sensor(scene_node&  n, scene_node::record_name const&  sensor_name)
{
    erase_record(n, make_sensor_node_record_id(sensor_name));
}

inline void  erase_sensor(scene&  s, scene_record_id const&  id)
{
    ASSUMPTION(id.get_folder_name() == get_sensors_folder_name());
    erase_record(s, id);
}


}

#endif
