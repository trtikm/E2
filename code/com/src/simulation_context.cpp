#include <com/simulation_context.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>

namespace com {


simulation_context_ptr  simulation_context::create(
        std::shared_ptr<angeo::collision_scene> const  collision_scene_ptr_,
        std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
        std::shared_ptr<ai::simulator> const  ai_simulator_ptr_
        )
{
    ASSUMPTION(collision_scene_ptr_ != nullptr && rigid_body_simulator_ptr_ != nullptr && ai_simulator_ptr_ == nullptr);
    return std::shared_ptr<simulation_context>(new simulation_context(
                collision_scene_ptr_,
                rigid_body_simulator_ptr_,
                ai_simulator_ptr_
                ));
}


simulation_context::simulation_context(
        std::shared_ptr<angeo::collision_scene> const  collision_scene_ptr_,
        std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
        std::shared_ptr<ai::simulator> const  ai_simulator_ptr_
        )
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
    , m_frames_provider()
    , m_collision_scene_ptr(collision_scene_ptr_)
    , m_rigid_body_simulator_ptr(rigid_body_simulator_ptr_)
    , m_ai_simulator_ptr(ai_simulator_ptr_)
    , m_frids_to_guids()
    , m_batches_to_guids()
    , m_coids_to_guids()
    , m_rbids_to_guids()
    , m_seids_to_guids()
    , m_acids_to_guids()
    , m_deids_to_guids()
    , m_agids_to_guids()
    , m_moveable_colliders()
    , m_moveable_rigid_bodies()
{
    m_root_folder = { OBJECT_KIND::FOLDER, m_folders.insert(folder_content_type("ROOT", invalid_object_guid())) };
}


/////////////////////////////////////////////////////////////////////////////////////
// ACCESS PATH API
/////////////////////////////////////////////////////////////////////////////////////


object_guid  simulation_context::from_absolute_path(std::string const&  path) const
{
    NOT_IMPLEMENTED_YET();
    return invalid_object_guid();
}


std::string  simulation_context::to_absolute_path(object_guid const  guid) const
{
    NOT_IMPLEMENTED_YET();
    return "";
}


object_guid  simulation_context::from_relative_path(object_guid const  base_guid, std::string const&  relative_path) const
{
    NOT_IMPLEMENTED_YET();
    return invalid_object_guid();
}


std::string  simulation_context::to_relative_path(object_guid const  guid, object_guid const  relative_base_guid) const
{
    NOT_IMPLEMENTED_YET();
    return "";
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


void  simulation_context::request_insert_folder(object_guid const  under_folder_guid, std::string const&  folder_name) const
{
    NOT_IMPLEMENTED_YET();
}


void  simulation_context::request_erase_non_root_empty_folder(object_guid const  folder_guid) const
{
    NOT_IMPLEMENTED_YET();
}


// Disabled (not const) for modules.


object_guid  simulation_context::insert_folder(object_guid const  under_folder_guid, std::string const&  folder_name)
{
    ASSUMPTION(folder_content(under_folder_guid).child_folders.count(folder_name) == 0ULL);
    object_guid const  new_folder_guid = { OBJECT_KIND::FOLDER, m_folders.insert(folder_content_type(folder_name, under_folder_guid)) };
    m_folders.at(under_folder_guid.index).child_folders.insert({ folder_name, new_folder_guid });
    return new_folder_guid;
}


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


void  simulation_context::request_insert_frame(object_guid const  under_folder_guid) const
{
    NOT_IMPLEMENTED_YET();
}


void  simulation_context::request_erase_frame(object_guid const  frame_guid) const
{
    NOT_IMPLEMENTED_YET();
}


// Disabled (not const) for modules.


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
        folder_content(frame_guid).child_folders.empty() &&
        folder_content(frame_guid).content.size() == 1UL &&
        folder_content(frame_guid).content.begin()->second == frame_guid
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


BATCH_CLASS  simulation_context::batch_class(object_guid const  batch_guid) const
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return m_batches.at(batch_guid.index).batch_class;
}


std::vector<object_guid> const&  simulation_context::frames_of_batch(object_guid const  batch_guid) const
{
    ASSUMPTION(is_valid_batch_guid(batch_guid));
    return m_batches.at(batch_guid.index).frames;
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


object_guid  simulation_context::insert_batch(object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
                                              gfx::batch const  batch)
{
    ASSUMPTION(folder_content(folder_guid).content.count(name) == 0UL);

    object_guid const  batch_guid = { OBJECT_KIND::BATCH, m_batches.insert({ batch.uid(), folder_guid.index, name, batch, cls }) };

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
        folder_content(batch_guid).content.count(elem.element_name) == 1UL &&
        folder_content(batch_guid).content.find(elem.element_name)->second == batch_guid
        );

    m_folders.at(elem.folder_index).content.erase(elem.element_name);
    m_batches_to_guids.erase(elem.id);
    m_batches.erase(batch_guid.index);
}

object_guid  simulation_context::load_batch(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        std::string const&  disk_path,
        gfx::effects_config  effects_config,
        std::string const&  skin_name
        )
{
    return insert_batch(folder_guid, name, cls, gfx::batch(disk_path, effects_config, skin_name));
}


object_guid  simulation_context::insert_batch_lines3d(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        std::vector<std::pair<vector3,vector3> > const&  lines,
        vector4 const&  common_colour
        )
{
    return insert_batch(folder_guid, name, cls,
        gfx::create_lines3d(lines, common_colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_wireframe_box(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, cls,
        gfx::create_wireframe_box(half_sizes_along_axes, colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_solid_box(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, cls,
        gfx::create_solid_box(half_sizes_along_axes, colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_wireframe_capsule(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, cls,
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
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, cls,
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
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, cls,
        gfx::create_wireframe_sphere(radius, num_lines_per_quarter_of_circle, colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_solid_smooth_sphere(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, cls,
        gfx::create_solid_smooth_sphere(radius, num_lines_per_quarter_of_circle, colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_solid_sphere(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, cls,
        gfx::create_solid_sphere(radius, num_lines_per_quarter_of_circle, colour, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_triangle_mesh(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        std::vector< std::array<float_32_bit, 3> > const& vertices,
        std::vector< std::array<float_32_bit, 3> > const& normals,
        vector4 const&  colour
        )
{
    return insert_batch(folder_guid, name, cls,
        gfx::create_triangle_mesh(vertices, normals, colour, gfx::FOG_TYPE::NONE, "")
        );
}


object_guid  simulation_context::insert_batch_wireframe_perspective_frustum(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
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
    return insert_batch(folder_guid, name, cls,
        gfx::create_wireframe_perspective_frustum(near_plane, far_plane, left_plane, right_plane, top_plane, bottom_plane,
                                                  colour, with_axis, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_grid(
        object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
        float_32_bit const  max_x_coordinate,
        float_32_bit const  max_y_coordinate,
        float_32_bit const  max_z_coordinate,
        float_32_bit const  step_along_x_axis,
        float_32_bit const  step_along_y_axis,
        std::array<float_32_bit, 4> const&  colour_for_x_lines,
        std::array<float_32_bit, 4> const&  colour_for_y_lines,
        std::array<float_32_bit, 4> const&  colour_for_highlighted_x_lines,
        std::array<float_32_bit, 4> const&  colour_for_highlighted_y_lines,
        std::array<float_32_bit, 4> const&  colour_for_central_x_line,
        std::array<float_32_bit, 4> const&  colour_for_central_y_line,
        std::array<float_32_bit, 4> const&  colour_for_central_z_line,
        natural_32_bit const  highlight_every,
        gfx::GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE const  main_exes_orientation_marker_type
        )
{
    return insert_batch(folder_guid, name, cls,
        gfx::create_grid(max_x_coordinate, max_y_coordinate, max_z_coordinate, step_along_x_axis, step_along_y_axis,
                         colour_for_x_lines, colour_for_y_lines, colour_for_highlighted_x_lines, colour_for_highlighted_y_lines,
                         colour_for_central_x_line, colour_for_central_y_line, colour_for_central_z_line, highlight_every,
                         main_exes_orientation_marker_type, gfx::FOG_TYPE::NONE)
        );
}


object_guid  simulation_context::insert_batch_default_grid(object_guid const  folder_guid, std::string const&  name,
                                                           BATCH_CLASS const  cls)
{
    return insert_batch_grid(folder_guid, name, cls,
                50.0f,
                50.0f,
                50.0f,
                1.0f,
                1.0f,
                { 0.4f, 0.4f, 0.4f, 1.0f },
                { 0.4f, 0.4f, 0.4f, 1.0f },
                { 0.5f, 0.5f, 0.5f, 1.0f },
                { 0.5f, 0.5f, 0.5f, 1.0f },
                { 1.0f, 0.0f, 0.0f, 1.0f },
                { 0.0f, 1.0f, 0.0f, 1.0f },
                { 0.0f, 0.0f, 1.0f, 1.0f },
                10U,
                gfx::GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE::TRIANGLE
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
    return { OBJECT_KIND::FOLDER, m_frames.at(collider_guid.index).folder_index };
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


angeo::COLLISION_MATERIAL_TYPE  simulation_context::collision_material_of(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_material(m_colliders.at(collider_guid.index).id);
}


// Disabled (not const) for modules.


angeo::collision_object_id  simulation_context::from_collider_guid(object_guid const  collider_guid)
{
    return m_colliders.at(collider_guid.index).id;
}


void  simulation_context::relocate_collider(object_guid const  collider_guid, matrix44 const&  world_matrix)
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    m_collision_scene_ptr->on_position_changed(m_colliders.at(collider_guid.index).id, world_matrix);
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


// Disabled (not const) for modules.


angeo::rigid_body_id  simulation_context::from_rigid_body_guid(object_guid const  rigid_body_guid)
{
    ASSUMPTION(is_valid_rigid_body_guid(rigid_body_guid));
    return m_rigid_bodies.at(rigid_body_guid.index).id;
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


object_guid  simulation_context::to_sensor_guid(ai::object_id const  seid) const
{
    return m_seids_to_guids.at(seid);
}


simulation_context::sensor_guid_iterator  simulation_context::sensors_begin() const
{
    return sensor_guid_iterator(m_sensors.valid_indices().begin());
}


simulation_context::sensor_guid_iterator  simulation_context::sensors_end() const
{
    return sensor_guid_iterator(m_sensors.valid_indices().end());
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


object_guid  simulation_context::to_activator_guid(ai::object_id const  acid) const
{
    return m_acids_to_guids.at(acid);
}


simulation_context::activator_guid_iterator  simulation_context::activators_begin() const
{
    return activator_guid_iterator(m_activators.valid_indices().begin());
}


simulation_context::activator_guid_iterator  simulation_context::activators_end() const
{
    return activator_guid_iterator(m_activators.valid_indices().end());
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


object_guid  simulation_context::to_device_guid(ai::object_id const  deid) const
{
    return m_deids_to_guids.at(deid);
}


simulation_context::device_guid_iterator  simulation_context::devices_begin() const
{
    return device_guid_iterator(m_devices.valid_indices().begin());
}


simulation_context::device_guid_iterator  simulation_context::devices_end() const
{
    return device_guid_iterator(m_devices.valid_indices().end());
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


object_guid  simulation_context::to_agent_guid(ai::object_id const  agid) const
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


}
