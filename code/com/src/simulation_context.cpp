#include <com/simulation_context.hpp>
#include <angeo/collision_scene.hpp>
#include <angeo/rigid_body_simulator.hpp>
#include <angeo/mass_and_inertia_tensor.hpp>
#include <ai/simulator.hpp>
#include <ai/scene_binding.hpp>
#include <utility/async_resource_load.hpp>
#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <algorithm>
#include <sstream>

namespace com {


simulation_context_ptr  simulation_context::create(
        std::shared_ptr<std::vector<std::shared_ptr<angeo::collision_scene> > > const  collision_scenes_ptr_,
        std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
        std::shared_ptr<com::device_simulator> const  device_simulator_ptr_,
        std::shared_ptr<ai::simulator> const  ai_simulator_ptr_,
        std::string const&  data_root_dir_
        )
{
    ASSUMPTION(collision_scenes_ptr_ != nullptr && !collision_scenes_ptr_->empty() &&
               rigid_body_simulator_ptr_ != nullptr && ai_simulator_ptr_ != nullptr);
    simulation_context_ptr const  ctx_ptr = std::shared_ptr<simulation_context>(new simulation_context(
                collision_scenes_ptr_,
                rigid_body_simulator_ptr_,
                device_simulator_ptr_,
                ai_simulator_ptr_,
                data_root_dir_
                ));
    ctx_ptr->m_self_ptr = ctx_ptr;
    return  ctx_ptr;
}


simulation_context::simulation_context(
        std::shared_ptr<std::vector<std::shared_ptr<angeo::collision_scene> > > const  collision_scenes_ptr_,
        std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
        std::shared_ptr<com::device_simulator> const  device_simulator_ptr_,
        std::shared_ptr<ai::simulator> const  ai_simulator_ptr_,
        std::string const&  data_root_dir_
        )
    : m_self_ptr()
    , m_root_folder()
    , m_folders()
    , m_frames()
    , m_batches()
    , m_colliders()
    , m_rigid_bodies()
    , m_timers()
    , m_sensors()
    , m_agents()
    , m_frames_provider()
    , m_collision_scenes_ptr(collision_scenes_ptr_)
    , m_rigid_body_simulator_ptr(rigid_body_simulator_ptr_)
    , m_device_simulator_ptr(device_simulator_ptr_)
    , m_ai_simulator_ptr(ai_simulator_ptr_)
    , m_frids_to_guids()
    , m_batches_to_guids()
    , m_coids_to_guids()
    , m_rbids_to_guids()
    , m_tmids_to_guids()
    , m_seids_to_guids()
    , m_agids_to_guids()
    , m_moveable_rigid_bodies()
    , m_collision_contacts()
    , m_from_colliders_to_contacts()
    , m_data_root_dir(canonical_path(data_root_dir_.empty() ? "." : data_root_dir_).string())
    , m_invalidated_guids()
    , m_relocated_frame_guids()
    // CACHES
    , m_cache_of_imported_scenes()
    , m_cache_of_imported_batches()
    , m_cache_of_imported_motion_templates()
    , m_cache_of_imported_agent_configs()
    // EARLY REQUESTS HANDLING
    , m_rigid_bodies_with_invalidated_shape()
    , m_pending_requests_early()
    , m_requests_early_insert_custom_constraint()
    , m_requests_early_insert_instant_constraint()
    // REQUESTS HANDLING
    , m_pending_requests()
    , m_requests_erase_folder()
    , m_requests_erase_frame()
    , m_requests_relocate_frame()
    , m_requests_set_parent_frame()
    , m_requests_erase_batch()
    , m_requests_enable_collider()
    , m_requests_enable_colliding()
    , m_requests_enable_colliding_by_path()
    , m_requests_insert_collider_box()
    , m_requests_insert_collider_capsule()
    , m_requests_insert_collider_sphere()
    , m_requests_erase_collider()
    , m_requests_insert_rigid_body()
    , m_requests_erase_rigid_body()
    , m_requests_set_linear_velocity()
    , m_requests_set_linear_velocity_by_path()
    , m_requests_set_angular_velocity()
    , m_requests_set_angular_velocity_by_path()
    , m_requests_mul_linear_velocity()
    , m_requests_mul_linear_velocity_by_path()
    , m_requests_mul_angular_velocity()
    , m_requests_mul_angular_velocity_by_path()
    , m_requests_set_linear_acceleration_from_source()
    , m_requests_set_angular_acceleration_from_source()
    , m_requests_del_linear_acceleration_from_source()
    , m_requests_del_angular_acceleration_from_source()
    , m_requests_erase_timer()
    , m_requests_erase_sensor()
    , m_requests_erase_agent()
    // LATE REQUESTS HANDLING
    , m_requests_late_scene_import()
    , m_requests_late_insert_agent()
{
    ASSUMPTION(!m_data_root_dir.empty());

    m_root_folder = { OBJECT_KIND::FOLDER, m_folders.insert(folder_content_type("ROOT", invalid_object_guid())) };

    std::replace(m_data_root_dir.begin(), m_data_root_dir.end(), '\\', '/');
    if (*m_data_root_dir.rbegin() != '/')
        m_data_root_dir.push_back('/');
}


simulation_context::~simulation_context()
{
    clear(true);
}


/////////////////////////////////////////////////////////////////////////////////////
// FOLDER API
/////////////////////////////////////////////////////////////////////////////////////


simulation_context::folder_content_type::folder_content_type()
    : folder_name()
    , parent_folder(invalid_object_guid())
    , child_folders()
    , content()
    , content_index()
{}


simulation_context::folder_content_type::folder_content_type(
        std::string const&  folder_name,
        object_guid const  parent_folder_guid
        )
    : folder_name(folder_name)
    , parent_folder(parent_folder_guid)
    , child_folders()
    , content()
    , content_index()
{
    ASSUMPTION(!folder_name.empty() && parent_folder == invalid_object_guid() || parent_folder.kind == OBJECT_KIND::FOLDER);
}


void  simulation_context::folder_content_type::insert_content(OBJECT_KIND const  kind, object_guid const  guid)
{
    insert_content(kind, guid, to_string(kind));
}


void  simulation_context::folder_content_type::insert_content(
        OBJECT_KIND const  kind,
        object_guid const  guid,
        std::string const&  name
        )
{
    content.insert({ name, guid });
    content_index[kind].insert(name);
}


void  simulation_context::folder_content_type::erase_content(OBJECT_KIND const  kind)
{
    erase_content(to_string(kind), kind);
}


void  simulation_context::folder_content_type::erase_content(std::string const&  name, OBJECT_KIND const  kind)
{
    content.erase(name);
    auto const  it = content_index.find(kind);
    it->second.erase(name);
    if (it->second.empty())
        content_index.erase(it);
}


bool  simulation_context::is_valid_folder_guid(object_guid const  folder_guid) const
{
    return folder_guid.kind == OBJECT_KIND::FOLDER && m_folders.valid(folder_guid.index);
}


object_guid  simulation_context::root_folder() const
{
    return m_root_folder;
}


bool  simulation_context::is_folder_empty(object_guid const  folder_guid) const
{
    folder_content_type const&  fct = folder_content(folder_guid);
    return fct.child_folders.empty() && fct.content.empty();
}


simulation_context::folder_content_type const&  simulation_context::folder_content(object_guid const  folder_guid) const
{
    ASSUMPTION(is_valid_folder_guid(folder_guid));
    return m_folders.at(folder_guid.index);
}


object_guid  simulation_context::folder_content_of_name(object_guid const  folder_guid, std::string const&  name) const
{
    folder_content_type::names_to_guids_map const&  content = folder_content(folder_guid).content;
    auto const  it = content.find(name);
    return it == content.end() ? invalid_object_guid() : it->second;
}


object_guid  simulation_context::folder_content_frame(object_guid const  folder_guid) const
{
    return folder_content_of_name(folder_guid, to_string(OBJECT_KIND::FRAME));
}


object_guid  simulation_context::folder_content_rigid_body(object_guid const  folder_guid) const
{
    return folder_content_of_name(folder_guid, to_string(OBJECT_KIND::RIGID_BODY));
}


object_guid  simulation_context::folder_content_agent(object_guid const  folder_guid) const
{
    return folder_content_of_name(folder_guid, to_string(OBJECT_KIND::AGENT));
}


object_guid  simulation_context::child_folder(object_guid const  folder_guid, std::string const&  child_folder_name) const
{
    folder_content_type::names_to_guids_map const&  children = folder_content(folder_guid).child_folders;
    auto const  it = children.find(child_folder_name);
    return it == children.end() ? invalid_object_guid() : it->second;
}


object_guid  simulation_context::parent_folder(object_guid const  folder_guid) const
{
    ASSUMPTION(is_valid_folder_guid(folder_guid));
    return folder_content(folder_guid).parent_folder;
}


object_guid  simulation_context::folder_of(object_guid const  guid) const
{
    switch (guid.kind)
    {
    case OBJECT_KIND::FOLDER: return parent_folder(guid);
    case OBJECT_KIND::FRAME: return folder_of_frame(guid);
    case OBJECT_KIND::BATCH: return folder_of_batch(guid);
    case OBJECT_KIND::COLLIDER: return folder_of_collider(guid);
    case OBJECT_KIND::RIGID_BODY: return folder_of_rigid_body(guid);
    case OBJECT_KIND::TIMER: return folder_of_timer(guid);
    case OBJECT_KIND::SENSOR: return folder_of_sensor(guid);
    case OBJECT_KIND::AGENT: return folder_of_agent(guid);
    case OBJECT_KIND::NONE: return invalid_object_guid();
    default: UNREACHABLE(); break;
    }
}


std::string const&  simulation_context::name_of_folder(object_guid const  folder_guid) const
{
    return folder_content(folder_guid).folder_name;
}


std::string const&  simulation_context::name_of(object_guid const  guid) const
{
    static std::string  empty;
    switch (guid.kind)
    {
    case OBJECT_KIND::FOLDER: return name_of_folder(guid);
    case OBJECT_KIND::FRAME: return name_of_frame(guid);
    case OBJECT_KIND::BATCH: return name_of_batch(guid);
    case OBJECT_KIND::COLLIDER: return name_of_collider(guid);
    case OBJECT_KIND::RIGID_BODY: return name_of_rigid_body(guid);
    case OBJECT_KIND::TIMER: return name_of_timer(guid);
    case OBJECT_KIND::SENSOR: return name_of_sensor(guid);
    case OBJECT_KIND::AGENT: return name_of_agent(guid);
    case OBJECT_KIND::NONE: return empty;
    default: UNREACHABLE(); break;
    }
}


simulation_context::folder_guid_iterator  simulation_context::folders_begin() const
{
    return folder_guid_iterator(m_folders.valid_indices().begin());
}


simulation_context::folder_guid_iterator  simulation_context::folders_end() const
{
    return folder_guid_iterator(m_folders.valid_indices().end());
}


void  simulation_context::for_each_parent_folder(object_guid  folder_guid, bool const  including_passed_folder,
                                                 folder_visitor_type const&  visitor) const
{
    ASSUMPTION(!including_passed_folder || is_valid_folder_guid(folder_guid));
    for (folder_guid = including_passed_folder ? folder_guid : folder_content(folder_guid).parent_folder;
            folder_guid != invalid_object_guid();
            folder_guid = folder_content(folder_guid).parent_folder)
        if (!visitor(folder_guid, folder_content(folder_guid)))
            return;;
}


void  simulation_context::for_each_child_folder(object_guid const  folder_guid, bool const  recursively,
                                                bool const  including_passed_folder, folder_visitor_type const&  visitor) const
{
    folder_content_type const&  fct = folder_content(folder_guid);
    for (auto const&  name_and_guid : including_passed_folder ? folder_content_type::names_to_guids_map{
                                                                        {fct.folder_name, folder_guid }
                                                                        } :
                                                                fct.child_folders)
        if (visitor(name_and_guid.second, folder_content(name_and_guid.second)) && recursively)
            for_each_child_folder(name_and_guid.second, recursively, false, visitor);
}


void  simulation_context::for_each_object_of_kind_under_folder(object_guid const  folder_guid, bool const  recursively,
                                                               OBJECT_KIND const  kind,
                                                               folder_content_visitor_type const&  visitor) const
{
    folder_content_type const&  fct = folder_content(folder_guid);
    auto const  content_it = fct.content_index.find(kind);
    if (content_it != fct.content_index.end())
        for (std::string const&  name : content_it->second)
            if (!visitor(fct.content.at(name)))
                return;
    if (recursively)
        for (auto const&  name_and_guid : fct.child_folders)
            for_each_object_of_kind_under_folder(name_and_guid.second, recursively, kind, visitor);
}


object_guid  simulation_context::insert_folder(object_guid const  under_folder_guid, std::string  folder_name,
                                               bool const  resolve_name_collision) const
{
    ASSUMPTION(is_valid_folder_guid(under_folder_guid));

    simulation_context* const  self = const_cast<simulation_context*>(this);

    {
        folder_content_type::names_to_guids_map&  child_folders = self->m_folders.at(under_folder_guid.index).child_folders;

        if (resolve_name_collision && child_folders.count(folder_name) != 0UL)
        {
            natural_32_bit  counter = 0U;
            for ( ; child_folders.count(folder_name+ '.' + std::to_string(counter)) != 0UL; ++counter)
                ;
            folder_name = folder_name + '.' + std::to_string(counter);
        }

        ASSUMPTION(child_folders.count(folder_name) == 0UL);
    }

    object_guid const  new_folder_guid = {
            OBJECT_KIND::FOLDER,
            self->m_folders.insert(folder_content_type(folder_name, under_folder_guid))
            };
    self->m_folders.at(under_folder_guid.index).child_folders.insert({ folder_name, new_folder_guid });
    return new_folder_guid;
}


void  simulation_context::request_erase_non_root_empty_folder(object_guid const  folder_guid) const
{
    m_requests_erase_folder.push_back(folder_guid);
    m_pending_requests.push_back(REQUEST_ERASE_FOLDER);
}


void  simulation_context::request_erase_non_root_folder(object_guid const  folder_guid) const
{
    folder_content_type const&  fct = folder_content(folder_guid);

    for (auto const&  name_and_guid : fct.child_folders)
        request_erase_non_root_folder(name_and_guid.second);

    object_guid  frame_guid = invalid_object_guid();
    object_guid  agent_guid = invalid_object_guid();
    for (auto const&  name_and_guid : fct.content)
        switch (name_and_guid.second.kind)
        {
        case OBJECT_KIND::FRAME:
            INVARIANT(frame_guid == invalid_object_guid());
            frame_guid = name_and_guid.second;
            break;
        case OBJECT_KIND::BATCH:
            request_erase_batch(name_and_guid.second);
            break;
        case OBJECT_KIND::COLLIDER:
            request_erase_collider(name_and_guid.second);
            break;
        case OBJECT_KIND::RIGID_BODY:
            request_erase_rigid_body(name_and_guid.second);
            break;
        case OBJECT_KIND::TIMER:
            request_erase_timer(name_and_guid.second);
            break;
        case OBJECT_KIND::SENSOR:
            request_erase_sensor(name_and_guid.second);
            break;
        case OBJECT_KIND::AGENT:
            INVARIANT(agent_guid == invalid_object_guid());
            agent_guid = name_and_guid.second;
            break;
        default: UNREACHABLE(); break;
        }
    if (agent_guid != invalid_object_guid())
        request_erase_agent(agent_guid);
    if (frame_guid != invalid_object_guid())
        request_erase_frame(frame_guid);

    request_erase_non_root_empty_folder(folder_guid);
}


// Disabled (not const) for modules.


void  simulation_context::erase_non_root_empty_folder(object_guid const  folder_guid)
{
    ASSUMPTION(is_folder_empty(folder_guid));
    m_invalidated_guids.insert(folder_guid);
    if (folder_guid != root_folder())
    {
        folder_content_type const&  erased_folder = m_folders.at(folder_guid.index);
        m_folders.at(erased_folder.parent_folder.index).child_folders.erase(erased_folder.folder_name);
        m_folders.erase(folder_guid.index);
    }
}


void  simulation_context::erase_non_root_folder(object_guid const  folder_guid)
{
    folder_content_type const&  fct = folder_content(folder_guid);

    m_invalidated_guids.insert(folder_guid);

    while (!fct.child_folders.empty())
        erase_non_root_folder(fct.child_folders.begin()->second);

    object_guid  frame_guid = invalid_object_guid();
    object_guid  agent_guid = invalid_object_guid();
    for (auto  it = fct.content.begin(), itc = it; it != fct.content.end(); itc = it)
    {
        ++it;
        switch (itc->second.kind)
        {
        case OBJECT_KIND::FRAME:
            INVARIANT(frame_guid == invalid_object_guid());
            frame_guid = itc->second;
            break;
        case OBJECT_KIND::BATCH:
            erase_batch(itc->second);
            break;
        case OBJECT_KIND::COLLIDER:
            erase_collider(itc->second);
            break;
        case OBJECT_KIND::RIGID_BODY:
            erase_rigid_body(itc->second);
            break;
        case OBJECT_KIND::TIMER:
            erase_timer(itc->second);
            break;
        case OBJECT_KIND::SENSOR:
            erase_sensor(itc->second);
            break;
        case OBJECT_KIND::AGENT:
            INVARIANT(agent_guid == invalid_object_guid());
            agent_guid = itc->second;
            break;
        default: UNREACHABLE(); break;
        }
    }
    if (agent_guid != invalid_object_guid())
        erase_agent(agent_guid);
    if (frame_guid != invalid_object_guid())
        erase_frame(frame_guid);

    erase_non_root_empty_folder(folder_guid);
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


object_guid  simulation_context::to_frame_guid(frame_id const  frid) const
{
    return m_frids_to_guids.at(frid);
}


simulation_context::frame_guid_iterator  simulation_context::frames_begin() const
{
    return frame_guid_iterator(m_frames.valid_indices().begin());
}


simulation_context::frame_guid_iterator  simulation_context::frames_end() const
{
    return frame_guid_iterator(m_frames.valid_indices().end());
}


object_guid  simulation_context::find_closest_frame(object_guid const  folder_guid, bool const  including_passed_folder) const
{
    object_guid  result = invalid_object_guid();
    for_each_parent_folder(folder_guid, including_passed_folder, [this, &result](object_guid, folder_content_type const&  fct) {
            auto const  it = fct.content.find(to_string(OBJECT_KIND::FRAME));
            if (it != fct.content.end())
            {
                ASSUMPTION(is_valid_frame_guid(it->second));
                result = it->second;
                return false;
            }
            return true;
        });
    return result;
}


object_guid  simulation_context::parent_frame(object_guid const  frame_guid) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    frame_id const  frid = m_frames_provider.parent(m_frames.at(frame_guid.index).id);
    return frid == invalid_frame_id() ? invalid_object_guid() : to_frame_guid(frid);
}


void  simulation_context::direct_children_frames(object_guid const  frame_guid, std::vector<object_guid>&  direct_children) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    for (frame_id  frid : m_frames_provider.children(m_frames.at(frame_guid.index).id))
        direct_children.push_back(to_frame_guid(frid));
}


angeo::coordinate_system const&  simulation_context::frame_coord_system(object_guid const  frame_guid) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.frame(m_frames.at(frame_guid.index).id);
}


angeo::coordinate_system_explicit const&  simulation_context::frame_explicit_coord_system(object_guid const  frame_guid) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.frame_explicit(m_frames.at(frame_guid.index).id);
}


angeo::coordinate_system const&  simulation_context::frame_coord_system_in_world_space(object_guid const  frame_guid) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.frame_in_world_space(m_frames.at(frame_guid.index).id);
}


angeo::coordinate_system_explicit const&  simulation_context::frame_explicit_coord_system_in_world_space(object_guid const  frame_guid) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.frame_explicit_in_world_space(m_frames.at(frame_guid.index).id);
}


matrix44 const&  simulation_context::frame_world_matrix(object_guid const  frame_guid) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.world_matrix(m_frames.at(frame_guid.index).id);
}


object_guid  simulation_context::insert_frame(object_guid const  under_folder_guid, object_guid const  parent_frame_guid,
                                              vector3 const  origin, quaternion const  orientation, bool const  relative_to_parent) const
{
    simulation_context* const  self = const_cast<simulation_context*>(this);
    object_guid const  frame_guid = self->insert_frame(under_folder_guid);
    if (is_valid_frame_guid(parent_frame_guid))
        self->set_parent_frame(frame_guid, parent_frame_guid);
    self->frame_relocate(frame_guid, origin, orientation, relative_to_parent);
    return frame_guid;
}


object_guid  simulation_context::insert_frame(object_guid const  under_folder_guid, object_guid const  parent_frame_guid,
                                              angeo::coordinate_system const&  frame, bool const  relative_to_parent) const
{
    return insert_frame(under_folder_guid, parent_frame_guid, frame.origin(), frame.orientation(), relative_to_parent);
}


void  simulation_context::request_erase_frame(object_guid const  frame_guid) const
{
    m_requests_erase_frame.push_back(frame_guid);
    m_pending_requests.push_back(REQUEST_ERASE_FRAME);
}


void  simulation_context::request_relocate_frame(object_guid const  frame_guid, vector3 const&  new_origin,
                                                 quaternion const&  new_orientation, bool const  relative_to_parent) const
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    m_requests_relocate_frame.push_back({ frame_guid, new_origin, new_orientation, relative_to_parent });
    m_pending_requests.push_back(REQUEST_RELOCATE_FRAME);
}


void  simulation_context::request_relocate_frame(object_guid const  frame_guid, angeo::coordinate_system const&  frame,
                                                 bool const  relative_to_parent) const
{
    request_relocate_frame(frame_guid, frame.origin(), frame.orientation(), relative_to_parent);
}


void  simulation_context::request_set_parent_frame(object_guid const  frame_guid, object_guid const  parent_frame_guid) const
{
    ASSUMPTION(
            is_valid_frame_guid(frame_guid) &&
            (is_valid_frame_guid(parent_frame_guid) || parent_frame_guid == invalid_object_guid())
            );
    m_requests_set_parent_frame.push_back({ frame_guid, parent_frame_guid });
    m_pending_requests.push_back(REQUEST_SET_PARENT_FRAME);
}


// Disabled (not const) for modules.


void  simulation_context::set_parent_frame(object_guid const  frame_guid, object_guid const  parent_frame_guid)
{
    ASSUMPTION(
        is_valid_frame_guid(frame_guid) &&
        (parent_frame_guid == invalid_object_guid() ||
        [this](object_guid const  folder_guid, object_guid  parent_folder_guid) -> bool {
            for ( ; parent_folder_guid != invalid_object_guid(); parent_folder_guid = folder_content(parent_folder_guid).parent_folder)
                if (parent_folder_guid == folder_guid)
                    return false;
            return true;
        }(folder_of_frame(frame_guid), folder_of_frame(parent_frame_guid)))
        );
    m_frames_provider.set_parent(
            m_frames.at(frame_guid.index).id,
            parent_frame_guid == invalid_object_guid() ? invalid_frame_id() : m_frames.at(parent_frame_guid.index).id
            );
    m_relocated_frame_guids.insert(frame_guid);
}


object_guid  simulation_context::insert_frame(object_guid const  under_folder_guid)
{
    ASSUMPTION(is_folder_empty(under_folder_guid));

    frame_id const  frid = m_frames_provider.insert();

    object_guid const  frame_guid = {
            OBJECT_KIND::FRAME,
            m_frames.insert({ frid, under_folder_guid.index, to_string(OBJECT_KIND::FRAME) })
            };

    m_frids_to_guids.insert({ frid, frame_guid });

    m_folders.at(under_folder_guid.index).insert_content(OBJECT_KIND::FRAME, frame_guid);

    object_guid const  parent_frame_guid = find_closest_frame(under_folder_guid, false);
    if (parent_frame_guid != invalid_object_guid())
        m_frames_provider.set_parent(frid, m_frames.at(parent_frame_guid.index).id);

    return frame_guid;
}


void  simulation_context::erase_frame(object_guid const  frame_guid)
{
    ASSUMPTION(
        is_valid_frame_guid(frame_guid) &&
        folder_content(folder_of_frame(frame_guid)).child_folders.empty() &&
        folder_content(folder_of_frame(frame_guid)).content.size() == 1UL &&
        folder_content(folder_of_frame(frame_guid)).content.begin()->second == frame_guid
        );
    m_invalidated_guids.insert(frame_guid);
    {
        std::vector<object_guid>  direct_children;
        direct_children_frames(frame_guid, direct_children);
        for (object_guid const  child_frame_guid : direct_children)
            m_relocated_frame_guids.insert(child_frame_guid);
    }
    auto const&  elem = m_frames.at(frame_guid.index);
    m_frames_provider.erase(elem.id);
    m_folders.at(elem.folder_index).erase_content(OBJECT_KIND::FRAME);
    m_frids_to_guids.erase(elem.id);
    m_frames.erase(frame_guid.index);
}


void  simulation_context::frame_translate(object_guid const  frame_guid, vector3 const&  shift)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    m_frames_provider.translate(m_frames.at(frame_guid.index).id, shift);
    m_relocated_frame_guids.insert(frame_guid);
}


void  simulation_context::frame_rotate(object_guid const  frame_guid, quaternion const&  rotation)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    m_frames_provider.rotate(m_frames.at(frame_guid.index).id, rotation);
    m_relocated_frame_guids.insert(frame_guid);
}


void  simulation_context::frame_set_origin(object_guid const  frame_guid, vector3 const&  new_origin)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    m_frames_provider.set_origin(m_frames.at(frame_guid.index).id, new_origin);
    m_relocated_frame_guids.insert(frame_guid);
}


void  simulation_context::frame_set_orientation(object_guid const  frame_guid, quaternion const&  new_orientation)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    m_frames_provider.set_orientation(m_frames.at(frame_guid.index).id, new_orientation);
    m_relocated_frame_guids.insert(frame_guid);
}


void  simulation_context::frame_relocate(object_guid const  frame_guid, vector3 const&  new_origin, quaternion const&  new_orientation,
                                         bool const  relative_to_parent)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    if (relative_to_parent)
        m_frames_provider.relocate_relative_to_parent(m_frames.at(frame_guid.index).id, new_origin, new_orientation);
    else
        m_frames_provider.relocate(m_frames.at(frame_guid.index).id, new_origin, new_orientation);
    m_relocated_frame_guids.insert(frame_guid);
}


void  simulation_context::frame_relocate(object_guid const  frame_guid, angeo::coordinate_system const&  new_coord_system,
                                         bool const  relative_to_parent)
{
    frame_relocate(frame_guid, new_coord_system.origin(), new_coord_system.orientation(), relative_to_parent);
}


void  simulation_context::frame_relocate_relative_to_parent(object_guid const  frame_guid, object_guid const  relocation_frame_guid)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid) && is_valid_frame_guid(relocation_frame_guid));
    m_frames_provider.relocate_relative_to_parent(
            m_frames.at(frame_guid.index).id,
            m_frames.at(relocation_frame_guid.index).id
            );
    m_relocated_frame_guids.insert(frame_guid);
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
    return { OBJECT_KIND::FOLDER, m_batches.at(batch_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_batch(object_guid const  batch_guid) const
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return m_batches.at(batch_guid.index).element_name;
}


object_guid  simulation_context::to_batch_guid(gfx::batch const  batch) const
{
    auto const  it = m_batches_to_guids.find(batch.uid());
    return it == m_batches_to_guids.end() ? invalid_object_guid() : it->second;
}


simulation_context::batch_guid_iterator  simulation_context::batches_begin() const
{
    return batch_guid_iterator(m_batches.valid_indices().begin());
}


simulation_context::batch_guid_iterator  simulation_context::batches_end() const
{
    return batch_guid_iterator(m_batches.valid_indices().end());
}


std::vector<object_guid> const&  simulation_context::frames_of_batch(object_guid const  batch_guid) const
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return m_batches.at(batch_guid.index).frames;
}


std::vector<matrix44> const&  simulation_context::matrices_to_pose_bones_of_batch(object_guid const  batch_guid) const
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return m_batches.at(batch_guid.index).matrices_to_pose_bones;
}


void  simulation_context::request_erase_batch(object_guid const  batch_guid) const
{
    m_requests_erase_batch.push_back(batch_guid);
    m_pending_requests.push_back(REQUEST_ERASE_BATCH);
}


// Disabled (not const) for modules.


std::string const&  simulation_context::from_batch_guid(object_guid const  batch_guid)
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return m_batches.at(batch_guid.index).id;
}


gfx::batch  simulation_context::from_batch_guid_to_batch(object_guid const  batch_guid)
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return m_batches.at(batch_guid.index).batch;
}


object_guid  simulation_context::insert_batch(object_guid const  folder_guid, std::string const&  name, gfx::batch const  batch,
                                              std::vector<object_guid> const&  frame_guids,
                                              std::vector<matrix44> const&  matrices_to_pose_bones)
{
    ASSUMPTION(folder_content(folder_guid).content.count(name) == 0UL);

    object_guid const  batch_guid = { OBJECT_KIND::BATCH, m_batches.insert({ batch.uid(), folder_guid.index, name, batch }) };

    m_batches_to_guids.insert({ batch.uid(), batch_guid });

    m_folders.at(folder_guid.index).insert_content(OBJECT_KIND::BATCH, batch_guid, name);

    if (frame_guids.empty())
    {
        object_guid const  frame_guid = find_closest_frame(folder_guid, true);
        ASSUMPTION(frame_guid != invalid_object_guid());

        m_batches.at(batch_guid.index).frames.push_back(frame_guid);
    }
    else
        m_batches.at(batch_guid.index).frames = frame_guids;

    m_batches.at(batch_guid.index).matrices_to_pose_bones = matrices_to_pose_bones;

    return batch_guid;
}


void  simulation_context::erase_batch(object_guid const  batch_guid)
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    m_invalidated_guids.insert(batch_guid);

    auto const&  elem = m_batches.at(batch_guid.index);

    ASSUMPTION(
        folder_content(folder_of_batch(batch_guid)).content.count(elem.element_name) == 1UL &&
        folder_content(folder_of_batch(batch_guid)).content.find(elem.element_name)->second == batch_guid
        );

    m_folders.at(elem.folder_index).erase_content(elem.element_name, OBJECT_KIND::BATCH);
    m_batches_to_guids.erase(elem.id);
    m_batches.erase(batch_guid.index);
}

object_guid  simulation_context::load_batch(
        object_guid const  folder_guid, std::string const&  name,
        std::string const&  relative_disk_path,
        std::string const&  skin_name,
        std::vector<object_guid> const&  frame_guids
        )
{
    return insert_batch(
            folder_guid,
            name,
            gfx::batch(
                    std::filesystem::path(get_data_root_dir()) / "batch" / relative_disk_path,
                    gfx::default_effects_config(),
                    skin_name
                    ),
            frame_guids
            );
}


object_guid  simulation_context::insert_batch_lines3d(
        object_guid const  folder_guid, std::string const&  name,
        std::vector<std::pair<vector3,vector3> > const&  lines,
        vector4 const&  common_colour
        )
{
    return insert_batch(folder_guid, name, gfx::create_lines3d(lines, common_colour, gfx::FOG_TYPE::NONE));
}


object_guid  simulation_context::insert_batch_lines3d(
        object_guid const  folder_guid, std::string const&  name,
        std::vector<std::pair<vector3,vector3> > const&  lines,
        std::vector< vector4 > const&  colours_of_lines
        )
{
    return insert_batch(folder_guid, name, gfx::create_lines3d(lines, colours_of_lines, gfx::FOG_TYPE::NONE));
}


object_guid  simulation_context::insert_batch_wireframe_box(
        object_guid const  folder_guid, std::string const&  name,
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, gfx::create_wireframe_box(half_sizes_along_axes, colour, gfx::FOG_TYPE::NONE));
}


object_guid  simulation_context::insert_batch_solid_box(
        object_guid const  folder_guid, std::string const&  name,
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, gfx::create_solid_box(half_sizes_along_axes, colour, gfx::FOG_TYPE::NONE));
}


object_guid  simulation_context::insert_batch_wireframe_capsule(
        object_guid const  folder_guid, std::string const&  name,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name,
        gfx::create_wireframe_capsule(
                half_distance_between_end_points,
                thickness_from_central_line,
                num_lines_per_quarter_of_circle,
                colour,
                gfx::FOG_TYPE::NONE
                )
        );
}


object_guid  simulation_context::insert_batch_solid_capsule(
        object_guid const  folder_guid, std::string const&  name,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name,
        gfx::create_solid_capsule(
                half_distance_between_end_points,
                thickness_from_central_line,
                num_lines_per_quarter_of_circle,
                colour,
                gfx::FOG_TYPE::NONE
                )
        );
}


object_guid  simulation_context::insert_batch_wireframe_sphere(
        object_guid const  folder_guid, std::string const&  name,
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name,
        gfx::create_wireframe_sphere(radius, num_lines_per_quarter_of_circle, colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_solid_smooth_sphere(
        object_guid const  folder_guid, std::string const&  name,
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name,
        gfx::create_solid_smooth_sphere(radius, num_lines_per_quarter_of_circle, colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_solid_sphere(
        object_guid const  folder_guid, std::string const&  name,
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name,
        gfx::create_solid_sphere(radius, num_lines_per_quarter_of_circle, colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_triangle_mesh(
        object_guid const  folder_guid, std::string const&  name,
        std::vector< std::array<float_32_bit, 3> > const& vertices,
        std::vector< std::array<float_32_bit, 3> > const& normals,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, gfx::create_triangle_mesh(vertices, normals, colour, gfx::FOG_TYPE::NONE, ""));
}


object_guid  simulation_context::insert_batch_wireframe_perspective_frustum(
        object_guid const  folder_guid, std::string const&  name,
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        vector4 const&  colour,
        bool const  with_axis
        )
{
    return insert_batch(folder_guid, name,
        gfx::create_wireframe_perspective_frustum(near_plane, far_plane, left_plane, right_plane, top_plane, bottom_plane,
                                                  colour, with_axis)
        );
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
    return { OBJECT_KIND::FOLDER, m_colliders.at(collider_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_collider(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_colliders.at(collider_guid.index).element_name;
}


object_guid  simulation_context::to_collider_guid(angeo::collision_object_id const  coid, collision_scene_index const  scene_index) const
{
    return m_coids_to_guids.at({coid, scene_index});
}


simulation_context::collider_guid_iterator  simulation_context::colliders_begin() const
{
    return collider_guid_iterator(m_colliders.valid_indices().begin());
}


simulation_context::collider_guid_iterator  simulation_context::colliders_end() const
{
    return collider_guid_iterator(m_colliders.valid_indices().end());
}


object_guid  simulation_context::frame_of_collider(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_colliders.at(collider_guid.index).frame;
}


object_guid  simulation_context::owner_of_collider(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_colliders.at(collider_guid.index).owner;
}


object_guid  simulation_context::rigid_body_of_collider(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_colliders.at(collider_guid.index).rigid_body;
}


angeo::COLLISION_MATERIAL_TYPE  simulation_context::collision_material_of(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return m_collision_scenes_ptr->at(collider.scene_index)->get_material(collider.id.front());
}


angeo::COLLISION_CLASS  simulation_context::collision_class_of(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return m_collision_scenes_ptr->at(collider.scene_index)->get_collision_class(collider.id.front());
}


angeo::COLLISION_SHAPE_TYPE  simulation_context::collider_shape_type(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return angeo::get_shape_type(m_colliders.at(collider_guid.index).id.front());
}


vector3 const&  simulation_context::collider_box_half_sizes_along_axes(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return m_collision_scenes_ptr->at(collider.scene_index)->get_box_half_sizes_along_axes(collider.id.front());
}


float_32_bit  simulation_context::collider_capsule_half_distance_between_end_points(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return m_collision_scenes_ptr->at(collider.scene_index)->get_capsule_half_distance_between_end_points(collider.id.front());
}


float_32_bit  simulation_context::collider_capsule_thickness_from_central_line(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return m_collision_scenes_ptr->at(collider.scene_index)->get_capsule_thickness_from_central_line(collider.id.front());
}


float_32_bit  simulation_context::collider_sphere_radius(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return m_collision_scenes_ptr->at(collider.scene_index)->get_sphere_radius(collider.id.front());
}

natural_32_bit  simulation_context::collider_num_coids(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return (natural_32_bit)collider.id.size();
}

bool  simulation_context::is_collider_enabled(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return m_collision_scenes_ptr->at(collider.scene_index)->is_collider_enabled(collider.id.front());
}


float_32_bit  simulation_context::collider_density_multiplier(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    return m_collision_scenes_ptr->at(collider.scene_index)->get_density_multiplier(collider.id.front());
}


simulation_context::collision_scene_index  simulation_context::collider_scene_index(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_colliders.at(collider_guid.index).scene_index;
}


object_guid  simulation_context::insert_collider_box(
        object_guid const  under_folder_guid, std::string const&  name,
        vector3 const&  half_sizes_along_axes,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        collision_scene_index const  scene_index
        ) const
{
    return const_cast<simulation_context*>(this)->insert_collider(under_folder_guid, name, scene_index,
        [this, &half_sizes_along_axes, material, collision_class, density_multiplier, scene_index]
        (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
            while (m_collision_scenes_ptr->size() < scene_index + 1UL)
                m_collision_scenes_ptr->push_back(std::make_shared<angeo::collision_scene>());
            coids.push_back(m_collision_scenes_ptr->at(scene_index)->insert_box(
                half_sizes_along_axes, W, material, collision_class, density_multiplier, is_dynamic));
        });
}


object_guid  simulation_context::insert_collider_capsule(
        object_guid const  under_folder_guid, std::string const&  name,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        collision_scene_index const  scene_index
        ) const
{
    return const_cast<simulation_context*>(this)->insert_collider(under_folder_guid, name, scene_index,
        [this, half_distance_between_end_points, thickness_from_central_line, material, collision_class, density_multiplier, scene_index]
            (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
            while (m_collision_scenes_ptr->size() < scene_index + 1UL)
                m_collision_scenes_ptr->push_back(std::make_shared<angeo::collision_scene>());
            coids.push_back(m_collision_scenes_ptr->at(scene_index)->insert_capsule(half_distance_between_end_points, thickness_from_central_line,
                                                                  W, material, collision_class, density_multiplier, is_dynamic));
        });
}


object_guid  simulation_context::insert_collider_sphere(
        object_guid const  under_folder_guid, std::string const&  name,
        float_32_bit const  radius,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        collision_scene_index const  scene_index
        ) const
{
    return const_cast<simulation_context*>(this)->insert_collider(under_folder_guid, name, scene_index,
        [this, radius, material, collision_class, density_multiplier, scene_index]
        (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
            while (m_collision_scenes_ptr->size() < scene_index + 1UL)
                m_collision_scenes_ptr->push_back(std::make_shared<angeo::collision_scene>());
            coids.push_back(m_collision_scenes_ptr->at(scene_index)->insert_sphere(
                    radius, W, material, collision_class, density_multiplier, is_dynamic
                    ));
        });
}


object_guid  simulation_context::insert_collider_triangle_mesh(
        object_guid const  under_folder_guid, std::string const&  name_prefix,
        natural_32_bit const  num_triangles,
        std::function<vector3(natural_32_bit, natural_8_bit)> const&  getter_of_end_points_in_model_space,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        collision_scene_index const  scene_index
        ) const
{
    return const_cast<simulation_context*>(this)->insert_collider(under_folder_guid, name_prefix, scene_index,
            [this, num_triangles, &getter_of_end_points_in_model_space, material, collision_class, scene_index]
            (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
                while (m_collision_scenes_ptr->size() < scene_index + 1UL)
                    m_collision_scenes_ptr->push_back(std::make_shared<angeo::collision_scene>());
                m_collision_scenes_ptr->at(scene_index)->insert_triangle_mesh(num_triangles, getter_of_end_points_in_model_space,
                                                            W, material, collision_class, is_dynamic, coids);
            });
}


object_guid  simulation_context::ray_cast_to_nearest_collider(
        vector3 const&  ray_origin,
        vector3 const&  ray_end,
        bool const  search_static,
        bool const  search_dynamic,
        collision_scene_index const  scene_index,
        float_32_bit* const  ray_parameter_to_nearest_collider,
        std::function<bool(object_guid, angeo::COLLISION_CLASS)> const&  collider_filter,
        float_32_bit const  min_parameter_value
        ) const
{
    if (m_collision_scenes_ptr->at(scene_index) == nullptr)
        return invalid_object_guid();
    angeo::collision_object_id  nearest_coid;
    if (!m_collision_scenes_ptr->at(scene_index)->ray_cast(
            ray_origin,
            ray_end,
            search_static,
            search_dynamic,
            &nearest_coid,
            ray_parameter_to_nearest_collider,
            [this,&collider_filter,scene_index](angeo::collision_object_id const  coid, angeo::COLLISION_CLASS const  cc) {
                    return collider_filter(to_collider_guid(coid, scene_index), cc);
                    },
            min_parameter_value
            ))
        return invalid_object_guid();
    return m_coids_to_guids.at({nearest_coid, scene_index});
}


void  simulation_context::request_enable_collider(object_guid const  collider_guid, bool const  state) const
{
    m_requests_enable_collider.push_back({collider_guid, state});
    m_pending_requests.push_back(REQUEST_ENABLE_COLLIDER);
}


void  simulation_context::request_enable_colliding(
        object_guid const  collider_1, object_guid const  collider_2, const bool  state
        ) const
{
    ASSUMPTION(collider_1 != collider_2);
    m_requests_enable_colliding.push_back({collider_1, collider_2, state});
    m_pending_requests.push_back(REQUEST_ENABLE_COLLIDING);
}


void  simulation_context::request_enable_colliding(object_guid const  base_folder_guid_1, std::string const&  relative_path_to_collider_1,
                                                   object_guid const  base_folder_guid_2, std::string const&  relative_path_to_collider_2,
                                                   const bool  state) const
{
    ASSUMPTION(base_folder_guid_1 != base_folder_guid_2 || relative_path_to_collider_1 != relative_path_to_collider_2);
    m_requests_enable_colliding_by_path.push_back({
            base_folder_guid_1, relative_path_to_collider_1,
            base_folder_guid_2, relative_path_to_collider_2,
            state
            });
    m_pending_requests.push_back(REQUEST_ENABLE_COLLIDING_BY_PATH);
}


void  simulation_context::request_insert_collider_box(
        object_guid const  under_folder_guid, std::string const&  name,
        vector3 const&  half_sizes_along_axes,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        collision_scene_index const  scene_index
        ) const
{
    request_data_insert_collider_box  box;
    box.under_folder_guid = under_folder_guid;
    box.name = name;
    box.half_sizes_along_axes = half_sizes_along_axes;
    box.material = material;
    box.collision_class = collision_class;
    box.density_multiplier = density_multiplier;
    box.scene_index = scene_index;
    m_requests_insert_collider_box.push_back(box);
    m_pending_requests.push_back(REQUEST_INSERT_COLLIDER_BOX);
}


void  simulation_context::request_insert_collider_capsule(
        object_guid const  under_folder_guid, std::string const&  name,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        collision_scene_index const  scene_index
        ) const
{
    request_data_insert_collider_capsule  capsule;
    capsule.under_folder_guid = under_folder_guid;
    capsule.name = name;
    capsule.half_distance_between_end_points = half_distance_between_end_points;
    capsule.thickness_from_central_line = thickness_from_central_line;
    capsule.material = material;
    capsule.collision_class = collision_class;
    capsule.density_multiplier = density_multiplier;
    capsule.scene_index = scene_index;
    m_requests_insert_collider_capsule.push_back(capsule);
    m_pending_requests.push_back(REQUEST_INSERT_COLLIDER_CAPSULE);
}


void  simulation_context::request_insert_collider_sphere(
        object_guid const  under_folder_guid, std::string const&  name,
        float_32_bit const  radius,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        collision_scene_index const  scene_index
        ) const
{
    request_data_insert_collider_sphere  sphere;
    sphere.under_folder_guid = under_folder_guid;
    sphere.name = name;
    sphere.radius = radius;
    sphere.material = material;
    sphere.collision_class = collision_class;
    sphere.density_multiplier = density_multiplier;
    sphere.scene_index = scene_index;
    m_requests_insert_collider_sphere.push_back(sphere);
    m_pending_requests.push_back(REQUEST_INSERT_COLLIDER_SPHERE);
}


void  simulation_context::request_erase_collider(object_guid const  collider_guid) const
{
    m_requests_erase_collider.push_back(collider_guid);
    m_pending_requests.push_back(REQUEST_ERASE_COLLIDER);
}


// Disabled (not const) for modules.


void  simulation_context::enable_collider(object_guid const  collider_guid, bool const  state)
{
    ASSUMPTION(
        is_valid_collider_guid(collider_guid) &&
        (!is_valid_sensor_guid(m_colliders.at(collider_guid.index).owner) ||
            is_sensor_enabled(m_colliders.at(collider_guid.index).owner) == state)
        );
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    for (angeo::collision_object_id  coid : collider.id)
        m_collision_scenes_ptr->at(collider.scene_index)->enable_collider(coid, state);
}


void  simulation_context::enable_colliding(object_guid const  collider_1, object_guid const  collider_2, const bool  state)
{
    ASSUMPTION(is_valid_collider_guid(collider_1) && is_valid_collider_guid(collider_2));
    folder_element_collider const&  collider1 = m_colliders.at(collider_1.index);
    folder_element_collider const&  collider2 = m_colliders.at(collider_2.index);
    for (angeo::collision_object_id  coid_1 : collider1.id)
        for (angeo::collision_object_id  coid_2 : collider2.id)
            if (state)
                m_collision_scenes_ptr->at(collider1.scene_index)->enable_colliding(coid_1, coid_2);
            else
                m_collision_scenes_ptr->at(collider2.scene_index)->disable_colliding(coid_1, coid_2);
}


std::vector<angeo::collision_object_id> const&  simulation_context::from_collider_guid(object_guid const  collider_guid)
{
    return m_colliders.at(collider_guid.index).id;
}


void  simulation_context::relocate_collider(object_guid const  collider_guid, matrix44 const&  world_matrix)
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    folder_element_collider const&  collider = m_colliders.at(collider_guid.index);
    for (angeo::collision_object_id  coid : collider.id)
        m_collision_scenes_ptr->at(collider.scene_index)->on_position_changed(coid, world_matrix);
}


object_guid  simulation_context::insert_collider(
        object_guid const  under_folder_guid, std::string const&  name, collision_scene_index const  scene_index,
        std::function<void(matrix44 const&, bool, std::vector<angeo::collision_object_id>&)> const&  coids_builder
        )
{
    ASSUMPTION(folder_content(under_folder_guid).content.count(name) == 0UL);

    object_guid const  frame_guid = find_closest_frame(under_folder_guid, true);
    ASSUMPTION(frame_guid != invalid_object_guid());

    object_guid  rigid_body_guid = invalid_object_guid();
    object_guid  owner_guid = invalid_object_guid();
    for_each_parent_folder(under_folder_guid, true,
        [this, &rigid_body_guid, &owner_guid](object_guid const  folder_guid, folder_content_type const&  fct) -> bool {
            auto  it = fct.content.find(to_string(OBJECT_KIND::RIGID_BODY));
            if (it != fct.content.end())
            {
                rigid_body_guid = it->second;
                if (owner_guid == invalid_object_guid())
                    owner_guid = rigid_body_guid;
                return owner_guid == rigid_body_guid;
            }
            it = fct.content.find(to_string(OBJECT_KIND::SENSOR));
            if (it != fct.content.end())
            {
                owner_guid = it->second;
                return rigid_body_guid == invalid_object_guid();
            }
            it = fct.content.find(to_string(OBJECT_KIND::AGENT));
            if (it != fct.content.end())
            {
                owner_guid = it->second;
                return rigid_body_guid == invalid_object_guid();
            }
            return true;
        });

    bool const  is_moveable_via_rigid_body = rigid_body_guid != invalid_object_guid() && is_rigid_body_moveable(rigid_body_guid);
    bool const  is_moveable_via_agent = owner_guid != invalid_object_guid() && owner_guid.kind == OBJECT_KIND::AGENT;

    std::vector<angeo::collision_object_id> coids;
    coids_builder(frame_world_matrix(frame_guid), is_moveable_via_rigid_body || is_moveable_via_agent, coids);
    ASSUMPTION(!coids.empty());

    object_guid const  collider_guid = {
            OBJECT_KIND::COLLIDER,
            m_colliders.insert({ coids, under_folder_guid.index, name, frame_guid, owner_guid, rigid_body_guid, scene_index })
            };

    for (angeo::collision_object_id  coid : coids)
        m_coids_to_guids.insert({ {coid,scene_index}, collider_guid });

    m_folders.at(under_folder_guid.index).insert_content(OBJECT_KIND::COLLIDER, collider_guid, name);

    if (owner_guid != invalid_object_guid())
        switch (owner_guid.kind)
        {
        case OBJECT_KIND::RIGID_BODY: break;
        case OBJECT_KIND::SENSOR: m_sensors.at(owner_guid.index).collider = collider_guid; break;
        case OBJECT_KIND::AGENT: m_agents.at(owner_guid.index).colliders.insert(collider_guid); break;
        default: UNREACHABLE(); break;
        }

    if (rigid_body_guid != invalid_object_guid())
        m_rigid_bodies.at(rigid_body_guid.index).colliders.push_back(collider_guid);

    if (is_moveable_via_rigid_body && scene_index == 0U)
        m_rigid_bodies_with_invalidated_shape.insert(rigid_body_guid);

    return collider_guid;
}


void  simulation_context::erase_collider(object_guid const  collider_guid)
{
    ASSUMPTION(
        is_valid_collider_guid(collider_guid) &&
        folder_content(folder_of_collider(collider_guid)).content.count(m_colliders.at(collider_guid.index).element_name) != 0UL
        );
    m_invalidated_guids.insert(collider_guid);

    auto const&  elem = m_colliders.at(collider_guid.index);
    if (elem.owner != invalid_object_guid())
        switch (elem.owner.kind)
        {
        case OBJECT_KIND::RIGID_BODY: break;
        case OBJECT_KIND::SENSOR: m_sensors.at(elem.owner.index).collider = invalid_object_guid(); break;
        case OBJECT_KIND::AGENT: m_agents.at(elem.owner.index).colliders.erase(collider_guid); break;
        default: UNREACHABLE(); break;
        }
    if (is_valid_rigid_body_guid(elem.rigid_body))
    {
        auto&  rb_colliders = m_rigid_bodies.at(elem.rigid_body.index).colliders;
        auto const  self_it = std::find(rb_colliders.begin(), rb_colliders.end(), collider_guid);
        ASSUMPTION(self_it != rb_colliders.end());
        rb_colliders.erase(self_it);
        if (is_rigid_body_moveable(elem.rigid_body)&& elem.scene_index == 0U)
            m_rigid_bodies_with_invalidated_shape.insert(elem.rigid_body);
    }
    for (angeo::collision_object_id  coid : elem.id)
        m_collision_scenes_ptr->at(elem.scene_index)->erase_object(coid);
    m_folders.at(elem.folder_index).erase_content(elem.element_name, OBJECT_KIND::COLLIDER);
    for (angeo::collision_object_id  coid : elem.id)
        m_coids_to_guids.erase({coid,elem.scene_index});
    m_colliders.erase(collider_guid.index);
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
    return { OBJECT_KIND::FOLDER, m_rigid_bodies.at(rigid_body_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_bodies.at(rigid_body_guid.index).element_name;
}


object_guid  simulation_context::to_rigid_body_guid(angeo::rigid_body_id const  rbid) const
{
    return m_rbids_to_guids.at(rbid);
}


simulation_context::rigid_body_guid_iterator  simulation_context::rigid_bodies_begin() const
{
    return rigid_body_guid_iterator(m_rigid_bodies.valid_indices().begin());
}


simulation_context::rigid_body_guid_iterator  simulation_context::rigid_bodies_end() const
{
    return rigid_body_guid_iterator(m_rigid_bodies.valid_indices().end());
}


bool  simulation_context::is_rigid_body_moveable(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_moveable_rigid_bodies.count(rigid_body_guid.index) != 0UL;
}


simulation_context::rigid_body_guid_iterator  simulation_context::moveable_rigid_bodies_begin() const
{
    return rigid_body_guid_iterator(m_moveable_rigid_bodies.begin());
}


simulation_context::rigid_body_guid_iterator  simulation_context::moveable_rigid_bodies_end() const
{
    return rigid_body_guid_iterator(m_moveable_rigid_bodies.end());
}


object_guid  simulation_context::frame_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_bodies.at(rigid_body_guid.index).frame;
}


float_32_bit  simulation_context::inverted_mass_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_inverted_mass(m_rigid_bodies.at(rigid_body_guid.index).id);
}


vector3 const&  simulation_context::mass_centre_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_position_of_mass_centre(m_rigid_bodies.at(rigid_body_guid.index).id);
}


quaternion const&  simulation_context::orientation_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_orientation(m_rigid_bodies.at(rigid_body_guid.index).id);
}


std::vector<object_guid> const&  simulation_context::colliders_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_bodies.at(rigid_body_guid.index).colliders;
}


vector3 const&  simulation_context::linear_velocity_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_linear_velocity(m_rigid_bodies.at(rigid_body_guid.index).id);
}


vector3 const&  simulation_context::angular_velocity_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_angular_velocity(m_rigid_bodies.at(rigid_body_guid.index).id);
}


vector3 const&  simulation_context::linear_acceleration_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_linear_acceleration(m_rigid_bodies.at(rigid_body_guid.index).id);
}


vector3 const&  simulation_context::angular_acceleration_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_angular_acceleration(m_rigid_bodies.at(rigid_body_guid.index).id);
}


vector3  simulation_context::initial_linear_acceleration_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_initial_linear_acceleration(m_rigid_bodies.at(rigid_body_guid.index).id);
}


vector3  simulation_context::initial_angular_acceleration_of_rigid_body(object_guid const  rigid_body_guid) const
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_body_simulator_ptr->get_initial_angular_acceleration(m_rigid_bodies.at(rigid_body_guid.index).id);
}


angeo::custom_constraint_id  simulation_context::acquire_fresh_custom_constraint_id_from_physics() const
{
    return m_rigid_body_simulator_ptr->gen_fresh_custom_constraint_id();
}


void  simulation_context::release_acquired_custom_constraint_id_back_to_physics(angeo::custom_constraint_id const  ccid) const
{
    m_rigid_body_simulator_ptr->release_generated_custom_constraint_id(ccid);
}


vector3  simulation_context::compute_velocity_of_point_of_rigid_body(
        object_guid const  rigid_body_guid, vector3 const&  point_in_world_space
        ) const
{
    return angeo::compute_velocity_of_point_of_rigid_body(
                mass_centre_of_rigid_body(rigid_body_guid),
                linear_velocity_of_rigid_body(rigid_body_guid),
                angular_velocity_of_rigid_body(rigid_body_guid),
                point_in_world_space    
                );
}


object_guid  simulation_context::insert_rigid_body(
        object_guid const  under_folder_guid,
        bool const  is_moveable,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  linear_acceleration,
        vector3 const&  angular_acceleration,
        float_32_bit const  inverted_mass,
        matrix33 const&  inverted_inertia_tensor
        ) const
{
    ASSUMPTION((
        folder_content(under_folder_guid).content.count(to_string(OBJECT_KIND::RIGID_BODY)) == 0UL &&
        [this, under_folder_guid]() -> bool {
            object_guid  rigid_body_guid = invalid_object_guid();
            for_each_parent_folder(under_folder_guid, true,
                [this, &rigid_body_guid](object_guid const  folder_guid, folder_content_type const&  fct) -> bool {
                    auto  it = fct.content.find(to_string(OBJECT_KIND::RIGID_BODY));
                    if (it == fct.content.end())
                        return true;
                    rigid_body_guid = it->second;
                    return false;
                });
            return rigid_body_guid == invalid_object_guid();
            }()
        ));

    object_guid const  frame_guid = find_closest_frame(under_folder_guid, true);
    ASSUMPTION(frame_guid != invalid_object_guid());
    angeo::coordinate_system const  frame = frame_coord_system_in_world_space(frame_guid);

    angeo::rigid_body_id const  rbid = m_rigid_body_simulator_ptr->insert_rigid_body(
            frame.origin(),
            frame.orientation(),
            inverted_mass,
            inverted_inertia_tensor,
            linear_velocity,
            angular_velocity,
            linear_acceleration,
            angular_acceleration
            );

    simulation_context* const  self = const_cast<simulation_context*>(this);

    object_guid const  rigid_body_guid = {
            OBJECT_KIND::RIGID_BODY,
            self->m_rigid_bodies.insert({ rbid, under_folder_guid.index, frame_guid })
            };

    self->m_rbids_to_guids.insert({ rbid, rigid_body_guid });

    self->m_folders.at(under_folder_guid.index).insert_content(OBJECT_KIND::RIGID_BODY, rigid_body_guid);

    for_each_object_of_kind_under_folder(under_folder_guid, true, OBJECT_KIND::COLLIDER,
        [self, rigid_body_guid](object_guid const  collider_guid) -> bool {
            self->m_rigid_bodies.at(rigid_body_guid.index).colliders.push_back(collider_guid);
            self->m_colliders.at(collider_guid.index).rigid_body = rigid_body_guid;
            return true;
        });

    if (is_moveable)
    {
        self->m_moveable_rigid_bodies.insert(rigid_body_guid.index);
        self->m_rigid_bodies_with_invalidated_shape.insert(rigid_body_guid);
    }

    return rigid_body_guid;
}


void  simulation_context::request_insert_rigid_body(
        object_guid const  under_folder_guid,
        bool const  is_moveable,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  linear_acceleration,
        vector3 const&  angular_acceleration,
        float_32_bit const  inverted_mass,
        matrix33 const&  inverted_inertia_tensor
        ) const
{
    request_data_insert_rigid_body  rigid_body;
    rigid_body.under_folder_guid = under_folder_guid;
    rigid_body.is_moveable = is_moveable;
    rigid_body.linear_velocity = linear_velocity;
    rigid_body.angular_velocity = angular_velocity;
    rigid_body.linear_acceleration = linear_acceleration;
    rigid_body.angular_acceleration = angular_acceleration;
    rigid_body.inverted_mass = inverted_mass;
    rigid_body.inverted_inertia_tensor = inverted_inertia_tensor;
    m_requests_insert_rigid_body.push_back(rigid_body);
    m_pending_requests.push_back(REQUEST_INSERT_RIGID_BODY);
}


void  simulation_context::request_erase_rigid_body(object_guid const  rigid_body_guid) const
{
    m_requests_erase_rigid_body.push_back(rigid_body_guid);
    m_pending_requests.push_back(REQUEST_ERASE_RIGID_BODY);
}


void  simulation_context::request_set_rigid_body_linear_velocity(object_guid const  rigid_body_guid, vector3 const&  velocity) const
{
    m_requests_set_linear_velocity.push_back({ rigid_body_guid, velocity });
    m_pending_requests.push_back(REQUEST_SET_LINEAR_VELOCITY);
}


void  simulation_context::request_set_rigid_body_linear_velocity(
        object_guid const  base_folder_guid, std::string const&  relative_path_to_rigid_body, vector3 const&  velocity
        ) const
{
    m_requests_set_linear_velocity_by_path.push_back({ base_folder_guid, relative_path_to_rigid_body, velocity });
    m_pending_requests.push_back(REQUEST_SET_LINEAR_VELOCITY_BY_PATH);
}


void  simulation_context::request_set_rigid_body_angular_velocity(object_guid const  rigid_body_guid,  vector3 const&  velocity) const
{
    m_requests_set_angular_velocity.push_back({ rigid_body_guid, velocity });
    m_pending_requests.push_back(REQUEST_SET_ANGULAR_VELOCITY);
}


void  simulation_context::request_set_rigid_body_angular_velocity(
        object_guid const  base_folder_guid, std::string const&  relative_path_to_rigid_body,  vector3 const&  velocity
        ) const
{
    m_requests_set_angular_velocity_by_path.push_back({ base_folder_guid, relative_path_to_rigid_body, velocity });
    m_pending_requests.push_back(REQUEST_SET_ANGULAR_VELOCITY_BY_PATH);
}


void  simulation_context::request_mul_rigid_body_linear_velocity(
        object_guid const  rigid_body_guid, vector3 const&  velocity_scale
        ) const
{
    m_requests_mul_linear_velocity.push_back({ rigid_body_guid, velocity_scale });
    m_pending_requests.push_back(REQUEST_MUL_LINEAR_VELOCITY);
}


void  simulation_context::request_mul_rigid_body_linear_velocity(
        object_guid const  base_folder_guid, std::string const&  relative_path_to_rigid_body, vector3 const&  velocity_scale
        ) const
{
    m_requests_mul_linear_velocity_by_path.push_back({ base_folder_guid, relative_path_to_rigid_body, velocity_scale });
    m_pending_requests.push_back(REQUEST_MUL_LINEAR_VELOCITY_BY_PATH);
}


void  simulation_context::request_mul_rigid_body_angular_velocity(
        object_guid const  rigid_body_guid,  vector3 const&  velocity_scale
        ) const
{
    m_requests_mul_angular_velocity.push_back({ rigid_body_guid, velocity_scale });
    m_pending_requests.push_back(REQUEST_MUL_ANGULAR_VELOCITY);
}


void  simulation_context::request_mul_rigid_body_angular_velocity(
        object_guid const  base_folder_guid, std::string const&  relative_path_to_rigid_body,  vector3 const&  velocity_scale
        ) const
{
    m_requests_mul_angular_velocity_by_path.push_back({ base_folder_guid, relative_path_to_rigid_body, velocity_scale });
    m_pending_requests.push_back(REQUEST_MUL_ANGULAR_VELOCITY_BY_PATH);
}


void  simulation_context::request_set_rigid_body_linear_acceleration_from_source(
        object_guid const  rigid_body_guid, rigid_body_acceleration_source_id const  source_id, vector3 const&  acceleration) const
{
    m_requests_set_linear_acceleration_from_source.push_back({ rigid_body_guid, source_id, acceleration });
    m_pending_requests.push_back(REQUEST_SET_LINEAR_ACCEL);
}


void  simulation_context::request_set_rigid_body_angular_acceleration_from_source(
        object_guid const  rigid_body_guid, rigid_body_acceleration_source_id const  source_id, vector3 const&  acceleration) const
{
    m_requests_set_angular_acceleration_from_source.push_back({ rigid_body_guid, source_id, acceleration });
    m_pending_requests.push_back(REQUEST_SET_ANGULAR_ACCEL);
}


void  simulation_context::request_remove_rigid_body_linear_acceleration_from_source(
        object_guid const  rigid_body_guid, rigid_body_acceleration_source_id const  source_id) const
{
    m_requests_del_linear_acceleration_from_source.push_back({ rigid_body_guid, source_id });
    m_pending_requests.push_back(REQUEST_DEL_LINEAR_ACCEL);
}


void  simulation_context::request_remove_rigid_body_angular_acceleration_from_source(
        object_guid const  rigid_body_guid, rigid_body_acceleration_source_id const  source_id) const
{
    m_requests_del_angular_acceleration_from_source.push_back({ rigid_body_guid, source_id });
    m_pending_requests.push_back(REQUEST_DEL_ANGULAR_ACCEL);
}


void  simulation_context::request_early_insertion_of_custom_constraint_to_physics(
        angeo::custom_constraint_id const  ccid,
        object_guid const  rigid_body_0, vector3 const&  linear_component_0, vector3 const&  angular_component_0,
        object_guid const  rigid_body_1, vector3 const&  linear_component_1, vector3 const&  angular_component_1,
        float_32_bit const  bias,
        float_32_bit const  variable_lower_bound, float_32_bit const  variable_upper_bound,
        float_32_bit const  initial_value_for_cache_miss
        ) const
{
    m_requests_early_insert_custom_constraint.push_back({
        ccid,
        rigid_body_0, linear_component_0, angular_component_0,
        rigid_body_1, linear_component_1, angular_component_1,
        bias,
        variable_lower_bound, variable_upper_bound,
        initial_value_for_cache_miss
        });
    m_pending_requests_early.push_back(REQUEST_INSERT_CUSTOM_CONSTRAINT);
}


void  simulation_context::request_early_insertion_of_instant_constraint_to_physics(
        object_guid const  rigid_body_0, vector3 const&  linear_component_0, vector3 const&  angular_component_0,
        object_guid const  rigid_body_1, vector3 const&  linear_component_1, vector3 const&  angular_component_1,
        float_32_bit const  bias,
        float_32_bit const  variable_lower_bound, float_32_bit const  variable_upper_bound,
        float_32_bit const  initial_value
        ) const
{
    m_requests_early_insert_instant_constraint.push_back({
        rigid_body_0, linear_component_0, angular_component_0,
        rigid_body_1, linear_component_1, angular_component_1,
        bias,
        variable_lower_bound, variable_upper_bound,
        initial_value
        });
    m_pending_requests_early.push_back(REQUEST_INSERT_INSTANT_CONSTRAINT);
}


// Disabled (not const) for modules.


angeo::rigid_body_id  simulation_context::from_rigid_body_guid(object_guid const  rigid_body_guid)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_bodies.at(rigid_body_guid.index).id;
}


void  simulation_context::erase_rigid_body(object_guid const  rigid_body_guid)
{
    ASSUMPTION(
        is_valid_rigid_body_guid(rigid_body_guid) &&
        folder_content(folder_of_rigid_body(rigid_body_guid)).content.count(m_rigid_bodies.at(rigid_body_guid.index).element_name) != 0UL
        );
    m_invalidated_guids.insert(rigid_body_guid);

    auto const&  elem = m_rigid_bodies.at(rigid_body_guid.index);
    for (object_guid  collider_guid : elem.colliders)
        m_colliders.at(collider_guid.index).rigid_body = invalid_object_guid();
    m_rigid_body_simulator_ptr->erase_rigid_body(elem.id);
    m_folders.at(elem.folder_index).erase_content(OBJECT_KIND::RIGID_BODY);
    m_rbids_to_guids.erase(elem.id);
    m_moveable_rigid_bodies.erase(rigid_body_guid.index);
    m_rigid_bodies.erase(rigid_body_guid.index);
}


void  simulation_context::set_rigid_body_mass_centre(object_guid const  rigid_body_guid, vector3 const&  position)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_position_of_mass_centre(m_rigid_bodies.at(rigid_body_guid.index).id, position);
}


void  simulation_context::set_rigid_body_orientation(object_guid const  rigid_body_guid, quaternion const&  orientation)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_orientation(m_rigid_bodies.at(rigid_body_guid.index).id, orientation);
}


void  simulation_context::set_rigid_body_inverted_mass(object_guid const  rigid_body_guid, float_32_bit const  inverted_mass)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_inverted_mass(m_rigid_bodies.at(rigid_body_guid.index).id, inverted_mass);
}


void  simulation_context::set_rigid_body_inverted_inertia_tensor(object_guid const  rigid_body_guid,
                                                                 matrix33 const&  inverted_inertia_tensor)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_inverted_inertia_tensor_in_local_space(m_rigid_bodies.at(rigid_body_guid.index).id,
                                                                           inverted_inertia_tensor);
}


void  simulation_context::set_rigid_body_linear_velocity(object_guid const  rigid_body_guid, vector3 const&  velocity)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_linear_velocity(m_rigid_bodies.at(rigid_body_guid.index).id, velocity);
}


void  simulation_context::set_rigid_body_angular_velocity(object_guid const  rigid_body_guid, vector3 const&  velocity)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_angular_velocity(m_rigid_bodies.at(rigid_body_guid.index).id, velocity);
}


void  simulation_context::set_rigid_body_linear_acceleration_from_source(
        object_guid const  rigid_body_guid, rigid_body_acceleration_source_id const  source_id, vector3 const&  accel)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_linear_acceleration_from_source(m_rigid_bodies.at(rigid_body_guid.index).id, source_id, accel);
}


void  simulation_context::set_rigid_body_angular_acceleration_from_source(
        object_guid const  rigid_body_guid, rigid_body_acceleration_source_id const  source_id, vector3 const&  accel)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_angular_acceleration_from_source(m_rigid_bodies.at(rigid_body_guid.index).id, source_id, accel);
}


void  simulation_context::remove_rigid_body_linear_acceleration_from_source(
        object_guid const  rigid_body_guid, rigid_body_acceleration_source_id const  source_id)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->remove_linear_acceleration_from_source(m_rigid_bodies.at(rigid_body_guid.index).id, source_id);
}


void  simulation_context::remove_rigid_body_angular_acceleration_from_source(
        object_guid const  rigid_body_guid, rigid_body_acceleration_source_id const  source_id)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->remove_angular_acceleration_from_source(m_rigid_bodies.at(rigid_body_guid.index).id, source_id);
}


void  simulation_context::insert_custom_constraint_to_physics(
        angeo::custom_constraint_id const  ccid,
        object_guid const  rigid_body_0, vector3 const&  linear_component_0, vector3 const&  angular_component_0,
        object_guid const  rigid_body_1, vector3 const&  linear_component_1, vector3 const&  angular_component_1,
        float_32_bit const  bias,
        float_32_bit const  variable_lower_bound, float_32_bit const  variable_upper_bound,
        float_32_bit const  initial_value_for_cache_miss
        )
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_0) && is_valid_rigid_body_guid(rigid_body_1));
    m_rigid_body_simulator_ptr->insert_custom_constraint(
            ccid,
            m_rigid_bodies.at(rigid_body_0.index).id, linear_component_0, angular_component_0,
            m_rigid_bodies.at(rigid_body_1.index).id, linear_component_1, angular_component_1,
            bias,
            [variable_lower_bound](std::vector<float_32_bit> const&) { return variable_lower_bound; },
            [variable_upper_bound](std::vector<float_32_bit> const&) { return variable_upper_bound; },
            initial_value_for_cache_miss
            );
}


void  simulation_context::insert_instant_constraint_to_physics(
        object_guid const  rigid_body_0, vector3 const&  linear_component_0, vector3 const&  angular_component_0,
        object_guid const  rigid_body_1, vector3 const&  linear_component_1, vector3 const&  angular_component_1,
        float_32_bit const  bias,
        float_32_bit const  variable_lower_bound, float_32_bit const  variable_upper_bound,
        float_32_bit const  initial_value
        )
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_0) && is_valid_rigid_body_guid(rigid_body_1));
    m_rigid_body_simulator_ptr->get_constraint_system().insert_constraint(
            m_rigid_bodies.at(rigid_body_0.index).id, linear_component_0, angular_component_0,
            m_rigid_bodies.at(rigid_body_1.index).id, linear_component_1, angular_component_1,
            bias,
            [variable_lower_bound](std::vector<float_32_bit> const&) { return variable_lower_bound; },
            [variable_upper_bound](std::vector<float_32_bit> const&) { return variable_upper_bound; },
            initial_value
            );
}


/////////////////////////////////////////////////////////////////////////////////////
// TIMERS API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_timer_guid(object_guid const  timer_guid) const
{
    return timer_guid.kind == OBJECT_KIND::TIMER && m_timers.valid(timer_guid.index);
}


bool  simulation_context::is_timer_enabled(object_guid const  timer_guid) const
{
    ASSUMPTION(is_valid_timer_guid(timer_guid));
    return m_device_simulator_ptr->is_timer_enabled(m_timers.at(timer_guid.index).id);
}


object_guid  simulation_context::folder_of_timer(object_guid const  timer_guid) const
{
    ASSUMPTION(is_valid_timer_guid(timer_guid));
    return { OBJECT_KIND::FOLDER, m_timers.at(timer_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_timer(object_guid const  timer_guid) const
{
    ASSUMPTION(is_valid_timer_guid(timer_guid));
    return m_timers.at(timer_guid.index).element_name;
}


object_guid  simulation_context::to_timer_guid(com::device_simulator::timer_id const  tid) const
{
    return m_tmids_to_guids.at(tid);
}


simulation_context::timer_guid_iterator  simulation_context::timers_begin() const
{
    return timer_guid_iterator(m_timers.valid_indices().begin());
}


simulation_context::timer_guid_iterator  simulation_context::timers_end() const
{
    return timer_guid_iterator(m_timers.valid_indices().end());
}


void  simulation_context::request_erase_timer(object_guid const  timer_guid) const
{
    m_requests_erase_timer.push_back(timer_guid);
    m_pending_requests.push_back(REQUEST_ERASE_TIMER);
}


// Disabled (not const) for modules.


object_guid  simulation_context::insert_timer(
        object_guid const  under_folder_guid, std::string const&  name, float_32_bit const  period_in_seconds_,
        natural_8_bit const target_enable_level_, natural_8_bit const  current_enable_level_)
{
    ASSUMPTION(folder_content(under_folder_guid).content.count(name) == 0UL);
    com::device_simulator::timer_id const  tid = m_device_simulator_ptr->insert_timer(
            period_in_seconds_, target_enable_level_, current_enable_level_
            );
    object_guid const  timer_guid = {
            OBJECT_KIND::TIMER,
            m_timers.insert({ tid, under_folder_guid.index, name })
            };
    m_tmids_to_guids.insert({ tid, timer_guid });
    m_folders.at(under_folder_guid.index).insert_content(OBJECT_KIND::TIMER, timer_guid, name);
    return timer_guid;
}


void  simulation_context::erase_timer(object_guid const  timer_guid)
{
    ASSUMPTION(is_valid_timer_guid(timer_guid));
    m_invalidated_guids.insert(timer_guid);
    auto const&  elem = m_timers.at(timer_guid.index);
    {
        auto const&  infos = m_device_simulator_ptr->request_infos_of_timer(elem.id);
        while (!infos.empty())
            m_device_simulator_ptr->erase_request_info(infos.back());
    }
    m_device_simulator_ptr->erase_timer(elem.id);
    m_folders.at(elem.folder_index).erase_content(elem.element_name, OBJECT_KIND::TIMER);
    m_tmids_to_guids.erase(elem.id);
    m_timers.erase(timer_guid.index);
}


/////////////////////////////////////////////////////////////////////////////////////
// SENSORS API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_sensor_guid(object_guid const  sensor_guid) const
{
    return sensor_guid.kind == OBJECT_KIND::SENSOR && m_sensors.valid(sensor_guid.index);
}


bool  simulation_context::is_sensor_enabled(object_guid const  sensor_guid) const
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    return m_device_simulator_ptr->is_sensor_enabled(m_sensors.at(sensor_guid.index).id);
}


object_guid  simulation_context::folder_of_sensor(object_guid const  sensor_guid) const
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    return { OBJECT_KIND::FOLDER, m_sensors.at(sensor_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_sensor(object_guid const  sensor_guid) const
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    return m_sensors.at(sensor_guid.index).element_name;
}


object_guid  simulation_context::collider_of_sensor(object_guid const  sensor_guid) const
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    return m_sensors.at(sensor_guid.index).collider;
}


object_guid  simulation_context::to_sensor_guid(com::device_simulator::sensor_id const  sid) const
{
    return m_seids_to_guids.at(sid);
}


simulation_context::sensor_guid_iterator  simulation_context::sensors_begin() const
{
    return sensor_guid_iterator(m_sensors.valid_indices().begin());
}


simulation_context::sensor_guid_iterator  simulation_context::sensors_end() const
{
    return sensor_guid_iterator(m_sensors.valid_indices().end());
}


void  simulation_context::request_erase_sensor(object_guid const  sensor_guid) const
{
    m_requests_erase_sensor.push_back(sensor_guid);
    m_pending_requests.push_back(REQUEST_ERASE_SENSOR);
}


// Disabled (not const) for modules.


object_guid  simulation_context::insert_sensor(
        object_guid const  under_folder_guid, std::string const&  name, object_guid const  collider_,
        std::unordered_set<object_guid> const&  triggers_, natural_8_bit const target_enable_level_,
        natural_8_bit const  current_enable_level_)
{
    ASSUMPTION((
        folder_content(under_folder_guid).content.count(name) == 0UL &&
        is_valid_collider_guid(collider_) &&
        [this, &triggers_]() -> bool {
            for (object_guid  collider_guid : triggers_)
                if (!is_valid_collider_guid(collider_guid))
                    return false;
            return true;
            }()
        ));
    com::device_simulator::sensor_id const  sid = m_device_simulator_ptr->insert_sensor(
            collider_, triggers_, target_enable_level_, current_enable_level_
            );
    object_guid const  sensor_guid = {
            OBJECT_KIND::SENSOR,
            m_sensors.insert({ sid, under_folder_guid.index, name, collider_ })
            };
    m_colliders.at(collider_.index).owner = sensor_guid;
    m_seids_to_guids.insert({ sid, sensor_guid });
    m_folders.at(under_folder_guid.index).insert_content(OBJECT_KIND::SENSOR, sensor_guid, name);
    return sensor_guid;
}


void  simulation_context::erase_sensor(object_guid const  sensor_guid)
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    m_invalidated_guids.insert(sensor_guid);
    auto const&  elem = m_sensors.at(sensor_guid.index);
    if (is_valid_collider_guid(elem.collider))
        m_colliders.at(elem.collider.index).owner = invalid_object_guid();
    auto const  remove_request_infos = [this](std::vector<com::device_simulator::request_info_id> const&  request_infos) {
        while (!request_infos.empty())
            m_device_simulator_ptr->erase_request_info(request_infos.back());
    };
    remove_request_infos(m_device_simulator_ptr->request_infos_touching_of_sensor(elem.id));
    remove_request_infos(m_device_simulator_ptr->request_infos_touch_begin_of_sensor(elem.id));
    remove_request_infos(m_device_simulator_ptr->request_infos_touch_end_of_sensor(elem.id));
    m_device_simulator_ptr->erase_sensor(elem.id);
    m_folders.at(elem.folder_index).erase_content(elem.element_name, OBJECT_KIND::SENSOR);
    m_seids_to_guids.erase(elem.id);
    m_sensors.erase(sensor_guid.index);
}


void  simulation_context::insert_trigger_collider_to_sensor(object_guid const  sensor_guid, object_guid const  collider_guid)
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid) && is_valid_collider_guid(collider_guid));
    m_device_simulator_ptr->insert_trigger_collider_to_sensor(m_sensors.at(sensor_guid.index).id, collider_guid);
}


void  simulation_context::erase_trigger_collider_to_sensor(object_guid const  sensor_guid, object_guid const  collider_guid)
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid) && is_valid_collider_guid(collider_guid));
    m_device_simulator_ptr->erase_trigger_collider_from_sensor(m_sensors.at(sensor_guid.index).id, collider_guid);
}


/////////////////////////////////////////////////////////////////////////////////////
// TIMER & SENSOR REQUEST INFOS API
/////////////////////////////////////////////////////////////////////////////////////


// Disabled (not const) for modules.


void  simulation_context::register_request_info(device_request_info_id const&  drid, com::device_simulator::request_info_id  rid)
{
    ASSUMPTION(
        (is_valid_timer_guid(drid.owner_guid) || is_valid_sensor_guid(drid.owner_guid)) &&
        m_device_simulator_ptr->is_valid_request_info_id(rid)
        );
    switch (drid.owner_guid.kind)
    {
    case OBJECT_KIND::TIMER:
        ASSUMPTION(drid.event_type == DEVICE_EVENT_TYPE::TIME_OUT);
        m_device_simulator_ptr->register_request_info_to_timer(rid, m_timers.at(drid.owner_guid.index).id);
        break;
    case OBJECT_KIND::SENSOR:
        switch(drid.event_type)
        {
        case DEVICE_EVENT_TYPE::TOUCHING:
            m_device_simulator_ptr->register_request_info_to_sensor(
                    rid, m_sensors.at(drid.owner_guid.index).id, com::device_simulator::SENSOR_EVENT_TYPE::TOUCHING
                    );
            break;
        case DEVICE_EVENT_TYPE::TOUCH_BEGIN:
            m_device_simulator_ptr->register_request_info_to_sensor(
                    rid, m_sensors.at(drid.owner_guid.index).id, com::device_simulator::SENSOR_EVENT_TYPE::TOUCH_BEGIN
                    );
            break;
        case DEVICE_EVENT_TYPE::TOUCH_END:
            m_device_simulator_ptr->register_request_info_to_sensor(
                    rid, m_sensors.at(drid.owner_guid.index).id, com::device_simulator::SENSOR_EVENT_TYPE::TOUCH_END
                    );
            break;
        default: UNREACHABLE(); break;
        }
        break;
    default: UNREACHABLE(); break;
    }
}


void  simulation_context::insert_request_info_increment_enable_level_of_timer(
        device_request_info_id const&  drid, object_guid const  timer_guid)
{
    ASSUMPTION(is_valid_timer_guid(timer_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_increment_enable_level_of_timer(
            m_timers.at(timer_guid.index).id
            ));
}


void  simulation_context::insert_request_info_decrement_enable_level_of_timer(
        device_request_info_id const&  drid, object_guid const  timer_guid)
{
    ASSUMPTION(is_valid_timer_guid(timer_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_decrement_enable_level_of_timer(
            m_timers.at(timer_guid.index).id
            ));
}


void  simulation_context::insert_request_info_reset_timer(device_request_info_id const&  drid, object_guid const  timer_guid)
{
    ASSUMPTION(is_valid_timer_guid(timer_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_reset_timer(
            m_timers.at(timer_guid.index).id
            ));
}


void  simulation_context::insert_request_info_increment_enable_level_of_sensor(
        device_request_info_id const&  drid, object_guid const  sensor_guid)
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_increment_enable_level_of_sensor(
            m_sensors.at(sensor_guid.index).id
            ));
}


void  simulation_context::insert_request_info_decrement_enable_level_of_sensor(
        device_request_info_id const&  drid, object_guid const  sensor_guid)
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_decrement_enable_level_of_sensor(
            m_sensors.at(sensor_guid.index).id
            ));
}


void  simulation_context::insert_request_info_import_scene(device_request_info_id const&  drid, import_scene_props const&  props)
{
    ASSUMPTION(is_valid_folder_guid(props.folder_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_import_scene(props));
}


void  simulation_context::insert_request_info_erase_folder(device_request_info_id const&  drid, object_guid const  folder_guid)
{
    ASSUMPTION(is_valid_folder_guid(folder_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_erase_folder(folder_guid));
}


void  simulation_context::insert_request_info_rigid_body_set_linear_velocity(
        device_request_info_id const&  drid, object_guid const  rb_guid, vector3 const&  linear_velocity)
{
    ASSUMPTION(is_valid_rigid_body_guid(rb_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_rigid_body_set_linear_velocity(
            rb_guid, linear_velocity
            ));
}


void  simulation_context::insert_request_info_rigid_body_set_angular_velocity(
        device_request_info_id const&  drid, object_guid const  rb_guid, vector3 const&  angular_velocity)
{
    ASSUMPTION(is_valid_rigid_body_guid(rb_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_rigid_body_set_angular_velocity(
            rb_guid, angular_velocity
            ));
}


void  simulation_context::insert_request_info_rigid_body_mul_linear_velocity(
        device_request_info_id const&  drid, object_guid const  rb_guid, vector3 const&  linear_velocity_scale)
{
    ASSUMPTION(is_valid_rigid_body_guid(rb_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_rigid_body_mul_linear_velocity(
            rb_guid, linear_velocity_scale
            ));
}


void  simulation_context::insert_request_info_rigid_body_mul_angular_velocity(
        device_request_info_id const&  drid, object_guid const  rb_guid, vector3 const&  angular_velocity_scale)
{
    ASSUMPTION(is_valid_rigid_body_guid(rb_guid));
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_rigid_body_mul_angular_velocity(
            rb_guid, angular_velocity_scale
            ));
}


void  simulation_context::insert_request_info_update_radial_force_field(
        device_request_info_id const&  drid, float_32_bit const  multiplier, float_32_bit const  exponent,
        float_32_bit const  min_radius, device_simulator::force_field_flags const  flags
        )
{
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_update_radial_force_field(
            multiplier, exponent, min_radius, flags
            ));
}


void  simulation_context::insert_request_info_update_linear_force_field(
        device_request_info_id const&  drid, vector3 const&  acceleration, device_simulator::force_field_flags const  flags
        )
{
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_update_linear_force_field(acceleration, flags));
}


void  simulation_context::insert_request_info_apply_force_field_resistance(
        device_request_info_id const&  drid, float_32_bit const  resistance_coef
        )
{
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_apply_force_field_resistance(resistance_coef));
}


void  simulation_context::insert_request_info_leave_force_field(device_request_info_id const&  drid)
{
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_leave_force_field());
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
    return { OBJECT_KIND::FOLDER, m_agents.at(agent_guid.index).folder_index };
}


std::string const&  simulation_context::name_of_agent(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    return m_agents.at(agent_guid.index).element_name;
}


std::unordered_set<object_guid> const&  simulation_context::colliders_of_agent(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    return m_agents.at(agent_guid.index).colliders;
}


object_guid  simulation_context::to_agent_guid(ai::agent_id const  agid) const
{
    return m_agids_to_guids.at(agid);
}


ai::agent_state_variables const&  simulation_context::agent_state_variables(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    simulation_context* const  self = const_cast<simulation_context*>(this);
    return m_ai_simulator_ptr->get_agent(self->from_agent_guid(agent_guid)).get_state_variables();
}


ai::agent_system_variables const&  simulation_context::agent_system_variables(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    simulation_context* const  self = const_cast<simulation_context*>(this);
    return m_ai_simulator_ptr->get_agent(self->from_agent_guid(agent_guid)).get_system_variables();
}


ai::agent_system_state const&  simulation_context::agent_system_state(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    simulation_context* const  self = const_cast<simulation_context*>(this);
    return m_ai_simulator_ptr->get_agent(self->from_agent_guid(agent_guid)).get_system_state();
}


ai::skeletal_motion_templates  simulation_context::agent_motion_templates(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    simulation_context* const  self = const_cast<simulation_context*>(this);
    return m_ai_simulator_ptr->get_agent(self->from_agent_guid(agent_guid)).get_motion_templates();
}


ai::scene_binding_ptr  simulation_context::agent_scene_binding(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    simulation_context* const  self = const_cast<simulation_context*>(this);
    return m_ai_simulator_ptr->get_agent(self->from_agent_guid(agent_guid)).get_binding();
}


ai::action_controller const&  simulation_context::agent_action_controller(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    simulation_context* const  self = const_cast<simulation_context*>(this);
    return m_ai_simulator_ptr->get_agent(self->from_agent_guid(agent_guid)).get_action_controller();
}


ai::sight_controller const&  simulation_context::agent_sight_controller(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    simulation_context* const  self = const_cast<simulation_context*>(this);
    return m_ai_simulator_ptr->get_agent(self->from_agent_guid(agent_guid)).get_sight_controller();
}


ai::cortex const&  simulation_context::agent_cortex(object_guid const  agent_guid) const
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    simulation_context* const  self = const_cast<simulation_context*>(this);
    return m_ai_simulator_ptr->get_agent(self->from_agent_guid(agent_guid)).get_cortex();
}


simulation_context::agent_guid_iterator  simulation_context::agents_begin() const
{
    return agent_guid_iterator(m_agents.valid_indices().begin());
}


simulation_context::agent_guid_iterator  simulation_context::agents_end() const
{
    return agent_guid_iterator(m_agents.valid_indices().end());
}


void  simulation_context::request_late_insert_agent(
        object_guid const  under_folder_guid,
        ai::agent_config const  config,
        std::vector<std::pair<std::string, gfx::batch> > const&  skeleton_attached_batches
        ) const
{
    ASSUMPTION(is_valid_folder_guid(under_folder_guid) && !skeleton_attached_batches.empty());

    m_requests_late_insert_agent.push_back({ 
            under_folder_guid,
            config,
            skeleton_attached_batches,
            ai::skeletal_motion_templates()
            });
}


void  simulation_context::request_erase_agent(object_guid const  agent_guid) const
{
    m_requests_erase_agent.push_back(agent_guid);
    m_pending_requests.push_back(REQUEST_ERASE_AGENT);
}


// Disabled (not const) for modules.


ai::agent_id  simulation_context::from_agent_guid(object_guid const  agent_guid)
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    return m_agents.at(agent_guid.index).id;
}


object_guid  simulation_context::insert_agent(
        object_guid const  under_folder_guid,
        ai::agent_config const  config,
        ai::skeletal_motion_templates const  motion_templates,
        std::vector<std::pair<std::string, gfx::batch> > const&  skeleton_attached_batches
        )
{
    ASSUMPTION(!m_self_ptr.expired());
    ASSUMPTION(
        ([this, under_folder_guid]() -> bool {
            folder_content_type const&  fct = folder_content(under_folder_guid);
            if (!fct.content.empty())
                return false;
            if (fct.child_folders.size() != 1UL || fct.child_folders.begin()->first != "motion_object")
                return false;
            folder_content_type const&  mo_fct = folder_content(fct.child_folders.begin()->second);
            if (!mo_fct.child_folders.empty())
                return false;
            if (mo_fct.content.size() != 1UL || mo_fct.content.begin()->first != to_string(OBJECT_KIND::FRAME))
                return false;
            return true;
            }()) &&
        motion_templates.loaded_successfully() &&
        [&skeleton_attached_batches]() -> bool {
            for (auto const&  name_and_batch : skeleton_attached_batches)
            {
                if (!name_and_batch.second.loaded_successfully())
                    return false;
                if (!name_and_batch.second.is_attached_to_skeleton())
                    return false;
            }
            return true;
            }()
        );

    ai::scene_binding_ptr const  binding = ai::scene_binding::create(
            m_self_ptr.lock(),
            under_folder_guid,
            motion_templates
            );

    ai::agent_id const  id = m_ai_simulator_ptr->insert_agent(config, motion_templates, binding);

    for (auto const&  name_and_batch : skeleton_attached_batches)
    {
        std::vector<matrix44>  to_bone_matrices;
        {
            matrix44 alignment_matrix;
            angeo::from_base_matrix(name_and_batch.second.get_skeleton_alignment().get_skeleton_alignment(), alignment_matrix);

            to_bone_matrices.resize(motion_templates.pose_frames().size());
            for (natural_32_bit  bone = 0U; bone != motion_templates.pose_frames().size(); ++bone)
            {
                angeo::to_base_matrix(motion_templates.pose_frames().at(bone), to_bone_matrices.at(bone));
                if (motion_templates.parents().at(bone) >= 0)
                    to_bone_matrices.at(bone) = to_bone_matrices.at(bone) * to_bone_matrices.at(motion_templates.parents().at(bone));
            }
            for (natural_32_bit  bone = 0U; bone != motion_templates.pose_frames().size(); ++bone)
                to_bone_matrices.at(bone) = to_bone_matrices.at(bone) * alignment_matrix;
        }

        insert_batch(
                under_folder_guid,
                name_and_batch.first,
                name_and_batch.second,
                binding->frame_guids_of_bones,
                to_bone_matrices
                );
    }
    object_guid const  agent_guid = {
            OBJECT_KIND::AGENT,
            m_agents.insert({ id, under_folder_guid.index, to_string(OBJECT_KIND::AGENT) })
            };

    m_agids_to_guids.insert({ id, agent_guid });

    m_folders.at(under_folder_guid.index).insert_content(OBJECT_KIND::AGENT, agent_guid);

    return agent_guid;
}


void  simulation_context::erase_agent(object_guid const  agent_guid)
{
    ASSUMPTION(is_valid_agent_guid(agent_guid));
    m_invalidated_guids.insert(agent_guid);

    auto const&  elem = m_agents.at(agent_guid.index);

    m_ai_simulator_ptr->erase_agent(elem.id);

    {
        folder_content_type const&  fct = folder_content(folder_of_agent(agent_guid));        
        auto const batch_it = fct.content.find("BATCH.agent");
        if (batch_it != fct.content.end())
            erase_batch(batch_it->second);
    }

    m_folders.at(elem.folder_index).erase_content(OBJECT_KIND::AGENT);
    m_agids_to_guids.erase(elem.id);
    m_agents.erase(agent_guid.index);
}


void  simulation_context::generate_navigation2d_data_from_collider(object_guid const  collider_guid_)
{
    ASSUMPTION(is_valid_collider_guid(collider_guid_));
    m_ai_simulator_ptr->get_naveditor()->add_navcomponents_2d(collider_guid_);
}


void  simulation_context::delete_navigation2d_data_generated_from_collider(object_guid const  collider_guid_)
{
    ASSUMPTION(is_valid_collider_guid(collider_guid_));
    m_ai_simulator_ptr->get_naveditor()->del_navcomponents_2d(collider_guid_);
}


/////////////////////////////////////////////////////////////////////////////////////
// COLLISION CONTACTS API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_valid_collision_contact_index(natural_32_bit const  contact_index) const
{
    return m_collision_contacts.valid(contact_index);
}


std::vector<natural_32_bit> const&  simulation_context::collision_contacts_of_collider(object_guid const  collider_guid) const
{
    static std::vector<natural_32_bit> const  no_contacts;
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    auto const it = m_from_colliders_to_contacts.find(collider_guid);
    return it == m_from_colliders_to_contacts.end() ? no_contacts : it->second;
}


collision_contact const&  simulation_context::get_collision_contact(natural_32_bit const  contact_index) const
{
    ASSUMPTION(is_valid_collision_contact_index(contact_index));
    return m_collision_contacts.at(contact_index);
}


void  simulation_context::collision_contacts_between_colliders(object_guid const  collider_guid_1, object_guid const  collider_guid_2,
                                                               std::vector<collision_contact const*>&  output) const
{
    std::vector<natural_32_bit> const&  all_indices = collision_contacts_of_collider(collider_guid_1);
    ASSUMPTION(is_valid_collider_guid(collider_guid_2));
    for (natural_32_bit idx : all_indices)
    {
        ASSUMPTION(is_valid_collision_contact_index(idx));
        collision_contact const&  contact = m_collision_contacts.at(idx);
        if (contact.first_collider() == collider_guid_2 || contact.second_collider() == collider_guid_2)
            output.push_back(&contact);
    }
}


natural_32_bit  simulation_context::num_collision_contacts() const
{
    return (natural_32_bit)m_collision_contacts.valid_indices().size();
}


simulation_context::collision_contacts_iterator  simulation_context::collision_contacts_begin() const
{
    return m_collision_contacts.begin();
}


simulation_context::collision_contacts_iterator  simulation_context::collision_contacts_end() const
{
    return m_collision_contacts.end();
}


natural_32_bit  simulation_context::compute_contacts_with_box(
        vector3 const&  half_sizes_along_axes,
        matrix44 const&  from_base_matrix,
        bool const  search_static,
        bool const  search_dynamic,
        collision_scene_index const  scene_index,
        std::function<bool(collision_contact const&)> const&  contact_acceptor,
        std::function<bool(object_guid, angeo::COLLISION_CLASS)> const&  collider_filter
        ) const
{
    natural_32_bit  num_contacts = 0U;
    m_collision_scenes_ptr->at(scene_index)->find_contacts_with_box(
            half_sizes_along_axes,
            from_base_matrix,
            search_static,
            search_dynamic,
            [this, &contact_acceptor, &num_contacts, scene_index](
                angeo::contact_id const&  cid,
                vector3 const&  contact_point,
                vector3 const&  unit_normal,
                float_32_bit const  penetration_depth) -> bool {
                    angeo::collision_object_id const  coid_2 = angeo::get_object_id(angeo::get_second_collider_id(cid));
                    object_guid const  collider_2_guid = to_collider_guid(coid_2, scene_index);
                    collision_contact const  contact{
                            scene_index, invalid_object_guid(), collider_2_guid, contact_point, unit_normal, penetration_depth
                            };
                    ++num_contacts;
                    return contact_acceptor(contact);
                    },
            [this, &collider_filter,scene_index](angeo::collision_object_id const  coid, angeo::COLLISION_CLASS const  cc) {
                return collider_filter(to_collider_guid(coid, scene_index), cc);
                }
            );
    return  num_contacts;
}


natural_32_bit  simulation_context::compute_contacts_with_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        matrix44 const&  from_base_matrix,
        bool const  search_static,
        bool const  search_dynamic,
        collision_scene_index const  scene_index,
        std::function<bool(collision_contact const&)> const&  contact_acceptor,
        std::function<bool(object_guid, angeo::COLLISION_CLASS)> const&  collider_filter
        ) const
{
    natural_32_bit  num_contacts = 0U;
    m_collision_scenes_ptr->at(scene_index)->find_contacts_with_capsule(
            half_distance_between_end_points,
            thickness_from_central_line,
            from_base_matrix,
            search_static,
            search_dynamic,
            [this, &contact_acceptor, &num_contacts, scene_index](
                angeo::contact_id const&  cid,
                vector3 const&  contact_point,
                vector3 const&  unit_normal,
                float_32_bit const  penetration_depth) -> bool {
                    angeo::collision_object_id const  coid = angeo::get_object_id(angeo::get_first_collider_id(cid));
                    object_guid const  collider_guid = to_collider_guid(coid, scene_index);
                    collision_contact const  contact{
                            scene_index, collider_guid, invalid_object_guid(), contact_point, unit_normal, penetration_depth
                            };
                    ++num_contacts;
                    return contact_acceptor(contact);
                    },
            [this, &collider_filter,scene_index](angeo::collision_object_id const  coid, angeo::COLLISION_CLASS const  cc) {
                return collider_filter(to_collider_guid(coid, scene_index), cc);
                }
            );
    return  num_contacts;
}


natural_32_bit  simulation_context::compute_contacts_with_sphere(
        float_32_bit const  radius,
        matrix44 const&  from_base_matrix,
        bool const  search_static,
        bool const  search_dynamic,
        collision_scene_index const  scene_index,
        std::function<bool(collision_contact const&)> const&  contact_acceptor,
        std::function<bool(object_guid, angeo::COLLISION_CLASS)> const&  collider_filter
        ) const
{
    natural_32_bit  num_contacts = 0U;
    m_collision_scenes_ptr->at(scene_index)->find_contacts_with_sphere(
            radius,
            from_base_matrix,
            search_static,
            search_dynamic,
            [this, &contact_acceptor, &num_contacts, scene_index](
                angeo::contact_id const&  cid,
                vector3 const&  contact_point,
                vector3 const&  unit_normal,
                float_32_bit const  penetration_depth) -> bool {
                    angeo::collision_object_id const  coid_2 = angeo::get_object_id(angeo::get_second_collider_id(cid));
                    object_guid const  collider_2_guid = to_collider_guid(coid_2, scene_index);
                    collision_contact const  contact{
                            scene_index, invalid_object_guid(), collider_2_guid, contact_point, unit_normal, penetration_depth
                            };
                    ++num_contacts;
                    return contact_acceptor(contact);
                    },
            [this, &collider_filter, scene_index](angeo::collision_object_id const  coid, angeo::COLLISION_CLASS const  cc) {
                return collider_filter(to_collider_guid(coid, scene_index), cc);
                }
            );
    return  num_contacts;
}


// Disabled (not const) for modules.


natural_32_bit  simulation_context::insert_collision_contact(collision_contact const&  cc)
{
    natural_32_bit const  index = m_collision_contacts.insert(cc);
    m_from_colliders_to_contacts[cc.first_collider()].push_back(index);
    m_from_colliders_to_contacts[cc.second_collider()].push_back(index);
    return index;
}


void  simulation_context::clear_collision_contacts()
{
    m_collision_contacts.clear();
    m_from_colliders_to_contacts.clear();
}


/////////////////////////////////////////////////////////////////////////////////////
// ACCESS PATH API
/////////////////////////////////////////////////////////////////////////////////////


bool  simulation_context::is_absolute_path(std::string const&  path) const
{
    ASSUMPTION(!path.empty());
    return path.front() == '/';
}


bool  simulation_context::is_path_to_folder(std::string const&  path) const
{
    ASSUMPTION(!path.empty());
    return path.back() == '/';
}


object_guid  simulation_context::from_absolute_path(std::string const&  path) const
{
    ASSUMPTION(is_absolute_path(path));

    std::vector<std::string>  names;
    boost::split(names, path, [](std::string::value_type c) -> bool { return c == '/'; });
    std::vector<std::string>  tmp_names;
    tmp_names.swap(names);
    for (std::string const&  name : tmp_names)
    {
        if (name.empty() || name == ".")
            continue;
        if (name == "..")
        {
            if (names.empty())
                return invalid_object_guid();
            names.pop_back();
            continue;
        }
        names.push_back(name);
    }

    object_guid  guid = root_folder();
    bool const  is_folder_path = is_path_to_folder(path);
    if (names.empty())
    {
        INVARIANT(is_folder_path);
        return guid;
    }
    for (natural_32_bit  i = 0U, n = (natural_32_bit)names.size() - 1U; i <= n; ++i)
    {
        std::string const&  name = names.at(i);
        folder_content_type const&  fct = folder_content(guid);
        folder_content_type::names_to_guids_map const&  guids_map = (i == n && !is_folder_path) ? fct.content : fct.child_folders;

        auto  it = guids_map.find(name);
        if (it == guids_map.end())
            return invalid_object_guid();
        guid = it->second;
    }
    return guid;
}


std::string  simulation_context::to_absolute_path(object_guid const  guid) const
{
    ASSUMPTION(guid != invalid_object_guid());

    std::vector<std::string>  names;
    for (object_guid  obj_guid = guid; obj_guid != root_folder(); obj_guid = folder_of(obj_guid))
        names.push_back(name_of(obj_guid));
    std::stringstream  sstr;
    for (auto it = names.rbegin(); it != names.rend(); ++it)
        sstr << "/" << *it;
    if (guid.kind == OBJECT_KIND::FOLDER)
        sstr << '/';
    std::string const  path = sstr.str();

    INVARIANT(is_absolute_path(path) && (guid.kind == OBJECT_KIND::FOLDER) == is_path_to_folder(path));

    return path;
}


object_guid  simulation_context::from_relative_path(object_guid const  base_guid, std::string const&  relative_path) const
{
    ASSUMPTION(!is_absolute_path(relative_path));
    return from_absolute_path(to_absolute_path(base_guid) + "/" + relative_path);
}


std::string  simulation_context::to_relative_path(object_guid const  guid, object_guid const  relative_base_guid) const
{
    ASSUMPTION(guid != invalid_object_guid() && relative_base_guid != invalid_object_guid());

    std::vector<std::string>  guid_names;
    for (object_guid  obj_guid = guid; obj_guid != root_folder(); obj_guid = folder_of(obj_guid))
        guid_names.push_back(name_of(obj_guid));
    std::reverse(guid_names.begin(), guid_names.end());

    std::vector<std::string>  base_names;
    for (object_guid  obj_guid = relative_base_guid; obj_guid != root_folder(); obj_guid = folder_of(obj_guid))
        base_names.push_back(name_of(obj_guid));
    std::reverse(base_names.begin(), base_names.end());

    auto  guid_it = guid_names.begin();
    auto  base_it = base_names.begin();
    for ( ; guid_it != guid_names.end() && base_it != base_names.end() && *guid_it == *base_it; ++guid_it, ++base_it)
        ;

    std::stringstream  sstr;
    for ( ; base_it != base_names.end(); ++base_it)
        sstr << "../";
    for ( ; guid_it != guid_names.end(); ++guid_it)
        sstr << *guid_it << '/';
    std::string  path = sstr.str();
    if (path.empty())
        path = "./";
    if (guid.kind != OBJECT_KIND::FOLDER && path.back() == '/')
        path.pop_back();

    INVARIANT(!is_absolute_path(path) && (guid.kind == OBJECT_KIND::FOLDER) == is_path_to_folder(path));

    return path;
}


/////////////////////////////////////////////////////////////////////////////////////
// SCENE IMPORT/EXPORT API
/////////////////////////////////////////////////////////////////////////////////////


std::string const&  simulation_context::get_data_root_dir() const
{
    return m_data_root_dir;
}


std::string  simulation_context::get_animation_root_dir() const
{
    return get_data_root_dir() + "animation/";
}


std::string  simulation_context::get_batch_root_dir() const
{
    return get_data_root_dir() + "batch/";
}


std::string  simulation_context::get_font_root_dir() const
{
    return get_data_root_dir() + "font/";
}


std::string  simulation_context::get_icon_root_dir() const
{
    return get_data_root_dir() + "icon/";
}


std::string  simulation_context::get_import_root_dir() const
{
    return get_data_root_dir() + "import/";
}


std::string  simulation_context::get_mesh_root_dir() const
{
    return get_data_root_dir() + "meshe/";
}


std::string  simulation_context::get_scene_root_dir() const
{
    return get_data_root_dir() + "scene/";
}


std::string  simulation_context::get_texture_root_dir() const
{
    return get_data_root_dir() + "texture/";
}


void  simulation_context::request_late_import_scene_from_directory(import_scene_props const&  props) const
{
    auto const  it = m_cache_of_imported_scenes.find(detail::imported_scene::key_from_path(props.import_dir));
    m_requests_late_scene_import.push_back({
            it == m_cache_of_imported_scenes.end() ? detail::imported_scene(props.import_dir) : it->second,
            props
            });
}


// Disabled (not const) for modules.


void  simulation_context::insert_imported_batch_to_cache(gfx::batch const  batch)
{
    m_cache_of_imported_batches.insert({batch.key(), batch });
}


/////////////////////////////////////////////////////////////////////////////////////
// REQUESTS PROCESSING API
/////////////////////////////////////////////////////////////////////////////////////


// Disabled (not const) for modules.


void  simulation_context::process_rigid_bodies_with_invalidated_shape()
{
    for (object_guid  rb_guid : m_rigid_bodies_with_invalidated_shape)
    {
        if (!is_valid_rigid_body_guid(rb_guid) || !is_rigid_body_moveable(rb_guid))
            continue;

        angeo::mass_and_inertia_tensor_builder  builder;
        for (object_guid  collider_guid : m_rigid_bodies.at(rb_guid.index).colliders)
        {
            if (collider_scene_index(collider_guid) != 0U)
                continue;
            switch (collision_class_of(collider_guid))
            {
            //case angeo::COLLISION_CLASS::STATIC_OBJECT:
            case angeo::COLLISION_CLASS::COMMON_MOVEABLE_OBJECT:
            //case angeo::COLLISION_CLASS::HEAVY_MOVEABLE_OBJECT:
            //case angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT:
                break;
            default: continue;
            }

            switch (collider_shape_type(collider_guid))
            {
            case angeo::COLLISION_SHAPE_TYPE::BOX:
                builder.insert_box(
                        collider_box_half_sizes_along_axes(collider_guid),
                        frame_world_matrix(frame_of_collider(collider_guid)),
                        collision_material_of(collider_guid),
                        collider_density_multiplier(collider_guid)
                        );
                break;
            case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
                builder.insert_capsule(
                        collider_capsule_half_distance_between_end_points(collider_guid),
                        collider_capsule_thickness_from_central_line(collider_guid),
                        frame_world_matrix(frame_of_collider(collider_guid)),
                        collision_material_of(collider_guid),
                        collider_density_multiplier(collider_guid)
                        );
                break;
            case angeo::COLLISION_SHAPE_TYPE::SPHERE:
                builder.insert_sphere(
                        translation_vector(frame_world_matrix(frame_of_collider(collider_guid))),
                        collider_sphere_radius(collider_guid),
                        collision_material_of(collider_guid),
                        collider_density_multiplier(collider_guid)
                        );
                break;
            default:
                NOT_IMPLEMENTED_YET();
                break;
            }
        }

        if (builder.empty())
            continue;

        float_32_bit  mass_inverted;
        matrix33  inertia_tensor_inverted;
        vector3  center_of_mass_in_world_space;
        builder.run(mass_inverted, inertia_tensor_inverted, center_of_mass_in_world_space);
        set_rigid_body_inverted_mass(rb_guid, mass_inverted);
        set_rigid_body_inverted_inertia_tensor(rb_guid, inertia_tensor_inverted);

        object_guid const  rb_frame_guid = frame_of_rigid_body(rb_guid);
        matrix44 const&  W = frame_world_matrix(rb_frame_guid);
        vector3 const  origin_shift_in_world_space = center_of_mass_in_world_space - translation_vector(W);
        frame_translate(rb_frame_guid, origin_shift_in_world_space);
        std::vector<object_guid>  child_frame_guids;
        direct_children_frames(rb_frame_guid, child_frame_guids);
        for (object_guid  child_guid : child_frame_guids)
            frame_translate(child_guid, -origin_shift_in_world_space);
        set_rigid_body_mass_centre(rb_guid, frame_coord_system_in_world_space(rb_frame_guid).origin());
    }

    clear_rigid_bodies_with_invalidated_shape();
}


void  simulation_context::clear_rigid_bodies_with_invalidated_shape()
{
    m_rigid_bodies_with_invalidated_shape.clear();
}


template<typename T>
struct  cursor_to_requests_list
{
    cursor_to_requests_list(std::list<T>&  data_) : data(&data_), record(&data_.front()) {}
    ~cursor_to_requests_list() { data->pop_front(); }
    T*  operator->() const { return record; }
    T  operator*() const { return *record; }
private:
    std::list<T>*  data;
    T*  record;
};

template<typename T>
inline cursor_to_requests_list<T>  make_request_cursor_to(std::list<T>&  data_) { return cursor_to_requests_list<T>(data_); }


void  simulation_context::process_pending_early_requests()
{
    for ( ; !m_pending_requests_early.empty(); m_pending_requests_early.pop_front())
        switch (m_pending_requests_early.front())
        {
        case REQUEST_INSERT_CUSTOM_CONSTRAINT: {
            auto  cursor = make_request_cursor_to(m_requests_early_insert_custom_constraint);
            insert_custom_constraint_to_physics(
                    cursor->ccid,
                    cursor->rigid_body_0,
                    cursor->linear_component_0,
                    cursor->angular_component_0,
                    cursor->rigid_body_1,
                    cursor->linear_component_1,
                    cursor->angular_component_1,
                    cursor->bias,
                    cursor->variable_lower_bound,
                    cursor->variable_upper_bound,
                    cursor->initial_value_for_cache_miss
                );
            } break;
        case REQUEST_INSERT_INSTANT_CONSTRAINT: {
            auto  cursor = make_request_cursor_to(m_requests_early_insert_instant_constraint);
            insert_instant_constraint_to_physics(
                cursor->rigid_body_0,
                cursor->linear_component_0,
                cursor->angular_component_0,
                cursor->rigid_body_1,
                cursor->linear_component_1,
                cursor->angular_component_1,
                cursor->bias,
                cursor->variable_lower_bound,
                cursor->variable_upper_bound,
                cursor->initial_value
                );
            } break;
        default: UNREACHABLE(); break;
        }
}


void  simulation_context::clear_pending_early_requests()
{
    m_pending_requests_early.clear();
    m_requests_early_insert_custom_constraint.clear();
    m_requests_early_insert_instant_constraint.clear();
}


bool  simulation_context::has_pending_requests() const
{
    return !m_pending_requests.empty();
}


void  simulation_context::process_pending_requests()
{
    for ( ; has_pending_requests(); m_pending_requests.pop_front())
        switch (m_pending_requests.front())
        {
        case REQUEST_ERASE_FOLDER:
            erase_non_root_empty_folder(*make_request_cursor_to(m_requests_erase_folder));
            break;
        case REQUEST_ERASE_FRAME:
            erase_frame(*make_request_cursor_to(m_requests_erase_frame));
            break;
        case REQUEST_RELOCATE_FRAME: {
            auto  cursor = make_request_cursor_to(m_requests_relocate_frame);
            frame_relocate(cursor->frame_guid, cursor->position, cursor->orientation, cursor->relative_to_parent);
            } break;
        case REQUEST_SET_PARENT_FRAME: {
            auto  cursor = make_request_cursor_to(m_requests_set_parent_frame);
            set_parent_frame(cursor->frame_guid, cursor->parent_frame_guid);
            } break;
        case REQUEST_ERASE_BATCH:
            erase_batch(*make_request_cursor_to(m_requests_erase_batch));
            break;
        case REQUEST_ENABLE_COLLIDER: {
            auto  cursor = make_request_cursor_to(m_requests_enable_collider);
            enable_collider(cursor->collider_guid, cursor->state);
            } break;
        case REQUEST_ENABLE_COLLIDING: {
            auto  cursor = make_request_cursor_to(m_requests_enable_colliding);
            enable_colliding(cursor->collider_1, cursor->collider_2, cursor->state);
            } break;
        case REQUEST_ENABLE_COLLIDING_BY_PATH: {
            auto  cursor = make_request_cursor_to(m_requests_enable_colliding_by_path);
            enable_colliding(from_relative_path(cursor->base_folder_guid_1, cursor->relative_path_to_collider_1),
                             from_relative_path(cursor->base_folder_guid_2, cursor->relative_path_to_collider_2),
                             cursor->state);
            } break;
        case REQUEST_INSERT_COLLIDER_BOX: {
            auto  cursor = make_request_cursor_to(m_requests_insert_collider_box);
            insert_collider_box(cursor->under_folder_guid, cursor->name, cursor->half_sizes_along_axes,
                                cursor->material, cursor->collision_class, cursor->density_multiplier,
                                cursor->scene_index);
            } break;
        case REQUEST_INSERT_COLLIDER_CAPSULE: {
            auto  cursor = make_request_cursor_to(m_requests_insert_collider_capsule);
            insert_collider_capsule(cursor->under_folder_guid, cursor->name, cursor->half_distance_between_end_points,
                                    cursor->thickness_from_central_line, cursor->material, cursor->collision_class,
                                    cursor->density_multiplier, cursor->scene_index);
            } break;
        case REQUEST_INSERT_COLLIDER_SPHERE: {
            auto  cursor = make_request_cursor_to(m_requests_insert_collider_sphere);
            insert_collider_sphere(cursor->under_folder_guid, cursor->name, cursor->radius, cursor->material,
                                   cursor->collision_class, cursor->density_multiplier, cursor->scene_index);
            } break;
        case REQUEST_ERASE_COLLIDER:
            erase_collider(*make_request_cursor_to(m_requests_erase_collider));
            break;
        case REQUEST_INSERT_RIGID_BODY: {
            auto  cursor = make_request_cursor_to(m_requests_insert_rigid_body);
            insert_rigid_body(cursor->under_folder_guid, cursor->is_moveable, cursor->linear_velocity,
                              cursor->angular_velocity, cursor->linear_acceleration, cursor->angular_acceleration,
                              cursor->inverted_mass, cursor->inverted_inertia_tensor);
            } break;
        case REQUEST_ERASE_RIGID_BODY:
            erase_rigid_body(*make_request_cursor_to(m_requests_erase_rigid_body));
            break;
        case REQUEST_SET_LINEAR_VELOCITY: {
            auto  cursor = make_request_cursor_to(m_requests_set_linear_velocity);
            set_rigid_body_linear_velocity(cursor->rb_guid, cursor->velocity);
            } break;
        case REQUEST_SET_LINEAR_VELOCITY_BY_PATH: {
            auto  cursor = make_request_cursor_to(m_requests_set_linear_velocity_by_path);
            set_rigid_body_linear_velocity(from_relative_path(cursor->base_folder_guid, cursor->relative_path_to_rigid_body),
                                           cursor->velocity);
            } break;
        case REQUEST_SET_ANGULAR_VELOCITY: {
            auto  cursor = make_request_cursor_to(m_requests_set_angular_velocity);
            set_rigid_body_angular_velocity(cursor->rb_guid, cursor->velocity);
            } break;
        case REQUEST_SET_ANGULAR_VELOCITY_BY_PATH: {
            auto  cursor = make_request_cursor_to(m_requests_set_angular_velocity_by_path);
            set_rigid_body_angular_velocity(from_relative_path(cursor->base_folder_guid, cursor->relative_path_to_rigid_body),
                                            cursor->velocity);
            } break;
        case REQUEST_MUL_LINEAR_VELOCITY: {
            auto  cursor = make_request_cursor_to(m_requests_mul_linear_velocity);
            vector3 const  velocity = mul_components(cursor->velocity_scale, linear_velocity_of_rigid_body(cursor->rb_guid));
            set_rigid_body_linear_velocity(cursor->rb_guid, velocity);
            } break;
        case REQUEST_MUL_LINEAR_VELOCITY_BY_PATH: {
            auto  cursor = make_request_cursor_to(m_requests_mul_linear_velocity_by_path);
            object_guid const  rb_guid = from_relative_path(cursor->base_folder_guid, cursor->relative_path_to_rigid_body);
            vector3 const  velocity = mul_components(cursor->velocity_scale, linear_velocity_of_rigid_body(rb_guid));
            set_rigid_body_linear_velocity(rb_guid, velocity);
            } break;
        case REQUEST_MUL_ANGULAR_VELOCITY: {
            auto  cursor = make_request_cursor_to(m_requests_mul_angular_velocity);
            vector3 const  velocity = mul_components(cursor->velocity_scale, angular_velocity_of_rigid_body(cursor->rb_guid));
            set_rigid_body_angular_velocity(cursor->rb_guid, velocity);
            } break;
        case REQUEST_MUL_ANGULAR_VELOCITY_BY_PATH: {
            auto  cursor = make_request_cursor_to(m_requests_mul_angular_velocity_by_path);
            object_guid const  rb_guid = from_relative_path(cursor->base_folder_guid, cursor->relative_path_to_rigid_body);
            vector3 const  velocity = mul_components(cursor->velocity_scale, angular_velocity_of_rigid_body(rb_guid));
            set_rigid_body_angular_velocity(rb_guid, velocity);
            } break;
        case REQUEST_SET_LINEAR_ACCEL: {
            auto  cursor = make_request_cursor_to(m_requests_set_linear_acceleration_from_source);
            set_rigid_body_linear_acceleration_from_source(cursor->rb_guid, cursor->source_id, cursor->acceleration);
            } break;
        case REQUEST_SET_ANGULAR_ACCEL: {
            auto  cursor = make_request_cursor_to(m_requests_set_angular_acceleration_from_source);
            set_rigid_body_angular_acceleration_from_source(cursor->rb_guid, cursor->source_id, cursor->acceleration);
            } break;
        case REQUEST_DEL_LINEAR_ACCEL: {
            auto  cursor = make_request_cursor_to(m_requests_del_linear_acceleration_from_source);
            remove_rigid_body_linear_acceleration_from_source(cursor->rb_guid, cursor->source_id);
            } break;
        case REQUEST_DEL_ANGULAR_ACCEL: {
            auto  cursor = make_request_cursor_to(m_requests_del_angular_acceleration_from_source);
            remove_rigid_body_angular_acceleration_from_source(cursor->rb_guid, cursor->source_id);
            } break;
        case REQUEST_ERASE_TIMER:
            erase_timer(*make_request_cursor_to(m_requests_erase_timer));
            break;
        case REQUEST_ERASE_SENSOR:
            erase_sensor(*make_request_cursor_to(m_requests_erase_sensor));
            break;
        case REQUEST_ERASE_AGENT:
            erase_agent(*make_request_cursor_to(m_requests_erase_agent));
            break;
        default: UNREACHABLE(); break;
        }
}


void  simulation_context::clear_pending_requests()
{
    m_pending_requests.clear();
    m_requests_erase_folder.clear();
    m_requests_erase_frame.clear();
    m_requests_relocate_frame.clear();
    m_requests_set_parent_frame.clear();
    m_requests_erase_batch.clear();
    m_requests_enable_collider.clear();
    m_requests_enable_colliding.clear();
    m_requests_enable_colliding_by_path.clear();
    m_requests_insert_collider_box.clear();
    m_requests_insert_collider_capsule.clear();
    m_requests_insert_collider_sphere.clear();
    m_requests_erase_collider.clear();
    m_requests_insert_rigid_body.clear();
    m_requests_erase_rigid_body.clear();
    m_requests_set_linear_velocity.clear();
    m_requests_set_linear_velocity_by_path.clear();
    m_requests_set_angular_velocity.clear();
    m_requests_set_angular_velocity_by_path.clear();
    m_requests_mul_linear_velocity.clear();
    m_requests_mul_linear_velocity_by_path.clear();
    m_requests_mul_angular_velocity.clear();
    m_requests_mul_angular_velocity_by_path.clear();
    m_requests_set_linear_acceleration_from_source.clear();
    m_requests_set_angular_acceleration_from_source.clear();
    m_requests_del_linear_acceleration_from_source.clear();
    m_requests_del_angular_acceleration_from_source.clear();
    m_requests_erase_timer.clear();
    m_requests_erase_sensor.clear();
    m_requests_erase_agent.clear();
}


void  simulation_context::process_pending_late_requests()
{
    m_requests_late_scene_import.update(
            [](request_data_imported_scene&  request) { return request.scene.is_load_finished(); },
            [this](request_data_imported_scene&  request) {
                if (request.scene.loaded_successfully())
#if BUILD_RELEASE() == 1
                    try
#endif
                    {
                        detail::import_scene(*this, request.scene, request.props);
                        if (request.props.store_in_cache)
                            m_cache_of_imported_scenes.insert({ request.scene.key(), request.scene });
                    }
#if BUILD_RELEASE() == 1
                    catch (std::exception const&  e)
                    {
                        LOG(LSL_ERROR, "Failed to import scene " << request.scene.key().get_unique_id() << ". Details: " << e.what());
                        // To prevent subsequent attempts to load the scene from disk.
                        m_cache_of_imported_scenes.insert({ request.scene.key(), request.scene });
                    }
#endif
                else
                    // To prevent subsequent attempts to load the scene from disk.
                    m_cache_of_imported_scenes.insert({ request.scene.key(), request.scene });
            });

    m_requests_late_insert_agent.update(
            [this](request_data_insert_agent&  request) {
                ASSUMPTION(!request.skeleton_attached_batches.empty());
                for (auto const&  name_and_batch : request.skeleton_attached_batches)
                {
                    if (!name_and_batch.second.is_load_finished())
                        return false;
                    if (!name_and_batch.second.loaded_successfully())
                        return true;
                }
                if (request.motion_templates.empty())
                {
                    ASSUMPTION(!request.skeleton_attached_batches.empty());
                    gfx::batch const&  first_batch = request.skeleton_attached_batches.front().second;
                    ASSUMPTION(first_batch.get_available_resources().skeletal() != nullptr);
                    request.motion_templates = ai::skeletal_motion_templates(
                            std::filesystem::path(get_data_root_dir())
                                / first_batch.get_available_resources().skeletal()->animation_dir(),
                            1U,
                            nullptr
                            );
                }
                return request.config.is_load_finished() && request.motion_templates.is_load_finished();
            },
            [this](request_data_insert_agent&  request) {
                if (!request.config.loaded_successfully())
                {
                    LOG(LSL_ERROR, "Failed to import agent config '" << request.config.key().get_unique_id() << "' " <<
                                   "for agent imported under folder '" << name_of_folder(request.under_folder_guid) << "'.");
                    return;
                }
                if (!request.motion_templates.loaded_successfully())
                {
                    LOG(LSL_ERROR, "Failed to import motion templates '" << request.motion_templates.key().get_unique_id() << "' " <<
                                   "for agent imported under folder '" << name_of_folder(request.under_folder_guid) << "'.");
                    return;
                }
                for (auto const&  name_and_batch : request.skeleton_attached_batches)
                    if (!name_and_batch.second.loaded_successfully())
                    {
                        LOG(LSL_ERROR, "Failed to import skeleton attached batch '" <<
                                       name_and_batch.second.key().get_unique_id() << "' " <<
                                       "for agent imported under folder '" << name_of_folder(request.under_folder_guid) << "'.");
                        return;
                    }
                if (!request.config.empty())
                    m_cache_of_imported_agent_configs.insert({ request.config.key(), request.config });
                if (!request.motion_templates.empty())
                    m_cache_of_imported_motion_templates.insert({ request.motion_templates.key(), request.motion_templates });
                insert_agent(
                        request.under_folder_guid,
                        request.config,
                        request.motion_templates,
                        request.skeleton_attached_batches
                        );
            });
}


void  simulation_context::clear_pending_late_requests()
{
    m_requests_late_scene_import.clear();
    m_requests_late_insert_agent.clear();
}


void  simulation_context::clear_invalidated_guids()
{
    m_invalidated_guids.clear();
}


void  simulation_context::clear_relocated_frame_guids()
{
    m_relocated_frame_guids.clear();
}


/////////////////////////////////////////////////////////////////////////////////////
// SCENE CLEAR API
/////////////////////////////////////////////////////////////////////////////////////


void  simulation_context::clear(bool const  also_caches)
{
    clear_rigid_bodies_with_invalidated_shape();
    clear_pending_early_requests();
    clear_pending_requests();
    clear_pending_late_requests();
    clear_rigid_bodies_with_invalidated_shape();
    clear_collision_contacts();

    if (also_caches)
    {
        m_cache_of_imported_scenes.clear();
        m_cache_of_imported_batches.clear();
        m_cache_of_imported_motion_templates.clear();
        m_cache_of_imported_agent_configs.clear();
    }

    folder_content_type const&  fct = folder_content(root_folder());
    INVARIANT(fct.content.empty());
    for (auto const&  name_and_guid : fct.child_folders)
        request_erase_non_root_folder(name_and_guid.second);

    process_pending_requests();

    INVARIANT(
        folder_content(root_folder()).content.empty() &&
        folder_content(root_folder()).child_folders.empty() &&
        m_pending_requests_early.empty() &&
        m_pending_requests.empty() &&
        m_requests_late_scene_import.empty() &&
        m_requests_late_insert_agent.empty()
        );

    clear_invalidated_guids();

    if (also_caches)
    {
        m_ai_simulator_ptr->clear();
        m_device_simulator_ptr->clear();
        m_rigid_body_simulator_ptr->clear();
        for (auto  ptr : *m_collision_scenes_ptr)
            ptr->clear();
        m_frames_provider.clear();
    }
}


}
