#include <com/simulation_context.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

namespace com {


simulation_context_ptr  simulation_context::create()
{
    return std::shared_ptr<simulation_context>(new simulation_context);
}

simulation_context::simulation_context()
    : m_root_folder()
    , m_folders()
    , m_frames()
    , m_batches()
    , m_colliders()
    , m_rigid_bodies()
    , m_sensors()
    , m_activators()
    , m_devices()
    , m_agents()
{
    m_root_folder = { OBJECT_KIND::FOLDER, m_folders.insert(folder_content_type("ROOT", invalid_object_guid())) };
}


/////////////////////////////////////////////////////////////////////////////////////
// FOLDER API
/////////////////////////////////////////////////////////////////////////////////////


simulation_context::folder_content_type::folder_content_type()
    : folder_name()
    , parent_folder(invalid_object_guid())
    , child_folders()
    , content()
{}


simulation_context::folder_content_type::folder_content_type(
        std::string const&  folder_name,
        object_guid const  parent_folder_guid
        )
    : folder_name(folder_name)
    , parent_folder(parent_folder_guid)
    , child_folders()
    , content()
{
    ASSUMPTION(!folder_name.empty() && parent_folder == invalid_object_guid() || parent_folder.kind == OBJECT_KIND::FOLDER);
}


bool  simulation_context::is_valid_folder_guid(object_guid const  folder_guid) const
{
    return folder_guid.kind == OBJECT_KIND::FOLDER && m_folders.valid(folder_guid.index);
}


object_guid  simulation_context::root_folder() const
{
    return m_root_folder;
}


object_guid  simulation_context::insert_folder(object_guid const  under_folder_guid, std::string const&  folder_name)
{
    ASSUMPTION(is_valid_folder_guid(under_folder_guid));
    object_guid const  new_folder_guid = { OBJECT_KIND::FOLDER, m_folders.insert(folder_content_type(folder_name, under_folder_guid)) };
    m_folders.at(under_folder_guid.index).child_folders.insert({ folder_name, new_folder_guid });
    return new_folder_guid;
}


void  simulation_context::erase_non_root_empty_folder(object_guid const  folder_guid)
{
    ASSUMPTION(is_valid_folder_guid(folder_guid) && folder_guid != m_root_folder && folder_content(folder_guid).child_folders.empty());
    folder_content_type const&  erased_folder = m_folders.at(folder_guid.index);
    m_folders.at(erased_folder.parent_folder.index).child_folders.erase(erased_folder.folder_name);
    m_folders.erase(folder_guid.index);
}


simulation_context::folder_content_type const&  simulation_context::folder_content(object_guid const  folder_guid) const
{
    ASSUMPTION(is_valid_folder_guid(folder_guid));
    return m_folders.at(folder_guid.index);
}


object_guid  simulation_context::folder_of(object_guid const  guid) const
{
    switch (guid.kind)
    {
    case OBJECT_KIND::FRAME: return folder_of_frame(guid);
    case OBJECT_KIND::BATCH: return folder_of_batch(guid);
    case OBJECT_KIND::COLLIDER: return folder_of_collider(guid);
    case OBJECT_KIND::RIGID_BODY: return folder_of_rigid_body(guid);
    case OBJECT_KIND::SENSOR: return folder_of_sensor(guid);
    case OBJECT_KIND::ACTIVATOR: return folder_of_activator(guid);
    case OBJECT_KIND::DEVICE: return folder_of_device(guid);
    case OBJECT_KIND::AGENT: return folder_of_agent(guid);
    case OBJECT_KIND::FOLDER: return guid == invalid_object_guid() ? invalid_object_guid() : folder_content(guid).parent_folder;
    case OBJECT_KIND::NONE: return invalid_object_guid();
    default:
        UNREACHABLE();
    }
}


std::string const&  simulation_context::name_of(object_guid const  guid) const
{
    static std::string  empty;
    switch (guid.kind)
    {
    case OBJECT_KIND::FRAME: return name_of_frame(guid);
    case OBJECT_KIND::BATCH: return name_of_batch(guid);
    case OBJECT_KIND::COLLIDER: return name_of_collider(guid);
    case OBJECT_KIND::RIGID_BODY: return name_of_rigid_body(guid);
    case OBJECT_KIND::SENSOR: return name_of_sensor(guid);
    case OBJECT_KIND::ACTIVATOR: return name_of_activator(guid);
    case OBJECT_KIND::DEVICE: return name_of_device(guid);
    case OBJECT_KIND::AGENT: return name_of_agent(guid);
    case OBJECT_KIND::FOLDER: return guid == invalid_object_guid() ? empty : folder_content(guid).folder_name;
    case OBJECT_KIND::NONE: return empty;
    default:
        UNREACHABLE();
    }
}


/////////////////////////////////////////////////////////////////////////////////////
// FRAMES API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_frame_guid(object_guid const  frame_guid) const
{
    return frame_guid.kind == OBJECT_KIND::FRAME && m_frames.valid(frame_guid.index);
}


object_guid  simulation_context::folder_of_frame(object_guid const  frame_guid) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return { OBJECT_KIND::FOLDER, m_frames.at(frame_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_frame(object_guid const  frame_guid) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames.at(frame_guid.index).element_name;
}


/////////////////////////////////////////////////////////////////////////////////////
// BATCHES API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_batch_guid(object_guid const  batch_guid) const
{
    return batch_guid.kind == OBJECT_KIND::BATCH && m_batches.valid(batch_guid.index);
}


object_guid  simulation_context::folder_of_batch(object_guid const  batch_guid) const
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return { OBJECT_KIND::FOLDER, m_frames.at(batch_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_batch(object_guid const  batch_guid) const
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return m_batches.at(batch_guid.index).element_name;
}


/////////////////////////////////////////////////////////////////////////////////////
// COLLIDERS API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_collider_guid(object_guid const  collider_guid) const
{
    return collider_guid.kind == OBJECT_KIND::COLLIDER && m_colliders.valid(collider_guid.index);
}


object_guid  simulation_context::folder_of_collider(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return { OBJECT_KIND::FOLDER, m_frames.at(collider_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_collider(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_colliders.at(collider_guid.index).element_name;
}


/////////////////////////////////////////////////////////////////////////////////////
// RIGID BODIES API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_rigid_body_guid(object_guid const  rigid_body_guid) const
{
    return rigid_body_guid.kind == OBJECT_KIND::RIGID_BODY && m_rigid_bodies.valid(rigid_body_guid.index);
}


object_guid  simulation_context::folder_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return { OBJECT_KIND::FOLDER, m_frames.at(rigid_body_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_bodies.at(rigid_body_guid.index).element_name;
}


/////////////////////////////////////////////////////////////////////////////////////
// SENSORS API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_sensor_guid(object_guid const  sensor_guid) const
{
    return sensor_guid.kind == OBJECT_KIND::SENSOR && m_sensors.valid(sensor_guid.index);
}


object_guid  simulation_context::folder_of_sensor(object_guid const  sensor_guid) const
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    return { OBJECT_KIND::FOLDER, m_frames.at(sensor_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_sensor(object_guid const  sensor_guid) const
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    return m_sensors.at(sensor_guid.index).element_name;
}


/////////////////////////////////////////////////////////////////////////////////////
// ACTIVATORS API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_activator_guid(object_guid const  activator_guid) const
{
    return activator_guid.kind == OBJECT_KIND::ACTIVATOR && m_activators.valid(activator_guid.index);
}


object_guid  simulation_context::folder_of_activator(object_guid const  activator_guid) const
{
    ASSUMPTION(is_valid_activator_guid(activator_guid));
    return { OBJECT_KIND::FOLDER, m_frames.at(activator_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_activator(object_guid const  activator_guid) const
{
    ASSUMPTION(is_valid_activator_guid(activator_guid));
    return m_activators.at(activator_guid.index).element_name;
}


/////////////////////////////////////////////////////////////////////////////////////
// DEVICES API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_device_guid(object_guid const  device_guid) const
{
    return device_guid.kind == OBJECT_KIND::DEVICE && m_devices.valid(device_guid.index);
}


object_guid  simulation_context::folder_of_device(object_guid const  device_guid) const
{
    ASSUMPTION(is_valid_device_guid(device_guid));
    return { OBJECT_KIND::FOLDER, m_frames.at(device_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_device(object_guid const  device_guid) const
{
    ASSUMPTION(is_valid_device_guid(device_guid));
    return m_devices.at(device_guid.index).element_name;
}


/////////////////////////////////////////////////////////////////////////////////////
// AGENTS API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_agent_guid(object_guid const  agent_guid) const
{
    return agent_guid.kind == OBJECT_KIND::AGENT && m_agents.valid(agent_guid.index);
}


object_guid  simulation_context::folder_of_agent(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    return { OBJECT_KIND::FOLDER, m_frames.at(agent_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_agent(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    return m_agents.at(agent_guid.index).element_name;
}


}
