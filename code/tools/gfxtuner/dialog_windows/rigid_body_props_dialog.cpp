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
        scn::rigid_body_props* const  props
        )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_auto_compute_mass_and_inertia_tensor(auto_compute_mass_and_inertia_tensor)
    , m_props(props)
    , m_ok(false)

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
{
    ASSUMPTION(m_auto_compute_mass_and_inertia_tensor != nullptr);
    ASSUMPTION(m_props != nullptr);

    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            m_widget_inverted_inertia_tensor[i][j] = new QLineEdit;

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

    m_widget_linear_velocity_x->setToolTip("x coordinate in world space.");
    m_widget_linear_velocity_y->setToolTip("y coordinate in world space.");
    m_widget_linear_velocity_z->setToolTip("z coordinate in world space.");

    m_widget_angular_velocity_x->setToolTip("x coordinate in world space.");
    m_widget_angular_velocity_y->setToolTip("y coordinate in world space.");
    m_widget_angular_velocity_z->setToolTip("z coordinate in world space.");

    m_widget_external_linear_acceleration_x->setToolTip("x coordinate in world space.");
    m_widget_external_linear_acceleration_y->setToolTip("y coordinate in world space.");
    m_widget_external_linear_acceleration_z->setToolTip("z coordinate in world space.");

    m_widget_external_angular_acceleration_x->setToolTip("x coordinate in world space.");
    m_widget_external_angular_acceleration_y->setToolTip("y coordinate in world space.");
    m_widget_external_angular_acceleration_z->setToolTip("z coordinate in world space.");

    m_widget_auto_compute_mass_and_inertia_tensor->setToolTip(
            "Whether to automatically compute inverted mass and inverted innertia tensor\n"
            "from colliders under the scene node with this rigid body, or to manually\n"
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
                    "and zero inverted inertia tensor cannot be moved by any force or torque;\n"
                    "when all elements of the inverted inertia tensor are zero except (2,2),\n"
                    "then the rigid body can rotate only along z-axis (of its frame)."
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
