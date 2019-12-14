#include <gfxtuner/window_tabs/tab_scene_record_collider.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/dialog_windows/collider_props_dialog.hpp>
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

namespace window_tabs { namespace tab_scene { namespace record_collider { namespace detail {


void  set_collider_props_to_defaults(std::string const&  shape_type, scn::collider_props&  props)
{
    props.m_shape_type = shape_type;
    props.m_as_dynamic = true;
    props.m_material = angeo::COLLISION_MATERIAL_TYPE::WOOD;
    props.m_density_multiplier = 1.0f;
    if (props.m_shape_type == "capsule")
    {
        props.m_capsule_half_distance_between_end_points = 0.05f;
        props.m_capsule_thickness_from_central_line = 0.05f;
    }
    else if (props.m_shape_type == "sphere")
    {
        props.m_sphere_radius = 0.05f;
    }
    else if (props.m_shape_type == "triangle mesh")
    {
        props.m_as_dynamic = false;
        props.m_triangle_mesh_buffers_directory =
            canonical_path(boost::filesystem::absolute(
                boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "meshes"
                ));
    }
    else
    {
        UNREACHABLE();
    }
}


void  insert_collider_to_simulator(widgets* const  w, scn::collider_props const&  props, scn::scene_record_id const&  record_id)
{
    if (props.m_shape_type == "capsule")
    {
        w->wnd()->glwindow().call_now(
                &simulator::insert_collision_capsule_to_scene_node,
                props.m_capsule_half_distance_between_end_points,
                props.m_capsule_thickness_from_central_line,
                props.m_material,
                props.m_density_multiplier,
                props.m_as_dynamic,
                std::cref(record_id)
                );
    }
    else if (props.m_shape_type == "sphere")
    {
        w->wnd()->glwindow().call_now(
                &simulator::insert_collision_sphere_to_scene_node,
                props.m_sphere_radius,
                props.m_material,
                props.m_density_multiplier,
                props.m_as_dynamic,
                std::cref(record_id)
                );
    }
    else if (props.m_shape_type == "triangle mesh")
    {
        TMPROF_BLOCK();

        qtgl::buffer  vertex_buffer(props.m_triangle_mesh_buffers_directory / "vertices.txt", std::numeric_limits<async::load_priority_type>::max());
        qtgl::buffer  index_buffer(props.m_triangle_mesh_buffers_directory / "indices.txt", std::numeric_limits<async::load_priority_type>::max());

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
                std::cref(record_id)
                );
    }
    else
    {
        UNREACHABLE();
    }
}


void  erase_collider_from_simulator(widgets* const  w, scn::scene_record_id const&  record_id)
{
    w->wnd()->glwindow().call_now(
            &simulator::erase_collision_object_from_scene_node,
            std::cref(record_id)
            );
}

void  read_collider_props_from_simulator(widgets* const  w, scn::scene_record_id const&  record_id, scn::collider_props&  props)
{
    props.m_shape_type = record_id.get_record_name();
    if (props.m_shape_type == "capsule")
        w->wnd()->glwindow().call_now(
                &simulator::get_collision_capsule_info,
                std::cref(record_id),
                std::ref(props.m_capsule_half_distance_between_end_points),
                std::ref(props.m_capsule_thickness_from_central_line),
                std::ref(props.m_material),
                std::ref(props.m_density_multiplier),
                std::ref(props.m_as_dynamic)
                );
    else if (props.m_shape_type == "sphere")
        w->wnd()->glwindow().call_now(
                &simulator::get_collision_sphere_info,
                std::cref(record_id),
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
                std::cref(record_id),
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
}


}}}}

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
    scn::scene_history_collider_insert::set_undo_processor(
        [w](scn::scene_history_collider_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_collider_folder_name());
            detail::erase_collider_from_simulator(w, history_node.get_id());
            remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
        });
    scn::scene_history_collider_insert::set_redo_processor(
        [w](scn::scene_history_collider_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_collider_folder_name());
            detail::insert_collider_to_simulator(w, history_node.get_collider_props(), history_node.get_id());
            insert_record_to_tree_widget(
                    w->scene_tree(),
                    history_node.get_id(),
                    w->get_record_icon(scn::get_collider_folder_name()),
                    w->get_folder_icon());
        });

    scn::scene_history_collider_update_props::set_undo_processor(
        [w](scn::scene_history_collider_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_collider_folder_name());
            detail::erase_collider_from_simulator(w, history_node.get_id());
            detail::insert_collider_to_simulator(w, history_node.get_collider_old_props(), history_node.get_id());
        });
    scn::scene_history_collider_update_props::set_redo_processor(
        [w](scn::scene_history_collider_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_collider_folder_name());
            detail::erase_collider_from_simulator(w, history_node.get_id());
            detail::insert_collider_to_simulator(w, history_node.get_collider_new_props(), history_node.get_id());
        });
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
                        std::shared_ptr<scn::collider_props>  props = std::make_shared<scn::collider_props>();
                        detail::set_collider_props_to_defaults(shape_type, *props);
                        dialog_windows::collider_props_dialog  dlg(w->wnd(), props.get());
                        dlg.exec();
                        if (!dlg.ok())
                            return{ "",{} };
                        return {
                            props->m_shape_type,
                            [w, props](scn::scene_record_id const&  record_id) -> void {
                                    detail::insert_collider_to_simulator(w, *props, record_id);
                                    w->get_scene_history()->insert<scn::scene_history_collider_insert>(record_id, *props, false);
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
                    scn::collider_props  props;
                    detail::read_collider_props_from_simulator(w, record_id, props);
                    scn::collider_props  old_props = props;
                    dialog_windows::collider_props_dialog  dlg(w->wnd(), &props);
                    dlg.exec();
                    if (!dlg.ok())
                        return;
                    w->get_scene_history()->insert<scn::scene_history_collider_update_props>(record_id, old_props, props, false);
                    detail::erase_collider_from_simulator(w, record_id);
                    detail::insert_collider_to_simulator(w, props, record_id);
                }
            });
}


void  register_record_handler_for_duplicate_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&, scn::scene_record_id const&)> >&
                duplicate_record_handlers
        )
{
    duplicate_record_handlers.insert({
            scn::get_collider_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  src_record_id, scn::scene_record_id const&  dst_record_id) -> void {
                    scn::collider_props  props;
                    detail::read_collider_props_from_simulator(w, src_record_id, props);
                    detail::insert_collider_to_simulator(w, props, dst_record_id);
                    w->get_scene_history()->insert<scn::scene_history_collider_insert>(dst_record_id, props, false);
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
                    scn::collider_props  props;
                    detail::read_collider_props_from_simulator(w, id, props);
                    w->get_scene_history()->insert<scn::scene_history_collider_insert>(id, props, true);
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
                                                           boost::property_tree::ptree const&,
                                                           std::unordered_map<std::string, boost::property_tree::ptree> const&,
                                                           bool)>>&
                load_record_handlers
        )
{
    load_record_handlers.insert({
            scn::get_collider_folder_name(),
            []( widgets* const  w,
                scn::scene_record_id const&  id,
                boost::property_tree::ptree const&  data,
                std::unordered_map<std::string, boost::property_tree::ptree> const&  infos,
                bool const  do_update_history) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::load_collider,
                            std::cref(data),
                            id.get_node_id()
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_collider_folder_name()),
                            w->get_folder_icon()
                            );
                    if (do_update_history)
                    {
                        scn::collider_props  props;
                        detail::read_collider_props_from_simulator(w, id, props);
                        w->get_scene_history()->insert<scn::scene_history_collider_insert>(id, props, false);

                    }
                }
            });
}


void  register_record_handler_for_save_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_node_ptr,
                                                           scn::scene_node_record_id const&,
                                                           boost::property_tree::ptree&,
                                                           std::unordered_map<std::string, boost::property_tree::ptree>&)>>&
                save_record_handlers
        )
{
    save_record_handlers.insert({
            scn::get_collider_folder_name(),
            []( widgets* const  w,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data,
                std::unordered_map<std::string, boost::property_tree::ptree>&  infos) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::save_collider,
                            *scn::get_collider(*node_ptr),
                            std::ref(data)
                            );
                }
            });
}


}}}
