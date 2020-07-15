#include <com/simulation_context.hpp>
#include <angeo/mass_and_inertia_tensor.hpp>
#include <utility/async_resource_load.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

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
    , m_requests_queue_scene_import()
    , m_cache_of_imported_scenes()
{
    m_root_folder = { OBJECT_KIND::FOLDER, m_folders.insert(folder_content_type("ROOT", invalid_object_guid())) };
}


/////////////////////////////////////////////////////////////////////////////////////
// REQUESTS PROCESSING API
/////////////////////////////////////////////////////////////////////////////////////


// Disabled (not const) for modules.
void  simulation_context::process_pending_requests()
{
    process_pending_requests_import_scene();
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
                import_scene({ &request.scene.hierarchy(), &request.scene.effects() }, request.folder_guid);
                    if (request.store_in_cache)
                        m_cache_of_imported_scenes.insert({ request.scene.key().get_unique_id(), request.scene });
                }
                catch (std::exception const&  e)
                {
                    LOG(error, "Failed to import scene " << request.scene.key().get_unique_id() << ". Details: " << e.what());
                    // To prevent subsequent attempts to load the scene from disk.
                    m_cache_of_imported_scenes.insert({ request.scene.key().get_unique_id(), request.scene });
                }
            else
                // To prevent subsequent attempts to load the scene from disk.
                m_cache_of_imported_scenes.insert({ request.scene.key().get_unique_id(), request.scene });
        }
        else
            ++i;
}


/////////////////////////////////////////////////////////////////////////////////////
// SCENE IMPORT/EXPORT API
/////////////////////////////////////////////////////////////////////////////////////


simulation_context::imported_scene_data::imported_scene_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_hierarchy()
    , m_effects()
{
    boost::filesystem::path const  scene_dir = finaliser->get_key().get_unique_id();

    {
        boost::filesystem::path  pathname = scene_dir / "hierarchy.json";

        if (!boost::filesystem::is_regular_file(pathname))
            throw std::runtime_error(msgstream() << "Cannot access scene file '" << pathname << "'.");

        std::ifstream  istr(pathname.string(), std::ios_base::binary);
        if (!istr.good())
            throw std::runtime_error(msgstream() << "Cannot open the scene file '" << pathname << "'.");

        boost::property_tree::read_json(istr, m_hierarchy);
    }

    {
        boost::filesystem::path  pathname = scene_dir / "effects.json";

        if (!boost::filesystem::is_regular_file(pathname))
            throw std::runtime_error(msgstream() << "Cannot access effects file '" << pathname << "'.");

        std::ifstream  istr(pathname.string(), std::ios_base::binary);
        if (!istr.good())
            throw std::runtime_error(msgstream() << "Cannot open the effects file '" << pathname << "'.");

        boost::property_tree::ptree  ptree;
        boost::property_tree::read_json(istr, ptree);
        for (auto it = ptree.begin(); it != ptree.end(); ++it)
        {
            gfx::effects_config::light_types  light_types;
            for (auto const& lt_and_tree : it->second.get_child("light_types"))
                light_types.insert((gfx::LIGHT_TYPE)lt_and_tree.second.get_value<int>());
            gfx::effects_config::lighting_data_types  lighting_data_types;
            for (auto const& ldt_and_tree : it->second.get_child("lighting_data_types"))
                lighting_data_types.insert({
                    (gfx::LIGHTING_DATA_TYPE)std::atoi(ldt_and_tree.first.c_str()),
                    (gfx::SHADER_DATA_INPUT_TYPE)ldt_and_tree.second.get_value<int>()
                });
            gfx::effects_config::shader_output_types  shader_output_types;
            for (auto const& sot_and_tree : it->second.get_child("shader_output_types"))
                shader_output_types.insert((gfx::SHADER_DATA_OUTPUT_TYPE)sot_and_tree.second.get_value<int>());

            m_effects[it->first] = gfx::effects_config(
                    nullptr,
                    light_types,
                    lighting_data_types,
                    (gfx::SHADER_PROGRAM_TYPE)it->second.get<int>("lighting_algo_location"),
                    shader_output_types,
                    (gfx::FOG_TYPE)it->second.get<int>("fog_type"),
                    (gfx::SHADER_PROGRAM_TYPE)it->second.get<int>("fog_algo_location")
                    );
        }
    }
}


simulation_context::imported_scene::imported_scene(boost::filesystem::path const&  path)
    : async::resource_accessor<imported_scene_data>(
            { "com::simulation_context::imported_scene", boost::filesystem::absolute(path).string() }, 1U, nullptr
            )
{}


void  simulation_context::request_import_scene_from_directory(
        std::string const&  directory_on_the_disk, object_guid const  under_folder_guid, bool const  cache_imported_scene
        ) const
{
    m_requests_queue_scene_import.push_back({ imported_scene(directory_on_the_disk), under_folder_guid, cache_imported_scene });
}


// Disabled (not const) for modules.


void  simulation_context::import_scene(import_scene_props const&  props, object_guid const  under_folder_guid)
{
    if (props.hierarchy->count("@pivot") != 0UL)
    {
        import_gfxtuner_scene(props, under_folder_guid);
        return;
    }
    // TODO!
}


void  simulation_context::import_gfxtuner_scene(import_scene_props const&  props, object_guid const  under_folder_guid)
{
    ASSUMPTION(props.hierarchy->count("@pivot") != 0UL);

    for (auto  it = props.hierarchy->begin(); it != props.hierarchy->end(); ++it)
    {
        if (it->first.empty() || it->first.front() == '@')
            continue;

        folder_content_type const&  fct = folder_content(under_folder_guid);

        std::string  name = it->first;
        if (fct.child_folders.count(name) != 0)
        {
            natural_32_bit  counter = 0U;
            for ( ; fct.child_folders.count(name + '.' + std::to_string(counter)) != 0U; ++counter)
                ;
            name = name + '.' + std::to_string(counter);
        }

        object_guid const  folder_guid = insert_folder(under_folder_guid, name);

        import_gfxtuner_scene_node({ &it->second, props.effects }, folder_guid);

        for_each_child_folder(folder_guid, true, true,
            [this](object_guid const  folder_guid, folder_content_type const&  fct) -> bool {
                auto const  it = fct.content.find(to_string(OBJECT_KIND::RIGID_BODY));
                if (it == fct.content.end() || it->second.kind != OBJECT_KIND::RIGID_BODY || !is_rigid_body_moveable(it->second))
                    return true;
                angeo::mass_and_inertia_tensor_builder  builder;
                for (object_guid  collider_guid : m_rigid_bodies.at(it->second.index).colliders)
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
                float_32_bit  mass_inverted;
                matrix33  inertia_tensor_inverted;
                vector3  center_of_mass_in_world_space;
                builder.run(mass_inverted, inertia_tensor_inverted, center_of_mass_in_world_space);
                set_rigid_body_inverted_mass(it->second, mass_inverted);
                set_rigid_body_inverted_inertia_tensor(it->second, inertia_tensor_inverted);

                object_guid const  rb_frame_guid = frame_of_rigid_body(it->second);
                matrix44 const&  W = frame_world_matrix(rb_frame_guid);
                vector3 const  origin_shift_in_world_space = center_of_mass_in_world_space - translation_vector(W);
                vector3 const  origin_shift_in_local_space = transform_vector(origin_shift_in_world_space, inverse44(W));
                frame_translate(rb_frame_guid, origin_shift_in_world_space);
                std::vector<object_guid>  child_frame_guids;
                direct_children_frames(rb_frame_guid, child_frame_guids);
                for (object_guid  child_guid : child_frame_guids)
                    frame_translate(child_guid, -origin_shift_in_world_space);
                set_rigid_body_mass_centre(it->second, frame_coord_system_in_world_space(rb_frame_guid).origin());
                
                return true;
            });
    }
}


void  simulation_context::import_gfxtuner_scene_node(import_scene_props const&  props, object_guid const  folder_guid)
{
    boost::property_tree::ptree const&  origin_tree = props.hierarchy->find("origin")->second;
    vector3  origin(
            origin_tree.get<scalar>("x"),
            origin_tree.get<scalar>("y"),
            origin_tree.get<scalar>("z")
            );

    boost::property_tree::ptree const&  orientation_tree = props.hierarchy->find("orientation")->second;
    quaternion  orientation = make_quaternion_xyzw(
            orientation_tree.get<scalar>("x"),
            orientation_tree.get<scalar>("y"),
            orientation_tree.get<scalar>("z"),
            orientation_tree.get<scalar>("w")
            );

    frame_relocate(insert_frame(folder_guid), origin, orientation);

    boost::property_tree::ptree const&  folders = props.hierarchy->find("folders")->second;

    bool  has_static_collider = false;
    for (auto folder_it = folders.begin(); folder_it != folders.end(); ++folder_it)
        if (folder_it->first == "collider" && !folder_it->second.begin()->second.get<bool>("is_dynamic"))
        {
            switch(angeo::read_collison_class_from_string(folder_it->second.begin()->second.get<std::string>("collision_class")))
            {
            case angeo::COLLISION_CLASS::RAY_CAST_SIGHT:
            case angeo::COLLISION_CLASS::SIGHT_TARGET:
            case angeo::COLLISION_CLASS::TRIGGER_ACTIVATOR:
            case angeo::COLLISION_CLASS::TRIGGER_GENERAL:
            case angeo::COLLISION_CLASS::TRIGGER_SPECIAL:
                continue;
            }

            object_guid  rigid_body_guid = invalid_object_guid();
            for_each_parent_folder(folder_guid, true,
                [this, &rigid_body_guid](object_guid const  folder_guid, folder_content_type const&  fct) -> bool {
                    auto  it = fct.content.find(to_string(OBJECT_KIND::RIGID_BODY));
                    if (it == fct.content.end())
                        return true;
                    rigid_body_guid = it->second;
                    return false;
                });
            if (rigid_body_guid != invalid_object_guid())
                m_moveable_rigid_bodies.erase(rigid_body_guid.index);

            has_static_collider = true;

            break;
        }

    auto rb_it = folders.find("rigid_body");
    if (rb_it != folders.not_found())
    {
        boost::property_tree::ptree const&  data = rb_it->second.begin()->second;

        auto const  load_vector = [&data](std::string const&  key) -> vector3 {
            boost::property_tree::path const  key_path(key, '/');
            return vector3(data.get<float_32_bit>(key_path / "x", 0.0f),
                            data.get<float_32_bit>(key_path / "y", 0.0f),
                            data.get<float_32_bit>(key_path / "z", 0.0f));
        };

        auto const  load_matrix33 = [&data](std::string const&  key) -> matrix33 {
            matrix33  M;
            boost::property_tree::path const  key_path(key, '/');
            M(0,0) = data.get<float_32_bit>(key_path / "00", 0.0f);
            M(0,1) = data.get<float_32_bit>(key_path / "01", 0.0f);
            M(0,2) = data.get<float_32_bit>(key_path / "02", 0.0f);
            M(1,0) = data.get<float_32_bit>(key_path / "10", 0.0f);
            M(1,1) = data.get<float_32_bit>(key_path / "11", 0.0f);
            M(1,2) = data.get<float_32_bit>(key_path / "12", 0.0f);
            M(2,0) = data.get<float_32_bit>(key_path / "20", 0.0f);
            M(2,1) = data.get<float_32_bit>(key_path / "21", 0.0f);
            M(2,2) = data.get<float_32_bit>(key_path / "22", 0.0f);
            return M;
        };

        insert_rigid_body(
                folder_guid,
                data.get<float_32_bit>("mass_inverted", 0.0f),
                load_matrix33("inertia_tensor_inverted"),
                load_vector("linear_velocity"),
                load_vector("angular_velocity"),
                load_vector("external_linear_acceleration"),
                load_vector("external_angular_acceleration"),
                !has_static_collider
                );
    }

    for (auto folder_it = folders.begin(); folder_it != folders.end(); ++folder_it)
        if (folder_it->first == "batches")
        {
            object_guid const  batches_folder_guid = insert_folder(folder_guid, folder_it->first);
            for (auto record_it = folder_it->second.begin(); record_it != folder_it->second.end(); ++record_it)
            {
                std::string const  batch_id = record_it->second.get<std::string>("id");
                if (boost::starts_with(batch_id, gfx::get_sketch_id_prefix()))
                {
                    boost::property_tree::ptree  props;
                    gfx::read_sketch_info_from_id(batch_id, props);
                    vector3  box_half_sizes_along_axes;
                    float_32_bit  capsule_half_distance;
                    float_32_bit  capsule_thickness;
                    float_32_bit  sphere_radius;
                    natural_8_bit  num_lines;
                    vector4  colour;
                    gfx::FOG_TYPE  fog_type;
                    bool wireframe;
                    if (gfx::parse_box_info_from_id(props, box_half_sizes_along_axes, colour, fog_type, wireframe))
                        if (wireframe)
                            insert_batch_wireframe_box(batches_folder_guid, record_it->first, box_half_sizes_along_axes, colour);
                        else
                            insert_batch_solid_box(batches_folder_guid, record_it->first, box_half_sizes_along_axes, colour);
                    else if (gfx::parse_capsule_info_from_id(props, capsule_half_distance, capsule_thickness, num_lines, colour, fog_type, wireframe))
                        if (wireframe)
                            insert_batch_wireframe_capsule(batches_folder_guid, record_it->first, capsule_half_distance,
                                                           capsule_thickness, num_lines, colour);
                        else
                            insert_batch_solid_capsule(batches_folder_guid, record_it->first, capsule_half_distance,
                                                       capsule_thickness, num_lines, colour);
                    else if (gfx::parse_sphere_info_from_id(props, sphere_radius, num_lines, colour, fog_type, wireframe))
                        if (wireframe)
                            insert_batch_wireframe_sphere(batches_folder_guid, record_it->first, sphere_radius, num_lines, colour);
                        else
                            insert_batch_solid_sphere(batches_folder_guid, record_it->first, sphere_radius, num_lines, colour);
                    else { UNREACHABLE(); }
                }
                else
                    load_batch(
                            batches_folder_guid,
                            to_string(OBJECT_KIND::BATCH) + record_it->first,
                            record_it->second.get<std::string>("id"),
                            props.effects->at(record_it->second.get<std::string>("effects")),
                            record_it->second.get<std::string>("skin")
                            );
            }
        }
        else if (folder_it->first == "collider")
        {
            boost::property_tree::ptree const&  data = folder_it->second.begin()->second;
            angeo::COLLISION_SHAPE_TYPE const  shape_type = angeo::as_collision_shape_type(data.get<std::string>("shape_type"));
            if (shape_type == angeo::COLLISION_SHAPE_TYPE::BOX)
                insert_collider_box(
                        folder_guid, to_string(OBJECT_KIND::COLLIDER) + ".box",
                        vector3(data.get<float_32_bit>("half_size_along_x"),
                                data.get<float_32_bit>("half_size_along_y"),
                                data.get<float_32_bit>("half_size_along_z")),
                        angeo::read_collison_material_from_string(data.get<std::string>("material")),
                        angeo::read_collison_class_from_string(data.get<std::string>("collision_class"))
                        );
            else if (shape_type == angeo::COLLISION_SHAPE_TYPE::CAPSULE)
                insert_collider_capsule(
                        folder_guid, to_string(OBJECT_KIND::COLLIDER) + ".capsule",
                        data.get<float_32_bit>("half_distance_between_end_points"),
                        data.get<float_32_bit>("thickness_from_central_line"),
                        angeo::read_collison_material_from_string(data.get<std::string>("material")),
                        angeo::read_collison_class_from_string(data.get<std::string>("collision_class"))
                        );
            else if (shape_type == angeo::COLLISION_SHAPE_TYPE::SPHERE)
                insert_collider_sphere(
                        folder_guid, to_string(OBJECT_KIND::COLLIDER) + ".sphere",
                        data.get<float_32_bit>("radius"),
                        angeo::read_collison_material_from_string(data.get<std::string>("material")),
                        angeo::read_collison_class_from_string(data.get<std::string>("collision_class"))
                        );
            else if (shape_type == angeo::COLLISION_SHAPE_TYPE::TRIANGLE)
            {
                boost::filesystem::path const  buffers_dir = data.get<std::string>("buffers_directory");
                gfx::buffer  vertex_buffer(buffers_dir / "vertices.txt", std::numeric_limits<async::load_priority_type>::max());
                gfx::buffer  index_buffer(buffers_dir / "indices.txt", std::numeric_limits<async::load_priority_type>::max());
                if (!vertex_buffer.wait_till_load_is_finished())
                    throw std::runtime_error("Load of file 'vertices.txt' under directory '" + buffers_dir.string() + "' for 'triangle mesh' collider.");
                if (!index_buffer.wait_till_load_is_finished())
                    throw std::runtime_error("Load of file 'indices.txt' under directory '" + buffers_dir.string() + "' for 'triangle mesh' collider.");

                struct  collider_triangle_mesh_vertex_getter
                {
                    collider_triangle_mesh_vertex_getter(gfx::buffer const  vertex_buffer_, gfx::buffer const  index_buffer_)
                        : vertex_buffer(vertex_buffer_)
                        , index_buffer(index_buffer_)
                    {
                        ASSUMPTION(vertex_buffer.loaded_successfully() && index_buffer.loaded_successfully());
                        ASSUMPTION(
                            vertex_buffer.num_bytes_per_component() == sizeof(float_32_bit) &&
                            vertex_buffer.num_components_per_primitive() == 3U &&
                            vertex_buffer.has_integral_components() == false
                            );
                        ASSUMPTION(
                            index_buffer.num_bytes_per_component() == sizeof(natural_32_bit) &&
                            index_buffer.num_components_per_primitive() == 3U &&
                            index_buffer.has_integral_components() == true
                            );
                    }

                    vector3  operator()(natural_32_bit const  triangle_index, natural_8_bit const  vertex_index) const
                    {
                        return vector3(((float_32_bit const*)vertex_buffer.data().data()) + 3U * read_index_buffer(triangle_index, vertex_index));
                    }

                    natural_32_bit  read_index_buffer(natural_32_bit const  triangle_index, natural_8_bit const  vertex_index) const
                    {
                        return *(((natural_32_bit const*)index_buffer.data().data()) + 3U * triangle_index + vertex_index);
                    }

                    gfx::buffer  get_vertex_buffer() const { return vertex_buffer; }
                    gfx::buffer  get_index_buffer() const { return index_buffer; }

                private:
                    gfx::buffer  vertex_buffer;
                    gfx::buffer  index_buffer;
                };

                std::vector<object_guid>  aux;
                insert_collider_triangle_mesh(
                        folder_guid, to_string(OBJECT_KIND::COLLIDER) + ".triangle.",
                        index_buffer.num_primitives(),
                        collider_triangle_mesh_vertex_getter(vertex_buffer, index_buffer),
                        angeo::read_collison_material_from_string(data.get<std::string>("material")),
                        angeo::read_collison_class_from_string(data.get<std::string>("collision_class")),
                        aux
                        );
            }
            else
            {
                NOT_IMPLEMENTED_YET();
            }
        }

    boost::property_tree::ptree const&  children = props.hierarchy->find("children")->second;
    for (auto child_it = children.begin(); child_it != children.end(); ++child_it)
        import_gfxtuner_scene_node({ &child_it->second, props.effects }, insert_folder(folder_guid, child_it->first));
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
        folder_content(batch_guid).content.count(elem.element_name) == 1UL &&
        folder_content(batch_guid).content.find(elem.element_name)->second == batch_guid
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


angeo::COLLISION_SHAPE_TYPE  simulation_context::collider_shape_type(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return angeo::get_shape_type(m_colliders.at(collider_guid.index).id);
}


vector3 const&  simulation_context::collider_box_half_sizes_along_axes(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_box_half_sizes_along_axes(m_colliders.at(collider_guid.index).id);
}


float_32_bit  simulation_context::collider_capsule_half_distance_between_end_points(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_capsule_half_distance_between_end_points(m_colliders.at(collider_guid.index).id);
}


float_32_bit  simulation_context::collider_capsule_thickness_from_central_line(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_capsule_thickness_from_central_line(m_colliders.at(collider_guid.index).id);
}


float_32_bit  simulation_context::collider_sphere_radius(object_guid const  collider_guid) const
{
    ASSUMPTION(is_valid_collider_guid(collider_guid));
    return m_collision_scene_ptr->get_sphere_radius(m_colliders.at(collider_guid.index).id);
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


void  simulation_context::insert_colliders(
        object_guid const  under_folder_guid,
        std::string const&  name,
        std::function<void(matrix44 const&, bool, std::vector<angeo::collision_object_id>&)> const&  coids_builder,
        std::vector<object_guid>&  output_collider_guids
        )
{
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
                return false;
            }
            it = fct.content.find(to_string(OBJECT_KIND::SENSOR));
            if (it != fct.content.end())
            {
                owner_guid = it->second;
                return rigid_body_guid == invalid_object_guid();
            }
            it = fct.content.find(to_string(OBJECT_KIND::ACTIVATOR));
            if (it != fct.content.end())
            {
                owner_guid = it->second;
                return rigid_body_guid == invalid_object_guid();
            }
            return true;
        });

    bool const  is_moveable = rigid_body_guid != invalid_object_guid() && is_rigid_body_moveable(rigid_body_guid);

    std::vector<angeo::collision_object_id> coids;
    coids_builder(frame_world_matrix(frame_guid), is_moveable, coids);
    ASSUMPTION(!coids.empty());

    for (natural_32_bit  i = 0U, n = (natural_32_bit)coids.size(); i < n; ++i)
    {
        angeo::collision_object_id  coid = coids.at(i);

        std::string const  coid_name = name + (n == 1U ? "": std::to_string(i));

        ASSUMPTION(folder_content(under_folder_guid).content.count(coid_name) == 0UL);

        object_guid  owner_of_collider_guid = owner_guid;
        switch (m_collision_scene_ptr->get_collision_class(coid))
        {
        case angeo::COLLISION_CLASS::RAY_CAST_SIGHT:
        case angeo::COLLISION_CLASS::SIGHT_TARGET:
        case angeo::COLLISION_CLASS::TRIGGER_ACTIVATOR:
        case angeo::COLLISION_CLASS::TRIGGER_GENERAL:
        case angeo::COLLISION_CLASS::TRIGGER_SPECIAL:
            if (owner_of_collider_guid == rigid_body_guid)
                owner_of_collider_guid = invalid_object_guid();
            break;
        default: break;
        }

        object_guid const  collider_guid = {
                OBJECT_KIND::COLLIDER,
                m_colliders.insert({ coid, under_folder_guid.index, coid_name, frame_guid, owner_of_collider_guid })
                };

        m_coids_to_guids.insert({ coid, collider_guid });

        m_folders.at(under_folder_guid.index).content.insert({ coid_name, collider_guid });

        if (owner_of_collider_guid != invalid_object_guid())
            switch (owner_of_collider_guid.kind)
            {
            case OBJECT_KIND::RIGID_BODY: m_rigid_bodies.at(owner_guid.index).colliders.push_back(collider_guid); break;
            // TODO:
            //case OBJECT_KIND::SENSOR: m_sensors.at(owner_guid.index).colliders.push_back(collider_guid); break;
            //case OBJECT_KIND::ACTIVATOR: m_activators.at(owner_guid.index).colliders.push_back(collider_guid); break;
            default: UNREACHABLE(); break;
            }

        if (is_moveable)
            m_moveable_colliders.insert(collider_guid.index);

        output_collider_guids.push_back(collider_guid);
    }
}


object_guid  simulation_context::insert_collider(
        object_guid const  under_folder_guid,
        std::string const&  name,
        std::function<angeo::collision_object_id(matrix44 const&, bool)> const&  coid_builder)
{
    std::vector<object_guid>  collider_guids;
    insert_colliders(
            under_folder_guid,
            name,
            [this, &coid_builder](matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
                coids.push_back(coid_builder(W, is_dynamic));
                },
            collider_guids);
    return collider_guids.front();
}


object_guid  simulation_context::insert_collider_box(
        object_guid const  under_folder_guid, std::string const&  name,
        vector3 const&  half_sizes_along_axes,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class
        )
{
    return insert_collider(under_folder_guid, name,
        [this, &half_sizes_along_axes, material, collision_class](matrix44 const&  W, bool const  is_dynamic) {
            return m_collision_scene_ptr->insert_box(half_sizes_along_axes, W, material, collision_class, is_dynamic);
        });
}


object_guid  simulation_context::insert_collider_capsule(
        object_guid const  under_folder_guid, std::string const&  name,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class
        )
{
    return insert_collider(under_folder_guid, name,
        [this, half_distance_between_end_points, thickness_from_central_line, material, collision_class]
            (matrix44 const&  W, bool const  is_dynamic) {
            return m_collision_scene_ptr->insert_capsule(half_distance_between_end_points, thickness_from_central_line,
                                                         W, material, collision_class, is_dynamic);
        });
}


object_guid  simulation_context::insert_collider_sphere(
        object_guid const  under_folder_guid, std::string const&  name,
        float_32_bit const  radius,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class
        )
{
    return insert_collider(under_folder_guid, name,
        [this, radius, material, collision_class](matrix44 const&  W, bool const  is_dynamic) {
            return m_collision_scene_ptr->insert_sphere(radius, W, material, collision_class, is_dynamic);
        });
}


void  simulation_context::insert_collider_triangle_mesh(
        object_guid const  under_folder_guid, std::string const&  name_prefix,
        natural_32_bit const  num_triangles,
        std::function<vector3(natural_32_bit, natural_8_bit)> const&  getter_of_end_points_in_model_space,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        std::vector<object_guid>&  output_guids_of_individual_triangles
        )
{
    insert_colliders(
            under_folder_guid,
            name_prefix,
            [this, num_triangles, &getter_of_end_points_in_model_space, material, collision_class]
                (matrix44 const&  W, bool const  is_dynamic, std::vector<angeo::collision_object_id>&  coids) {
                m_collision_scene_ptr->insert_triangle_mesh(num_triangles, getter_of_end_points_in_model_space,
                                                            W, material, collision_class, is_dynamic, coids);
                },
            output_guids_of_individual_triangles);
}


void  simulation_context::erase_collider(object_guid const  collider_guid)
{
    ASSUMPTION(
        is_valid_collider_guid(collider_guid) &&
        folder_content(folder_of_collider(collider_guid)).content.count(m_colliders.at(collider_guid.index).element_name) != 0UL
        );

    auto const&  elem = m_colliders.at(collider_guid.index);
    m_collision_scene_ptr->erase_object(elem.id);
    m_folders.at(elem.folder_index).content.erase(elem.element_name);
    m_coids_to_guids.erase(elem.id);
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


object_guid  simulation_context::insert_rigid_body(
        object_guid const  under_folder_guid,
        float_32_bit const  mass_inverted,
        matrix33 const&  inertia_tensor_inverted,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  external_linear_acceleration,
        vector3 const&  external_angular_acceleration,
        bool const  is_moveable
        )
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
            mass_inverted,
            inertia_tensor_inverted,
            linear_velocity,
            angular_velocity,
            external_linear_acceleration,
            external_angular_acceleration
            );

    object_guid const  rigid_body_guid = {
            OBJECT_KIND::RIGID_BODY,
            m_rigid_bodies.insert({ rbid, under_folder_guid.index, frame_guid })
            };

    m_rbids_to_guids.insert({ rbid, rigid_body_guid });

    m_folders.at(under_folder_guid.index).content.insert({ to_string(OBJECT_KIND::RIGID_BODY), rigid_body_guid });

    if (is_moveable)
        m_moveable_rigid_bodies.insert(rigid_body_guid.index);

    return rigid_body_guid;
}


void  simulation_context::erase_rigid_body(object_guid const  rigid_body_guid)
{
    ASSUMPTION(
        is_valid_rigid_body_guid(rigid_body_guid) &&
        folder_content(folder_of_rigid_body(rigid_body_guid)).content.count(m_rigid_bodies.at(rigid_body_guid.index).element_name) != 0UL
        );

    auto const&  elem = m_rigid_bodies.at(rigid_body_guid.index);
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
