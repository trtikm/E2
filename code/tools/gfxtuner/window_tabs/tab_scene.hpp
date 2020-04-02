#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_HPP_INCLUDED

#   include <gfxtuner/window_tabs/tab_scene_tree_widget.hpp>
#   include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#   include <angeo/tensor_math.hpp>
#   include <scene/scene.hpp>
#   include <scene/scene_history.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <boost/filesystem.hpp>
#   include <boost/filesystem/path.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QLineEdit>
#   include <QPushButton>
#   include <QTreeWidget>
#   include <QColor>
#   include <QIcon>
#   include <string>
#   include <vector>
#   include <unordered_set>
#   include <unordered_map>
#   include <functional>
#   include <tuple>


struct  program_window;


namespace window_tabs { namespace tab_scene {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    tree_widget*  scene_tree() const { return m_scene_tree; }

    QLineEdit* coord_system_pos_x() const { return m_coord_system_pos_x; }
    QLineEdit* coord_system_pos_y() const { return m_coord_system_pos_y; }
    QLineEdit* coord_system_pos_z() const { return m_coord_system_pos_z; }

    QLineEdit* coord_system_rot_w() const { return m_coord_system_rot_w; }
    QLineEdit* coord_system_rot_x() const { return m_coord_system_rot_x; }
    QLineEdit* coord_system_rot_y() const { return m_coord_system_rot_y; }
    QLineEdit* coord_system_rot_z() const { return m_coord_system_rot_z; }

    QLineEdit* coord_system_yaw() const { return m_coord_system_yaw; }
    QLineEdit* coord_system_pitch() const { return m_coord_system_pitch; }
    QLineEdit* coord_system_roll() const { return m_coord_system_roll; }

    QLineEdit* get_coord_system_axis_x_x() const { return m_coord_system_axis_x_x; }
    QLineEdit* get_coord_system_axis_x_y() const { return m_coord_system_axis_x_y; }
    QLineEdit* get_coord_system_axis_x_z() const { return m_coord_system_axis_x_z; }
    QLineEdit* get_coord_system_axis_y_x() const { return m_coord_system_axis_y_x; }
    QLineEdit* get_coord_system_axis_y_y() const { return m_coord_system_axis_y_y; }
    QLineEdit* get_coord_system_axis_y_z() const { return m_coord_system_axis_y_z; }
    QLineEdit* get_coord_system_axis_z_x() const { return m_coord_system_axis_z_x; }
    QLineEdit* get_coord_system_axis_z_y() const { return m_coord_system_axis_z_y; }
    QLineEdit* get_coord_system_axis_z_z() const { return m_coord_system_axis_z_z; }

    QPushButton* get_coord_system_reset_button() const { return m_coord_system_reset_button; }

    void  on_simulator_started();
    void  process_pending_scene_load_requst_if_any();

    void  on_scene_hierarchy_item_selected();
    void  on_scene_hierarchy_item_update_action(tree_widget_item* const  item);
    void  on_scene_insert_coord_system();
    void  on_scene_insert_record(std::string const&  record_kind, std::string const&  mode);
    void  on_scene_erase_selected();
    void  on_scene_duplicate_selected();
    void  on_scene_change_parent_of_selected();
    void  on_scene_rename_scene_object();

    void  on_coord_system_pos_changed();
    void  on_coord_system_rot_changed();
    void  on_coord_system_rot_tait_bryan_changed();

    void  on_coord_system_position_started();
    void  coord_system_position_listener();
    void  on_coord_system_position_finished();

    void  on_coord_system_rotation_started();
    void  coord_system_rotation_listener();
    void  on_coord_system_rotation_finished();

    void  on_scene_coord_system_reset();

    void  on_scene_mode_selection();
    void  on_scene_mode_translation();
    void  on_scene_mode_rotation();

    void  on_scene_toggle_pivot_selection();
    void  on_scene_move_selection_to_pivot();
    void  on_scene_move_pivot_to_selection();
    void  on_scene_agent_reset_skeleton_pose();

    void  on_scene_undo();
    void  on_scene_redo();

    void  on_look_at_selection(std::function<void(vector3 const&, float_32_bit const*)> const&  solver);

    void  on_simulation_paused();
    void  on_simulation_resumed();

    void  simulation_nodes_changed_listener();

    void  on_scene_escape_widget();

    bool  is_editing_enabled() const;

    void  selection_changed_listener();
    bool  processing_selection_change() const { return m_processing_selection_change; }

    scn::scene_history_ptr  get_scene_history();

    QIcon const&  get_node_icon() const { return m_node_icon; }
    QIcon const&  get_folder_icon() const { return m_folder_icon; }
    QIcon const&  get_record_icon(std::string const&  record_kind) const { return m_icons_of_records.at(record_kind); }

    void  clear_scene();
    void  open_scene(boost::filesystem::path const&  scene_root_dir);
    void  save_scene(boost::filesystem::path const&  scene_root_dir);

    void  save();
    void  reload_scene(boost::filesystem::path const&  scene_root_dir);

    void  import_scene(boost::filesystem::path const&  scene_root_dir);

private:
    void  duplicate_subtree(
            tree_widget_item const* const  source_item,
            tree_widget_item* const  target_item,
            tree_widgent_items_cache const&  tree_item_children_cache
            );
    std::string  choose_scene_node_name(std::string const&  orig_name, scn::scene_node_id const&  parent_item_id);

    tree_widget_item*  insert_coord_system(
                scn::scene_node_id const&  id,
                vector3 const&  origin,
                quaternion const&  orientation,
                tree_widget_item* const  parent_tree_item
                );

    void  erase_coord_system(scn::scene_node_id const&  id, tree_widget_item* const  tree_item);

    void  add_tree_item_to_selection(tree_widget_item* const  item);

    void  erase_scene_record(scn::scene_record_id const&  id);
    void  erase_subtree_at_root_item(tree_widget_item* const  root_item, std::unordered_set<void*>&  erased_items, bool const  is_root = true);

    void  load_scene_record(
            scn::scene_record_id const&  id,
            boost::property_tree::ptree const&  data,
            std::unordered_map<std::string, boost::property_tree::ptree> const&  infos,
            bool const  do_update_history
            );
    void  save_scene_record(
            scn::scene_node_ptr const  node_ptr,
            scn::scene_node_record_id const&  id,
            boost::property_tree::ptree&  data,
            std::unordered_map<std::string, boost::property_tree::ptree>&  infos
            );

    tree_widget_item*  load_scene_node(
            scn::scene_node_id const&  id,
            boost::property_tree::ptree const&  node_tree,
            std::unordered_map<std::string, boost::property_tree::ptree> const&  infos,
            tree_widget_item*  parent_item
            );

    void  save_scene_node(
            tree_widget_item* const  item_ptr,
            boost::property_tree::ptree&  save_tree,
            std::unordered_map<std::string, boost::property_tree::ptree>&  infos
            );

    tree_widget_item*  import_scene_node(
            scn::scene_node_id const&  id,
            boost::property_tree::ptree const&  node_tree,
            std::unordered_map<std::string, boost::property_tree::ptree> const&  infos,
            tree_widget_item*  parent_item
            );

    void  update_coord_system_location_widgets();
    void  enable_coord_system_location_widgets(bool const  state, bool const  read_only);
    void  refresh_text_in_coord_system_location_widgets(scn::scene_node_ptr const  node_ptr);
    void  refresh_text_in_coord_system_position_widgets(vector3 const&  p);
    void  refresh_text_in_coord_system_rotation_widgets(quaternion const&  q);

    void  set_window_title();

    void  rebuild_tree_widget_according_to_scene(
            scn::scene_node_ptr const  node_ptr,
            tree_widget_item* const  parent_tree_item
            );

    program_window*  m_wnd;

    tree_widget*  m_scene_tree;
    QIcon  m_node_icon;
    QIcon  m_folder_icon;
    std::unordered_map<std::string, QIcon> m_icons_of_records;

    std::unordered_map<std::string, std::pair<bool,
                       std::function<std::pair<std::string, std::function<bool(scn::scene_record_id const&)>>
                                     (widgets*, std::string const&, std::unordered_set<std::string> const&)> > >
            m_insert_record_handlers;
    std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)> >
            m_update_record_handlers;
    std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&, scn::scene_record_id const&)> >
            m_duplicate_record_handlers;
    std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>
            m_erase_record_handlers;
    std::unordered_map<std::string, std::function<void(widgets*,
                                                       scn::scene_record_id const&,
                                                       boost::property_tree::ptree const&,
                                                       std::unordered_map<std::string, boost::property_tree::ptree> const&,
                                                       bool)>>
            m_load_record_handlers;
    std::unordered_map<std::string, std::function<void(widgets*,
                                                       scn::scene_node_ptr,
                                                       scn::scene_node_record_id const&,
                                                       boost::property_tree::ptree&,
                                                       std::unordered_map<std::string, boost::property_tree::ptree>&)>>
            m_save_record_handlers;

    bool  m_processing_selection_change;
    bool  m_is_coord_system_pos_change_processing_disabled;

    natural_64_bit  m_save_commit_id;

    QLineEdit*  m_coord_system_pos_x;
    QLineEdit*  m_coord_system_pos_y;
    QLineEdit*  m_coord_system_pos_z;

    QLineEdit*  m_coord_system_rot_w;
    QLineEdit*  m_coord_system_rot_x;
    QLineEdit*  m_coord_system_rot_y;
    QLineEdit*  m_coord_system_rot_z;

    QLineEdit*  m_coord_system_yaw;
    QLineEdit*  m_coord_system_pitch;
    QLineEdit*  m_coord_system_roll;

    QLineEdit*  m_coord_system_axis_x_x;
    QLineEdit*  m_coord_system_axis_x_y;
    QLineEdit*  m_coord_system_axis_x_z;
    QLineEdit*  m_coord_system_axis_y_x;
    QLineEdit*  m_coord_system_axis_y_y;
    QLineEdit*  m_coord_system_axis_y_z;
    QLineEdit*  m_coord_system_axis_z_x;
    QLineEdit*  m_coord_system_axis_z_y;
    QLineEdit*  m_coord_system_axis_z_z;

    QPushButton*  m_coord_system_reset_button;

    std::unordered_map<scn::scene_node_id, angeo::coordinate_system>  m_coord_system_location_backup_buffer;

    boost::filesystem::path  m_pending_scene_dir_to_load;

    std::vector<scn::scene_record_id>  m_selected_tree_items_on_simulation_resumed_event;
};


QWidget*  make_scene_tab_content(widgets const&  w);


}}

#endif
