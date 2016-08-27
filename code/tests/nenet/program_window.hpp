#ifndef PROGRAM_WINDOW_HPP_INCLUDED
#   define PROGRAM_WINDOW_HPP_INCLUDED

#   include "./simulator.hpp"
#   include <qtgl/window.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/filesystem/path.hpp>
#   include <QMainWindow>
#   include <QLabel>
#   include <QWidget>
#   include <QSplitter>
#   include <QTabWidget>
#   include <QLineEdit>
#   include <QTextEdit>
#   include <QCheckBox>
#   include <memory>


struct program_window : public QMainWindow
{
    program_window(boost::filesystem::path const&  ptree_pathname);
    ~program_window();

    boost::property_tree::ptree&  ptree() { return *m_ptree; }

public slots:

    void  on_tab_changed(int const  tab_index);

    void  on_clear_colour_changed();
    void  on_clear_colour_set(QColor const&  colour);
    void  on_clear_colour_choose();
    void  on_clear_colour_reset();

    void  on_camera_pos_changed();
    void  camera_position_listener();

    void  on_camera_rot_changed();
    void  on_camera_rot_tait_bryan_changed();
    void  camera_rotation_listener();
    void  update_camera_rot_widgets(quaternion const&  q);

    void  on_nenet_param_simulation_speed_changed();
    void  on_nenet_param_time_step_changed();
    void  on_nenet_param_mini_spiking_potential_magnitude();
    void  on_nenet_param_average_mini_spiking_period_in_seconds();
    void  on_nenet_param_spiking_potential_magnitude();
    void  on_nenet_param_resting_potential();
    void  on_nenet_param_spiking_threshold();
    void  on_nenet_param_after_spike_potential();
    void  on_nenet_param_potential_descend_coef();
    void  on_nenet_param_potential_ascend_coef();
    void  on_nenet_param_max_connection_distance();
    void  on_nenet_param_output_terminal_velocity_max_magnitude();
    void  on_nenet_param_output_terminal_velocity_min_magnitude();

    void on_selection_changed();

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

    QLineEdit*  m_clear_colour_component_red;
    QLineEdit*  m_clear_colour_component_green;
    QLineEdit*  m_clear_colour_component_blue;

    QLineEdit*  m_camera_pos_x;
    QLineEdit*  m_camera_pos_y;
    QLineEdit*  m_camera_pos_z;

    QLineEdit*  m_camera_rot_w;
    QLineEdit*  m_camera_rot_x;
    QLineEdit*  m_camera_rot_y;
    QLineEdit*  m_camera_rot_z;

    QLineEdit*  m_camera_yaw;
    QLineEdit*  m_camera_pitch;
    QLineEdit*  m_camera_roll;

    QCheckBox*  m_camera_save_pos_rot;

    QLineEdit*  m_nenet_param_time_step;
    QLineEdit*  m_nenet_param_simulation_speed;
    QLineEdit*  m_nenet_param_mini_spiking_potential_magnitude;
    QLineEdit*  m_nenet_param_average_mini_spiking_period_in_seconds;
    QLineEdit*  m_nenet_param_spiking_potential_magnitude;
    QLineEdit*  m_nenet_param_resting_potential;
    QLineEdit*  m_nenet_param_spiking_threshold;
    QLineEdit*  m_nenet_param_after_spike_potential;
    QLineEdit*  m_nenet_param_potential_descend_coef;
    QLineEdit*  m_nenet_param_potential_ascend_coef;
    QLineEdit*  m_nenet_param_max_connection_distance;
    QLineEdit*  m_nenet_param_output_terminal_velocity_max_magnitude;
    QLineEdit*  m_nenet_param_output_terminal_velocity_min_magnitude;

    QTextEdit*  m_selected_props;

    QLabel*  m_spent_real_time;
    QLabel*  m_spent_simulation_time;
    QLabel*  m_spent_times_ratio;
    QLabel*  m_num_passed_simulation_steps;
};


#endif
