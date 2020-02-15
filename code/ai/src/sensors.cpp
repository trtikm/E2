#include <ai/sensors.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {


sensors::sensors(simulator* const  simulator_, scene_ptr const  scene_)
    : m_sensors()
    , m_simulator(simulator_)
    , m_scene(scene_)
{
    ASSUMPTION(m_simulator != nullptr && m_scene != nullptr);
}


sensor_id  sensors::insert(
        scene::record_id const&  sensor_rid,
        SENSOR_KIND const  sensor_kind,
        object_id const&  owner_id_,
        bool const  enabled_,
        property_map const&  cfg_,
        std::vector<scene::node_id> const&  collider_nids_
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(
        sensor_rid.valid() &&
        !sensor_rid.is_node_reference() &&
        !sensor_rid.is_folder_reference()
        );

    sensor_id  id = 0U;
    for (; id != m_sensors.size(); ++id)
        if (m_sensors.at(id) == nullptr)
            break;

    auto const  props = std::make_shared<sensor_props>();
    props->sensor_ptr = nullptr;
    props->sensor_rid = sensor_rid;
    props->sensor_kind = sensor_kind;
    props->owner_id = owner_id_;
    props->enabled = enabled_;
    props->cfg = std::make_shared<property_map>(cfg_);
    props->collider_nids = std::make_shared<std::vector<scene::node_id> >(collider_nids_);

    if (id == m_sensors.size())
        m_sensors.resize(m_sensors.size() + 1U, nullptr);
    m_sensors.at(id) = props;

    m_from_rid_to_id[sensor_rid] = id;

    return id;
}


void  sensors::erase(sensor_id const  id)
{
    m_from_rid_to_id.erase(m_sensors.at(id)->sensor_rid);
    m_sensors.at(id) = nullptr;
}


void  sensors::construct_sensor(sensor_id const  id, sensor_props&  props)
{
    TMPROF_BLOCK();

    props.sensor_ptr = std::make_unique<sensor>(
                            m_simulator,
                            props.sensor_kind,
                            object_id{ OBJECT_KIND::SENSOR, id },
                            props.sensor_rid,
                            props.owner_id,
                            props.enabled,
                            props.cfg,
                            props.collider_nids
                            );
}


object_id const& sensors::get_owner(sensor_id const  id_)
{
    ASSUMPTION(id_ < m_sensors.size());
    if (m_sensors.at(id_)->sensor_ptr == nullptr)
        return m_sensors.at(id_)->owner_id;
    else
        return m_sensors.at(id_)->sensor_ptr->get_owner_id();
}


void  sensors::set_owner(sensor_id const  id_, object_id const&  owner_id_)
{
    ASSUMPTION(id_ < m_sensors.size());
    if (m_sensors.at(id_)->sensor_ptr == nullptr)
        m_sensors.at(id_)->owner_id = owner_id_;
    else
        m_sensors.at(id_)->sensor_ptr->set_owner_id(owner_id_);
}


bool  sensors::is_enabled(sensor_id const  id_)
{
    ASSUMPTION(id_ < m_sensors.size());
    if (m_sensors.at(id_)->sensor_ptr == nullptr)
        return m_sensors.at(id_)->enabled;
    else
        return m_sensors.at(id_)->sensor_ptr->is_enabled();
}


void  sensors::set_enabled(sensor_id const  id_, bool const  state_)
{
    ASSUMPTION(id_ < m_sensors.size());
    if (m_sensors.at(id_)->sensor_ptr == nullptr)
        m_sensors.at(id_)->enabled = state_;
    else
        m_sensors.at(id_)->sensor_ptr->set_enabled(state_);
}


void  sensors::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    for (natural_32_bit  id = 0U; id != m_sensors.size(); ++id)
    {
        auto const  props = m_sensors.at(id);
        if (props != nullptr)
            if (props->sensor_ptr != nullptr)
                props->sensor_ptr->next_round(time_step_in_seconds);
            else
                construct_sensor(id, *props);
    }
}


void  sensors::on_collision_contact(
        sensor_id const  id,
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info const&  contact_info,
        object_id const&  other_id,
        scene::node_id const&  other_collider_nid
        )
{
    ASSUMPTION(id < m_sensors.size() && m_sensors.at(id)->sensor_ptr != nullptr);
    m_sensors.at(id)->sensor_ptr->on_collision_contact(collider_nid, contact_info, other_id, other_collider_nid);
}


}
