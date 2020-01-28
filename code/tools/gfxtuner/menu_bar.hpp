#ifndef E2_TOOL_GFXTUNER_MENU_BAR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_MENU_BAR_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <QWidget>
#   include <QMenuBar>
#   include <QMenu>
#   include <QAction>
#   include <boost/filesystem.hpp>
#   include <deque>
#   include <vector>
#   include <string>
#   include <tuple>

struct  program_window;


struct  menu_bar
{
    struct  record_item_info
    {
        bool  is_separator() const { return action_ptr == nullptr; }
        record_item_info() : action_ptr(nullptr), folder_name(), mode() {}
        record_item_info(QAction* const  a, std::string const&  f, std::string const&  m) : action_ptr(a), folder_name(f), mode(m) {}
        QAction*  action_ptr;
        std::string  folder_name;
        std::string  mode;
    };

    menu_bar(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    QMenuBar*  get_menu_bar() const { return m_menu_bar; }

    QMenu*  get_menu_file() const { return m_menu_file; }
    QAction*  get_file_action_new_scene() const { return m_file_action_new_scene; }
    QAction*  get_file_action_open_scene() const { return m_file_action_open_scene; }
    QMenu*    get_file_submenu_open_recent_scene() const { return m_file_submenu_open_recent_scene; }
    QAction*  get_file_action_reload_scene() const { return m_file_action_reload_scene; }
    QAction*  get_file_action_import_scene() const { return m_file_action_import_scene; }
    QAction*  get_file_action_save_scene() const { return m_file_action_save_scene; }
    QAction*  get_file_action_save_as_scene() const { return m_file_action_save_as_scene; }
    QAction*  get_file_action_exit() const { return m_file_action_exit; }

    QMenu*  get_menu_edit() const { return m_menu_edit; }
    QAction*  get_edit_action_insert_coord_system() const { return m_edit_action_insert_coord_system; }
    std::vector<record_item_info> const&  get_edit_actions_of_records() const { return m_record_menu_items; }
    QAction*  get_edit_action_erase_selected() const { return m_edit_action_erase_selected; }
    QAction*  get_edit_action_duplicate_selected() const { return m_edit_action_duplicate_selected; }
    QAction*  get_edit_action_change_parent_of_selected() const { return m_edit_action_change_parent_of_selected; }
    QAction*  get_edit_action_rename_scene_object() const { return m_edit_action_rename_scene_object; }
    QAction*  get_edit_action_mode_select() const { return m_edit_action_mode_select; }
    QAction*  get_edit_action_mode_translate() const { return m_edit_action_mode_translate; }
    QAction*  get_edit_action_mode_rotate() const { return m_edit_action_mode_rotate; }
    QAction*  get_edit_action_toggle_pivot_selection() const { return m_edit_action_toggle_pivot_selection; }
    QAction*  get_edit_action_move_selection_to_pivot() const { return m_edit_action_move_selection_to_pivot; }
    QAction*  get_edit_action_move_pivot_to_selection() const { return m_edit_action_move_pivot_to_selection; }
    QAction*  get_edit_action_agent_reset_skeleton_pose() const { return m_edit_action_agent_reset_skeleton_pose; }
    QAction*  get_edit_action_undo() const { return m_edit_action_undo; }
    QAction*  get_edit_action_redo() const { return m_edit_action_redo; }

    QMenu*  get_menu_simulation() const { return m_menu_simulation; }
    QAction*  get_simulation_action_toggle_pause() const { return m_simulation_action_toggle_pause; }
    QAction*  get_simulation_action_single_step() const { return m_simulation_action_single_step; }

    QMenu*  get_menu_view() const noexcept { return m_menu_view; }
    QAction*  get_view_action_double_camera_speed() const { return m_view_action_double_camera_speed; }
    QAction*  get_view_action_half_camera_speed() const { return m_view_action_half_camera_speed; }
    QAction*  get_view_action_look_at_selection() const { return m_view_action_look_at_selection; }

    // "File" actions
    void  on_file_action_new_scene();
    std::string  on_file_action_open_scene();
    std::string  on_file_action_reload_scene();
    std::string  on_file_action_import_scene();
    std::string  on_file_action_save_scene();
    std::string  on_file_action_save_as_scene();
    bool  on_file_action_exit();

    boost::filesystem::path const&  get_default_scene_root_dir() const { return m_default_scene_root_dir; }
    boost::filesystem::path const&  get_current_scene_dir() const { return m_current_scene_dir; }
    void  set_current_scene_dir(boost::filesystem::path const&  path);

    void  on_simulation_paused();
    void  on_simulation_resumed();

    void  save();

    void  rebuild_recent_scenes_submenu();

private:
    void  load();
    void  toggle_enable_state_of_menu_items_for_simulation_mode(bool const  simulation_resumed);
    std::deque<boost::filesystem::path> const&  get_recent_scenes() const { return m_recent_scenes; }

    program_window*  m_wnd;

    QMenuBar*  m_menu_bar;

    QMenu*  m_menu_file;
    QAction*  m_file_action_new_scene;
    QAction*  m_file_action_open_scene;
    QMenu*  m_file_submenu_open_recent_scene;
    QAction*  m_file_action_reload_scene;
    QAction*  m_file_action_import_scene;
    QAction*  m_file_action_save_scene;
    QAction*  m_file_action_save_as_scene;
    QAction*  m_file_action_exit;
    boost::filesystem::path  m_default_scene_root_dir;
    std::deque<boost::filesystem::path>  m_recent_scenes;
    boost::filesystem::path  m_current_scene_dir;

    QMenu*  m_menu_edit;
    QAction*  m_edit_action_insert_coord_system;
    std::vector<record_item_info>  m_record_menu_items;
    QAction*  m_edit_action_erase_selected;
    QAction*  m_edit_action_duplicate_selected;
    QAction*  m_edit_action_change_parent_of_selected;
    QAction*  m_edit_action_rename_scene_object;
    QAction*  m_edit_action_mode_select;
    QAction*  m_edit_action_mode_translate;
    QAction*  m_edit_action_mode_rotate;
    QAction*  m_edit_action_toggle_pivot_selection;
    QAction*  m_edit_action_move_selection_to_pivot;
    QAction*  m_edit_action_move_pivot_to_selection;
    QAction*  m_edit_action_agent_reset_skeleton_pose;
    QAction*  m_edit_action_undo;
    QAction*  m_edit_action_redo;

    QMenu*  m_menu_simulation;
    QAction* m_simulation_action_toggle_pause;
    QAction* m_simulation_action_single_step;

    QMenu*  m_menu_view;
    QAction*  m_view_action_double_camera_speed;
    QAction*  m_view_action_half_camera_speed;
    QAction*  m_view_action_look_at_selection;
};


void  make_menu_bar_content(menu_bar&  w);


#endif
