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
            scene::node_id const&  agent_nid,
            skeletal_motion_templates const  motion_templates,
            AGENT_KIND const  agent_kind,
            from_sensor_event_to_sensor_action_map const&  sensor_actions,
            retina_ptr const  retina_or_null
            );
    void  erase_agent(agent_id const  id);

    device_id  insert_device(
            scene::node_id const&  device_nid,
            skeletal_motion_templates const  motion_templates,  // can be empty, when the device does not use skeletal animations.
            DEVICE_KIND const  device_kind,
            from_sensor_event_to_sensor_action_map const&  sensor_actions
            );
    void  erase_device(device_id const  id);

    sensor_id  insert_sensor(
            scene::node_id const&  sensor_nid,
            SENSOR_KIND const  sensor_kind,
            object_id const& owner_id_,
            property_map const&  cfg_
            );
    void  erase_sensor(sensor_id const  id);

    void  clear();

    scene_ptr  get_scene_ptr() const { return m_scene; }

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            input_devices::keyboard_props const&  keyboard,
            input_devices::mouse_props const&  mouse,
            input_devices::window_props const&  window
            );

    void  on_collision_contact(
            object_id const&  id,
            scene::node_id const&  collider_nid,
            scene::collicion_contant_info const&  contact_info,
            object_id const&  other_id,
            scene::node_id const&  other_collider_nid
            );

    void  on_sensor_event(sensor const&  s);

    void  on_simulator_event_insert(scene::node_id const&  where_nid, scene::node_id const&  what_nid);
    void  on_simulator_event_erase(object_id const&  oid);

    agents const&  get_agents() const { return m_agents; }
    devices const&  get_devices() const { return m_devices; }
    sensors const&  get_sensors() const { return m_sensors; }

private:
    scene_ptr  m_scene;
    agents  m_agents;
    devices  m_devices;
    sensors  m_sensors;
};


}

#endif
