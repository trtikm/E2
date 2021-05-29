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
}


async::key_type  imported_scene::key_from_path(boost::filesystem::path const&  path)
{
    return { "com::detail::imported_scene", boost::filesystem::absolute(path).string() };
}


struct  delayed_import_tasks
{
    struct  guid_and_ptree
    {
        object_guid  guid;
        boost::property_tree::ptree const*  ptree;
    };

    std::vector<guid_and_ptree>  request_infos;
    std::vector<guid_and_ptree>  sensor_triggers;
};


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


static void  read_import_props(
        import_scene_props&  props,
        simulation_context const&  ctx,
        object_guid const  folder_guid,
        boost::property_tree::ptree const&  ptree
        )
{
    props.import_dir = ctx.get_import_root_dir() + ptree.get<std::string>("import_dir");
    props.folder_guid = ptree.count("under_folder") == 0UL ?
            ctx.root_folder() :
            ctx.from_relative_path(folder_guid, ptree.get<std::string>("under_folder"));
    props.relocation_frame_guid = ptree.count("relocation_frame") == 0UL ?
            invalid_object_guid() :
            ctx.from_relative_path(folder_guid, ptree.get<std::string>("relocation_frame"));
    if (ptree.count("origin") == 0UL && ptree.count("orientation") == 0UL)
        props.relocation_frame_ptr = nullptr;
    else
    {
        vector3 const  origin = ptree.count("origin") == 0UL ? vector3_zero() : import_vector3(ptree.get_child("origin"));
        quaternion const  orientation = ptree.count("orientation") == 0UL ? quaternion_identity() :
                                                                            import_quaternion(ptree.get_child("orientation"));
        props.relocation_frame_ptr = angeo::coordinate_system::create(origin, orientation);
    }
    ASSUMPTION(props.relocation_frame_guid == invalid_object_guid() || props.relocation_frame_ptr == nullptr);
    props.store_in_cache = ptree.count("cache_imported_scene") != 0U ? ptree.get<bool>("cache_imported_scene") : true;
    props.apply_linear_velocity = ptree.count("linear_velocity") != 0U;
    props.apply_angular_velocity = ptree.count("angular_velocity") != 0U;
    if (props.apply_linear_velocity)
        props.linear_velocity = import_vector3(ptree.get_child("linear_velocity"));
    if (props.apply_angular_velocity)
        props.angular_velocity = import_vector3(ptree.get_child("angular_velocity"));
    props.motion_frame_guid = ptree.count("motion_frame") == 0UL ?
            invalid_object_guid() :
            ctx.from_relative_path(folder_guid, ptree.get<std::string>("motion_frame"));
    if (props.apply_linear_velocity && props.motion_frame_guid != invalid_object_guid() &&
            ptree.count("add_motion_frame_velocity") != 0U)
        props.add_motion_frame_velocity = ptree.get<bool>("add_motion_frame_velocity");
}


static void  import_frame(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        object_guid const  relocation_frame_guid,
        angeo::coordinate_system_const_ptr const  relocation_frame_ptr
        )
{
    object_guid const  frame_guid = ctx.insert_frame(folder_guid);
    if (relocation_frame_guid != invalid_object_guid())
        ctx.frame_relocate_relative_to_parent(frame_guid, relocation_frame_guid);
    else if (relocation_frame_ptr != nullptr)
    {
        angeo::coordinate_system  loaded_frame {
                import_vector3(hierarchy.find("origin")->second),
                import_quaternion(hierarchy.find("orientation")->second)
                };
        angeo::coordinate_system  relocated_frame;
        angeo::from_coordinate_system(*relocation_frame_ptr, loaded_frame, relocated_frame);
        ctx.frame_relocate(frame_guid, relocated_frame, true);
    }
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
        std::string const&  name
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
            batch_guid = ctx.load_batch(
                    folder_guid,
                    name,
                    hierarchy.get<std::string>("path"),
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
                angeo::read_collison_class_from_string(hierarchy.get<std::string>("collision_class")),
                (natural_8_bit)std::stoul(hierarchy.get<std::string>("collision_scene_index"))
                );
    else if (collider_kind == "CAPSULE")
        ctx.insert_collider_capsule(
                folder_guid,
                name,
                hierarchy.get<float_32_bit>("half_distance_between_end_points"),
                hierarchy.get<float_32_bit>("thickness_from_central_line"),
                angeo::read_collison_material_from_string(hierarchy.get<std::string>("collision_material")),
                angeo::read_collison_class_from_string(hierarchy.get<std::string>("collision_class")),
                (natural_8_bit)std::stoul(hierarchy.get<std::string>("collision_scene_index"))
                );
    else if (collider_kind == "SPHERE")
        ctx.insert_collider_sphere(
                folder_guid,
                name,
                hierarchy.get<float_32_bit>("radius"),
                angeo::read_collison_material_from_string(hierarchy.get<std::string>("collision_material")),
                angeo::read_collison_class_from_string(hierarchy.get<std::string>("collision_class")),
                (natural_8_bit)std::stoul(hierarchy.get<std::string>("collision_scene_index"))
                );
    else if (collider_kind == "TRIANGLE_MESH")
    {
        boost::filesystem::path const  buffers_dir =
                boost::filesystem::path(ctx.get_data_root_dir()) / "mesh" / hierarchy.get<std::string>("path");
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
                angeo::read_collison_class_from_string(hierarchy.get<std::string>("collision_class")),
                (natural_8_bit)std::stoul(hierarchy.get<std::string>("collision_scene_index"))
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
    bool const  is_moveable = hierarchy.get<bool>("is_moveable");
    ctx.insert_rigid_body(
            folder_guid,
            is_moveable,
            is_moveable ? import_vector3(hierarchy.get_child("linear_velocity")) : vector3_zero(),
            is_moveable ? import_vector3(hierarchy.get_child("angular_velocity")) : vector3_zero(),
            is_moveable ? import_vector3(hierarchy.get_child("external_linear_acceleration")) : vector3_zero(),
            is_moveable ? import_vector3(hierarchy.get_child("external_angular_acceleration")) : vector3_zero()
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
    else if (kind == "IMPORT_SCENE")
    {
        import_scene_props  props;
        read_import_props(props, ctx, owner_guid, hierarchy);
        ctx.insert_request_info_import_scene({ owner_guid, to_device_event(hierarchy.get<std::string>("event")) }, props);
    }
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
    else if (kind == "MUL_LINEAR_VELOCITY")
        ctx.insert_request_info_rigid_body_mul_linear_velocity(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("rigid_body")),
                import_vector3(hierarchy.get_child("linear_velocity_scale"))
                );
    else if (kind == "MUL_ANGULAR_VELOCITY")
        ctx.insert_request_info_rigid_body_mul_angular_velocity(
                { owner_guid, to_device_event(hierarchy.get<std::string>("event")) },
                ctx.from_relative_path(owner_guid, hierarchy.get<std::string>("rigid_body")),
                import_vector3(hierarchy.get_child("angular_velocity_scale"))
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
        delayed_import_tasks&  delayed_tasks,
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
    boost::property_tree::ptree const&  infos = hierarchy.find("request_infos")->second;
    for (auto  infos_it = infos.begin(); infos_it != infos.end(); ++infos_it)
        delayed_tasks.request_infos.push_back({ timer_guid, &infos_it->second });
}


static void  import_sensor(
        simulation_context&  ctx,
        delayed_import_tasks&  delayed_tasks,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        std::string const&  name
        )
{
    object_guid const  collider_guid = ctx.from_relative_path(folder_guid, hierarchy.get<std::string>("collider"));
    ASSUMPTION(ctx.is_valid_collider_guid(collider_guid));

    object_guid const  sensor_guid = ctx.insert_sensor(
            folder_guid,
            name,
            collider_guid,
            {},
            hierarchy.get<natural_32_bit>("target_enable_level"),
            hierarchy.get<natural_32_bit>("current_enable_level")
            );
    ctx.request_enable_collider(collider_guid, ctx.is_sensor_enabled(sensor_guid));

    delayed_tasks.sensor_triggers.push_back({ sensor_guid, &hierarchy });

    boost::property_tree::ptree const&  infos = hierarchy.find("request_infos")->second;
    for (auto  infos_it = infos.begin(); infos_it != infos.end(); ++infos_it)
        delayed_tasks.request_infos.push_back({ sensor_guid, &infos_it->second });
}


static void  import_agent(
        simulation_context&  ctx,
        boost::property_tree::ptree const&  hierarchy,
        object_guid const  folder_guid,
        std::string const&  name
        )
{
    ASSUMPTION(name == to_string(OBJECT_KIND::AGENT));

    std::vector<std::pair<std::string, gfx::batch> >  skeleton_attached_batches;
    {
        std::string const  skin = hierarchy.get<std::string>("skeleton_batch_skin");
        boost::filesystem::path const  batch_path =
            boost::filesystem::path(ctx.get_data_root_dir()) / "batch" / hierarchy.get<std::string>("skeleton_batch_disk_path")
            ;
        auto const  add_batch_for_path = [&ctx, &skin, &skeleton_attached_batches](boost::filesystem::path const&  p) -> void {
            gfx::batch const  agent_batch(p, gfx::default_effects_config(), skin);
            ctx.insert_imported_batch_to_cache(agent_batch);
            skeleton_attached_batches.push_back({ "BATCH." + p.filename().replace_extension("").string(), agent_batch});
        };
        if (boost::to_lower_copy(boost::filesystem::extension(batch_path)) == ".txt")
            add_batch_for_path(batch_path);
        else
            for (boost::filesystem::directory_entry const& entry : boost::filesystem::directory_iterator(batch_path))
                if (boost::to_lower_copy(entry.path().filename().extension().string()) == ".txt")
                    add_batch_for_path(entry.path());
    }

    ctx.request_late_insert_agent(
            folder_guid,
            ai::agent_config(
                    boost::filesystem::path(ctx.get_data_root_dir()) / "ai" / hierarchy.get<std::string>("kind"),
                    1U,
                    nullptr
                    ),
            skeleton_attached_batches
            );
}


static void  import_under_folder(
        simulation_context&  ctx,
        delayed_import_tasks&  delayed_tasks,
        object_guid const  folder_guid,
        boost::property_tree::ptree const&  hierarchy,
        object_guid  relocation_frame_guid,
        angeo::coordinate_system_const_ptr  relocation_frame_ptr
        )
{
    auto const  content = hierarchy.find("content");
    if (content != hierarchy.not_found())
    {
        std::unordered_map<OBJECT_KIND, std::vector<boost::property_tree::ptree::const_iterator> >  load_tasks;
        for (auto content_it = content->second.begin(); content_it != content->second.end(); ++content_it)
        {
            std::string const  onject_kind = content_it->second.get<std::string>("object_kind");
            if (onject_kind == "FRAME")
                load_tasks[OBJECT_KIND::FRAME].push_back(content_it);
            else if (onject_kind == "BATCH")
                load_tasks[OBJECT_KIND::BATCH].push_back(content_it);
            else if (onject_kind == "COLLIDER")
                load_tasks[OBJECT_KIND::COLLIDER].push_back(content_it);
            else if (onject_kind == "RIGID_BODY")
                load_tasks[OBJECT_KIND::RIGID_BODY].push_back(content_it);
            else if (onject_kind == "TIMER")
                load_tasks[OBJECT_KIND::TIMER].push_back(content_it);
            else if (onject_kind == "SENSOR")
                load_tasks[OBJECT_KIND::SENSOR].push_back(content_it);
            else if (onject_kind == "AGENT")
                load_tasks[OBJECT_KIND::AGENT].push_back(content_it);
            else
            { UNREACHABLE(); }
        }
        for (auto  content_it : load_tasks[OBJECT_KIND::FRAME])
            import_frame(ctx, content_it->second, folder_guid, relocation_frame_guid, relocation_frame_ptr);
        for (auto  content_it : load_tasks[OBJECT_KIND::BATCH])
            import_batch(ctx, content_it->second, folder_guid, content_it->first);
        for (auto  content_it : load_tasks[OBJECT_KIND::RIGID_BODY])
            import_rigid_body(ctx, content_it->second, folder_guid, content_it->first);
        for (auto  content_it : load_tasks[OBJECT_KIND::COLLIDER])
            import_collider(ctx, content_it->second, folder_guid, content_it->first);
        for (auto  content_it : load_tasks[OBJECT_KIND::TIMER])
            import_timer(ctx, delayed_tasks, content_it->second, folder_guid, content_it->first);
        for (auto  content_it : load_tasks[OBJECT_KIND::SENSOR])
            import_sensor(ctx, delayed_tasks, content_it->second, folder_guid, content_it->first);
        for (auto  content_it : load_tasks[OBJECT_KIND::AGENT])
            import_agent(ctx, content_it->second, folder_guid, content_it->first);

        if (!load_tasks[OBJECT_KIND::FRAME].empty())
        {
            relocation_frame_guid = invalid_object_guid();
            relocation_frame_ptr = nullptr;
        }
    }

    auto const  folders = hierarchy.find("folders");
    if (folders != hierarchy.not_found())
        for (auto folder_it = folders->second.begin(); folder_it != folders->second.end(); ++folder_it)
            import_under_folder(
                    ctx,
                    delayed_tasks,
                    ctx.insert_folder(folder_guid, folder_it->first, false),
                    folder_it->second,
                    relocation_frame_guid,
                    relocation_frame_ptr
                    );

    auto const  imports = hierarchy.find("imports");
    if (imports != hierarchy.not_found())
        for (auto import_it = imports->second.begin(); import_it != imports->second.end(); ++import_it)
        {
            import_scene_props  props;
            read_import_props(props, ctx, folder_guid, import_it->second);
            ctx.request_late_import_scene_from_directory(props);
        }
}


void  apply_initial_velocities_to_imported_rigid_bodies(
        simulation_context&  ctx,
        object_guid const  folder_guid,
        import_scene_props const&  props
        )
{
    if (!props.apply_linear_velocity && !props.apply_angular_velocity)
        return;

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
        if (props.motion_frame_guid == invalid_object_guid())
        {
            if (props.apply_linear_velocity)
                lin_vel = props.linear_velocity;
            if (props.apply_angular_velocity)
                ang_vel = props.angular_velocity;
        }
        else
        {
            if (props.apply_linear_velocity)
            {
                lin_vel = transform_vector(props.linear_velocity, ctx.frame_world_matrix(props.motion_frame_guid));
                if (props.add_motion_frame_velocity)
                {
                    object_guid  rb_guid = invalid_object_guid();
                    ctx.for_each_parent_folder(ctx.folder_of_frame(props.motion_frame_guid), true,
                            [&rb_guid](object_guid, simulation_context::folder_content_type const&  fct) {
                                auto const  it = fct.content.find(to_string(OBJECT_KIND::RIGID_BODY));
                                if (it != fct.content.end())
                                {
                                    rb_guid = it->second;
                                    return false;
                                }
                                return true;
                        });
                    if (ctx.is_valid_rigid_body_guid(rb_guid))
                        lin_vel += ctx.compute_velocity_of_point_of_rigid_body(
                                        rb_guid,
                                        ctx.frame_coord_system_in_world_space(props.motion_frame_guid).origin()
                                        );
                }
            }
            if (props.apply_angular_velocity)
                ang_vel = transform_vector(props.angular_velocity, ctx.frame_world_matrix(props.motion_frame_guid));
        }
        for (object_guid rb_guid : rigid_body_guids)
        {
            if (props.apply_linear_velocity)
                ctx.set_rigid_body_linear_velocity(rb_guid, lin_vel);
            if (props.apply_angular_velocity)
                ctx.set_rigid_body_angular_velocity(rb_guid, ang_vel);
        }
    }
}


extern void  import_gfxtuner_scene(
        simulation_context&  ctx,
        imported_scene const  scene,
        import_scene_props const&  props
        );


void  import_scene(simulation_context&  ctx, imported_scene const  scene, import_scene_props const&  props)
{
    if (scene.hierarchy().count("@pivot") != 0UL)
    {
        import_gfxtuner_scene(ctx, scene, props);
        return;
    }

    delayed_import_tasks  delayed_tasks;
    auto const  folders = scene.hierarchy().find("folders");
    if (folders != scene.hierarchy().not_found())
        for (auto it = folders->second.begin(); it != folders->second.end(); ++it)
        {
            object_guid const  folder_guid = ctx.insert_folder(props.folder_guid, it->first, true);
            import_under_folder(ctx, delayed_tasks, folder_guid, it->second, props.relocation_frame_guid, props.relocation_frame_ptr);
            apply_initial_velocities_to_imported_rigid_bodies(ctx, folder_guid, props);
        }
    for (auto request_infos_it = delayed_tasks.request_infos.begin(); request_infos_it != delayed_tasks.request_infos.end(); ++request_infos_it)
        import_reques_info(ctx, *request_infos_it->ptree, request_infos_it->guid);
    for (auto trigger_it = delayed_tasks.sensor_triggers.begin(); trigger_it != delayed_tasks.sensor_triggers.end(); ++trigger_it)
    {
        auto const  it = trigger_it->ptree->find("trigger_collider");
        if (it != trigger_it->ptree->not_found())
            ctx.insert_trigger_collider_to_sensor(
                    trigger_it->guid,
                    ctx.from_relative_path(trigger_it->guid, it->second.get_value<std::string>())
                    );
    }

    ctx.process_rigid_bodies_with_invalidated_shape();

    auto const  imports = scene.hierarchy().find("imports");
    if (imports != scene.hierarchy().not_found())
        for (auto import_it = imports->second.begin(); import_it != imports->second.end(); ++import_it)
        {
            import_scene_props  import_props;
            read_import_props(import_props, ctx, ctx.root_folder(), import_it->second);
            ctx.request_late_import_scene_from_directory(import_props);
        }
}


}}
