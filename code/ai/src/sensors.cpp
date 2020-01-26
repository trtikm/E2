#include <ai/sensors.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {


sensors::sensors(scene_ptr const  scene_)
    : m_sensors()
    , m_scene(scene_)
{
    ASSUMPTION(m_scene != nullptr);
}


sensor_id  sensors::insert(scene::node_id const&  sensor_nid, SENSOR_KIND const  sensor_kind)
{
    TMPROF_BLOCK();

    ASSUMPTION(sensor_nid.valid());

    sensor_id  id = 0U;
    for (; id != m_sensors.size(); ++id)
        if (m_sensors.at(id) == nullptr)
            break;

    auto const  props = std::make_shared<sensor_props>();
    props->sensor_ptr = nullptr;
    props->sensor_nid = sensor_nid;
    props->sensor_kind = sensor_kind;

    if (id == m_sensors.size())
        m_sensors.resize(m_sensors.size() + 1U, nullptr);
    m_sensors.at(id) = props;

    return id;
}


void  sensors::construct_sensor(sensor_id const  id, sensor_props&  props)
{
    TMPROF_BLOCK();

    props.sensor_ptr = std::make_unique<sensor>(props.sensor_kind);
}


void  sensors::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    for (natural_32_bit  id = 0U; id != m_sensors.size(); ++id)
    {
        auto const  props = m_sensors.at(id);
        if (props != nullptr)
            if (props->sensor_ptr == nullptr)
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
}


}
