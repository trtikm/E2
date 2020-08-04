#include <com/simulation_context.hpp>
#include <angeo/collision_scene.hpp>
#include <angeo/rigid_body_simulator.hpp>
#include <angeo/mass_and_inertia_tensor.hpp>
#include <utility/async_resource_load.hpp>
#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <sstream>

namespace com {


simulation_context_ptr  simulation_context::create(
        std::shared_ptr<angeo::collision_scene> const  collision_scene_ptr_,
        std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
        std::shared_ptr<com::device_simulator> const  device_simulator_ptr_,
        std::shared_ptr<aiold::simulator> const  ai_simulator_ptr_,
        std::string const&  data_root_dir_
        )
{
    ASSUMPTION(collision_scene_ptr_ != nullptr && rigid_body_simulator_ptr_ != nullptr && ai_simulator_ptr_ == nullptr);
    return std::shared_ptr<simulation_context>(new simulation_context(
                collision_scene_ptr_,
                rigid_body_simulator_ptr_,
                device_simulator_ptr_,
                ai_simulator_ptr_,
                data_root_dir_
                ));
}


simulation_context::simulation_context(
        std::shared_ptr<angeo::collision_scene> const  collision_scene_ptr_,
        std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
        std::shared_ptr<com::device_simulator> const  device_simulator_ptr_,
        std::shared_ptr<aiold::simulator> const  ai_simulator_ptr_,
        std::string const&  data_root_dir_
        )
    : m_root_folder()
    , m_folders()
    , m_frames()
    , m_batches()
    , m_colliders()
    , m_rigid_bodies()
    , m_timers()
    , m_sensors()
    , m_agents()
    , m_frames_provider()
    , m_collision_scene_ptr(collision_scene_ptr_)
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
    , m_moveable_colliders()
    , m_moveable_rigid_bodies()
    , m_collision_contacts()
    , m_from_colliders_to_contacts()
    , m_data_root_dir(canonical_path(data_root_dir_.empty() ? "." : data_root_dir_).string())
    // EARLY REQUESTS HANDLING
    , m_rigid_bodies_with_invalidated_shape()
    , m_pending_requests_early()
    , m_requests_early_insert_custom_constraint()
    , m_requests_early_insert_instant_constraint()
    // REQUESTS HANDLING
    , m_pending_requests()
    , m_requests_erase_folder()
    , m_requests_erase_frame()
    , m_requests_erase_batch()
    , m_requests_enable_collider()
    , m_requests_enable_colliding()
    , m_requests_erase_collider()
    , m_requests_erase_rigid_body()
    , m_requests_set_linear_velocity()
    , m_requests_set_angular_velocity()
    , m_requests_set_linear_acceleration_from_source()
    , m_requests_set_angular_acceleration_from_source()
    , m_requests_del_linear_acceleration_from_source()
    , m_requests_del_angular_acceleration_from_source()
    , m_requests_erase_timer()
    , m_requests_erase_sensor()
    // SCENE IMPORT REQUESTS HANDLING
    , m_requests_queue_scene_import()
    , m_cache_of_imported_scenes()
    , m_cache_of_imported_effect_configs()
    , m_cache_of_imported_batches()
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


object_guid  simulation_context::folder_of_folder(object_guid const  folder_guid) const
{
    ASSUMPTION(is_valid_folder_guid(folder_guid));
    return folder_content(folder_guid).parent_folder;
}


object_guid  simulation_context::folder_of(object_guid const  guid) const
{
    switch (guid.kind)
    {
    case OBJECT_KIND::FOLDER: return folder_of_folder(guid);
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


std::string const&  simulation_context::name_of_folder(object_guid const  guid) const
{
    return folder_content(guid).folder_name;
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


object_guid  simulation_context::insert_folder(object_guid const  under_folder_guid, std::string const&  folder_name) const
{
    ASSUMPTION(folder_content(under_folder_guid).child_folders.count(folder_name) == 0ULL);
    simulation_context* const  self = const_cast<simulation_context*>(this);
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
    for (auto const&  name_and_guid : fct.content)
        switch (name_and_guid.second.kind)
        {
        case OBJECT_KIND::FRAME:
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
            NOT_IMPLEMENTED_YET();
            break;
        default: UNREACHABLE(); break;
        }
    if (frame_guid != invalid_object_guid())
        request_erase_frame(frame_guid);

    request_erase_non_root_empty_folder(folder_guid);
}


// Disabled (not const) for modules.


void  simulation_context::erase_non_root_empty_folder(object_guid const  folder_guid)
{
    ASSUMPTION(is_folder_empty(folder_guid));
    if (folder_guid != root_folder())
    {
        folder_content_type const&  erased_folder = m_folders.at(folder_guid.index);
        m_folders.at(erased_folder.parent_folder.index).child_folders.erase(erased_folder.folder_name);
        m_folders.erase(folder_guid.index);
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
    return to_frame_guid(m_frames_provider.parent(m_frames.at(frame_guid.index).id));
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
                                              vector3 const&  origin, quaternion const&  orientation) const
{
    simulation_context* const  self = const_cast<simulation_context*>(this);
    object_guid const  frame_guid = self->insert_frame(under_folder_guid);
    if (is_valid_frame_guid(parent_frame_guid))
        self->set_parent_frame(frame_guid, parent_frame_guid);
    self->frame_relocate(frame_guid, origin, orientation);
    return frame_guid;
}


void  simulation_context::request_erase_frame(object_guid const  frame_guid) const
{
    m_requests_erase_frame.push_back(frame_guid);
    m_pending_requests.push_back(REQUEST_ERASE_FRAME);
}


// Disabled (not const) for modules.


void  simulation_context::set_parent_frame(object_guid const  frame_guid, object_guid const  parent_frame_guid)
{
    ASSUMPTION(
        is_valid_frame_guid(frame_guid) &&
        [this](object_guid const  folder_guid, object_guid  parent_folder_guid) -> bool {
            for ( ; parent_folder_guid != invalid_object_guid(); parent_folder_guid = folder_content(parent_folder_guid).parent_folder)
                if (parent_folder_guid == folder_guid)
                    return false;
            return true;
        }(folder_of_frame(frame_guid), folder_of_frame(parent_frame_guid))
        );
    m_frames_provider.set_parent(m_frames.at(frame_guid.index).id, m_frames.at(parent_frame_guid.index).id);
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

    m_folders.at(under_folder_guid.index).content.insert({ to_string(OBJECT_KIND::FRAME), frame_guid });

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
    auto const&  elem = m_frames.at(frame_guid.index);
    m_frames_provider.erase(elem.id);
    m_folders.at(elem.folder_index).content.erase(elem.element_name);
    m_frids_to_guids.erase(elem.id);
    m_frames.erase(frame_guid.index);
}


void  simulation_context::frame_translate(object_guid const  frame_guid, vector3 const&  shift)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.translate(m_frames.at(frame_guid.index).id, shift);
}


void  simulation_context::frame_rotate(object_guid const  frame_guid, quaternion const&  rotation)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.rotate(m_frames.at(frame_guid.index).id, rotation);
}


void  simulation_context::frame_set_origin(object_guid const  frame_guid, vector3 const&  new_origin)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.set_origin(m_frames.at(frame_guid.index).id, new_origin);
}


void  simulation_context::frame_set_orientation(object_guid const  frame_guid, quaternion const&  new_orientation)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.set_orientation(m_frames.at(frame_guid.index).id, new_orientation);
}


void  simulation_context::frame_relocate(object_guid const  frame_guid, angeo::coordinate_system const& new_coord_system)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.relocate(m_frames.at(frame_guid.index).id, new_coord_system);
}


void  simulation_context::frame_relocate(object_guid const  frame_guid, vector3 const&  new_origin, quaternion const&  new_orientation)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.relocate(m_frames.at(frame_guid.index).id, new_origin, new_orientation);
}


void  simulation_context::frame_relocate_relative_to_parent(object_guid const  frame_guid, vector3 const&  new_origin,
                                                            quaternion const&  new_orientation)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid));
    return m_frames_provider.relocate_relative_to_parent(m_frames.at(frame_guid.index).id, new_origin, new_orientation);
}


void  simulation_context::frame_relocate_relative_to_parent(object_guid const  frame_guid, object_guid const  relocation_frame_guid)
{
    ASSUMPTION(is_valid_frame_guid(frame_guid) && is_valid_frame_guid(relocation_frame_guid));
    return m_frames_provider.relocate_relative_to_parent(
                m_frames.at(frame_guid.index).id,
                m_frames.at(relocation_frame_guid.index).id
                );
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
    return m_batches_to_guids.at(batch.uid());
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


object_guid  simulation_context::insert_batch(object_guid const  folder_guid, std::string const&  name, gfx::batch const  batch)
{
    ASSUMPTION(folder_content(folder_guid).content.count(name) == 0UL);

    object_guid const  batch_guid = { OBJECT_KIND::BATCH, m_batches.insert({ batch.uid(), folder_guid.index, name, batch }) };

    m_batches_to_guids.insert({ batch.uid(), batch_guid });

    m_folders.at(folder_guid.index).content.insert({ name, batch_guid });

    object_guid const  frame_guid = find_closest_frame(folder_guid, true);
    ASSUMPTION(frame_guid != invalid_object_guid());

    m_batches.at(batch_guid.index).frames.push_back(frame_guid);

    return batch_guid;
}


void  simulation_context::erase_batch(object_guid const  batch_guid)
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));

    auto const&  elem = m_batches.at(batch_guid.index);

    ASSUMPTION(
        folder_content(folder_of_batch(batch_guid)).content.count(elem.element_name) == 1UL &&
        folder_content(folder_of_batch(batch_guid)).content.find(elem.element_name)->second == batch_guid
        );

    m_folders.at(elem.folder_index).content.erase(elem.element_name);
    m_batches_to_guids.erase(elem.id);
    m_batches.erase(batch_guid.index);
}

object_guid  simulation_context::load_batch(
        object_guid const  folder_guid, std::string const&  name,
        std::string const&  disk_path,
        gfx::effects_config  effects_config,
        std::string const&  skin_name
        )
{
    return insert_batch(folder_guid, name, gfx::batch(disk_path, effects_config, skin_name));
}


object_guid  simulation_context::insert_batch_lines3d(
        object_guid const  folder_guid, std::string const&  name,
        std::vector<std::pair<vector3,vector3> > const&  lines,
        vector4 const&  common_colour
        )
{
    return insert_batch(folder_guid, name, gfx::create_lines3d(lines, common_colour, gfx::FOG_TYPE::NONE));
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
                                                  colour, with_axis, gfx::FOG_TYPE::NONE)
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


object_guid  simulation_context::to_collider_guid(angeo::collision_object_id const  coid) const
{
    return m_coids_to_guids.at(coid);
}


simulation_context::collider_guid_iterator  simulation_context::colliders_begin() const
{
    return collider_guid_iterator(m_colliders.valid_indices().begin());
}


simulation_context::collider_guid_iterator  simulation_context::colliders_end() const
{
    return collider_guid_iterator(m_colliders.valid_indices().end());
}


bool  simulation_context::is_collider_moveable(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_moveable_colliders.count(collider_guid.index) != 0UL;
}


simulation_context::collider_guid_iterator  simulation_context::moveable_colliders_begin() const
{
    return collider_guid_iterator(m_moveable_colliders.begin());
}


simulation_context::collider_guid_iterator  simulation_context::moveable_colliders_end() const
{
    return collider_guid_iterator(m_moveable_colliders.end());
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
    return m_collision_scene_ptr->get_material(m_colliders.at(collider_guid.index).id.front());
}


angeo::COLLISION_CLASS  simulation_context::collision_class_of(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_collision_class(m_colliders.at(collider_guid.index).id.front());
}


angeo::COLLISION_SHAPE_TYPE  simulation_context::collider_shape_type(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return angeo::get_shape_type(m_colliders.at(collider_guid.index).id.front());
}


vector3 const&  simulation_context::collider_box_half_sizes_along_axes(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_box_half_sizes_along_axes(m_colliders.at(collider_guid.index).id.front());
}


float_32_bit  simulation_context::collider_capsule_half_distance_between_end_points(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_capsule_half_distance_between_end_points(m_colliders.at(collider_guid.index).id.front());
}


float_32_bit  simulation_context::collider_capsule_thickness_from_central_line(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_capsule_thickness_from_central_line(m_colliders.at(collider_guid.index).id.front());
}


float_32_bit  simulation_context::collider_sphere_radius(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_sphere_radius(m_colliders.at(collider_guid.index).id.front());
}


bool  simulation_context::is_collider_enabled(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->is_collider_enabled(m_colliders.at(collider_guid.index).id.front());
}


object_guid  simulation_context::insert_collider_box(
        object_guid const  under_folder_guid, std::string const&  name,
        vector3 const&  half_sizes_along_axes,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class
        ) const
{
    return const_cast<simulation_context*>(this)->insert_collider(under_folder_guid, name,
        [this, &half_sizes_along_axes, material, collision_class]
        (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
            coids.push_back(m_collision_scene_ptr->insert_box(half_sizes_along_axes, W, material, collision_class, is_dynamic));
        });
}


object_guid  simulation_context::insert_collider_capsule(
        object_guid const  under_folder_guid, std::string const&  name,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class
        ) const
{
    return const_cast<simulation_context*>(this)->insert_collider(under_folder_guid, name,
        [this, half_distance_between_end_points, thickness_from_central_line, material, collision_class]
            (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
            coids.push_back(m_collision_scene_ptr->insert_capsule(half_distance_between_end_points, thickness_from_central_line,
                                                                  W, material, collision_class, is_dynamic));
        });
}


object_guid  simulation_context::insert_collider_sphere(
        object_guid const  under_folder_guid, std::string const&  name,
        float_32_bit const  radius,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class
        ) const
{
    return const_cast<simulation_context*>(this)->insert_collider(under_folder_guid, name,
        [this, radius, material, collision_class]
        (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
            coids.push_back(m_collision_scene_ptr->insert_sphere(radius, W, material, collision_class, is_dynamic));
        });
}


object_guid  simulation_context::insert_collider_triangle_mesh(
        object_guid const  under_folder_guid, std::string const&  name_prefix,
        natural_32_bit const  num_triangles,
        std::function<vector3(natural_32_bit, natural_8_bit)> const&  getter_of_end_points_in_model_space,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class
        ) const
{
    return const_cast<simulation_context*>(this)->insert_collider(under_folder_guid, name_prefix,
            [this, num_triangles, &getter_of_end_points_in_model_space, material, collision_class]
            (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
                m_collision_scene_ptr->insert_triangle_mesh(num_triangles, getter_of_end_points_in_model_space,
                                                            W, material, collision_class, is_dynamic, coids);
            });
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
    m_requests_enable_colliding.push_back({collider_1, collider_2, state});
    m_pending_requests.push_back(REQUEST_ENABLE_COLLIDING);
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
    for (angeo::collision_object_id  coid : m_colliders.at(collider_guid.index).id)
        m_collision_scene_ptr->enable_collider(coid, state);
}


void  simulation_context::enable_colliding(object_guid const  collider_1, object_guid const  collider_2, const bool  state)
{
    ASSUMPTION(is_valid_collider_guid(collider_1) && is_valid_collider_guid(collider_2));
    for (angeo::collision_object_id  coid_1 : m_colliders.at(collider_1.index).id)
        for (angeo::collision_object_id  coid_2 : m_colliders.at(collider_2.index).id)
            if (state)
                m_collision_scene_ptr->enable_colliding(coid_1, coid_2);
            else
                m_collision_scene_ptr->disable_colliding(coid_1, coid_2);
}


std::vector<angeo::collision_object_id> const&  simulation_context::from_collider_guid(object_guid const  collider_guid)
{
    return m_colliders.at(collider_guid.index).id;
}


void  simulation_context::relocate_collider(object_guid const  collider_guid, matrix44 const&  world_matrix)
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    for (angeo::collision_object_id  coid : m_colliders.at(collider_guid.index).id)
        m_collision_scene_ptr->on_position_changed(coid, world_matrix);
}


object_guid  simulation_context::insert_collider(
        object_guid const  under_folder_guid, std::string const&  name,
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
    //ASSUMPTION(owner_guid != invalid_object_guid());

    bool const  is_moveable = rigid_body_guid != invalid_object_guid() && is_rigid_body_moveable(rigid_body_guid);

    std::vector<angeo::collision_object_id> coids;
    coids_builder(frame_world_matrix(frame_guid), is_moveable, coids);
    ASSUMPTION(!coids.empty());

    object_guid const  collider_guid = {
            OBJECT_KIND::COLLIDER,
            m_colliders.insert({ coids, under_folder_guid.index, name, frame_guid, owner_guid, rigid_body_guid })
            };

    for (angeo::collision_object_id  coid : coids)
        m_coids_to_guids.insert({ coid, collider_guid });

    m_folders.at(under_folder_guid.index).content.insert({ name, collider_guid });

if (owner_guid != invalid_object_guid())
    switch (owner_guid.kind)
    {
    case OBJECT_KIND::RIGID_BODY: break;
    case OBJECT_KIND::SENSOR: m_sensors.at(owner_guid.index).collider = collider_guid; break;
    // TODO:
    //case OBJECT_KIND::AGENT: m_agents.at(owner_guid.index).colliders.push_back(collider_guid); break;
    default: UNREACHABLE(); break;
    }

    if (rigid_body_guid != invalid_object_guid())
        m_rigid_bodies.at(rigid_body_guid.index).colliders.push_back(collider_guid);

    if (is_moveable)
    {
        m_moveable_colliders.insert(collider_guid.index);
        m_rigid_bodies_with_invalidated_shape.insert(rigid_body_guid);
    }

    return collider_guid;
}


void  simulation_context::erase_collider(object_guid const  collider_guid)
{
    ASSUMPTION(
        is_valid_collider_guid(collider_guid) &&
        folder_content(folder_of_collider(collider_guid)).content.count(m_colliders.at(collider_guid.index).element_name) != 0UL
        );

    auto const&  elem = m_colliders.at(collider_guid.index);
    if (elem.owner != invalid_object_guid())
        switch (elem.owner.kind)
        {
        case OBJECT_KIND::RIGID_BODY: break;
        case OBJECT_KIND::SENSOR: m_sensors.at(elem.owner.index).collider = invalid_object_guid(); break;
        // TODO:
        //case OBJECT_KIND::AGENT: m_agents.at(owner_guid.index).colliders.push_back(collider_guid); break;
        default: UNREACHABLE(); break;
        }
    if (is_valid_rigid_body_guid(elem.rigid_body))
    {
        auto&  rb_colliders = m_rigid_bodies.at(elem.rigid_body.index).colliders;
        auto const  self_it = std::find(rb_colliders.begin(), rb_colliders.end(), collider_guid);
        ASSUMPTION(self_it != rb_colliders.end());
        rb_colliders.erase(self_it);
        if (is_rigid_body_moveable(elem.rigid_body))
            m_rigid_bodies_with_invalidated_shape.insert(elem.rigid_body);
    }
    for (angeo::collision_object_id  coid : elem.id)
        m_collision_scene_ptr->erase_object(coid);
    m_folders.at(elem.folder_index).content.erase(elem.element_name);
    for (angeo::collision_object_id  coid : elem.id)
        m_coids_to_guids.erase(coid);
    m_moveable_colliders.erase(collider_guid.index);
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
        vector3 const&  angular_acceleration
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

    angeo::rigid_body_id const  rbid = m_rigid_body_simulator_ptr->insert_rigid_body(
            frame_coord_system_in_world_space(frame_guid).origin(),
            frame_coord_system_in_world_space(frame_guid).orientation(),
            0.0f,
            matrix33_zero(),
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

    self->m_folders.at(under_folder_guid.index).content.insert({ to_string(OBJECT_KIND::RIGID_BODY), rigid_body_guid });

    for_each_child_folder(under_folder_guid, true, true,
        [self, rigid_body_guid](object_guid const  folder_guid, folder_content_type const&  fct) -> bool {
            for (auto const&  name_and_guid : fct.content)
                if (name_and_guid.second.kind == OBJECT_KIND::COLLIDER)
                {
                    self->m_rigid_bodies.at(rigid_body_guid.index).colliders.push_back(name_and_guid.second);
                    self->m_colliders.at(name_and_guid.second.index).rigid_body = rigid_body_guid;
                }
            return true;
        });

    if (is_moveable)
    {
        self->m_moveable_rigid_bodies.insert(rigid_body_guid.index);
        for (object_guid  collider_guid : m_rigid_bodies.at(rigid_body_guid.index).colliders)
            self->m_moveable_colliders.insert(collider_guid.index);
        self->m_rigid_bodies_with_invalidated_shape.insert(rigid_body_guid);
    }

    return rigid_body_guid;
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


void  simulation_context::request_set_rigid_body_angular_velocity(object_guid const  rigid_body_guid,  vector3 const&  velocity) const
{
    m_requests_set_angular_velocity.push_back({ rigid_body_guid, velocity });
    m_pending_requests.push_back(REQUEST_SET_ANGULAR_VELOCITY);
}


void  simulation_context::request_set_rigid_body_linear_acceleration_from_source(
        object_guid const  rigid_body_guid, object_guid const  source_guid, vector3 const&  acceleration) const
{
    m_requests_set_linear_acceleration_from_source.push_back({ rigid_body_guid, source_guid, acceleration });
    m_pending_requests.push_back(REQUEST_SET_LINEAR_ACCEL);
}


void  simulation_context::request_set_rigid_body_angular_acceleration_from_source(
        object_guid const  rigid_body_guid, object_guid const  source_guid, vector3 const&  acceleration) const
{
    m_requests_set_angular_acceleration_from_source.push_back({ rigid_body_guid, source_guid, acceleration });
    m_pending_requests.push_back(REQUEST_SET_ANGULAR_ACCEL);
}


void  simulation_context::request_remove_rigid_body_linear_acceleration_from_source(
        object_guid const  rigid_body_guid, object_guid const  source_guid) const
{
    m_requests_del_linear_acceleration_from_source.push_back({ rigid_body_guid, source_guid });
    m_pending_requests.push_back(REQUEST_DEL_LINEAR_ACCEL);
}


void  simulation_context::request_remove_rigid_body_angular_acceleration_from_source(
        object_guid const  rigid_body_guid, object_guid const  source_guid) const
{
    m_requests_del_angular_acceleration_from_source.push_back({ rigid_body_guid, source_guid });
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

    auto const&  elem = m_rigid_bodies.at(rigid_body_guid.index);
    for (object_guid  collider_guid : elem.colliders)
    {
        m_colliders.at(collider_guid.index).rigid_body = invalid_object_guid();
        m_moveable_colliders.erase(collider_guid.index);
    }
    m_rigid_body_simulator_ptr->erase_rigid_body(elem.id);
    m_folders.at(elem.folder_index).content.erase(elem.element_name);
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
        object_guid const  rigid_body_guid, object_guid const  source_guid, vector3 const&  accel)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_linear_acceleration_from_source(m_rigid_bodies.at(rigid_body_guid.index).id, source_guid, accel);
}


void  simulation_context::set_rigid_body_angular_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid,
                                                        vector3 const&  accel)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->set_angular_acceleration_from_source(m_rigid_bodies.at(rigid_body_guid.index).id, source_guid, accel);
}


void  simulation_context::remove_rigid_body_linear_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->remove_linear_acceleration_from_source(m_rigid_bodies.at(rigid_body_guid.index).id, source_guid);
}


void  simulation_context::remove_rigid_body_angular_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    m_rigid_body_simulator_ptr->remove_angular_acceleration_from_source(m_rigid_bodies.at(rigid_body_guid.index).id, source_guid);
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
    m_folders.at(under_folder_guid.index).content.insert({ name, timer_guid });
    return timer_guid;
}


void  simulation_context::erase_timer(object_guid const  timer_guid)
{
    ASSUMPTION(is_valid_timer_guid(timer_guid));
    auto const&  elem = m_timers.at(timer_guid.index);
    {
        auto const&  infos = m_device_simulator_ptr->request_infos_of_timer(elem.id);
        while (!infos.empty())
            m_device_simulator_ptr->erase_request_info(infos.back());
    }
    m_device_simulator_ptr->erase_timer(elem.id);
    m_folders.at(elem.folder_index).content.erase(elem.element_name);
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
    m_folders.at(under_folder_guid.index).content.insert({ name, sensor_guid });
    return sensor_guid;
}


void  simulation_context::erase_sensor(object_guid const  sensor_guid)
{
    ASSUMPTION(is_valid_sensor_guid(sensor_guid));
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
    m_folders.at(elem.folder_index).content.erase(elem.element_name);
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


void  simulation_context::insert_request_info_update_radial_force_field(
        device_request_info_id const&  drid, float_32_bit const  multiplier, float_32_bit const  exponent,
        float_32_bit const  min_radius, bool const  use_mass
        )
{
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_update_radial_force_field(
            multiplier, exponent, min_radius, use_mass
            ));
}


void  simulation_context::insert_request_info_update_linear_force_field(
        device_request_info_id const&  drid, vector3 const&  acceleration, bool const  use_mass
        )
{
    register_request_info(drid, m_device_simulator_ptr->insert_request_info_update_linear_force_field(acceleration, use_mass));
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


object_guid  simulation_context::to_agent_guid(aiold::object_id const  agid) const
{
    return m_agids_to_guids.at(agid);
}


simulation_context::agent_guid_iterator  simulation_context::agents_begin() const
{
    return agent_guid_iterator(m_agents.valid_indices().begin());
}


simulation_context::agent_guid_iterator  simulation_context::agents_end() const
{
    return agent_guid_iterator(m_agents.valid_indices().end());
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


simulation_context::collision_contact const&  simulation_context::get_collision_contact(natural_32_bit const  contact_index) const
{
    ASSUMPTION(is_valid_collision_contact_index(contact_index));
    return m_collision_contacts.at(contact_index);
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
            ASSUMPTION(!names.empty());
            names.pop_back();
            continue;
        }
        names.push_back(name);
    }

    object_guid  guid = root_folder();
    INVARIANT(!names.empty());
    bool const  is_folder_path = is_path_to_folder(path);
    for (natural_32_bit  i = 0U, n = (natural_32_bit)names.size() - 1U; i <= n; ++i)
    {
        std::string const&  name = names.at(i);
        folder_content_type const&  fct = folder_content(guid);
        folder_content_type::names_to_guids_map const&  guids_map = (i == n && !is_folder_path) ? fct.content : fct.child_folders;

        auto  it = guids_map.find(name);
        ASSUMPTION(it != guids_map.end());
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
        path.push_back('.');
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


void  simulation_context::request_import_scene_from_directory(import_scene_props const&  props) const
{
    auto const  it = m_cache_of_imported_scenes.find(detail::imported_scene::key_from_path(props.import_dir));
    m_requests_queue_scene_import.push_back({
            it == m_cache_of_imported_scenes.end() ? detail::imported_scene(props.import_dir) : it->second,
            props
            });
}


// Disabled (not const) for modules.


void  simulation_context::insert_imported_batch_to_cache(gfx::batch const  batch)
{
    m_cache_of_imported_batches.insert({batch.key(), batch });
}


void  simulation_context::insert_imported_effects_config_to_cache(gfx::effects_config const  effects_config)
{
    m_cache_of_imported_effect_configs.insert({effects_config.key(), effects_config });
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
            switch (collision_class_of(collider_guid))
            {
            //case angeo::COLLISION_CLASS::STATIC_OBJECT:
            case angeo::COLLISION_CLASS::COMMON_MOVEABLE_OBJECT:
            //case angeo::COLLISION_CLASS::HEAVY_MOVEABLE_OBJECT:
            case angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT:
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
                        1.0f // TODO!
                        );
                break;
            case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
                builder.insert_capsule(
                        collider_capsule_half_distance_between_end_points(collider_guid),
                        collider_capsule_thickness_from_central_line(collider_guid),
                        frame_world_matrix(frame_of_collider(collider_guid)),
                        collision_material_of(collider_guid),
                        1.0f // TODO!
                        );
                break;
            case angeo::COLLISION_SHAPE_TYPE::SPHERE:
                builder.insert_sphere(
                        translation_vector(frame_world_matrix(frame_of_collider(collider_guid))),
                        collider_sphere_radius(collider_guid),
                        collision_material_of(collider_guid),
                        1.0f // TODO!
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
        vector3 const  origin_shift_in_local_space = transform_vector(origin_shift_in_world_space, inverse44(W));
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


void  simulation_context::process_pending_early_requests()
{
    std::vector<request_data_insertion_of_custom_constraint>::const_iterator  insert_custom_constraint_it =
        m_requests_early_insert_custom_constraint.begin();
    std::vector<request_data_insertion_of_instant_constraint>::const_iterator  insert_instant_constraint_it =
        m_requests_early_insert_instant_constraint.begin();

    for (REQUEST_EARLY_KIND  kind : m_pending_requests_early)
        switch (kind)
        {
        case REQUEST_INSERT_CUSTOM_CONSTRAINT:
            insert_custom_constraint_to_physics(
                insert_custom_constraint_it->ccid,
                insert_custom_constraint_it->rigid_body_0,
                insert_custom_constraint_it->linear_component_0, insert_custom_constraint_it->angular_component_0,
                insert_custom_constraint_it->rigid_body_1,
                insert_custom_constraint_it->linear_component_1, insert_custom_constraint_it->angular_component_1,
                insert_custom_constraint_it->bias,
                insert_custom_constraint_it->variable_lower_bound, insert_custom_constraint_it->variable_upper_bound,
                insert_custom_constraint_it->initial_value_for_cache_miss
                );
            ++insert_custom_constraint_it;
            break;
        case REQUEST_INSERT_INSTANT_CONSTRAINT:
            insert_instant_constraint_to_physics(
                insert_instant_constraint_it->rigid_body_0,
                insert_instant_constraint_it->linear_component_0, insert_instant_constraint_it->angular_component_0,
                insert_instant_constraint_it->rigid_body_1,
                insert_instant_constraint_it->linear_component_1, insert_instant_constraint_it->angular_component_1,
                insert_instant_constraint_it->bias,
                insert_instant_constraint_it->variable_lower_bound, insert_instant_constraint_it->variable_upper_bound,
                insert_instant_constraint_it->initial_value
                );
            ++insert_instant_constraint_it;
            break;
        default: UNREACHABLE(); break;
        }

    clear_pending_early_requests();
}


void  simulation_context::clear_pending_early_requests()
{
    m_pending_requests_early.clear();
    m_requests_early_insert_custom_constraint.clear();
    m_requests_early_insert_instant_constraint.clear();
}


void  simulation_context::process_pending_requests()
{
    std::vector<object_guid>::const_iterator  erase_folder_it = m_requests_erase_folder.begin();
    std::vector<object_guid>::const_iterator  erase_frame_it = m_requests_erase_frame.begin();
    std::vector<object_guid>::const_iterator  erase_batch_it = m_requests_erase_batch.begin();
    std::vector<request_data_enable_collider>::const_iterator  enable_collider_it = m_requests_enable_collider.begin();
    std::vector<request_data_enable_colliding>::const_iterator  enable_colliding_it = m_requests_enable_colliding.begin();
    std::vector<object_guid>::const_iterator  erase_collider_it = m_requests_erase_collider.begin();
    std::vector<request_data_set_velocity>::const_iterator  set_linear_velocity_it = m_requests_set_linear_velocity.begin();
    std::vector<request_data_set_velocity>::const_iterator  set_angular_velocity_it = m_requests_set_angular_velocity.begin();
    std::vector<object_guid>::const_iterator  erase_rigid_body_it = m_requests_erase_rigid_body.begin();
    std::vector<request_data_set_acceleration_from_source>::const_iterator  set_linear_acceleration_it =
        m_requests_set_linear_acceleration_from_source.begin();
    std::vector<request_data_set_acceleration_from_source>::const_iterator  set_angular_acceleration_it =
        m_requests_set_angular_acceleration_from_source.begin();
    std::vector<request_data_del_acceleration_from_source>::const_iterator  del_linear_acceleration_it =
        m_requests_del_linear_acceleration_from_source.begin();
    std::vector<request_data_del_acceleration_from_source>::const_iterator  del_angular_acceleration_it =
        m_requests_del_angular_acceleration_from_source.begin();
    std::vector<object_guid>::const_iterator  erase_timer_it = m_requests_erase_timer.begin();
    std::vector<object_guid>::const_iterator  erase_sensor_it = m_requests_erase_sensor.begin();

    for (REQUEST_KIND  kind : m_pending_requests)
        switch (kind)
        {
        case REQUEST_ERASE_FOLDER:
            erase_non_root_empty_folder(*erase_folder_it);
            ++erase_folder_it;
            break;
        case REQUEST_ERASE_FRAME:
            erase_frame(*erase_frame_it);
            ++erase_frame_it;
            break;
        case REQUEST_ERASE_BATCH:
            erase_batch(*erase_batch_it);
            ++erase_batch_it;
            break;
        case REQUEST_ENABLE_COLLIDER:
            enable_collider(enable_collider_it->collider_guid, enable_collider_it->state);
            ++enable_collider_it;
            break;
        case REQUEST_ENABLE_COLLIDING:
            enable_colliding(enable_colliding_it->collider_1, enable_colliding_it->collider_2, enable_colliding_it->state);
            ++enable_colliding_it;
            break;
        case REQUEST_ERASE_COLLIDER:
            erase_collider(*erase_collider_it);
            ++erase_collider_it;
            break;
        case REQUEST_ERASE_RIGID_BODY:
            erase_rigid_body(*erase_rigid_body_it);
            ++erase_rigid_body_it;
            break;
        case REQUEST_SET_LINEAR_VELOCITY:
            set_rigid_body_linear_velocity(set_linear_velocity_it->rb_guid, set_linear_velocity_it->velocity);
            ++set_linear_velocity_it;
            break;
        case REQUEST_SET_ANGULAR_VELOCITY:
            set_rigid_body_angular_velocity(set_angular_velocity_it->rb_guid, set_angular_velocity_it->velocity);
            ++set_angular_velocity_it;
            break;
        case REQUEST_SET_LINEAR_ACCEL:
            set_rigid_body_linear_acceleration_from_source(set_linear_acceleration_it->rb_guid, set_linear_acceleration_it->source_guid,
                                                           set_linear_acceleration_it->acceleration);
            ++set_linear_acceleration_it;
            break;
        case REQUEST_SET_ANGULAR_ACCEL:
            set_rigid_body_angular_acceleration_from_source(set_angular_acceleration_it->rb_guid, set_angular_acceleration_it->source_guid,
                                                           set_angular_acceleration_it->acceleration);
            ++set_angular_acceleration_it;
            break;
        case REQUEST_DEL_LINEAR_ACCEL:
            remove_rigid_body_linear_acceleration_from_source(del_linear_acceleration_it->rb_guid, del_linear_acceleration_it->source_guid);
            ++del_linear_acceleration_it;
            break;
        case REQUEST_DEL_ANGULAR_ACCEL:
            remove_rigid_body_angular_acceleration_from_source(del_angular_acceleration_it->rb_guid, del_angular_acceleration_it->source_guid);
            ++del_angular_acceleration_it;
            break;
        case REQUEST_ERASE_TIMER:
            erase_timer(*erase_timer_it);
            ++erase_timer_it;
            break;
        case REQUEST_ERASE_SENSOR:
            erase_sensor(*erase_sensor_it);
            ++erase_sensor_it;
            break;
        default: UNREACHABLE(); break;
        }

    clear_pending_requests();
}


void  simulation_context::clear_pending_requests()
{
    m_pending_requests.clear();
    m_requests_erase_folder.clear();
    m_requests_erase_frame.clear();
    m_requests_erase_batch.clear();
    m_requests_enable_collider.clear();
    m_requests_enable_colliding.clear();
    m_requests_erase_collider.clear();
    m_requests_erase_rigid_body.clear();
    m_requests_set_linear_velocity.clear();
    m_requests_set_angular_velocity.clear();
    m_requests_set_linear_acceleration_from_source.clear();
    m_requests_set_angular_acceleration_from_source.clear();
    m_requests_del_linear_acceleration_from_source.clear();
    m_requests_del_angular_acceleration_from_source.clear();
    m_requests_erase_timer.clear();
    m_requests_erase_sensor.clear();
}


void  simulation_context::process_pending_requests_import_scene()
{
    for (natural_32_bit  i = 0U; i < (natural_32_bit)m_requests_queue_scene_import.size(); )
        if (m_requests_queue_scene_import.at(i).scene.is_load_finished())
        {
            std::swap(m_requests_queue_scene_import.at(i), m_requests_queue_scene_import.back());
            request_props_imported_scene const  request = m_requests_queue_scene_import.back();
            m_requests_queue_scene_import.pop_back();

            if (request.scene.loaded_successfully())
                try
                {
                    detail::import_scene(*this, request.scene, request.props);
                    if (request.props.store_in_cache)
                        m_cache_of_imported_scenes.insert({ request.scene.key(), request.scene });
                }
                catch (std::exception const&  e)
                {
                    LOG(error, "Failed to import scene " << request.scene.key().get_unique_id() << ". Details: " << e.what());
                    // To prevent subsequent attempts to load the scene from disk.
                    m_cache_of_imported_scenes.insert({ request.scene.key(), request.scene });
                }
            else
                // To prevent subsequent attempts to load the scene from disk.
                m_cache_of_imported_scenes.insert({ request.scene.key(), request.scene });
        }
        else
            ++i;
}


void  simulation_context::clear_pending_requests_import_scene()
{
    m_requests_queue_scene_import.clear();
}


/////////////////////////////////////////////////////////////////////////////////////
// SCENE CLEAR API
/////////////////////////////////////////////////////////////////////////////////////


void  simulation_context::clear(bool const  also_caches)
{
    clear_rigid_bodies_with_invalidated_shape();
    clear_pending_early_requests();
    clear_pending_requests();
    clear_pending_requests_import_scene();
    clear_rigid_bodies_with_invalidated_shape();

    m_collision_contacts.clear();
    m_from_colliders_to_contacts.clear();

    if (also_caches)
    {
        m_cache_of_imported_scenes.clear();
        m_cache_of_imported_effect_configs.clear();
        m_cache_of_imported_batches.clear();
    }

    folder_content_type const&  fct = folder_content(root_folder());
    for (auto const&  name_and_guid : fct.child_folders)
        request_erase_non_root_folder(name_and_guid.second);
    INVARIANT(fct.content.empty());

    process_pending_requests();
}


}
