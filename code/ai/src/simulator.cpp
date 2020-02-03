#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {


simulator::simulator(scene_ptr const  scene_)
    : m_scene(scene_)
    , m_agents(this, scene_)
    , m_devices(this, scene_)
    , m_sensors(this, scene_)
{
    ASSUMPTION(m_scene != nullptr);
}


agent_id  simulator::insert_agent(
        scene::node_id const&  agent_nid,
        skeletal_motion_templates const  motion_templates,
        AGENT_KIND const  agent_kind,
        from_sensor_record_to_sensor_action_map const&  sensor_actions,
        retina_ptr const  retina_or_null
        )
{
    return m_agents.insert(agent_nid, motion_templates, agent_kind, sensor_actions, retina_or_null);
}


void  simulator::erase_agent(agent_id const  id)
{
    m_agents.erase(id);
}


device_id  simulator::insert_device(
        scene::node_id const&  device_nid,
        skeletal_motion_templates const  motion_templates,
        DEVICE_KIND const  device_kind,
        from_sensor_record_to_sensor_action_map const&  sensor_actions
        )
{
    return m_devices.insert(device_nid, motion_templates, device_kind, sensor_actions);
}


void  simulator::erase_device(device_id const  id)
{
    m_devices.erase(id);
}


sensor_id  simulator::insert_sensor(
        scene::record_id const&  sensor_rid,
        SENSOR_KIND const  sensor_kind,
        object_id const& owner_id_,
        property_map const&  cfg_
        )
{
    return m_sensors.insert(sensor_rid, sensor_kind, owner_id_, cfg_);
}


void  simulator::erase_sensor(sensor_id const  id)
{
    m_sensors.erase(id);
}


void  simulator::clear()
{
    m_agents.clear();
    m_devices.clear();
    m_sensors.clear();
}


void  simulator::next_round(
        float_32_bit const  time_step_in_seconds,
        input_devices::keyboard_props const&  keyboard,
        input_devices::mouse_props const&  mouse,
        input_devices::window_props const&  window
        )
{
    TMPROF_BLOCK();

    m_sensors.next_round(time_step_in_seconds);
    m_devices.next_round(time_step_in_seconds);
    m_agents.next_round(time_step_in_seconds, keyboard, mouse, window);
}


void  simulator::on_collision_contact(
        object_id const&  id,
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info const&  contact_info,
        object_id const&  other_id,
        scene::node_id const&  other_collider_nid
        )
{
    switch (id.kind)
    {
    case OBJECT_KIND::AGENT:
        m_agents.on_collision_contact(id.index, collider_nid, contact_info, other_id, other_collider_nid);
        break;
    case OBJECT_KIND::DEVICE:
        m_devices.on_collision_contact(id.index, collider_nid, contact_info, other_id, other_collider_nid);
        break;
    case OBJECT_KIND::SENSOR:
        m_sensors.on_collision_contact(id.index, collider_nid, contact_info, other_id, other_collider_nid);
        break;
    default: UNREACHABLE();
    }
}


void  simulator::on_sensor_event(sensor const&  s)
{
    switch (s.get_owner_id().kind)
    {
    case OBJECT_KIND::AGENT:
        m_agents.on_sensor_event(s.get_owner_id().index, s);
        break;
    case OBJECT_KIND::DEVICE:
        m_devices.on_sensor_event(s.get_owner_id().index, s);
        break;
    default: UNREACHABLE(); // Includes also OBJECT_KIND::SENSOR, because a sensor cannot send an event to a sensor.
    }
}


}
