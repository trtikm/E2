#include <gfxtuner/dialog_windows/rigid_body_props_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>

namespace dialog_windows {


rigid_body_props_dialog::rigid_body_props_dialog(
        program_window* const  wnd,
        bool* const  auto_compute_mass_and_inertia_tensor,
        scn::rigid_body_props* const  props,
        matrix44 const&  rigid_body_world_matrix,
        matrix44 const&  pivot_world_matrix
        )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_auto_compute_mass_and_inertia_tensor(auto_compute_mass_and_inertia_tensor)
    , m_props(props)
    , m_ok(false)

    , m_world_frame_radio_button(new QRadioButton("World"))
    , m_local_frame_radio_button(new QRadioButton("Local"))
    , m_pivot_frame_radio_button(new QRadioButton("Pivot"))
    , m_checked_frame_radio_button(nullptr)

    , m_widget_linear_velocity_x(new QLineEdit)
    , m_widget_linear_velocity_y(new QLineEdit)
    , m_widget_linear_velocity_z(new QLineEdit)

    , m_widget_angular_velocity_x(new QLineEdit)
    , m_widget_angular_velocity_y(new QLineEdit)
    , m_widget_angular_velocity_z(new QLineEdit)

    , m_widget_external_linear_acceleration_x(new QLineEdit)
    , m_widget_external_linear_acceleration_y(new QLineEdit)
    , m_widget_external_linear_acceleration_z(new QLineEdit)

    , m_widget_external_angular_acceleration_x(new QLineEdit)
    , m_widget_external_angular_acceleration_y(new QLineEdit)
    , m_widget_external_angular_acceleration_z(new QLineEdit)

    , m_widget_auto_compute_mass_and_inertia_tensor(
        [](rigid_body_props_dialog* dlg, bool const  auto_compute_mass_and_inertia_tensor) {
            struct s : public QCheckBox {
                s(rigid_body_props_dialog* dlg, bool const  auto_compute_mass_and_inertia_tensor)
                    : QCheckBox("Compute inverted mass and inverted inertia tensor from colliders.")
                {
                    setChecked(auto_compute_mass_and_inertia_tensor);
                    QObject::connect(this, &QCheckBox::stateChanged, dlg, &rigid_body_props_dialog::on_auto_compute_mass_and_inertia_tensor_changed);
                }
            };
            return new s(dlg, auto_compute_mass_and_inertia_tensor);
        }(this, auto_compute_mass_and_inertia_tensor)
        
        )

    , m_widget_inverted_mass(new QLineEdit)

    , m_widget_inverted_inertia_tensor()
    , m_rigid_body_world_matrix(rigid_body_world_matrix)
    , m_pivot_world_matrix(pivot_world_matrix)
{
    ASSUMPTION(m_auto_compute_mass_and_inertia_tensor != nullptr);
    ASSUMPTION(m_props != nullptr);

    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            m_widget_inverted_inertia_tensor[i][j] = new QLineEdit;

    m_world_frame_radio_button->setChecked(true);
    m_local_frame_radio_button->setChecked(false);
    m_pivot_frame_radio_button->setChecked(false);
    m_checked_frame_radio_button = m_world_frame_radio_button;
    QObject::connect(m_world_frame_radio_button, SIGNAL(toggled(bool)), this, SLOT(on_world_frame_radio_button_use_changed(bool)));
    QObject::connect(m_local_frame_radio_button, SIGNAL(toggled(bool)), this, SLOT(on_local_frame_radio_button_use_changed(bool)));
    QObject::connect(m_pivot_frame_radio_button, SIGNAL(toggled(bool)), this, SLOT(on_pivot_frame_radio_button_use_changed(bool)));

    m_widget_linear_velocity_x->setText(QString::number(m_props->m_linear_velocity(0)));
    m_widget_linear_velocity_y->setText(QString::number(m_props->m_linear_velocity(1)));
    m_widget_linear_velocity_z->setText(QString::number(m_props->m_linear_velocity(2)));

    m_widget_angular_velocity_x->setText(QString::number(m_props->m_angular_velocity(0)));
    m_widget_angular_velocity_y->setText(QString::number(m_props->m_angular_velocity(1)));
    m_widget_angular_velocity_z->setText(QString::number(m_props->m_angular_velocity(2)));

    m_widget_external_linear_acceleration_x->setText(QString::number(m_props->m_external_linear_acceleration(0)));
    m_widget_external_linear_acceleration_y->setText(QString::number(m_props->m_external_linear_acceleration(1)));
    m_widget_external_linear_acceleration_z->setText(QString::number(m_props->m_external_linear_acceleration(2)));

    m_widget_external_angular_acceleration_x->setText(QString::number(m_props->m_external_angular_acceleration(0)));
    m_widget_external_angular_acceleration_y->setText(QString::number(m_props->m_external_angular_acceleration(1)));
    m_widget_external_angular_acceleration_z->setText(QString::number(m_props->m_external_angular_acceleration(2)));

    m_widget_auto_compute_mass_and_inertia_tensor->setChecked(*m_auto_compute_mass_and_inertia_tensor);

    m_widget_inverted_mass->setText(QString::number(m_props->m_mass_inverted));

    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            m_widget_inverted_inertia_tensor[i][j]->setText(QString::number(m_props->m_inertia_tensor_inverted(i, j)));

    m_widget_linear_velocity_x->setToolTip("x coordinate in selected frame of reference.");
    m_widget_linear_velocity_y->setToolTip("y coordinate in selected frame of reference.");
    m_widget_linear_velocity_z->setToolTip("z coordinate in selected frame of reference.");

    m_widget_angular_velocity_x->setToolTip("x coordinate in selected frame of reference.");
    m_widget_angular_velocity_y->setToolTip("y coordinate in selected frame of reference.");
    m_widget_angular_velocity_z->setToolTip("z coordinate in selected frame of reference.");

    m_widget_external_linear_acceleration_x->setToolTip("x coordinate in selected frame of reference.");
    m_widget_external_linear_acceleration_y->setToolTip("y coordinate in selected frame of reference.");
    m_widget_external_linear_acceleration_z->setToolTip("z coordinate in selected frame of reference.");

    m_widget_external_angular_acceleration_x->setToolTip("x coordinate in selected frame of reference.");
    m_widget_external_angular_acceleration_y->setToolTip("y coordinate in selected frame of reference.");
    m_widget_external_angular_acceleration_z->setToolTip("z coordinate in selected frame of reference.");

    m_widget_auto_compute_mass_and_inertia_tensor->setToolTip(
            "Whether to automatically compute inverted mass and inverted innertia tensor\n"
            "from moveable colliders under the scene node of this rigid body, or to manually\n"
            "specify the mass and the inertia tensor using the widgets below."
            );

    m_widget_inverted_mass->setToolTip("Inverted mass of the rigid body.");
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
        {
            std::string const  msg = msgstream() << "Element (" << i << "," << j << ") of the inverted inertia tensor.";
            m_widget_inverted_inertia_tensor[i][j]->setToolTip(QString(msg.c_str()));
        }

    set_enable_state_of_mass_and_inertia_tensor(!*m_auto_compute_mass_and_inertia_tensor);

    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QWidget* const  reference_frame_group = new QGroupBox("Reference frame");
        {
            reference_frame_group->setToolTip(
                "These radio buttons allow you to choose a reference coord. system for velocity and acceleration\n"
                "vectors below. However, after you press OK button, the vectors are always automatically transformed\n"
                "to world coord. system.\n"
                "NOTE: the innertia tesor is always in the local coord. system."
                );
            QHBoxLayout* const  radio_buttons_layout = new QHBoxLayout;
            {
                m_world_frame_radio_button->setToolTip("Vectors below are in WORLD coord. system.");
                radio_buttons_layout->addWidget(m_world_frame_radio_button);

                m_local_frame_radio_button->setToolTip("Vectors below are in LOCAL coord. system.");
                radio_buttons_layout->addWidget(m_local_frame_radio_button);

                m_pivot_frame_radio_button->setToolTip("Vectors below are in @pivot's coord. system.");
                radio_buttons_layout->addWidget(m_pivot_frame_radio_button);
            }
            reference_frame_group->setLayout(radio_buttons_layout);
        }
        dlg_layout->addWidget(reference_frame_group);

        auto const  insert_vector_group =
            [](std::string const&  group_name, QLineEdit* const  x_edit, QLineEdit* const  y_edit, QLineEdit* const  z_edit)
                -> QWidget*
            {
                QWidget* const group = new QGroupBox(group_name.c_str());
                {
                    QHBoxLayout* const coords_layout = new QHBoxLayout;
                    {
                        coords_layout->addWidget(x_edit);
                        coords_layout->addWidget(y_edit);
                        coords_layout->addWidget(z_edit);
                    }
                    group->setLayout(coords_layout);
                }
                return group;
            };

        dlg_layout->addWidget(insert_vector_group(
                "Linear velocity [m/s]",
                m_widget_linear_velocity_x,
                m_widget_linear_velocity_y,
                m_widget_linear_velocity_z
                ));

        dlg_layout->addWidget(insert_vector_group(
                "Angular velocity [m/s]",
                m_widget_angular_velocity_x,
                m_widget_angular_velocity_y,
                m_widget_angular_velocity_z
                ));

        dlg_layout->addWidget(insert_vector_group(
                "External linear acceleration [m/(s*s)]",
                m_widget_external_linear_acceleration_x,
                m_widget_external_linear_acceleration_y,
                m_widget_external_linear_acceleration_z
                ));

        dlg_layout->addWidget(insert_vector_group(
                "External angular acceleration [m/(s*s)]",
                m_widget_external_angular_acceleration_x,
                m_widget_external_angular_acceleration_y,
                m_widget_external_angular_acceleration_z
                ));

        dlg_layout->addWidget(m_widget_auto_compute_mass_and_inertia_tensor);

        QWidget* const mass_and_inertia_tensor_group = new QGroupBox("Inverted mass and inverted inertia tensor");
        {
            mass_and_inertia_tensor_group->setToolTip(
                    "In these widgets you can manually specify distribution of mass in the local\n"
                    "coordinate system (i.e. frame) of the rigid body.\n"
                    "NOTE: Insertion of mass and inertia tensor in the inverted forms has advantage\n"
                    "that you can also define unreal rigid bodies, i.e. those for which the inverted\n"
                    "forms normally do no exist. For example, rigid body with zero inverted mass\n"
                    "and zero inverted inertia tensor cannot change its motion by any force or torque;\n"
                    "when all elements of the inverted inertia tensor are zero except (2,2),\n"
                    "then any torque can rotate the rigid body only along z-axis (of its frame)."
                    );
            QVBoxLayout* const group_layout = new QVBoxLayout;
            {
                QHBoxLayout* const mass_layout = new QHBoxLayout;
                {
                    mass_layout->addWidget(new QLabel("Inverted mass [1/kg]: "));
                    mass_layout->addWidget(m_widget_inverted_mass);
                }
                group_layout->addLayout(mass_layout);

                group_layout->addWidget(new QLabel("Inverted inertia tensor (in the local frame of the ridid body):"));
                QHBoxLayout* const inertia_tensor_row_1_layout = new QHBoxLayout;
                {
                    inertia_tensor_row_1_layout->addWidget(m_widget_inverted_inertia_tensor[0][0]);
                    inertia_tensor_row_1_layout->addWidget(m_widget_inverted_inertia_tensor[0][1]);
                    inertia_tensor_row_1_layout->addWidget(m_widget_inverted_inertia_tensor[0][2]);
                }
                group_layout->addLayout(inertia_tensor_row_1_layout);
                QHBoxLayout* const inertia_tensor_row_2_layout = new QHBoxLayout;
                {
                    inertia_tensor_row_2_layout->addWidget(m_widget_inverted_inertia_tensor[1][0]);
                    inertia_tensor_row_2_layout->addWidget(m_widget_inverted_inertia_tensor[1][1]);
                    inertia_tensor_row_2_layout->addWidget(m_widget_inverted_inertia_tensor[1][2]);
                }
                group_layout->addLayout(inertia_tensor_row_2_layout);
                QHBoxLayout* const inertia_tensor_row_3_layout = new QHBoxLayout;
                {
                    inertia_tensor_row_3_layout->addWidget(m_widget_inverted_inertia_tensor[2][0]);
                    inertia_tensor_row_3_layout->addWidget(m_widget_inverted_inertia_tensor[2][1]);
                    inertia_tensor_row_3_layout->addWidget(m_widget_inverted_inertia_tensor[2][2]);
                }
                group_layout->addLayout(inertia_tensor_row_3_layout);
            }
            mass_and_inertia_tensor_group->setLayout(group_layout);
        }
        dlg_layout->addWidget(mass_and_inertia_tensor_group);
        dlg_layout->addWidget(new QLabel(
                "NOTE: Whenever there is inserted a NOT moveable collider under the scene node\n"
                "of this rigid body, then all properties of the rigid body editable in the dialog\n"
                "will automatically be reset to zeros (to make the rigid body not moveable too)."
                ));

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(
                [](rigid_body_props_dialog* wnd) {
                struct OK : public QPushButton {
                    OK(rigid_body_props_dialog* wnd) : QPushButton("OK")
                    {
                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                    }
                };
                return new OK(wnd);
            }(this)
                );
            buttons_layout->addWidget(
                [](rigid_body_props_dialog* wnd) {
                struct Close : public QPushButton {
                    Close(rigid_body_props_dialog* wnd) : QPushButton("Cancel")
                    {
                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(reject()));
                    }
                };
                return new Close(wnd);
            }(this)
                );
            buttons_layout->addStretch(1);
        }
        dlg_layout->addLayout(buttons_layout);
        //dlg_layout->setAlignment(buttons_layout, Qt::Alignment(Qt::AlignmentFlag::AlignRight));
    }
    this->setLayout(dlg_layout);
    this->setWindowTitle("Rigid body");
    //this->resize(300,100);
}


void rigid_body_props_dialog::on_auto_compute_mass_and_inertia_tensor_changed(int const  state)
{
    *m_auto_compute_mass_and_inertia_tensor = (state != 0);
    set_enable_state_of_mass_and_inertia_tensor(!*m_auto_compute_mass_and_inertia_tensor);
}


void  rigid_body_props_dialog::on_world_frame_radio_button_use_changed(bool const  is_checked)
{
}


void  rigid_body_props_dialog::on_local_frame_radio_button_use_changed(bool const  is_checked)
{
    if (is_checked)
    {
    }
    else
    {

    }
}


void  rigid_body_props_dialog::on_pivot_frame_radio_button_use_changed(bool const  is_checked)
{
}


void  rigid_body_props_dialog::set_enable_state_of_mass_and_inertia_tensor(bool const  set_enabled)
{
    m_widget_inverted_mass->setEnabled(set_enabled);
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            m_widget_inverted_inertia_tensor[i][j]->setEnabled(set_enabled);
}


void  rigid_body_props_dialog::accept()
{
    m_props->m_linear_velocity = {
            (float_32_bit)std::atof(qtgl::to_string(m_widget_linear_velocity_x->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_linear_velocity_y->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_linear_velocity_z->text()).c_str())
            };

    m_props->m_angular_velocity = {
            (float_32_bit)std::atof(qtgl::to_string(m_widget_angular_velocity_x->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_angular_velocity_y->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_angular_velocity_z->text()).c_str())
            };

    m_props->m_external_linear_acceleration = {
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_linear_acceleration_x->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_linear_acceleration_y->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_linear_acceleration_z->text()).c_str())
            };

    m_props->m_external_angular_acceleration = {
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_angular_acceleration_x->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_angular_acceleration_y->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_angular_acceleration_z->text()).c_str())
            };

    *m_auto_compute_mass_and_inertia_tensor = m_widget_auto_compute_mass_and_inertia_tensor->isChecked();

    if (*m_auto_compute_mass_and_inertia_tensor == false)
    {
        m_props->m_mass_inverted = (float_32_bit)std::atof(qtgl::to_string(m_widget_inverted_mass->text()).c_str());

        for (int i = 0; i != 3; ++i)
            for (int j = 0; j != 3; ++j)
                m_props->m_inertia_tensor_inverted(i,j) =
                        (float_32_bit)std::atof(qtgl::to_string(m_widget_inverted_inertia_tensor[i][j]->text()).c_str());
    }        

    m_ok = true;

    QDialog::accept();
}

void  rigid_body_props_dialog::reject()
{
    QDialog::reject();
}


}
