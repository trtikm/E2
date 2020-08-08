#include <com/import_scene_props.hpp>
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


extern void  apply_initial_velocities_to_imported_rigid_bodies(
        simulation_context&  ctx,
        object_guid const  folder_guid,
        import_scene_props const&  props
        );


static bool  is_static_collider_in_tree(boost::property_tree::ptree const&  ptree)
{
    boost::property_tree::ptree const&  folders = ptree.find("folders")->second;
    for (auto folder_it = folders.begin(); folder_it != folders.end(); ++folder_it)
        if (folder_it->first == "collider" && !folder_it->second.begin()->second.get<bool>("is_dynamic")
                && angeo::read_collison_class_from_string(folder_it->second.begin()->second.get<std::string>("collision_class"))
                   == angeo::COLLISION_CLASS::STATIC_OBJECT)
            return true;

    boost::property_tree::ptree const&  children = ptree.find("children")->second;
    for (auto child_it = children.begin(); child_it != children.end(); ++child_it)
        if (is_static_collider_in_tree(child_it->second))
            return true;

    return false;
}


static void  import_gfxtuner_scene_node(
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
    {
        boost::property_tree::ptree const&  origin_tree = hierarchy.find("origin")->second;
        vector3 const  origin = vector3(origin_tree.get<scalar>("x"), origin_tree.get<scalar>("y"), origin_tree.get<scalar>("z"));

        boost::property_tree::ptree const&  orientation_tree = hierarchy.find("orientation")->second;
        quaternion const  orientation = make_quaternion_xyzw(
                orientation_tree.get<scalar>("x"),
                orientation_tree.get<scalar>("y"),
                orientation_tree.get<scalar>("z"),
                orientation_tree.get<scalar>("w")
                );
        ctx.frame_relocate(frame_guid, origin, orientation);
    }

    boost::property_tree::ptree const&  folders = hierarchy.find("folders")->second;

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

        ctx.insert_rigid_body(
                folder_guid,
                !is_static_collider_in_tree(hierarchy),
                load_vector("linear_velocity"),
                load_vector("angular_velocity"),
                load_vector("external_linear_acceleration"),
                load_vector("external_angular_acceleration")
                );
    }
    for (auto folder_it = folders.begin(); folder_it != folders.end(); ++folder_it)
        if (folder_it->first == "batches")
        {
            object_guid const  batches_folder_guid = ctx.insert_folder(folder_guid, folder_it->first, false);
            for (auto record_it = folder_it->second.begin(); record_it != folder_it->second.end(); ++record_it)
            {
                object_guid  batch_guid;
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
                            batch_guid = ctx.insert_batch_wireframe_box(batches_folder_guid, record_it->first,
                                                                        box_half_sizes_along_axes, colour);
                        else
                            batch_guid = ctx.insert_batch_solid_box(batches_folder_guid, record_it->first, box_half_sizes_along_axes,
                                                                    colour);
                    else if (gfx::parse_capsule_info_from_id(props, capsule_half_distance, capsule_thickness, num_lines, colour,
                                                             fog_type, wireframe))
                        if (wireframe)
                            batch_guid = ctx.insert_batch_wireframe_capsule(batches_folder_guid, record_it->first, capsule_half_distance,
                                                                            capsule_thickness, num_lines, colour);
                        else
                            batch_guid = ctx.insert_batch_solid_capsule(batches_folder_guid, record_it->first, capsule_half_distance,
                                                                        capsule_thickness, num_lines, colour);
                    else if (gfx::parse_sphere_info_from_id(props, sphere_radius, num_lines, colour, fog_type, wireframe))
                        if (wireframe)
                            batch_guid = ctx.insert_batch_wireframe_sphere(batches_folder_guid, record_it->first, sphere_radius,
                                                                           num_lines, colour);
                        else
                            batch_guid = ctx.insert_batch_solid_sphere(batches_folder_guid, record_it->first, sphere_radius, num_lines,
                                                                       colour);
                    else { UNREACHABLE(); }
                }
                else
                {
                    batch_guid = ctx.load_batch(
                            batches_folder_guid,
                            to_string(OBJECT_KIND::BATCH) + record_it->first,
                            record_it->second.get<std::string>("id"),
                            record_it->second.get<std::string>("skin")
                            );
                }
                
                gfx::batch const  batch = ctx.from_batch_guid_to_batch(batch_guid);
                ctx.insert_imported_batch_to_cache(batch);
            }
        }
        else if (folder_it->first == "collider")
        {
            boost::property_tree::ptree const&  data = folder_it->second.begin()->second;
            angeo::COLLISION_SHAPE_TYPE const  shape_type = angeo::as_collision_shape_type(data.get<std::string>("shape_type"));
            if (shape_type == angeo::COLLISION_SHAPE_TYPE::BOX)
                ctx.insert_collider_box(
                        folder_guid, to_string(OBJECT_KIND::COLLIDER) + ".box",
                        vector3(data.get<float_32_bit>("half_size_along_x"),
                                data.get<float_32_bit>("half_size_along_y"),
                                data.get<float_32_bit>("half_size_along_z")),
                        angeo::read_collison_material_from_string(data.get<std::string>("material")),
                        angeo::read_collison_class_from_string(data.get<std::string>("collision_class"))
                        );
            else if (shape_type == angeo::COLLISION_SHAPE_TYPE::CAPSULE)
                ctx.insert_collider_capsule(
                        folder_guid, to_string(OBJECT_KIND::COLLIDER) + ".capsule",
                        data.get<float_32_bit>("half_distance_between_end_points"),
                        data.get<float_32_bit>("thickness_from_central_line"),
                        angeo::read_collison_material_from_string(data.get<std::string>("material")),
                        angeo::read_collison_class_from_string(data.get<std::string>("collision_class"))
                        );
            else if (shape_type == angeo::COLLISION_SHAPE_TYPE::SPHERE)
                ctx.insert_collider_sphere(
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

                ctx.insert_collider_triangle_mesh(
                        folder_guid, to_string(OBJECT_KIND::COLLIDER) + ".triangle.",
                        index_buffer.num_primitives(),
                        collider_triangle_mesh_vertex_getter(vertex_buffer, index_buffer),
                        angeo::read_collison_material_from_string(data.get<std::string>("material")),
                        angeo::read_collison_class_from_string(data.get<std::string>("collision_class"))
                        );
            }
            else
            {
                NOT_IMPLEMENTED_YET();
            }
        }

    boost::property_tree::ptree const&  children = hierarchy.find("children")->second;
    for (auto child_it = children.begin(); child_it != children.end(); ++child_it)
        import_gfxtuner_scene_node(
                ctx,
                child_it->second,
                ctx.insert_folder(folder_guid, child_it->first, false),
                invalid_object_guid()
                );
}


void  import_gfxtuner_scene(
        simulation_context&  ctx,
        imported_scene const  scene,
        com::import_scene_props const&  props
        )
{
    ASSUMPTION(scene.hierarchy().count("@pivot") != 0UL);

    for (auto  it = scene.hierarchy().begin(); it != scene.hierarchy().end(); ++it)
    {
        if (it->first.empty() || it->first.front() == '@')
            continue;

        object_guid const  folder_guid = ctx.insert_folder(props.folder_guid, it->first, true);
        import_gfxtuner_scene_node(ctx, it->second, folder_guid, props.relocation_frame_guid);
        apply_initial_velocities_to_imported_rigid_bodies(ctx, folder_guid, props);
    }

    ctx.process_rigid_bodies_with_invalidated_shape();
}


}}
