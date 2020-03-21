#include <ai/devices.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {


devices::devices(simulator* const  simulator_, scene_ptr const  scene_)
    : m_devices()
    , m_simulator(simulator_)
    , m_scene(scene_)
{
    ASSUMPTION(m_simulator != nullptr && m_scene != nullptr);
}


device_id  devices::insert(
        scene::record_id const&  device_rid,
        skeletal_motion_templates const  motion_templates,
        DEVICE_KIND const  device_kind,
        from_sensor_record_to_sensor_action_map const&  sensor_actions
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(device_rid.valid());

    device_id  id = 0U;
    for (; id != m_devices.size(); ++id)
        if (m_devices.at(id) == nullptr)
            break;

    auto const  props = std::make_shared<device_props>();
    props->device_ptr = nullptr;
    props->device_rid = device_rid;
    props->motion_templates = motion_templates;
    props->m_sensor_actions = std::make_shared<from_sensor_record_to_sensor_action_map>(sensor_actions);
    props->device_kind = device_kind;

    if (id == m_devices.size())
        m_devices.resize(m_devices.size() + 1U, nullptr);
    m_devices.at(id) = props;

    return id;
}


void  devices::construct_device(device_id const  id, device_props&  props)
{
    TMPROF_BLOCK();

    blackboard_device_ptr const  bb = device::create_blackboard(props.device_kind);
    {
        // General blackboard setup
        bb->m_self_id = device_to_object_id(id);
        bb->m_motion_templates = props.motion_templates;
        bb->m_scene = m_scene;
        bb->m_self_rid = props.device_rid;
        bb->initialise_bone_nids();
        bb->m_state = 0U;
        bb->m_sensor_actions = props.m_sensor_actions;
        bb->m_simulator_ptr = m_simulator;

        // Device's blackboard setup
        bb->m_device_kind = props.device_kind;
        device::create_modules(bb);
    }
    props.device_ptr = std::make_unique<device>(bb);
}


void  devices::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    for (natural_32_bit  id = 0U; id != m_devices.size(); ++id)
    {
        auto const  props = m_devices.at(id);
        if (props != nullptr)
            if (props->device_ptr != nullptr)
                props->device_ptr->next_round(time_step_in_seconds);
            else if (props->motion_templates.empty() || props->motion_templates.loaded_successfully())
                construct_device(id, *props);
    }
}


void  devices::on_collision_contact(
        device_id const  id,
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info_ptr const  contact_info,
        object_id const&  other_id,
        scene::node_id const&  other_collider_nid
        )
{
    ASSUMPTION(id < m_devices.size() && m_devices.at(id) != nullptr);
    if (m_devices.at(id)->device_ptr != nullptr)
    {
        // Process the contact here. However, so far nothing to do though...
    }
}


void  devices::on_sensor_event(device_id const  id, sensor const&  s, sensor::other_object_info const&  other)
{
    ASSUMPTION(id < m_devices.size() && m_devices.at(id) != nullptr);
    if (m_devices.at(id)->device_ptr != nullptr)
        m_devices.at(id)->device_ptr->on_sensor_event(s, other);
}


}
