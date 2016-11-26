#ifndef E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_DRAW_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_DRAW_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QLineEdit>
#   include <QCheckBox>
#   include <QColor>


struct  program_window;


namespace window_tabs { namespace tab_draw {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const noexcept;

    QLineEdit* camera_pos_x() const noexcept;
    QLineEdit* camera_pos_y() const noexcept;
    QLineEdit* camera_pos_z() const noexcept;

    QLineEdit* camera_rot_w() const noexcept;
    QLineEdit* camera_rot_x() const noexcept;
    QLineEdit* camera_rot_y() const noexcept;
    QLineEdit* camera_rot_z() const noexcept;

    QLineEdit* camera_yaw() const noexcept;
    QLineEdit* camera_pitch() const noexcept;
    QLineEdit* camera_roll() const noexcept;

    QCheckBox* camera_save_pos_rot() const noexcept;

    QLineEdit* camera_far_plane() const noexcept;

    QLineEdit*  clear_colour_component_red() const noexcept;
    QLineEdit*  clear_colour_component_green() const noexcept;
    QLineEdit*  clear_colour_component_blue() const noexcept;

    QLineEdit* dbg_camera_far_plane() const noexcept;
    QCheckBox* dbg_camera_synchronised() const noexcept;
    QCheckBox* dbg_frustum_sector_enumeration() const noexcept;
    QCheckBox* dbg_raycast_sector_enumeration() const noexcept;

    void  on_camera_pos_changed();
    void  camera_position_listener();

    void  on_camera_rot_changed();
    void  on_camera_rot_tait_bryan_changed();
    void  camera_rotation_listener();
    void  update_camera_rot_widgets(quaternion const&  q);

    void  on_camera_far_changed();

    void  on_clear_colour_changed();
    void  on_clear_colour_set(QColor const&  colour);
    void  on_clear_colour_choose();
    void  on_clear_colour_reset();

    void  dbg_on_camera_far_changed();
    void  dbg_on_camera_sync_changed(int);
    void  dbg_on_frustum_sector_enumeration(int);
    void  dbg_on_raycast_sector_enumeration(int);

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

    QLineEdit*  m_clear_colour_component_red;
    QLineEdit*  m_clear_colour_component_green;
    QLineEdit*  m_clear_colour_component_blue;

    QLineEdit*  m_dbg_camera_far_plane;
    QCheckBox*  m_dbg_camera_synchronised;
    QCheckBox*  m_dbg_frustum_sector_enumeration;
    QCheckBox*  m_dbg_raycast_sector_enumeration;
};


QWidget*  make_draw_tab_content(widgets const&  w);


}}

#endif
