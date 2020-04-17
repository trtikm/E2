#ifndef AI_SIMULATOR_HPP_INCLUDED
#   define AI_SIMULATOR_HPP_INCLUDED

#   include <ai/agents.hpp>
#   include <ai/devices.hpp>
#   include <ai/sensors.hpp>
#   include <ai/input_devices.hpp>
#   include <ai/scene.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/property_map.hpp>
#   include <ai/sensor_action.hpp>
#   include <memory>
#   include <vector>

namespace ai {


struct simulator
{
    explicit simulator(scene_ptr const  scene_);

    agent_id  insert_agent(
            scene::record_id const&  agent_rid,
            skeletal_motion_templates const  motion_templates,
            AGENT_KIND const  agent_kind,
            from_sensor_record_to_sensor_action_map const&  sensor_actions,
            retina_ptr const  retina_or_null
            );
    void  erase_agent(agent_id const  id);

    device_id  insert_device(
            scene::record_id const&  device_rid,
            skeletal_motion_templates const  motion_templates,  // can be empty, when the device does not use skeletal animations.
            DEVICE_KIND const  device_kind,
            from_sensor_record_to_sensor_action_map const&  sensor_actions
            );
    void  erase_device(device_id const  id);

    sensor_id  insert_sensor(
            scene::record_id const&  sensor_rid,
            SENSOR_KIND const  sensor_kind,
            object_id const&  owner_id_,
            bool const  enabled_,
            property_map const&  cfg_,
            std::vector<scene::node_id> const&  collider_nids_
            );
    void  erase_sensor(sensor_id const  id);
    object_id const&  get_owner_of_sensor(sensor_id const  id_);
    void  set_owner_of_sensor(sensor_id const  id_, object_id const&  owner_id_);
    bool  is_sensor_enabled(sensor_id const  id_);
    void  set_sensor_enabled(sensor_id const  id_, bool const  state_);
    void  set_sensor_enabled(scene::record_id const&  sensor_rid, bool const  state_);

    void  clear();

    scene_ptr  get_scene_ptr() const { return m_scene; }

    scene::record_id const*  get_record_id(object_id const&  oid) const;
    object_id const*  get_object_id(scene::record_id const&  rid) const;

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            input_devices::keyboard_props const&  keyboard,
            input_devices::mouse_props const&  mouse,
            input_devices::window_props const&  window
            );

    void  on_collision_contact(object_id const&  id, scene::collicion_contant_info_ptr const  contact_info, object_id const&  other_id);

    void  on_sensor_event(sensor const&  s, sensor::other_object_info const&  other);

    agents const&  get_agents() const { return m_agents; }
    devices const&  get_devices() const { return m_devices; }
    sensors const&  get_sensors() const { return m_sensors; }

private:

    void  on_insert_object(object_id const&  oid,  scene::record_id const&  rid);
    void  on_erase_object(object_id const&  oid);

    scene_ptr  m_scene;
    agents  m_agents;
    devices  m_devices;
    sensors  m_sensors;
    std::unordered_map<object_id, scene::record_id>  m_from_oid_to_rid;
    std::unordered_map<scene::record_id, object_id>  m_from_rid_to_oid;
};


}

#endif
