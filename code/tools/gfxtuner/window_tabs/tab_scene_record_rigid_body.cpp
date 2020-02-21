#include <gfxtuner/window_tabs/tab_scene_record_rigid_body.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/dialog_windows/rigid_body_props_dialog.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene.hpp>
#include <scene/scene_history.hpp>
#include <scene/scene_node_record_id.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <scene/records/rigid_body/rigid_body.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <memory>

namespace window_tabs { namespace tab_scene { namespace record_rigid_body { namespace detail {


bool  is_valid(matrix33 const&  M)
{
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            if (std::isnan(M(i, j)) || std::isinf(M(i, j)))
                return false;
    return true;
}


bool  is_zero(matrix33 const&  M)
{
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            if (std::fabsf(M(i,j)) > 0.0001f)
                return false;
    return true;
}


}}}}

namespace window_tabs { namespace tab_scene { namespace record_rigid_body {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    icons_of_records.insert({
        scn::get_rigid_body_folder_name(),
        QIcon((boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/icons/rigid_body.png").string().c_str())
        });
}


void  register_record_undo_redo_processors(widgets* const  w)
{
    scn::scene_history_rigid_body_insert::set_undo_processor(
        [w](scn::scene_history_rigid_body_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_rigid_body_from_scene_node,
                    std::cref(history_node.get_id().get_node_id())
                    );
            remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
        });
    scn::scene_history_rigid_body_insert::set_redo_processor(
        [w](scn::scene_history_rigid_body_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::insert_rigid_body_to_scene_node,
                    std::cref(history_node.get_props()),
                    history_node.auto_compute_mass_and_inertia_tensor(),
                    std::cref(history_node.get_id().get_node_id())
                    );
            insert_record_to_tree_widget(
                    w->scene_tree(),
                    history_node.get_id(),
                    w->get_record_icon(scn::get_rigid_body_folder_name()),
                    w->get_folder_icon());
        });


    scn::scene_history_rigid_body_update_props::set_undo_processor(
        [w](scn::scene_history_rigid_body_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_rigid_body_from_scene_node,
                    std::cref(history_node.get_id().get_node_id())
                    );
            w->wnd()->glwindow().call_now(
                    &simulator::insert_rigid_body_to_scene_node,
                    std::cref(history_node.get_old_props()),
                    history_node.get_old_auto_compute_mass_and_inertia_tensor(),
                    std::cref(history_node.get_id().get_node_id())
                    );
        });
    scn::scene_history_rigid_body_update_props::set_redo_processor(
        [w](scn::scene_history_rigid_body_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_rigid_body_from_scene_node,
                    std::cref(history_node.get_id().get_node_id())
                    );
            w->wnd()->glwindow().call_now(
                    &simulator::insert_rigid_body_to_scene_node,
                    std::cref(history_node.get_new_props()),
                    history_node.get_new_auto_compute_mass_and_inertia_tensor(),
                    std::cref(history_node.get_id().get_node_id())
                    );
        });
}



void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string, std::pair<bool,
                           std::function<std::pair<std::string, std::function<bool(scn::scene_record_id const&)> >
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)> > >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            {
                false, // There cannot be mutiple records in one folder.
                [](widgets* const  w, std::string const&  mode, std::unordered_set<std::string> const&  used_names)
                    -> std::pair<std::string, std::function<bool(scn::scene_record_id const&)>> {
                        if (used_names.size() != 0UL)
                        {
                            w->wnd()->print_status_message("ERROR: A coordinate system node may contain at most one rigid body.", 10000);
                            return{ "",{} };
                        }
                        return {
                            scn::get_rigid_body_record_name(),
                            [w](scn::scene_record_id const&  record_id) -> bool {
                                    std::shared_ptr<scn::rigid_body_props>  rb_props = std::make_shared<scn::rigid_body_props>();
                                    rb_props->m_linear_velocity = { 0.0f, 0.0f, 0.0f };
                                    rb_props->m_angular_velocity ={ 0.0f, 0.0f, 0.0f };
                                    rb_props->m_external_linear_acceleration = { 0.0f, 0.0f, -9.81f };
                                    rb_props->m_external_angular_acceleration ={ 0.0f, 0.0f, 0.0f };
                                    rb_props->m_mass_inverted = 0.0f;
                                    rb_props->m_inertia_tensor_inverted = matrix33_zero();
                                    bool  auto_compute_mass_and_inertia_tensor = true;
                                    dialog_windows::rigid_body_props_dialog  dlg(
                                            w->wnd(),
                                            &auto_compute_mass_and_inertia_tensor,
                                            rb_props.get(),
                                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, std::cref(record_id.get_node_id()))
                                                                ->get_world_matrix(),
                                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_id())
                                                                ->get_world_matrix()
                                            );
                                    dlg.exec();
                                    if (!dlg.ok())
                                        return false;
                                    w->wnd()->glwindow().call_now(
                                            &simulator::insert_rigid_body_to_scene_node,
                                            std::cref(*rb_props),
                                            auto_compute_mass_and_inertia_tensor,
                                            std::cref(record_id.get_node_id())
                                            );
                                    w->get_scene_history()->insert<scn::scene_history_rigid_body_insert>(
                                            record_id,
                                            auto_compute_mass_and_inertia_tensor,
                                            *rb_props,
                                            false
                                            );
                                    return true;
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
            scn::get_rigid_body_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  record_id) -> void {
                    bool  auto_compute_mass_and_inertia_tensor;
                    scn::rigid_body_props  rb_props;
                    w->wnd()->glwindow().call_now(
                            &simulator::get_rigid_body_info,
                            std::cref(record_id.get_node_id()),
                            std::ref(auto_compute_mass_and_inertia_tensor),
                            std::ref(rb_props)
                            );
                    w->wnd()->glwindow().call_now(&simulator::get_scene_node, std::cref(record_id.get_node_id()))->get_world_matrix();

                    bool const  old_auto_compute_mass_and_inertia_tensor = auto_compute_mass_and_inertia_tensor;
                    scn::rigid_body_props  rb_old_props = rb_props;
                    dialog_windows::rigid_body_props_dialog  dlg(
                            w->wnd(),
                            &auto_compute_mass_and_inertia_tensor,
                            &rb_props,
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, std::cref(record_id.get_node_id()))
                                                ->get_world_matrix(),
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_id())
                                                ->get_world_matrix()
                            );
                    dlg.exec();
                    if (!dlg.ok())
                        return;
                    w->get_scene_history()->insert<scn::scene_history_rigid_body_update_props>(
                            record_id,
                            old_auto_compute_mass_and_inertia_tensor,
                            rb_old_props,
                            auto_compute_mass_and_inertia_tensor,
                            rb_props,
                            false);
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_rigid_body_from_scene_node,
                            std::cref(record_id.get_node_id())
                            );
                    w->wnd()->glwindow().call_now(
                            &simulator::insert_rigid_body_to_scene_node,
                            std::cref(rb_props),
                            auto_compute_mass_and_inertia_tensor,
                            std::cref(record_id.get_node_id())
                            );
                }
            });
}


void  register_record_handler_for_duplicate_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&, scn::scene_record_id const&)> >&
                duplicate_record_handlers
        )
{
    duplicate_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  src_record_id, scn::scene_record_id const&  dst_record_id) -> void {
                    bool  auto_compute_mass_and_inertia_tensor;
                    scn::rigid_body_props  rb_props;
                    w->wnd()->glwindow().call_now(
                            &simulator::get_rigid_body_info,
                            std::cref(src_record_id.get_node_id()),
                            std::ref(auto_compute_mass_and_inertia_tensor),
                            std::ref(rb_props)
                            );
                    w->wnd()->glwindow().call_now(
                            &simulator::insert_rigid_body_to_scene_node,
                            std::cref(rb_props),
                            auto_compute_mass_and_inertia_tensor,
                            std::cref(dst_record_id.get_node_id())
                            );
                    w->get_scene_history()->insert<scn::scene_history_rigid_body_insert>(
                            dst_record_id,
                            auto_compute_mass_and_inertia_tensor,
                            rb_props,
                            false
                            );
                }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)> >&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id) -> void {
                    bool  auto_compute_mass_and_inertia_tensor;
                    scn::rigid_body_props  rb_props;
                    w->wnd()->glwindow().call_now(
                            &simulator::get_rigid_body_info,
                            std::cref(id.get_node_id()),
                            std::ref(auto_compute_mass_and_inertia_tensor),
                            std::ref(rb_props)
                            );
                    w->get_scene_history()->insert<scn::scene_history_rigid_body_insert>(
                            id,
                            auto_compute_mass_and_inertia_tensor,
                            rb_props,
                            true
                            );
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_rigid_body_from_scene_node,
                            std::cref(id.get_node_id())
                            );
                }
            });
}


void  register_record_handler_for_load_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_record_id const&,
                                                           boost::property_tree::ptree const&,
                                                           std::unordered_map<std::string, boost::property_tree::ptree> const&,
                                                           bool)> >&
                load_record_handlers
        )
{
    load_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            []( widgets* const  w,
                scn::scene_record_id const&  id,
                boost::property_tree::ptree const&  data,
                std::unordered_map<std::string, boost::property_tree::ptree> const&  infos,
                bool const  do_update_history) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::load_rigid_body,
                            std::cref(data),
                            std::cref(id.get_node_id())
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_rigid_body_folder_name()),
                            w->get_folder_icon()
                            );
                    if (do_update_history)
                    {
                        bool  auto_compute_mass_and_inertia_tensor;
                        scn::rigid_body_props  rb_props;
                        w->wnd()->glwindow().call_now(
                                &simulator::get_rigid_body_info,
                                std::cref(id.get_node_id()),
                                std::ref(auto_compute_mass_and_inertia_tensor),
                                std::ref(rb_props)
                                );
                        w->get_scene_history()->insert<scn::scene_history_rigid_body_insert>(
                                id,
                                auto_compute_mass_and_inertia_tensor,
                                rb_props,
                                false
                                );
                    }
                }
            });
}


void  register_record_handler_for_save_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_node_ptr,
                                                           scn::scene_node_record_id const&,
                                                           boost::property_tree::ptree&,
                                                           std::unordered_map<std::string, boost::property_tree::ptree>&)> >&
                save_record_handlers
        )
{
    save_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            []( widgets* const  w,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data,
                std::unordered_map<std::string, boost::property_tree::ptree>&  infos) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::save_rigid_body,
                            node_ptr->get_id(),
                            std::ref(data)
                            );
                }
            });
}


}}}
