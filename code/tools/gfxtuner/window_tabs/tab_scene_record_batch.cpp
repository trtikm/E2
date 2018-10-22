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
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

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
