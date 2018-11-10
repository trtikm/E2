#include <gfxtuner/window_tabs/tab_scene_record_collider.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_collider_props_dialog.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <angeo/collision_material.hpp>
#include <scene/scene.hpp>
#include <scene/scene_history.hpp>
#include <scene/scene_node_record_id.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <QFileDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <memory>

namespace window_tabs { namespace tab_scene { namespace record_collider {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    icons_of_records.insert({
        scn::get_collider_folder_name(),
        QIcon((boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/icons/collider.png").string().c_str())
        });
}


void  register_record_undo_redo_processors(widgets* const  w)
{
    //scn::scene_history_batch_insert::set_undo_processor(
    //    [w](scn::scene_history_batch_insert const&  history_node) {
    //        INVARIANT(history_node.get_id().get_folder_name() == scn::get_collider_folder_name());
    //        w->wnd()->glwindow().call_now(
    //                &simulator::erase_batch_from_scene_node,
    //                history_node.get_id().get_record_name(),
    //                history_node.get_id().get_node_name()
    //                );
    //        remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
    //    });
    //scn::scene_history_batch_insert::set_redo_processor(
    //    [w](scn::scene_history_batch_insert const&  history_node) {
    //        INVARIANT(history_node.get_id().get_folder_name() == scn::get_collider_folder_name());
    //        w->wnd()->glwindow().call_now(
    //                &simulator::insert_batch_to_scene_node,
    //                history_node.get_id().get_record_name(),
    //                history_node.get_batch_pathname(),
    //                history_node.get_id().get_node_name()
    //                );
    //        insert_record_to_tree_widget(
    //                w->scene_tree(),
    //                history_node.get_id(),
    //                w->get_record_icon(scn::get_collider_folder_name()),
    //                w->get_folder_icon());
    //    });
}



void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string, std::pair<bool,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)>> >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_collider_folder_name(),
            {
                false, // there cannot be multiple records in the folder.
                [](widgets* const  w, std::string const&  shape_type, std::unordered_set<std::string> const&  used_names)
                    -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                        if (used_names.size() != 0UL)
                        {
                            w->wnd()->print_status_message("ERROR: A coordinate system node may contain at most one collider.", 10000);
                            return {"", {}};
                        }

                        std::shared_ptr<detail::collider_props_dialog::collider_props>  props =
                                std::make_shared<detail::collider_props_dialog::collider_props>();
                        props->m_shape_type = shape_type;
                        props->m_as_dynamic = true;
                        props->m_material = angeo::COLLISION_MATERIAL_TYPE::WOOD;
                        props->m_density_multiplier = 1.0f;
                        if (props->m_shape_type == "capsule")
                        {
                            props->m_capsule_half_distance_between_end_points = 0.05f;
                            props->m_capsule_thickness_from_central_line = 0.05f;
                        }
                        else if (props->m_shape_type == "sphere")
                        {
                            props->m_sphere_radius = 0.05f;
                        }
                        else if (props->m_shape_type == "triangle mesh")
                        {
                            props->m_as_dynamic = false;
                            props->m_triangle_mesh_buffers_directory =
                                canonical_path(boost::filesystem::absolute(
                                    boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "meshes"
                                    ));
                        }
                        else
                        {
                            UNREACHABLE();
                        }
                        detail::collider_props_dialog  dlg(w->wnd(), props.get());
                        dlg.exec();
                        if (!dlg.ok())
                            return{ "",{} };
                        return {
                            props->m_shape_type,
                            [w, props](scn::scene_record_id const&  record_id) -> void {
                                    if (props->m_shape_type == "capsule")
                                    {
                                        w->wnd()->glwindow().call_now(
                                                &simulator::insert_collision_capsule_to_scene_node,
                                                props->m_capsule_half_distance_between_end_points,
                                                props->m_capsule_thickness_from_central_line,
                                                props->m_material,
                                                props->m_density_multiplier,
                                                props->m_as_dynamic,
                                                record_id
                                                );
                                        //w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, false);
                                    }
                                    else if (props->m_shape_type == "sphere")
                                    {
                                        w->wnd()->glwindow().call_now(
                                                &simulator::insert_collision_sphere_to_scene_node,
                                                props->m_sphere_radius,
                                                props->m_material,
                                                props->m_density_multiplier,
                                                props->m_as_dynamic,
                                                record_id
                                                );
                                        //w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, false);
                                    }
                                    else if (props->m_shape_type == "triangle mesh")
                                    {
                                        qtgl::buffer  vertex_buffer(props->m_triangle_mesh_buffers_directory / "vertices.txt");
                                        qtgl::buffer  index_buffer(props->m_triangle_mesh_buffers_directory / "indices.txt");

                                        if (!vertex_buffer.wait_till_load_is_finished())
                                        {
                                            w->wnd()->print_status_message("ERROR: Failed to load vertex buffer '" + vertex_buffer.key().get_unique_id() + "'.", 10000);
                                            return;
                                        }
                                        if (!index_buffer.wait_till_load_is_finished())
                                        {
                                            w->wnd()->print_status_message("ERROR: Failed to load index buffer '" + index_buffer.key().get_unique_id() + "'.", 10000);
                                            return;
                                        }

                                        w->wnd()->glwindow().call_now(
                                                &simulator::insert_collision_trianle_mesh_to_scene_node,
                                                vertex_buffer,
                                                index_buffer,
                                                props->m_material,
                                                props->m_density_multiplier,
                                                // props->m_as_dynamic, <-- Is always assumed to be 'false'.
                                                record_id
                                                );
                                        //w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, false);
                                    }
                                    else
                                    {
                                        UNREACHABLE();
                                    }
                                }
                            };
                    }
                }
            });
}


void  register_record_handler_for_update_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)> >&  update_record_handlers
        )
{
    update_record_handlers.insert({
            scn::get_collider_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  record_id) -> void {
                    detail::collider_props_dialog::collider_props  props;
                    props.m_shape_type = record_id.get_record_name();
                    if (props.m_shape_type == "capsule")
                        w->wnd()->glwindow().call_now(
                                &simulator::get_collision_capsule_info,
                                record_id,
                                std::ref(props.m_capsule_half_distance_between_end_points),
                                std::ref(props.m_capsule_thickness_from_central_line),
                                std::ref(props.m_material),
                                std::ref(props.m_density_multiplier),
                                std::ref(props.m_as_dynamic)
                                );
                    else if (props.m_shape_type == "sphere")
                        w->wnd()->glwindow().call_now(
                                &simulator::get_collision_sphere_info,
                                record_id,
                                std::ref(props.m_sphere_radius),
                                std::ref(props.m_material),
                                std::ref(props.m_density_multiplier),
                                std::ref(props.m_as_dynamic)
                                );
                    else if (props.m_shape_type == "triangle mesh")
                    {
                        qtgl::buffer  vertex_buffer;
                        qtgl::buffer  index_buffer;
                        w->wnd()->glwindow().call_now(
                                &simulator::get_collision_triangle_mesh_info,
                                record_id,
                                std::ref(vertex_buffer),
                                std::ref(index_buffer),
                                std::ref(props.m_material),
                                std::ref(props.m_density_multiplier)
                                );
                        props.m_triangle_mesh_buffers_directory =
                                boost::filesystem::path(vertex_buffer.key().get_unique_id()).parent_path();
                        props.m_as_dynamic = false;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                    detail::collider_props_dialog  dlg(w->wnd(), &props);
                    dlg.exec();
                    if (!dlg.ok())
                        return;
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_collision_object_from_scene_node,
                            record_id
                            );
                    if (props.m_shape_type == "capsule")
                        w->wnd()->glwindow().call_now(
                                &simulator::insert_collision_capsule_to_scene_node,
                                props.m_capsule_half_distance_between_end_points,
                                props.m_capsule_thickness_from_central_line,
                                props.m_material,
                                props.m_density_multiplier,
                                props.m_as_dynamic,
                                record_id
                                );
                    else if (props.m_shape_type == "sphere")
                        w->wnd()->glwindow().call_now(
                                &simulator::insert_collision_sphere_to_scene_node,
                                props.m_sphere_radius,
                                props.m_material,
                                props.m_density_multiplier,
                                props.m_as_dynamic,
                                record_id
                                );
                    else if (props.m_shape_type == "triangle mesh")
                    {
                        qtgl::buffer  vertex_buffer(props.m_triangle_mesh_buffers_directory / "vertices.txt");
                        qtgl::buffer  index_buffer(props.m_triangle_mesh_buffers_directory / "indices.txt");

                        if (!vertex_buffer.wait_till_load_is_finished())
                        {
                            w->wnd()->print_status_message("ERROR: Failed to load vertex buffer '" + vertex_buffer.key().get_unique_id() + "'.", 10000);
                            return;
                        }
                        if (!index_buffer.wait_till_load_is_finished())
                        {
                            w->wnd()->print_status_message("ERROR: Failed to load index buffer '" + index_buffer.key().get_unique_id() + "'.", 10000);
                            return;
                        }

                        w->wnd()->glwindow().call_now(
                                &simulator::insert_collision_trianle_mesh_to_scene_node,
                                vertex_buffer,
                                index_buffer,
                                props.m_material,
                                props.m_density_multiplier,
                                // props.m_as_dynamic, <-- Is always assumed to be 'false'.
                                record_id
                                );
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_collider_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id) -> void {
                    scn::scene_node_ptr const  node_ptr =
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, id.get_node_name());
                    INVARIANT(node_ptr != nullptr);
                    //w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                    //        id,
                    //        scn::get_batch(*node_ptr, id.get_record_name()).path_component_of_uid(),
                    //        true
                    //        );
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_collision_object_from_scene_node,
                            id
                            );
                }
            });
}


void  register_record_handler_for_load_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_record_id const&,
                                                           boost::property_tree::ptree const&)>>&
                load_record_handlers
        )
{
    load_record_handlers.insert({
            scn::get_collider_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id, boost::property_tree::ptree const&  data) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::load_collider,
                            std::cref(data),
                            id.get_node_name()
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_collider_folder_name()),
                            w->get_folder_icon()
                            );
                }
            });
}


void  register_record_handler_for_save_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_node_ptr,
                                                           scn::scene_node_record_id const&,
                                                           boost::property_tree::ptree&)>>&
                save_record_handlers
        )
{
    save_record_handlers.insert({
            scn::get_collider_folder_name(),
            []( widgets* const  w,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::save_collider,
                            *scn::get_collider(*node_ptr),
                            std::ref(data)
                            );
                }
            });
}


}}}
