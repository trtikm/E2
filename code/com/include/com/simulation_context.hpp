#ifndef COM_SIMULATION_CONTEXT_HPP_INCLUDED
#   define COM_SIMULATION_CONTEXT_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <com/frame_of_reference.hpp>
#   include <gfx/batch.hpp>
#   include <angeo/collision_object_id.hpp>
#   include <angeo/rigid_body.hpp>
#   include <ai/object_id.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/dynamic_array.hpp>
#   include <string>
#   include <memory>

namespace com {


struct simulation_context;
using  simulation_context_ptr = std::shared_ptr<simulation_context>;


struct simulation_context
{
    static simulation_context_ptr  create();

    /////////////////////////////////////////////////////////////////////////////////////
    // FOLDER API
    /////////////////////////////////////////////////////////////////////////////////////

    struct  folder_content_type
    {
        folder_content_type();
        folder_content_type(std::string const&  folder_name, object_guid const  parent_folder_guid);
        std::string  folder_name;
        object_guid  parent_folder;
        std::unordered_map<std::string, object_guid>  child_folders;    // Only folder guids.
        std::unordered_map<std::string, object_guid>  content;          // All other guids.
    };

    bool  is_valid_folder_guid(object_guid const  folder_guid) const;
    object_guid  root_folder() const;
    object_guid  insert_folder(object_guid const  under_folder_guid, std::string const&  folder_name);
    void  erase_non_root_empty_folder(object_guid const  folder_guid);
    folder_content_type const&  folder_content(object_guid const  folder_guid) const;
    object_guid  folder_of(object_guid const  guid) const;
    std::string const&  name_of(object_guid const  guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // FRAMES API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_valid_frame_guid(object_guid const  frame_guid) const;
    object_guid  folder_of_frame(object_guid const  frame_guid) const;
    std::string const&  name_of_frame(object_guid const  frame_guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // BATCHES API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_valid_batch_guid(object_guid const  batch_guid) const;
    object_guid  folder_of_batch(object_guid const  batch_guid) const;
    std::string const&  name_of_batch(object_guid const  batch_guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // COLLIDERS API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_valid_collider_guid(object_guid const  collider_guid) const;
    object_guid  folder_of_collider(object_guid const  collider_guid) const;
    std::string const&  name_of_collider(object_guid const  collider_guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // RIGID BODIES API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_valid_rigid_body_guid(object_guid const  rigid_body_guid) const;
    object_guid  folder_of_rigid_body(object_guid const  rigid_body_guid) const;
    std::string const&  name_of_rigid_body(object_guid const  rigid_body_guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // SENSORS API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_valid_sensor_guid(object_guid const  sensor_guid) const;
    object_guid  folder_of_sensor(object_guid const  sensor_guid) const;
    std::string const&  name_of_sensor(object_guid const  sensor_guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // ACTIVATORS API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_valid_activator_guid(object_guid const  activator_guid) const;
    object_guid  folder_of_activator(object_guid const  activator_guid) const;
    std::string const&  name_of_activator(object_guid const  activator_guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // DEVICES API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_valid_device_guid(object_guid const  device_guid) const;
    object_guid  folder_of_device(object_guid const  device_guid) const;
    std::string const&  name_of_device(object_guid const  device_guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // AGENTS API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_valid_agent_guid(object_guid const  agent_guid) const;
    object_guid  folder_of_agent(object_guid const  agent_guid) const;
    std::string const&  name_of_agent(object_guid const  agent_guid) const;

private:

    using  index_type = object_guid::index_type;

    template<typename T>
    struct  folder_element
    {
        using  module_specific_id = T;
        module_specific_id  id;
        index_type  folder_index;
        std::string  element_name;
    };

    object_guid  m_root_folder;
    dynamic_array<folder_content_type, index_type>  m_folders;
    dynamic_array<folder_element<frame_id>, index_type>  m_frames;
    dynamic_array<folder_element<gfx::batch>, index_type>  m_batches;
    dynamic_array<folder_element<angeo::collision_object_id>, index_type>  m_colliders;
    dynamic_array<folder_element<angeo::rigid_body_id>, index_type>  m_rigid_bodies;
    dynamic_array<folder_element<ai::object_id>, index_type>  m_sensors;
    dynamic_array<folder_element<ai::object_id>, index_type>  m_activators;
    dynamic_array<folder_element<ai::object_id>, index_type>  m_devices;
    dynamic_array<folder_element<ai::object_id>, index_type>  m_agents;

    simulation_context();
    simulation_context(simulation_context const&) = delete;
    simulation_context(simulation_context const&&) = delete;
    simulation_context& operator=(simulation_context const&) = delete;
    simulation_context& operator=(simulation_context const&&) = delete;
};


}

#endif
