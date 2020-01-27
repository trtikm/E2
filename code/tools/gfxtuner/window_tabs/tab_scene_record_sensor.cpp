#include <gfxtuner/window_tabs/tab_scene_record_sensor.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/window_tabs/skeleton_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/dialog_windows/sensor_props_dialog.hpp>
#include <scene/scene.hpp>
#include <scene/scene_history.hpp>
#include <scene/scene_node_record_id.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <ai/sensors.hpp>
#include <ai/skeleton_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <memory>
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

namespace window_tabs { namespace tab_scene { namespace record_sensor {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    icons_of_records.insert({
        scn::get_sensors_folder_name(),
        QIcon((boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/icons/sensor.png").string().c_str())
        });
}


void  register_record_undo_redo_processors(widgets* const  w)
{
    scn::scene_history_sensor_insert::set_undo_processor(
        [w](scn::scene_history_sensor_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_sensors_folder_name());
            w->wnd()->glwindow().call_now(&simulator::erase_sensor, std::cref(history_node.get_id()));
            remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
        });
    scn::scene_history_sensor_insert::set_redo_processor(
        [w](scn::scene_history_sensor_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_sensors_folder_name());
            w->wnd()->glwindow().call_now(&simulator::insert_sensor, std::cref(history_node.get_id()), std::cref(history_node.get_props()));
            insert_record_to_tree_widget(
                    w->scene_tree(),
                    history_node.get_id(),
                    w->get_record_icon(scn::get_sensors_folder_name()),
                    w->get_folder_icon());
        });

    scn::scene_history_sensor_update_props::set_undo_processor(
        [w](scn::scene_history_sensor_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_sensors_folder_name());
            w->wnd()->glwindow().call_now(&simulator::erase_sensor, std::cref(history_node.get_id()));
            w->wnd()->glwindow().call_now(&simulator::insert_sensor, std::cref(history_node.get_id()), std::cref(history_node.get_old_props()));
        });
    scn::scene_history_sensor_update_props::set_redo_processor(
        [w](scn::scene_history_sensor_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_sensors_folder_name());
            w->wnd()->glwindow().call_now(&simulator::erase_sensor, std::cref(history_node.get_id()));
            w->wnd()->glwindow().call_now(&simulator::insert_sensor, std::cref(history_node.get_id()), std::cref(history_node.get_new_props()));
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
            scn::get_sensors_folder_name(),
            {
                true, // Allows multiple records in the folder (i.e. to open the name-selection dialog).
                [](widgets* const  w, std::string const&, std::unordered_set<std::string> const&  used_names)
                    -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                        return {
                            "sensor",
                            [w](scn::scene_record_id const&  record_id) -> void {
                                    scn::sensor_props const  props {
                                        ai::SENSOR_KIND::DEFAULT
                                    };
                                    w->wnd()->glwindow().call_now(&simulator::insert_sensor, std::cref(record_id), std::cref(props));
                                    w->get_scene_history()->insert<scn::scene_history_sensor_insert>(record_id, props, false);
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
            scn::get_sensors_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  record_id) -> void {
                    scn::sensor_props  old_props;
                    w->wnd()->glwindow().call_now(&simulator::get_sensor_info, std::cref(record_id), std::ref(old_props));
                    dialog_windows::sensor_props_dialog  dlg(w->wnd(), old_props);
                    dlg.exec();
                    if (!dlg.ok())
                        return;
                    w->get_scene_history()->insert<scn::scene_history_sensor_update_props>(
                            record_id,
                            old_props,
                            dlg.get_new_props(),
                            false);
                    w->wnd()->glwindow().call_now(&simulator::erase_sensor, std::cref(record_id));
                    w->wnd()->glwindow().call_now(&simulator::insert_sensor, std::cref(record_id), std::cref(dlg.get_new_props()));
                }
            });
}


void  register_record_handler_for_duplicate_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&, scn::scene_record_id const&)> >&
                duplicate_record_handlers
        )
{
    duplicate_record_handlers.insert({
            scn::get_sensors_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  src_record_id, scn::scene_record_id const&  dst_record_id) -> void {
                    scn::sensor_props  props;
                    w->wnd()->glwindow().call_now(&simulator::get_sensor_info, std::cref(src_record_id), std::ref(props));
                    w->wnd()->glwindow().call_now(&simulator::insert_sensor, std::cref(dst_record_id), std::cref(props));
                    w->get_scene_history()->insert<scn::scene_history_sensor_insert>(dst_record_id, props, false);
                }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_sensors_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id) -> void {
                    scn::sensor_props  props;
                    w->wnd()->glwindow().call_now(&simulator::get_sensor_info, std::cref(id), std::ref(props));
                    w->get_scene_history()->insert<scn::scene_history_sensor_insert>(id, props, true);
                    w->wnd()->glwindow().call_now(&simulator::erase_sensor, std::cref(id));
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
            scn::get_sensors_folder_name(),
            []( widgets* const  w,
                scn::scene_record_id const&  id,
                boost::property_tree::ptree const&  data,
                std::unordered_map<std::string, boost::property_tree::ptree> const&  infos,
                bool const  do_update_history) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::load_sensor,
                            std::cref(data),
                            std::cref(id)
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_sensors_folder_name()),
                            w->get_folder_icon()
                            );
                    if (do_update_history)
                    {
                        scn::sensor_props  props;
                        w->wnd()->glwindow().call_now(&simulator::get_sensor_info, std::cref(id), std::ref(props));
                        w->get_scene_history()->insert<scn::scene_history_sensor_insert>(id, props, false);
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
            scn::get_sensors_folder_name(),
            []( widgets* const  w,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data,
                std::unordered_map<std::string, boost::property_tree::ptree>&  infos) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::save_sensor,
                            node_ptr,
                            std::cref(id),
                            std::ref(data)
                            );
                }
            });
}


}}}
