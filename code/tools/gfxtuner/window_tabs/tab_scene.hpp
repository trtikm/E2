#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <scene/scene.hpp>
#   include <boost/filesystem.hpp>
#   include <boost/filesystem/path.hpp>
#   include <QWidget>
#   include <QTreeWidget>
#   include <QColor>
#   include <string>
#   include <unordered_set>
#   include <unordered_map>
#   include <functional>


struct  program_window;


namespace window_tabs { namespace tab_scene {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    QTreeWidget*  scene_tree() const { return m_scene_tree; }

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

    void  on_simulator_started() { clear_scene(); }

    void  on_scene_hierarchy_item_selected();
    void  on_scene_insert_coord_system();
    void  on_scene_insert_batch();
    void  on_scene_erase_selected();

    void  on_coord_system_pos_changed();
    void  on_coord_system_rot_changed();
    void  on_coord_system_rot_tait_bryan_changed();

    void  on_coord_system_position_started();
    void  coord_system_position_listener();
    void  on_coord_system_position_finished();

    void  on_coord_system_rotation_started();
    void  coord_system_rotation_listener();
    void  on_coord_system_rotation_finished();

    void  on_scene_mode_selection();
    void  on_scene_mode_translation();
    void  on_scene_mode_rotation();

    void  on_scene_toggle_pivot_selection();
    void  on_scene_move_selection_to_pivot();
    void  on_scene_move_pivot_to_selection();

    void  on_scene_undo();
    void  on_scene_redo();

    void  on_look_at_selection(std::function<void(vector3 const&, float_32_bit const*)> const&  solver);

    void  on_pause();

    bool  is_editing_enabled() const;

    void  selection_changed_listener();
    bool  processing_selection_change() const { return m_processing_selection_change; }

    void  clear_scene();
    void  open_scene(boost::filesystem::path const&  scene_root_dir);
    void  save_scene(boost::filesystem::path const&  scene_root_dir);

    void  save();

private:

    QTreeWidgetItem*  insert_coord_system(
                std::string const&  name,
                vector3 const&  origin,
                quaternion const&  orientation,
                QTreeWidgetItem* const  parent_tree_item);
    QTreeWidgetItem*  insert_batch(
                QTreeWidgetItem* const  node_item,
                std::string const&  batch_name,
                boost::filesystem::path const  batch_pathname
                );

    void  add_tree_item_to_selection(QTreeWidgetItem* const  item);

    void  erase_subtree_at_root_item(QTreeWidgetItem* const  root_item, std::unordered_set<QTreeWidgetItem*>&  erased_items);

    void  update_coord_system_location_widgets();
    void  enable_coord_system_location_widgets(bool const  state, bool const  read_only);
    void  refresh_text_in_coord_system_location_widgets(scn::scene_node_ptr const  node_ptr);
    void  refresh_text_in_coord_system_rotation_widgets(quaternion const&  q);

    void  set_window_title();

    program_window*  m_wnd;

    QTreeWidget*  m_scene_tree;
    QIcon  m_node_icon;
    QIcon  m_batch_icon;

    bool  m_processing_selection_change;

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

    std::unordered_map<std::string, angeo::coordinate_system>  m_coord_system_location_backup_buffer;
};


QWidget*  make_scene_tab_content(widgets const&  w);


}}

#endif
