#ifndef E2_TOOL_GFXTUNER_PROGRAM_WINDOW_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_PROGRAM_WINDOW_HPP_INCLUDED

#   include <gfxtuner/simulator.hpp>
#   include <gfxtuner/window_tabs/tab_draw.hpp>
#   include <gfxtuner/window_tabs/tab_scene.hpp>
#   include <gfxtuner/status_bar.hpp>
#   include <gfxtuner/menu_bar.hpp>
#   include <qtgl/window.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/filesystem/path.hpp>
#   include <QMainWindow>
#   include <QWidget>
#   include <QSplitter>
#   include <QTabWidget>
#   include <QColor>
#   include <QEvent>
#   include <QTimerEvent>
#   include <QCloseEvent>
#   include <memory>


struct program_window : public QMainWindow
{
    program_window(boost::filesystem::path const&  ptree_pathname);
    ~program_window();

    boost::property_tree::ptree&  ptree() { return *m_ptree; }
    qtgl::window<simulator>&  glwindow() noexcept { return m_glwindow; }

    void print_status_message(std::string const&  msg, natural_32_bit const  num_miliseconds_to_show=2000)
    { m_status_bar.print_status_message(msg, num_miliseconds_to_show); }

public slots:

    void  on_tab_changed(int const  tab_index);

    /// Slots for DRAW tab
    void  on_draw_camera_pos_changed() { m_tab_draw_widgets.on_camera_pos_changed(); }
    void  on_draw_camera_rot_changed() { m_tab_draw_widgets.on_camera_rot_changed(); }
    void  on_draw_camera_rot_tait_bryan_changed() { m_tab_draw_widgets.on_camera_rot_tait_bryan_changed(); }

    void  on_draw_camera_far_changed() { m_tab_draw_widgets.on_camera_far_changed(); }
    void  on_draw_camera_speed_changed() { m_tab_draw_widgets.on_camera_speed_changed(); }

    void  on_draw_clear_colour_changed() { m_tab_draw_widgets.on_clear_colour_changed(); }
    void  on_draw_clear_colour_set(QColor const&  colour) { m_tab_draw_widgets.on_clear_colour_set(colour); }
    void  on_draw_clear_colour_choose() { m_tab_draw_widgets.on_clear_colour_choose(); }
    void  on_draw_clear_colour_reset() { m_tab_draw_widgets.on_clear_colour_reset(); }

    void  on_draw_show_grid_changed(int const  value) { m_tab_draw_widgets.on_show_grid_changed(value); }

    void  draw_camera_position_listener() { m_tab_draw_widgets.camera_position_listener(); }
    void  draw_camera_rotation_listener() { m_tab_draw_widgets.camera_rotation_listener(); }
    void  draw_update_camera_rot_widgets(quaternion const&  q) { m_tab_draw_widgets.update_camera_rot_widgets(q); }

    /// Slots for SCENE tab
    void on_scene_hierarchy_item_selected() { m_tab_scene_widgets.on_scene_hierarchy_item_selected(); }

    void  on_scene_insert_coord_system() { m_tab_scene_widgets.on_scene_insert_coord_system(); }
    void  on_scene_insert_batch() { m_tab_scene_widgets.on_scene_insert_batch(); }
    void  on_scene_erase_selected() { m_tab_scene_widgets.on_scene_erase_selected(); }

    void  on_scene_coord_system_pos_changed() { m_tab_scene_widgets.on_coord_system_pos_changed(); }
    void  on_scene_coord_system_rot_changed() { m_tab_scene_widgets.on_coord_system_rot_changed(); }
    void  on_scene_coord_system_rot_tait_bryan_changed() { m_tab_scene_widgets.on_coord_system_rot_tait_bryan_changed(); }

    void  scene_coord_system_position_listener() { m_tab_scene_widgets.coord_system_position_listener(); }
    void  scene_coord_system_rotation_listener() { m_tab_scene_widgets.coord_system_rotation_listener(); }
    void  scene_selection_listener() { m_tab_scene_widgets.selection_changed_listener(); }

    /// Status bar
    void  status_bar_edit_mode_listener() { m_status_bar.edit_mode_listener(); }

    /// Slots for menu actions
    void  on_menu_new_scene() { m_menu_bar.on_new_scene(); m_tab_scene_widgets.clear_scene(); }
    void  on_menu_open_scene() { if (m_menu_bar.on_open_scene()) m_tab_scene_widgets.open_scene(m_menu_bar.get_current_scene_dir()); }
    void  on_menu_save_scene() { if (m_menu_bar.on_save_scene()) m_tab_scene_widgets.save_scene(m_menu_bar.get_current_scene_dir()); }
    void  on_menu_save_as_scene() { if (m_menu_bar.on_save_as_scene()) m_tab_scene_widgets.save_scene(m_menu_bar.get_current_scene_dir()); }
    void  on_menu_exit() { if (m_menu_bar.on_exit()) PostQuitMessage(0); }

private:

    Q_OBJECT

    bool  event(QEvent* const event);
    void  timerEvent(QTimerEvent* const event);
    void  closeEvent(QCloseEvent* const  event);

    boost::filesystem::path  m_ptree_pathname;
    std::unique_ptr<boost::property_tree::ptree>  m_ptree;
    qtgl::window<simulator>  m_glwindow;
    bool  m_has_focus;
    bool  m_focus_just_received;
    int  m_idleTimerId;
    bool  m_is_this_first_timer_event;

    QWidget*  m_gl_window_widget;

    QSplitter*  m_splitter;
    QTabWidget*  m_tabs;

    window_tabs::tab_draw::widgets  m_tab_draw_widgets;
    window_tabs::tab_scene::widgets  m_tab_scene_widgets;

    menu_bar  m_menu_bar;
    status_bar  m_status_bar;
};


#endif
