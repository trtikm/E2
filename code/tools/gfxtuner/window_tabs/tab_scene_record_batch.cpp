#include <gfxtuner/window_tabs/tab_scene_record_batch.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/dialog/effects_config_dialog.hpp>
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
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_id().get_node_id())
                    );
            remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
        });
    scn::scene_history_batch_insert::set_redo_processor(
        [w](scn::scene_history_batch_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::insert_batch_to_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_batch_pathname()),
                    std::cref(history_node.get_skin_name()),
                    history_node.get_effects_config(),
                    std::cref(history_node.get_id().get_node_id())
                    );
            insert_record_to_tree_widget(
                    w->scene_tree(),
                    history_node.get_id(),
                    w->get_record_icon(scn::get_batches_folder_name()),
                    w->get_folder_icon()
                    );
        });

    scn::scene_history_batch_update_props::set_undo_processor(
        [w](scn::scene_history_batch_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_batch_from_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_id().get_node_id())
                    );
            w->wnd()->glwindow().call_now(
                    &simulator::insert_batch_to_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_batch_pathname()),
                    std::cref(history_node.get_old_skin_name()),
                    history_node.get_old_effects_config(),
                    std::cref(history_node.get_id().get_node_id())
                    );
        });
    scn::scene_history_batch_update_props::set_redo_processor(
        [w](scn::scene_history_batch_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_batch_from_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_id().get_node_id())
                    );
            w->wnd()->glwindow().call_now(
                    &simulator::insert_batch_to_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_batch_pathname()),
                    std::cref(history_node.get_new_skin_name()),
                    history_node.get_new_effects_config(),
                    std::cref(history_node.get_id().get_node_id())
                    );
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
            scn::get_batches_folder_name(),
            {
                true, // allows multiple records in the folder (i.e. to open the name-selection dialog).
                [](widgets* const  w, std::string const&, std::unordered_set<std::string> const&  used_names)
                    -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                    boost::filesystem::path const&  root_dir =
                            canonical_path(boost::filesystem::absolute(
                                boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "batches"
                                ));
                    QFileDialog  dialog(w->wnd());
                    dialog.setDirectory(root_dir.string().c_str());
                    dialog.setFileMode(QFileDialog::ExistingFile);
                    if (!dialog.exec())
                        return {"", {}};
                    QStringList const  selected = dialog.selectedFiles();
                    if (selected.size() != 1)
                    {
                        w->wnd()->print_status_message("ERROR: Exactly one file must be provided.", 10000);
                        return {"", {}};
                    }
                    boost::filesystem::path const  pathname = qtgl::to_string(selected.front());
                    std::string  batch_name;
                    {
                        std::string const  batch_dir = pathname.parent_path().filename().string();
                        if (batch_dir != "batches")
                        {
                            batch_name.append(batch_dir);
                            batch_name.push_back('.');
                        }
                        boost::filesystem::path  batch_file_name = pathname.filename();
                        if (batch_file_name.has_extension())
                            batch_file_name.replace_extension("");
                        batch_name.append(batch_file_name.string());
                    }
                    return {
                        batch_name,
                        [pathname, w](scn::scene_record_id const&  record_id) -> void {
                                w->wnd()->glwindow().call_now(
                                        &simulator::insert_batch_to_scene_node,
                                        std::cref(record_id.get_record_name()),
                                        std::cref(pathname),
                                        std::cref("default"),
                                        w->wnd()->glwindow().call_now(&simulator::get_effects_config),
                                        std::cref(record_id.get_node_id())
                                        );
                                w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                                        record_id,
                                        pathname,
                                        "default",
                                        w->wnd()->glwindow().call_now(&simulator::get_effects_config),
                                        false
                                        );
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
            scn::get_batches_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  record_id) -> void {
                    scn::scene_node_ptr const  src_node_ptr =
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, record_id.get_node_id());

                    boost::filesystem::path  pathname;
                    qtgl::effects_config old_effects_config;
                    std::string  old_skin_name;
                    std::vector<std::string>  skin_names;
                    {
                        qtgl::batch const  old_batch = scn::get_batch(*src_node_ptr, record_id.get_record_name());
                        pathname = old_batch.get_path();
                        old_effects_config = old_batch.get_effects_config();
                        old_skin_name = old_batch.get_skin_name();
                        for (auto const& elem : old_batch.get_available_resources().skins())
                            skin_names.push_back(elem.first);
                    }
                    dialog::effects_config_dialog  dlg(w->wnd(), old_effects_config.resource_const(), old_skin_name, skin_names);
                    dlg.exec();
                    if (!dlg.ok())
                        return;
                    qtgl::effects_config const  new_effects_config = qtgl::effects_config::make(dlg.get_new_effects_data());
                    std::string const  new_skin_name = dlg.get_new_skin_name();

                    w->get_scene_history()->insert<scn::scene_history_batch_update_props>(
                            std::cref(record_id),
                            std::cref(pathname),
                            std::cref(old_skin_name),
                            old_effects_config,
                            std::cref(new_skin_name),
                            new_effects_config,
                            false);
                    w->wnd()->glwindow().call_now(
                            &simulator::replace_batch_in_scene_node,
                            std::cref(record_id.get_record_name()),
                            std::cref(new_skin_name),
                            new_effects_config,
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
            scn::get_batches_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  src_record_id, scn::scene_record_id const&  dst_record_id) -> void {
                    scn::scene_node_ptr const  src_node_ptr =
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, src_record_id.get_node_id());
                    qtgl::batch const  b = scn::get_batch(*src_node_ptr, src_record_id.get_record_name());
                    w->wnd()->glwindow().call_now(
                            &simulator::insert_batch_to_scene_node,
                            std::cref(dst_record_id.get_record_name()),
                            std::cref(b.get_path()),
                            std::cref(b.get_skin_name()),
                            b.get_effects_config(),
                            std::cref(dst_record_id.get_node_id())
                            );
                    w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                            dst_record_id,
                            b.get_path(),
                            b.get_skin_name(),
                            b.get_effects_config(),
                            false
                            );
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
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, id.get_node_id());
                    INVARIANT(parent_node_ptr != nullptr);
                    {
                        qtgl::batch const  b = scn::get_batch(*parent_node_ptr, id.get_record_name());
                        w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                                id,
                                b.get_path(),
                                b.get_skin_name(),
                                b.get_effects_config(),
                                true
                                );
                    }
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_batch_from_scene_node,
                            std::cref(id.get_record_name()),
                            std::cref(id.get_node_id())
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
                            std::cref(id.get_record_name()),
                            std::cref(pathname),
                            std::cref("default"),
                            w->wnd()->glwindow().call_now(&simulator::get_effects_config),
                            std::cref(id.get_node_id())
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


void  register_record_handler_for_save_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_node_ptr,
                                                           scn::scene_node_record_id const&,
                                                           boost::property_tree::ptree&)>>&
                save_record_handlers
        )
{
    save_record_handlers.insert({
            scn::get_batches_folder_name(),
            []( widgets*,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data) -> void {
                    qtgl::batch const  b = scn::get_batch(*node_ptr, id.get_record_name());
                    data.put("pathname", b.get_path());
                }
            });
}


}}}
