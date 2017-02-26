#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_DRAW_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_DRAW_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <QWidget>
#   include <QLineEdit>
#   include <QCheckBox>
#   include <QColor>


struct  program_window;


namespace window_tabs { namespace tab_draw {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    QLineEdit* camera_pos_x() const { return m_camera_pos_x; }
    QLineEdit* camera_pos_y() const { return m_camera_pos_y; }
    QLineEdit* camera_pos_z() const { return m_camera_pos_z; }

    QLineEdit* camera_rot_w() const { return m_camera_rot_w; }
    QLineEdit* camera_rot_x() const { return m_camera_rot_x; }
    QLineEdit* camera_rot_y() const { return m_camera_rot_y; }
    QLineEdit* camera_rot_z() const { return m_camera_rot_z; }

    QLineEdit* camera_yaw() const { return m_camera_yaw; }
    QLineEdit* camera_pitch() const { return m_camera_pitch; }
    QLineEdit* camera_roll() const { return m_camera_roll; }

    QCheckBox* camera_save_pos_rot() const { return m_camera_save_pos_rot; }

    QLineEdit* camera_far_plane() const { return m_camera_far_plane; }
    QLineEdit* camera_speed() const { return m_camera_speed; }

    QLineEdit*  clear_colour_component_red() const { return m_clear_colour_component_red; }
    QLineEdit*  clear_colour_component_green() const { return m_clear_colour_component_green; }
    QLineEdit*  clear_colour_component_blue() const { return m_clear_colour_component_blue; }

    QCheckBox* show_grid() const { return m_show_grid; }

    void  on_camera_pos_changed();
    void  camera_position_listener();

    void  on_camera_rot_changed();
    void  on_camera_rot_tait_bryan_changed();
    void  camera_rotation_listener();
    void  update_camera_rot_widgets(quaternion const&  q);

    void  on_camera_far_changed();
    void  on_camera_speed_changed();

    void  on_clear_colour_changed();
    void  on_clear_colour_set(QColor const&  colour);
    void  on_clear_colour_choose();
    void  on_clear_colour_reset();

    void  on_show_grid_changed(int const  value);

    void  save();

private:
    program_window*  m_wnd;

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

    QLineEdit*  m_camera_far_plane;
    QLineEdit*  m_camera_speed;

    QLineEdit*  m_clear_colour_component_red;
    QLineEdit*  m_clear_colour_component_green;
    QLineEdit*  m_clear_colour_component_blue;

    QCheckBox*  m_show_grid;
};


QWidget*  make_draw_tab_content(widgets const&  w);


}}

#endif
