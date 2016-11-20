#ifndef E2_TOOL_NETVIEWER_PROGRAM_WINDOW_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_PROGRAM_WINDOW_HPP_INCLUDED

#   include <netviewer/simulator.hpp>
#   include <netviewer/window_tabs/tab_camera.hpp>
#   include <netviewer/window_tabs/tab_draw.hpp>
#   include <netviewer/status_bar.hpp>
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

public slots:

    void  on_tab_changed(int const  tab_index);

    /// Slots for DRAW tab
    void  on_clear_colour_changed() { m_tab_draw_widgets.on_clear_colour_changed(); }
    void  on_clear_colour_set(QColor const&  colour) { m_tab_draw_widgets.on_clear_colour_set(colour); }
    void  on_clear_colour_choose() { m_tab_draw_widgets.on_clear_colour_choose(); }
    void  on_clear_colour_reset() { m_tab_draw_widgets.on_clear_colour_reset(); }

    /// Slots for CAMERA tab
    void  on_camera_pos_changed() { m_tab_camera_widgets.on_camera_pos_changed(); }
    void  camera_position_listener() { m_tab_camera_widgets.camera_position_listener(); }
    void  on_camera_rot_changed() { m_tab_camera_widgets.on_camera_rot_changed(); }
    void  on_camera_rot_tait_bryan_changed() { m_tab_camera_widgets.on_camera_rot_tait_bryan_changed(); }
    void  camera_rotation_listener() { m_tab_camera_widgets.camera_rotation_listener(); }
    void  update_camera_rot_widgets(quaternion const&  q) { m_tab_camera_widgets.update_camera_rot_widgets(q); }
    void  on_camera_far_changed() { m_tab_camera_widgets.on_camera_far_changed(); }
    void  dbg_on_camera_far_changed() { m_tab_camera_widgets.dbg_on_camera_far_changed(); }
    void  dbg_on_camera_sync_changed(int value) { m_tab_camera_widgets.dbg_on_camera_sync_changed(value); }

//    void  on_nenet_param_simulation_speed_changed();
//    void  on_nenet_param_time_step_changed();
//    void  on_nenet_param_mini_spiking_potential_magnitude();
//    void  on_nenet_param_average_mini_spiking_period_in_seconds();
//    void  on_nenet_param_spiking_potential_magnitude();
//    void  on_nenet_param_resting_potential();
//    void  on_nenet_param_spiking_threshold();
//    void  on_nenet_param_after_spike_potential();
//    void  on_nenet_param_potential_descend_coef();
//    void  on_nenet_param_potential_ascend_coef();
//    void  on_nenet_param_max_connection_distance();
//    void  on_nenet_param_output_terminal_velocity_max_magnitude();
//    void  on_nenet_param_output_terminal_velocity_min_magnitude();

//    void on_selection_changed();

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

    QWidget*  m_gl_window_widget;

    QSplitter*  m_splitter;
    QTabWidget*  m_tabs;

    window_tabs::tab_draw::widgets  m_tab_draw_widgets;
    window_tabs::tab_camera::widgets  m_tab_camera_widgets;

    status_bar  m_status_bar;

//    QLineEdit*  m_nenet_param_time_step;
//    QLineEdit*  m_nenet_param_simulation_speed;
//    QLineEdit*  m_nenet_param_mini_spiking_potential_magnitude;
//    QLineEdit*  m_nenet_param_average_mini_spiking_period_in_seconds;
//    QLineEdit*  m_nenet_param_spiking_potential_magnitude;
//    QLineEdit*  m_nenet_param_resting_potential;
//    QLineEdit*  m_nenet_param_spiking_threshold;
//    QLineEdit*  m_nenet_param_after_spike_potential;
//    QLineEdit*  m_nenet_param_potential_descend_coef;
//    QLineEdit*  m_nenet_param_potential_ascend_coef;
//    QLineEdit*  m_nenet_param_max_connection_distance;
//    QLineEdit*  m_nenet_param_output_terminal_velocity_max_magnitude;
//    QLineEdit*  m_nenet_param_output_terminal_velocity_min_magnitude;

//    QTextEdit*  m_selected_props;
};


#endif
