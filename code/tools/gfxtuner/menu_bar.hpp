#ifndef E2_TOOL_GFXTUNER_MENU_BAR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_MENU_BAR_HPP_INCLUDED

#   include <QWidget>
#   include <QMenuBar>
#   include <QMenu>
#   include <QAction>
#   include <vector>
#   include <boost/filesystem.hpp>

struct  program_window;


struct  menu_bar
{
    menu_bar(program_window* const  wnd);

    program_window*  wnd() const noexcept { return m_wnd; }

    QMenuBar*  get_menu_bar() const noexcept { return m_menu_bar; }

    QMenu*  get_menu_file() const noexcept { return m_menu_file; }
    QAction*  get_action_new_scene() const noexcept { return m_action_new_scene; }
    QAction*  get_action_open_scene() const noexcept { return m_action_open_scene; }
    QMenu*    get_menu_open_recent_scene() const noexcept { return m_menu_open_recent_scene; }
    QAction*  get_action_save_scene() const noexcept { return m_action_save_scene; }
    QAction*  get_action_save_as_scene() const noexcept { return m_action_save_as_scene; }
    QAction*  get_action_exit() const noexcept { return m_action_exit; }

    // "File" actions
    void  on_new_scene();
    bool  on_open_scene();
    void  on_open_recent_scene();
    bool  on_save_scene();
    bool  on_save_as_scene();
    bool  on_exit();

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
    QAction*  m_action_new_scene;
    QAction*  m_action_open_scene;
    QMenu*  m_menu_open_recent_scene;
    QAction*  m_action_save_scene;
    QAction*  m_action_save_as_scene;
    QAction*  m_action_exit;
    boost::filesystem::path  m_default_scene_root_dir;
    std::vector<boost::filesystem::path>  m_recent_scenes;
    boost::filesystem::path  m_current_scene_dir;
};


void  make_menu_bar_content(menu_bar const&  w);


#endif
