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
    , m_from_oid_to_rid()
    , m_from_rid_to_oid()
{
    ASSUMPTION(m_scene != nullptr);
}


agent_id  simulator::insert_agent(
        scene::record_id const&  agent_rid,
        skeletal_motion_templates const  motion_templates,
        AGENT_KIND const  agent_kind,
        from_sensor_record_to_sensor_action_map const&  sensor_actions,
        retina_ptr const  retina_or_null
        )
{
    auto const  id = m_agents.insert(agent_rid, motion_templates, agent_kind, sensor_actions, retina_or_null);
    on_insert_object({ OBJECT_KIND::AGENT, id }, agent_rid);
    return id;
}


void  simulator::erase_agent(agent_id const  id)
{
    on_erase_object({ OBJECT_KIND::AGENT, id });
    m_agents.erase(id);
}


device_id  simulator::insert_device(
        scene::record_id const&  device_rid,
        skeletal_motion_templates const  motion_templates,
        DEVICE_KIND const  device_kind,
        from_sensor_record_to_sensor_action_map const&  sensor_actions
        )
{
    auto const  id = m_devices.insert(device_rid, motion_templates, device_kind, sensor_actions);
    on_insert_object({ OBJECT_KIND::DEVICE, id }, device_rid);
    return id;
}


void  simulator::erase_device(device_id const  id)
{
    on_erase_object({ OBJECT_KIND::DEVICE, id });
    m_devices.erase(id);
}


sensor_id  simulator::insert_sensor(
        scene::record_id const&  sensor_rid,
        SENSOR_KIND const  sensor_kind,
        object_id const&  owner_id_,
        bool const  enabled_,
        property_map const&  cfg_,
        std::vector<scene::node_id> const&  collider_nids_
        )
{
    auto const  id = m_sensors.insert(sensor_rid, sensor_kind, owner_id_, enabled_, cfg_, collider_nids_);
    on_insert_object({ OBJECT_KIND::SENSOR, id }, sensor_rid);
    return id;
}


void  simulator::erase_sensor(sensor_id const  id)
{
    on_erase_object({ OBJECT_KIND::SENSOR, id });
    m_sensors.erase(id);
}


void  simulator::on_insert_object(object_id const&  oid,  scene::record_id const&  rid)
{
    m_from_oid_to_rid.insert({ oid, rid });
    m_from_rid_to_oid.insert({ rid, oid });
}


void  simulator::on_erase_object(object_id const&  oid)
{
    auto const  it = m_from_oid_to_rid.find(oid);
    INVARIANT(it != m_from_oid_to_rid.end());
    m_from_rid_to_oid.erase(it->second);
    m_from_oid_to_rid.erase(it);
}


scene::record_id const*  simulator::get_record_id(object_id const&  oid) const
{
    auto const  it = m_from_oid_to_rid.find(oid);
    return it == m_from_oid_to_rid.end() ? nullptr : &it->second;
}


object_id const*  simulator::get_object_id(scene::record_id const&  rid) const
{
    auto const  it = m_from_rid_to_oid.find(rid);
    return it == m_from_rid_to_oid.end() ? nullptr : &it->second;
}


object_id const& simulator::get_owner_of_sensor(sensor_id const  id_)
{
    return m_sensors.get_owner(id_);
}


void  simulator::set_owner_of_sensor(sensor_id const  id_, object_id const&  owner_id_)
{
    m_sensors.set_owner(id_, owner_id_);
}


bool  simulator::is_sensor_enabled(sensor_id const  id_)
{
    return m_sensors.is_enabled(id_);
}


void  simulator::set_sensor_enabled(sensor_id const  id_, bool const  state_)
{
    m_sensors.set_enabled(id_, state_);
}


void  simulator::set_sensor_enabled(scene::record_id const&  sensor_rid, bool const  state_)
{
    auto const  oid_ptr = get_object_id(sensor_rid);
    ASSUMPTION(oid_ptr != nullptr && oid_ptr->kind == OBJECT_KIND::SENSOR);
    set_sensor_enabled(oid_ptr->index, state_);
}


void  simulator::clear()
{
    m_agents.clear();
    m_devices.clear();
    m_sensors.clear();

    m_from_oid_to_rid.clear();
    m_from_rid_to_oid.clear();
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
        scene::collicion_contant_info_ptr const  contact_info,
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


void  simulator::on_sensor_event(sensor const&  s, sensor::other_object_info const&  other)
{
    switch (s.get_owner_id().kind)
    {
    case OBJECT_KIND::AGENT:
        m_agents.on_sensor_event(s.get_owner_id().index, s, other);
        break;
    case OBJECT_KIND::DEVICE:
        m_devices.on_sensor_event(s.get_owner_id().index, s, other);
        break;
    default: UNREACHABLE(); // Includes also OBJECT_KIND::SENSOR, because a sensor cannot send an event to a sensor.
    }
}


}
