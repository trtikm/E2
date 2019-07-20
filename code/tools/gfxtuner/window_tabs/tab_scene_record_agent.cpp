#include <gfxtuner/window_tabs/tab_scene_record_batch.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene.hpp>
#include <scene/scene_history.hpp>
#include <scene/scene_node_record_id.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <scene/records/agent/agent.hpp>
#include <ai/agents.hpp>
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

namespace window_tabs { namespace tab_scene { namespace record_agent { namespace detail {


void  insert_skeleton_joint_nodes_under_agent_node(
        scn::scene_record_id const&  agent_record_id,
        scn::skeleton_props_const_ptr const  skeleton_props,
        widgets* const  w
        )
{
    std::vector<std::pair<scn::scene_node_id, tree_widget_item*> >  inserted_nodes{
            { agent_record_id.get_node_id(), w->scene_tree()->find(agent_record_id.get_node_id()) }
            };
    for (natural_32_bit i = 0U; i != skeleton_props->skeletal_motion_templates.pose_frames().size(); ++i)
    {
        scn::scene_node_id const  bone_node_id =
                inserted_nodes.at(skeleton_props->skeletal_motion_templates.hierarchy().parents().at(i) + 1).first
                / skeleton_props->skeletal_motion_templates.names().at(i);
        w->wnd()->glwindow().call_now(
                &simulator::insert_scene_node_at,
                bone_node_id,
                skeleton_props->skeletal_motion_templates.pose_frames().at(i).origin(),
                skeleton_props->skeletal_motion_templates.pose_frames().at(i).orientation()
                );
        auto const  bone_tree_item =
                insert_coord_system_to_tree_widget(
                        w->scene_tree(),
                        bone_node_id,
                        skeleton_props->skeletal_motion_templates.pose_frames().at(i).origin(),
                        skeleton_props->skeletal_motion_templates.pose_frames().at(i).orientation(),
                        w->get_node_icon(),
                        inserted_nodes.at(skeleton_props->skeletal_motion_templates.hierarchy().parents().at(i) + 1).second
                        );
        w->get_scene_history()->insert<scn::scene_history_coord_system_insert>(
                bone_node_id,
                skeleton_props->skeletal_motion_templates.pose_frames().at(i).origin(),
                skeleton_props->skeletal_motion_templates.pose_frames().at(i).orientation(),
                false
                );
        inserted_nodes.push_back({ bone_node_id, bone_tree_item });
    }
}


}}}}

namespace window_tabs { namespace tab_scene { namespace record_agent {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    icons_of_records.insert({
        scn::get_agent_folder_name(),
        QIcon((boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/icons/agent.png").string().c_str())
        });
}


void  register_record_undo_redo_processors(widgets* const  w)
{
}



void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string, std::pair<bool,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)>> >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_agent_folder_name(),
            {
                false, // There cannot be mutiple records in one folder.
                [](widgets* const  w, std::string const&, std::unordered_set<std::string> const&  used_names)
                    -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                        if (used_names.size() != 0UL)
                        {
                            w->wnd()->print_status_message("ERROR: A coordinate system node may contain at most one agent entity.", 10000);
                            return{ "",{} };
                        }

                        boost::filesystem::path  skeleton_dir;
                        {
                            QFileDialog  dialog(w->wnd());
                            dialog.setDirectory(
                                canonical_path(boost::filesystem::absolute(
                                    boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "animations" / "skeletal"
                                    )).string().c_str()
                                );
                            dialog.setFileMode(QFileDialog::Directory);
                            if (!dialog.exec())
                                return{ "",{} };
                            QStringList const  selected_dirs = dialog.selectedFiles();
                            if (selected_dirs.size() != 1)
                            {
                                w->wnd()->print_status_message("ERROR: Exactly one skeleton directory must be provided.", 10000);
                                return{ "",{} };
                            }
                            skeleton_dir = qtgl::to_string(selected_dirs.front());
                        }

                        ai::skeletal_motion_templates const  skeletal_motion_templates(skeleton_dir, 100U);
                        if (!skeletal_motion_templates.wait_till_load_is_finished())
                        {
                            w->wnd()->print_status_message("ERROR: Load of skeletal_motion_templates '" + skeletal_motion_templates.key().get_unique_id() + "' has FAILED!", 10000);
                            return{ "",{} };
                        }

                        return {
                            scn::get_agent_record_name(),
                            [w, skeleton_dir, skeletal_motion_templates](
                                scn::scene_record_id const&  record_id) -> void
                                {
                                    scn::skeleton_props_ptr const  props =
                                        scn::create_skeleton_props(skeleton_dir, skeletal_motion_templates);
                                    detail::insert_skeleton_joint_nodes_under_agent_node(record_id, props, w);
                                    w->wnd()->glwindow().call_now(
                                            &simulator::insert_agent,
                                            std::cref(record_id),
                                            props
                                            );
                                    //w->get_scene_history()->insert<scn::scene_history_agent_insert>(record_id, false);
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
}


void  register_record_handler_for_duplicate_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&, scn::scene_record_id const&)> >&
                duplicate_record_handlers
        )
{
    duplicate_record_handlers.insert({
            scn::get_agent_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  src_record_id, scn::scene_record_id const&  dst_record_id) -> void {
                    scn::skeleton_props_const_ptr const  skeleton_props =
                            w->wnd()->glwindow().call_now(&simulator::get_agent_info, std::cref(src_record_id.get_node_id()));
                    //detail::insert_skeleton_joint_nodes_under_agent_node(dst_record_id, skeleton_props, w);
                    w->wnd()->glwindow().call_now(
                            &simulator::insert_agent,
                            std::cref(dst_record_id),
                            skeleton_props
                            );
                    //w->get_scene_history()->insert<scn::scene_history_agent_insert>(dst_record_id, false);
                }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_agent_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id) -> void {
                    //w->get_scene_history()->insert<scn::scene_history_agent_insert>(id, true);
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_agent,
                            std::cref(id)
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
            scn::get_agent_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id, boost::property_tree::ptree const&  data) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::load_agent,
                            std::cref(data),
                            std::cref(id)
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_agent_folder_name()),
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
            scn::get_agent_folder_name(),
            []( widgets* const  w,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::save_agent,
                            node_ptr,
                            std::ref(data)
                            );
                }
            });
}


}}}
