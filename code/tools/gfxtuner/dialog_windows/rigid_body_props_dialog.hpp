#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_RIGID_BODY_PROPS_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_RIGID_BODY_PROPS_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <scene/records/rigid_body/rigid_body.hpp>
#   include <QDialog>
#   include <QLabel>
#   include <QPushButton>
#   include <QCheckBox>
#   include <QComboBox>
#   include <QLineEdit>
#   include <QGroupBox>
#   include <array>

namespace dialog_windows {


struct  rigid_body_props_dialog : public QDialog
{
    rigid_body_props_dialog(
            program_window* const  wnd,
            bool* const  auto_compute_mass_and_inertia_tensor,
            struct  scn::rigid_body_props* const  props
            );

    bool  ok() const { return m_ok; }

    void on_auto_compute_mass_and_inertia_tensor_changed(int const  state);

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    bool*  m_auto_compute_mass_and_inertia_tensor;
    scn::rigid_body_props*  m_props;
    bool  m_ok;

    void  set_enable_state_of_mass_and_inertia_tensor(bool const set_enabled);

    QLineEdit*  m_widget_linear_velocity_x;
    QLineEdit*  m_widget_linear_velocity_y;
    QLineEdit*  m_widget_linear_velocity_z;

    QLineEdit*  m_widget_angular_velocity_x;
    QLineEdit*  m_widget_angular_velocity_y;
    QLineEdit*  m_widget_angular_velocity_z;

    QLineEdit*  m_widget_external_linear_acceleration_x;
    QLineEdit*  m_widget_external_linear_acceleration_y;
    QLineEdit*  m_widget_external_linear_acceleration_z;

    QLineEdit*  m_widget_external_angular_acceleration_x;
    QLineEdit*  m_widget_external_angular_acceleration_y;
    QLineEdit*  m_widget_external_angular_acceleration_z;

    QCheckBox*  m_widget_auto_compute_mass_and_inertia_tensor;

    QLineEdit*  m_widget_inverted_mass;

    std::array<std::array<QLineEdit*, 3>, 3>  m_widget_inverted_inertia_tensor;
};


}

#endif
