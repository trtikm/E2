#ifndef E2_TOOL_GFXTUNER_PROGRAM_WINDOW_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_PROGRAM_WINDOW_HPP_INCLUDED

#   include <gfxtuner/simulation/simulator.hpp>
#   include <gfxtuner/window_tabs/tab_draw.hpp>
#   include <gfxtuner/window_tabs/tab_scene.hpp>
#   include <gfxtuner/window_tabs/tab_statistics.hpp>
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

    void  print_status_message(std::string const&  msg, natural_32_bit const  num_miliseconds_to_show=2000)
    { m_status_bar.print_status_message(msg, num_miliseconds_to_show); }

    void  set_title(std::string const&  text = "");
    void  set_focus_to_glwindow(bool const  immediatelly = true)
    {
        if (immediatelly)
        {
            if (!m_gl_window_widget->hasFocus())
                m_gl_window_widget->setFocus();
        }
        else
            m_focus_just_received = true;
    }

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
    void  on_draw_save_pos_rot_changed(int const  value) { m_tab_draw_widgets.on_save_pos_rot_changed(value); }

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

    void  on_scene_coord_system_position_started() { m_tab_scene_widgets.on_coord_system_position_started(); }
    void  scene_coord_system_position_listener() { m_tab_scene_widgets.coord_system_position_listener(); }
    void  on_scene_coord_system_position_finished() { m_tab_scene_widgets.on_coord_system_position_finished(); }

    void  on_scene_coord_system_rotation_started() { m_tab_scene_widgets.on_coord_system_rotation_started(); }
    void  scene_coord_system_rotation_listener() { m_tab_scene_widgets.coord_system_rotation_listener(); }
    void  on_scene_coord_system_rotation_finished() { m_tab_scene_widgets.on_coord_system_rotation_finished(); }

    void  scene_selection_listener() { m_tab_scene_widgets.selection_changed_listener(); }
    void  scene_pause_listener() { m_tab_scene_widgets.on_pause(); }

    /// Status bar
    void  status_bar_edit_mode_listener() { m_status_bar.edit_mode_listener(); }

    /// Slots for menu actions
    void  on_menu_new_scene() { m_menu_bar.on_file_action_new_scene(); m_tab_scene_widgets.clear_scene(); }
    void  on_menu_open_scene() { if (m_menu_bar.on_file_action_open_scene()) m_tab_scene_widgets.open_scene(get_current_scene_dir()); }
    void  on_menu_save_scene() { if (m_menu_bar.on_file_action_save_scene()) m_tab_scene_widgets.save_scene(get_current_scene_dir()); }
    void  on_menu_save_as_scene() { if (m_menu_bar.on_file_action_save_as_scene()) m_tab_scene_widgets.save_scene(get_current_scene_dir()); }
    void  on_menu_exit() { if (m_menu_bar.on_file_action_exit()) PostQuitMessage(0); }

    void  on_menu_edit_insert_coord_system() { m_tab_scene_widgets.on_scene_insert_coord_system(); }
    void  on_menu_edit_insert_batch() { m_tab_scene_widgets.on_scene_insert_batch(); }
    void  on_menu_edit_erase_selected() { m_tab_scene_widgets.on_scene_erase_selected(); }
    void  on_menu_edit_mode_selection() { m_tab_scene_widgets.on_scene_mode_selection(); }
    void  on_menu_edit_mode_translation() { m_tab_scene_widgets.on_scene_mode_translation(); }
    void  on_menu_edit_mode_rotation() { m_tab_scene_widgets.on_scene_mode_rotation(); }
    void  on_menu_edit_toggle_pivot_selection() { m_tab_scene_widgets.on_scene_toggle_pivot_selection(); }
    void  on_menu_edit_move_selection_to_pivot() { m_tab_scene_widgets.on_scene_move_selection_to_pivot(); }
    void  on_menu_edit_move_pivot_to_selection() { m_tab_scene_widgets.on_scene_move_pivot_to_selection(); }
    void  on_menu_edit_undo() { m_tab_scene_widgets.on_scene_undo(); }
    void  on_menu_edit_redo() { m_tab_scene_widgets.on_scene_redo(); }

    void  on_menu_view_double_camera_speed() { m_tab_draw_widgets.on_double_camera_speed(); }
    void  on_menu_view_half_camera_speed() { m_tab_draw_widgets.on_half_camera_speed(); }
    void  on_menu_view_look_at_selection()
    {
        m_tab_scene_widgets.on_look_at_selection(
            std::bind(
                &window_tabs::tab_draw::widgets::on_look_at,
                &m_tab_draw_widgets,
                std::placeholders::_1,
                std::placeholders::_2
                )
            );
    }

    boost::filesystem::path&  get_current_scene_dir() { return m_menu_bar.get_current_scene_dir(); }


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
    window_tabs::tab_statistics::widgets*  m_tab_statistics_widgets;

    menu_bar  m_menu_bar;
    status_bar  m_status_bar;
};


#endif
