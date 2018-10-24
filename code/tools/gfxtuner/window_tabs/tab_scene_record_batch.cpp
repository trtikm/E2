#include <gfxtuner/window_tabs/tab_scene_record_batch.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene.hpp>
#include <scene/scene_history.hpp>
#include <scene/scene_node_record_id.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <QFileDialog>

namespace window_tabs { namespace tab_scene { namespace record_batch {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    icons_of_records.insert({
        scn::get_batches_folder_name(),
        QIcon((boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/icons/batch.png").string().c_str())
        });
}


void  register_record_undo_redo_processors(widgets* const  w)
{
    scn::scene_history_batch_insert::set_undo_processor(
        [w](scn::scene_history_batch_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_batch_from_scene_node,
                    history_node.get_id().get_record_name(),
                    history_node.get_id().get_node_name()
                    );
            remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
        });
    scn::scene_history_batch_insert::set_redo_processor(
        [w](scn::scene_history_batch_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::insert_batch_to_scene_node,
                    history_node.get_id().get_record_name(),
                    history_node.get_batch_pathname(),
                    history_node.get_id().get_node_name()
                    );
            insert_record_to_tree_widget(
                    w->scene_tree(),
                    history_node.get_id(),
                    w->get_record_icon(scn::get_batches_folder_name()),
                    w->get_folder_icon());
        });
}



void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::unordered_set<std::string> const&)> >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_batches_folder_name(),
            [](widgets* const  w, std::unordered_set<std::string> const&  used_names)
                -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                boost::filesystem::path const&  root_dir =
                        canonical_path(boost::filesystem::absolute(
                            boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "meshes"
                            ));
                QFileDialog  dialog(w->wnd());
                dialog.setDirectory(root_dir.string().c_str());
                dialog.setFileMode(QFileDialog::DirectoryOnly);
                if (!dialog.exec())
                    return {"", {}};
                QStringList const  selected = dialog.selectedFiles();
                if (selected.size() != 1)
                {
                    w->wnd()->print_status_message("ERROR: Exactly one folder must be selected/provided.", 10000);
                    return {"", {}};
                }
                boost::filesystem::path const  pathname = qtgl::to_string(selected.front());
                boost::filesystem::path const  batch_dir = pathname.parent_path().filename();
                boost::filesystem::path  batch_name = pathname.filename();
                if (batch_name.has_extension())
                    batch_name.replace_extension("");
                return {
                    batch_dir.string() + "/" + batch_name.string(),
                    [pathname, w](scn::scene_record_id const&  record_id) -> void {
                            w->wnd()->glwindow().call_now(
                                    &simulator::insert_batch_to_scene_node,
                                    record_id.get_record_name(),
                                    pathname,
                                    record_id.get_node_name()
                                    );
                            w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, pathname, false);
                        }
                    };
                }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_batches_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id) -> void {
                    scn::scene_node_ptr const  parent_node_ptr =
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, id.get_node_name());
                    INVARIANT(parent_node_ptr != nullptr);
                    w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                            id,
                            scn::get_batch(*parent_node_ptr, id.get_record_name()).path_component_of_uid(),
                            true
                            );
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_batch_from_scene_node,
                            id.get_record_name(),
                            id.get_node_name()
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
            scn::get_batches_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id, boost::property_tree::ptree const&  data) -> void {
                    boost::filesystem::path const  pathname = data.get<std::string>("pathname");
                    w->wnd()->glwindow().call_now(
                            &simulator::insert_batch_to_scene_node,
                            id.get_record_name(),
                            pathname,
                            id.get_node_name()
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_batches_folder_name()),
                            w->get_folder_icon()
                            );
                }
            });
}


}}}
