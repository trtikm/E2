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
    //scn::scene_history_batch_insert::set_undo_processor(
    //    [w](scn::scene_history_batch_insert const&  history_node) {
    //        INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
    //        w->wnd()->glwindow().call_now(
    //                &simulator::erase_batch_from_scene_node,
    //                history_node.get_id().get_record_name(),
    //                history_node.get_id().get_node_name()
    //                );
    //        remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
    //    });
    //scn::scene_history_batch_insert::set_redo_processor(
    //    [w](scn::scene_history_batch_insert const&  history_node) {
    //        INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
    //        w->wnd()->glwindow().call_now(
    //                &simulator::insert_batch_to_scene_node,
    //                history_node.get_id().get_record_name(),
    //                history_node.get_batch_pathname(),
    //                history_node.get_id().get_node_name()
    //                );
    //        insert_record_to_tree_widget(
    //                w->scene_tree(),
    //                history_node.get_id(),
    //                w->get_record_icon(scn::get_rigid_body_folder_name()),
    //                w->get_folder_icon());
    //    });
}



void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)> >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            [](widgets* const  w, std::string const&, std::unordered_set<std::string> const&  used_names)
                -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                    if (used_names.size() != 0UL)
                    {
                        w->wnd()->print_status_message("ERROR: A coordinate system node may contain at most one rigid body.", 10000);
                        return{ "",{} };
                    }
                    w->wnd()->print_status_message("ERROR: Insertion of rigid body is not implemented yet.", 10000);
                    return{ "",{} };
                    //return {
                    //    scn::get_rigid_body_record_name(),
                    //    [w](scn::scene_record_id const&  record_id) -> void {
                    //            w->wnd()->glwindow().call_now(
                    //                    &simulator::insert_rigid_body_to_scene_node,
                    //                    //TODO!
                    //                    record_id.get_node_name()
                    //                    );
                    //            w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, pathname, false);
                    //        }
                    //    };
                }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
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
                            &simulator::erase_rigid_body_from_scene_node,
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
    //load_record_handlers.insert({
    //        scn::get_rigid_body_folder_name(),
    //        [](widgets* const  w, scn::scene_record_id const&  id, boost::property_tree::ptree const&  data) -> void {
    //                boost::filesystem::path const  pathname = data.get<std::string>("pathname");
    //                w->wnd()->glwindow().call_now(
    //                        &simulator::insert_batch_to_scene_node,
    //                        id.get_record_name(),
    //                        pathname,
    //                        id.get_node_name()
    //                        );
    //                insert_record_to_tree_widget(
    //                        w->scene_tree(),
    //                        id,
    //                        w->get_record_icon(scn::get_rigid_body_folder_name()),
    //                        w->get_folder_icon()
    //                        );
    //            }
    //        });
}


}}}
