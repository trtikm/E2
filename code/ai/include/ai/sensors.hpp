#ifndef AI_SENSORS_HPP_INCLUDED
#   define AI_SENSORS_HPP_INCLUDED

#   include <ai/sensor.hpp>
#   include <ai/sensor_id.hpp>
#   include <ai/sensor_kind.hpp>
#   include <ai/object_id.hpp>
#   include <ai/scene.hpp>
#   include <memory>
#   include <vector>

namespace ai {


struct  simulator;


struct  sensors
{
    explicit sensors(simulator* const  simulator_, scene_ptr const  scene_);

    sensor_id  insert(
            scene::record_id const&  sensor_rid,
            SENSOR_KIND const  sensor_kind,
            object_id const& owner_id_,
            property_map const&  cfg_,
            std::vector<scene::node_id> const&  collider_nids_
            );
    void  erase(sensor_id const  id) { m_sensors.at(id) = nullptr; }

    void  clear() { m_sensors.clear(); }

    bool  ready(sensor_id const  id) { return m_sensors.at(id)->sensor_ptr.operator bool(); }
    sensor&  at(sensor_id const  id) { return *m_sensors.at(id)->sensor_ptr; }
    sensor const&  at(sensor_id const  id) const { return *m_sensors.at(id)->sensor_ptr; }

    scene_ptr  get_scene_ptr() const { return m_scene; }

    object_id const&  get_owner(sensor_id const  id_);
    void  set_owner(sensor_id const  id_, object_id const&  owner_id_);

    void  next_round(float_32_bit const  time_step_in_seconds);

    void  on_collision_contact(
            sensor_id const  id,
            scene::node_id const&  collider_nid,
            scene::collicion_contant_info const&  contact_info,
            object_id const&  other_id,
            scene::node_id const&  other_collider_nid
            );

private:

    struct  sensor_props
    {
        std::unique_ptr<sensor>  sensor_ptr;
        scene::record_id  sensor_rid;
        SENSOR_KIND  sensor_kind;
        object_id  owner_id;
        std::shared_ptr<property_map>  cfg;
        std::shared_ptr<std::vector<scene::node_id> >  collider_nids;
    };

    void  construct_sensor(sensor_id const  id, sensor_props&  props);

    std::vector<std::shared_ptr<sensor_props> >  m_sensors; // Should be 'std::vector<std::unique_ptr<sensor_props> >', but does not compile :-(
    simulator*  m_simulator;
    scene_ptr  m_scene;
};


}

#endif
