#ifndef E2_TOOL_GFXTUNER_MENU_BAR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_MENU_BAR_HPP_INCLUDED

#   include <QWidget>
#   include <QMenuBar>
#   include <QMenu>
#   include <QAction>
#   include <boost/filesystem.hpp>
#   include <vector>
#   include <map>
#   include <string>

struct  program_window;


struct  menu_bar
{
    menu_bar(program_window* const  wnd);

    program_window*  wnd() const noexcept { return m_wnd; }

    QMenuBar*  get_menu_bar() const noexcept { return m_menu_bar; }

    QMenu*  get_menu_file() const { return m_menu_file; }
    QAction*  get_file_action_new_scene() const { return m_file_action_new_scene; }
    QAction*  get_file_action_open_scene() const { return m_file_action_open_scene; }
    QMenu*    get_file_submenu_open_recent_scene() const { return m_file_submenu_open_recent_scene; }
    QAction*  get_file_action_save_scene() const { return m_file_action_save_scene; }
    QAction*  get_file_action_save_as_scene() const { return m_file_action_save_as_scene; }
    QAction*  get_file_action_exit() const { return m_file_action_exit; }

    QMenu*  get_menu_edit() const noexcept { return m_menu_edit; }
    QAction*  get_edit_action_insert_coord_system() const { return m_edit_action_insert_coord_system; }
    std::map<std::string, QAction*> const& get_edit_actions_of_records() const { return m_record_menu_items; }
    QAction*  get_edit_action_erase_selected() const { return m_edit_action_erase_selected; }
    QAction*  get_edit_action_mode_select() const { return m_edit_action_mode_select; }
    QAction*  get_edit_action_mode_translate() const { return m_edit_action_mode_translate; }
    QAction*  get_edit_action_mode_rotate() const { return m_edit_action_mode_rotate; }
    QAction*  get_edit_action_toggle_pivot_selection() const { return m_edit_action_toggle_pivot_selection; }
    QAction*  get_edit_action_move_selection_to_pivot() const { return m_edit_action_move_selection_to_pivot; }
    QAction*  get_edit_action_move_pivot_to_selection() const { return m_edit_action_move_pivot_to_selection; }
    QAction*  get_edit_action_undo() const { return m_edit_action_undo; }
    QAction*  get_edit_action_redo() const { return m_edit_action_redo; }

    QMenu*  get_menu_view() const noexcept { return m_menu_view; }
    QAction*  get_view_action_double_camera_speed() const { return m_view_action_double_camera_speed; }
    QAction*  get_view_action_half_camera_speed() const { return m_view_action_half_camera_speed; }
    QAction*  get_view_action_look_at_selection() const { return m_view_action_look_at_selection; }

    // "File" actions
    void  on_file_action_new_scene();
    bool  on_file_action_open_scene();
    void  on_file_action_open_recent_scene();
    bool  on_file_action_save_scene();
    bool  on_file_action_save_as_scene();
    bool  on_file_action_exit();

    boost::filesystem::path const&  get_default_scene_root_dir() const { return m_default_scene_root_dir; }
    std::vector<boost::filesystem::path> const&  get_recent_scenes() const { return m_recent_scenes; }
    std::vector<boost::filesystem::path>&  get_recent_scenes() { return m_recent_scenes; }
    boost::filesystem::path const&  get_current_scene_dir() const { return m_current_scene_dir; }
    boost::filesystem::path&  get_current_scene_dir() { return m_current_scene_dir; }

    // Save menu props/config.
    void  save();

private:
    program_window*  m_wnd;

    QMenuBar*  m_menu_bar;

    QMenu*  m_menu_file;
    QAction*  m_file_action_new_scene;
    QAction*  m_file_action_open_scene;
    QMenu*  m_file_submenu_open_recent_scene;
    QAction*  m_file_action_save_scene;
    QAction*  m_file_action_save_as_scene;
    QAction*  m_file_action_exit;
    boost::filesystem::path  m_default_scene_root_dir;
    std::vector<boost::filesystem::path>  m_recent_scenes;
    boost::filesystem::path  m_current_scene_dir;

    QMenu*  m_menu_edit;
    QAction*  m_edit_action_insert_coord_system;
    std::map<std::string, QAction*>  m_record_menu_items;
    QAction*  m_edit_action_erase_selected;
    QAction*  m_edit_action_mode_select;
    QAction*  m_edit_action_mode_translate;
    QAction*  m_edit_action_mode_rotate;
    QAction*  m_edit_action_toggle_pivot_selection;
    QAction*  m_edit_action_move_selection_to_pivot;
    QAction*  m_edit_action_move_pivot_to_selection;
    QAction*  m_edit_action_undo;
    QAction*  m_edit_action_redo;

    QMenu*  m_menu_view;
    QAction*  m_view_action_double_camera_speed;
    QAction*  m_view_action_half_camera_speed;
    QAction*  m_view_action_look_at_selection;
};


void  make_menu_bar_content(menu_bar const&  w);


#endif
