#include <com/detail/import_scene.hpp>
#include <com/simulation_context.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

namespace com { namespace detail {


imported_scene_data::imported_scene_data(async::finalise_load_on_destroy_ptr const  finaliser)
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
            m_effects.insert({it->first, it->second});
    }
}


async::key_type  imported_scene::key_from_path(boost::filesystem::path const&  path)
{
    return { "com::detail::imported_scene", boost::filesystem::absolute(path).string() };
}


gfx::effects_config  import_effects_config(boost::property_tree::ptree const&  ptree)
{
    gfx::effects_config::light_types  light_types;
    for (auto const& lt_and_tree : ptree.get_child("light_types"))
        light_types.insert((gfx::LIGHT_TYPE)lt_and_tree.second.get_value<int>());
    gfx::effects_config::lighting_data_types  lighting_data_types;
    for (auto const& ldt_and_tree : ptree.get_child("lighting_data_types"))
        lighting_data_types.insert({
            (gfx::LIGHTING_DATA_TYPE)std::atoi(ldt_and_tree.first.c_str()),
            (gfx::SHADER_DATA_INPUT_TYPE)ldt_and_tree.second.get_value<int>()
        });
    gfx::effects_config::shader_output_types  shader_output_types;
    for (auto const& sot_and_tree : ptree.get_child("shader_output_types"))
        shader_output_types.insert((gfx::SHADER_DATA_OUTPUT_TYPE)sot_and_tree.second.get_value<int>());

    return gfx::effects_config(
            nullptr,
            light_types,
            lighting_data_types,
            (gfx::SHADER_PROGRAM_TYPE)ptree.get<int>("lighting_algo_location"),
            shader_output_types,
            (gfx::FOG_TYPE)ptree.get<int>("fog_type"),
            (gfx::SHADER_PROGRAM_TYPE)ptree.get<int>("fog_algo_location")
            );
}


static vector3  import_vector3(boost::property_tree::ptree const&  hierarchy)
{
    return vector3(hierarchy.get<scalar>("x"), hierarchy.get<scalar>("y"), hierarchy.get<scalar>("z"));
}


static quaternion  import_quaternion(boost::property_tree::ptree const&  hierarchy)
{
    return make_quaternion_xyzw(
                hierarchy.get<scalar>("x"),
                hierarchy.get<scalar>("y"),
                hierarchy.get<scalar>("z"),
                hierarchy.get<scalar>("w")
                );
}


static vector4  import_colour(boost::property_tree::ptree const&  hierarchy)
{
    return vector4(hierarchy.get<scalar>("r"), hierarchy.get<scalar>("g"), hierarchy.get<scalar>("b"), hierarchy.get<scalar>("a"));
}


static void  import_frame(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        object_guid const  relocation_frame_guid
        )
{
    object_guid const  frame_guid = ctx.insert_frame(folder_guid);
    if (relocation_frame_guid != invalid_object_guid())
        ctx.frame_relocate_relative_to_parent(frame_guid, relocation_frame_guid);
    else
        ctx.frame_relocate(
                frame_guid,
                import_vector3(hierarchy.find("origin")->second),
                import_quaternion(hierarchy.find("orientation")->second)
                );
}


static void  import_batch(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        std::string const&  name,
        std::unordered_map<std::string, boost::property_tree::ptree> const&  effects
        )
{
    object_guid  batch_guid;
    {
        std::string const  batch_kind = hierarchy.get<std::string>("batch_kind");
        if (batch_kind == "GENERIC_BOX")
            batch_guid = ctx.insert_batch_solid_box(
                    folder_guid,
                    name,
                    import_vector3(hierarchy.get_child("half_sizes_along_axes")),
                    import_colour(hierarchy.get_child("colour"))
                    );
        else if (batch_kind == "GENERIC_CAPSULE")
            batch_guid = ctx.insert_batch_solid_capsule(
                    folder_guid,
                    name,
                    hierarchy.get<float_32_bit>("half_distance_between_end_points"),
                    hierarchy.get<float_32_bit>("thickness_from_central_line"),
                    hierarchy.get<natural_32_bit>("num_lines_per_quarter_of_circle"),
                    import_colour(hierarchy.get_child("colour"))
                    );
        else if (batch_kind == "GENERIC_SPHERE")
            batch_guid = ctx.insert_batch_solid_sphere(
                    folder_guid,
                    name,
                    hierarchy.get<float_32_bit>("radius"),
                    hierarchy.get<natural_32_bit>("num_lines_per_quarter_of_circle"),
                    import_colour(hierarchy.get_child("colour"))
                    );
        else if (batch_kind == "REGULAR_GFX_BATCH")
        {
            gfx::effects_config  effects_config; 
            {
                auto const  it = effects.find(hierarchy.get<std::string>("effects"));
                INVARIANT(it != effects.end());
                effects_config = import_effects_config(it->second);
                ctx.insert_imported_effects_config_to_cache(effects_config);
            }

            batch_guid = ctx.load_batch(
                    folder_guid,
                    name,
                    hierarchy.get<std::string>("id"),
                    effects_config,
                    hierarchy.get<std::string>("skin")
                    );
        }
        else
        { UNREACHABLE(); }
    }
    ctx.insert_imported_batch_to_cache(ctx.from_batch_guid_to_batch(batch_guid));
}


static void  import_collider(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        std::string const&  name
        )
{
    std::string const  collider_kind = hierarchy.get<std::string>("collider_kind");
    if (collider_kind == "BOX")
        ctx.insert_collider_box(
                folder_guid,
                name,
                import_vector3(hierarchy.get_child("half_sizes_along_axes")),
                angeo::read_collison_material_from_string(hierarchy.get<std::string>("collision_material")),
                angeo::read_collison_class_from_string(hierarchy.get<std::string>("collision_class"))
                );
    else if (collider_kind == "CAPSULE")
        ctx.insert_collider_capsule(
                folder_guid,
                name,
                hierarchy.get<float_32_bit>("half_distance_between_end_points"),
                hierarchy.get<float_32_bit>("thickness_from_central_line"),
                angeo::read_collison_material_from_string(hierarchy.get<std::string>("collision_material")),
                angeo::read_collison_class_from_string(hierarchy.get<std::string>("collision_class"))
                );
    else if (collider_kind == "SPHERE")
        ctx.insert_collider_sphere(
                folder_guid,
                name,
                hierarchy.get<float_32_bit>("radius"),
                angeo::read_collison_material_from_string(hierarchy.get<std::string>("collision_material")),
                angeo::read_collison_class_from_string(hierarchy.get<std::string>("collision_class"))
                );
    else if (collider_kind == "TRIANGLE_MESH")
    {
        boost::filesystem::path const  buffers_dir = hierarchy.get<std::string>("path");
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

        ctx.insert_collider_triangle_mesh(
                folder_guid,
                name,
                index_buffer.num_primitives(),
                collider_triangle_mesh_vertex_getter(vertex_buffer, index_buffer),
                angeo::read_collison_material_from_string(hierarchy.get<std::string>("collision_material")),
                angeo::read_collison_class_from_string(hierarchy.get<std::string>("collision_class"))
                );
    }
    else
    { NOT_IMPLEMENTED_YET(); }
}


static void  import_rigid_body(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        std::string const&  name
        )
{
    ASSUMPTION(name == "RIGID_BODY");
    ctx.insert_rigid_body(
            folder_guid,
            hierarchy.get<bool>("is_moveable"),
            import_vector3(hierarchy.get_child("linear_velocity")),
            import_vector3(hierarchy.get_child("angular_velocity")),
            import_vector3(hierarchy.get_child("external_linear_acceleration")),
            import_vector3(hierarchy.get_child("external_angular_acceleration"))
            );
}


static void  import_reques_info(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  owner_guid
        )
{
    auto const  to_device_event = [](std::string const&  name) {
        if (name == "TOUCHING") return simulation_context::DEVICE_EVENT_TYPE::TOUCHING;
        else if (name == "TOUCH_BEGIN") return simulation_context::DEVICE_EVENT_TYPE::TOUCH_BEGIN;
        else if (name == "TOUCH_END") return simulation_context::DEVICE_EVENT_TYPE::TOUCH_END;
        else if (name == "TIME_OUT") return simulation_context::DEVICE_EVENT_TYPE::TIME_OUT;
        else { UNREACHABLE(); }
    };

    std::string const  kind = hierarchy.get<std::string>("kind");
    if (kind == "INCREMENT_ENABLE_LEVEL_OF_TIMER")
        ctx.insert_request_info_increment_enable_level_of_timer(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("timer"))
                );
    else if (kind == "DECREMENT_ENABLE_LEVEL_OF_TIMER")
        ctx.insert_request_info_decrement_enable_level_of_timer(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("timer"))
                );
    else if (kind == "RESET_TIMER")
        ctx.insert_request_info_reset_timer(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("timer"))
                );
    else if (kind == "INCREMENT_ENABLE_LEVEL_OF_SENSOR")
        ctx.insert_request_info_increment_enable_level_of_sensor(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("sensor"))
                );
    else if (kind == "DECREMENT_ENABLE_LEVEL_OF_SENSOR")
        ctx.insert_request_info_decrement_enable_level_of_sensor(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("sensor"))
                );
    else if (kind == "ctx.insert_SCENE")
        ctx.insert_request_info_import_scene(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                hierarchy.get<std::string>("import_dir"),
                hierarchy.count("under_folder") == 0UL ?
                        ctx.root_folder() :
                        ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("under_folder")),
                hierarchy.count("relocation_frame") == 0UL ?
                        invalid_object_guid() :
                        ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("relocation_frame")),
                hierarchy.get<bool>("cache_imported_scene"),
                import_vector3(hierarchy.get_child("linear_velocity")),
                import_vector3(hierarchy.get_child("angular_velocity")),
                hierarchy.count("motion_frame") == 0UL ?
                        invalid_object_guid() :
                        ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("motion_frame"))
                );
    else if (kind == "ERASE_FOLDER")
        ctx.insert_request_info_erase_folder(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("erase_folder"))
                );
    else if (kind == "SET_LINEAR_VELOCITY")
        ctx.insert_request_info_rigid_body_set_linear_velocity(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("rigid_body")),
                import_vector3(hierarchy.get_child("linear_velocity"))
                );
    else if (kind == "SET_ANGULAR_VELOCITY")
        ctx.insert_request_info_rigid_body_set_angular_velocity(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("rigid_body")),
                import_vector3(hierarchy.get_child("angular_velocity"))
                );
    else if (kind == "UPDATE_RADIAL_FORCE_FIELD")
        ctx.insert_request_info_update_radial_force_field(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                hierarchy.get<float_32_bit>("multiplier"),
                hierarchy.get<float_32_bit>("exponent"),
                hierarchy.get<float_32_bit>("min_radius"),
                hierarchy.get<bool>("use_mass")
                );
    else if (kind == "UPDATE_LINEAR_FORCE_FIELD")
        ctx.insert_request_info_update_linear_force_field(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                import_vector3(hierarchy.get_child("acceleration")),
                hierarchy.get<bool>("use_mass")
                );
    else if (kind == "LEAVE_FORCE_FIELD")
        ctx.insert_request_info_leave_force_field(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) }
                );
    else
    { UNREACHABLE(); }
}


static void  import_timer(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        std::string const&  name
        )
{
    object_guid const  timer_guid = ctx.insert_timer(
            folder_guid,
            name,
            hierarchy.get<float_32_bit>("period_in_seconds"),
            hierarchy.get<natural_32_bit>("target_enable_level"),
            hierarchy.get<natural_32_bit>("current_enable_level")
            );
    boost::property_tree::ptree const&  request_infos = hierarchy.find("request_infos")->second;
    for (auto request_infos_it = request_infos.begin(); request_infos_it != request_infos.end(); ++request_infos_it)
        import_reques_info(ctx, request_infos_it->second, timer_guid);
}


static void  import_sensor(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        std::string const&  name
        )
{
    object_guid const  sensor_guid = ctx.insert_sensor(
            folder_guid,
            name,
            ctx.from_relative_path(folder_guid, hierarchy.get<std::string>("collider")),
            {},
            hierarchy.get<natural_32_bit>("target_enable_level"),
            hierarchy.get<natural_32_bit>("current_enable_level")
            );
    boost::property_tree::ptree const&  request_infos = hierarchy.find("request_infos")->second;
    for (auto request_infos_it = request_infos.begin(); request_infos_it != request_infos.end(); ++request_infos_it)
        import_reques_info(ctx, request_infos_it->second, sensor_guid);
}


static void  import_agent(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        std::string const&  name
        )
{
    NOT_IMPLEMENTED_YET();
}


static void  import_under_folder(
        simulation_context&  ctx,
        object_guid const  folder_guid,
        boost::property_tree::ptree const&  hierarchy,
        std::unordered_map<std::string, boost::property_tree::ptree> const&  effects,
        object_guid const  relocation_frame_guid
        )
{
    boost::property_tree::ptree const&  content = hierarchy.find("content")->second;
    for (auto content_it = content.begin(); content_it != content.end(); ++content_it)
    {
        std::string const  onject_kind = content_it->second.get<std::string>("object_kind");
        if (onject_kind == "FRAME")
            import_frame(ctx, content_it->second, folder_guid, relocation_frame_guid);
        else if (onject_kind == "BATCH")
            import_batch(ctx, content_it->second, folder_guid, content_it->first, effects);
        else if (onject_kind == "COLLIDER")
            import_collider(ctx, content_it->second, folder_guid, content_it->first);
        else if (onject_kind == "RIGID_BODY")
            import_rigid_body(ctx, content_it->second, folder_guid, content_it->first);
        else if (onject_kind == "TIMER")
            import_timer(ctx, content_it->second, folder_guid, content_it->first);
        else if (onject_kind == "SENSOR")
            import_sensor(ctx, content_it->second, folder_guid, content_it->first);
        else if (onject_kind == "AGENT")
            import_agent(ctx, content_it->second, folder_guid, content_it->first);
        else
        { UNREACHABLE(); }
    }

    boost::property_tree::ptree const&  folders = hierarchy.find("folders")->second;
    for (auto folder_it = folders.begin(); folder_it != folders.end(); ++folder_it)
        import_under_folder(
                ctx,
                ctx.insert_folder(folder_guid, folder_it->first),
                folder_it->second,
                effects,
                invalid_object_guid()
                );
}


void  apply_initial_velocities_to_imported_rigid_bodies(
        simulation_context&  ctx,
        object_guid const  folder_guid,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        object_guid const  motion_frame_guid
        )
{
    std::vector<object_guid>  rigid_body_guids;
    ctx.for_each_child_folder(folder_guid, true, true,
        [&rigid_body_guids](object_guid const  folder_guid, simulation_context::folder_content_type const&  fct) -> bool {
            auto  it = fct.content.find(to_string(OBJECT_KIND::RIGID_BODY));
            if (it != fct.content.end())
            {
                rigid_body_guids.push_back(it->second);
                return false;
            }
            return true;
        });
    if (!rigid_body_guids.empty())
    {
        vector3  lin_vel, ang_vel;
        if (motion_frame_guid == invalid_object_guid())
        {
            lin_vel = linear_velocity;
            ang_vel = angular_velocity;
        }
        else
        {
            lin_vel = transform_vector(linear_velocity, ctx.frame_world_matrix(motion_frame_guid));
            ang_vel = transform_vector(angular_velocity, ctx.frame_world_matrix(motion_frame_guid));
        }
        for (object_guid rb_guid : rigid_body_guids)
        {
            ctx.set_rigid_body_linear_velocity(rb_guid, lin_vel);
            ctx.set_rigid_body_angular_velocity(rb_guid, ang_vel);
        }
    }
}


std::string  generate_unique_folder_name_from(simulation_context const&  ctx, object_guid const  folder_guid, std::string  name)
{
    simulation_context::folder_content_type const&  fct = ctx.folder_content(folder_guid);
    if (fct.child_folders.count(name) != 0)
    {
        natural_32_bit  counter = 0U;
        for ( ; fct.child_folders.count(name + '.' + std::to_string(counter)) != 0U; ++counter)
            ;
        name = name + '.' + std::to_string(counter);
    }
    return name;
}


extern void  import_gfxtuner_scene(
        simulation_context&  ctx,
        imported_scene const  scene,
        object_guid const  under_folder_guid,
        object_guid const  relocation_frame_guid,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        object_guid const  motion_frame_guid
        );


void  import_scene(
        simulation_context&  ctx,
        imported_scene const  scene,
        object_guid const  under_folder_guid,
        object_guid const  relocation_frame_guid,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        object_guid const  motion_frame_guid
        )
{
    if (scene.hierarchy().count("@pivot") != 0UL)
    {
        import_gfxtuner_scene(ctx, scene, under_folder_guid, relocation_frame_guid,
                              linear_velocity, angular_velocity, motion_frame_guid);
        return;
    }

    boost::property_tree::ptree const&  folders = scene.hierarchy().find("folders")->second;
    for (auto  it = folders.begin(); it != folders.end(); ++it)
    {
        object_guid const  folder_guid =
                ctx.insert_folder(under_folder_guid, generate_unique_folder_name_from(ctx, under_folder_guid, it->first));
        import_under_folder(ctx, folder_guid, it->second, scene.effects(), relocation_frame_guid);
        apply_initial_velocities_to_imported_rigid_bodies(ctx, folder_guid, linear_velocity, angular_velocity, motion_frame_guid);
    }

    ctx.process_rigid_bodies_with_invalidated_shape();
}


}}
