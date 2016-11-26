#include <netviewer/window_tabs/tab_draw.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QColorDialog>
#include <QString>
#include <QIntValidator>
#include <QDoubleValidator>

namespace window_tabs { namespace tab_draw {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)

    , m_camera_pos_x(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.pos.x", 10.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_pos_y(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.pos.y", 10.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_pos_z(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.pos.z", 4.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_rot_w(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.rot.w", 0.293152988f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_rot_x(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.rot.x", 0.245984003f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_rot_y(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.rot.y", 0.593858004f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_rot_z(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.rot.z", 0.707732975f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_yaw(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_pitch(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_roll(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_save_pos_rot(new QCheckBox("Save position and rotation"))

    , m_camera_far_plane(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    //setValidator(new QDoubleValidator(1.0, 1000.0, 1));
                    setText(QString::number(wnd->ptree().get("camera.far_plane", 500.0)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_far_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_clear_colour_component_red(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.red", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_clear_colour_component_green(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.green", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_clear_colour_component_blue(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.blue", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_dbg_camera_far_plane(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QDoubleValidator(1.0, 1000.0, 1));
                    setText(QString::number(wnd->ptree().get("dbg.camera.far_plane", 50.0)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(dbg_on_camera_far_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_dbg_camera_synchronised(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Synchronise with scene camera")
                {
                    setCheckState( wnd->ptree().get("dbg.camera.synchronised", true) ?
                                       Qt::CheckState::Checked :
                                       Qt::CheckState::Unchecked );
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(dbg_on_camera_sync_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_dbg_frustum_sector_enumeration(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Enable frustum sector enumeration")
                {
                    setCheckState( wnd->ptree().get("dbg.camera.frustum_sector_enumeration", false) ?
                                       Qt::CheckState::Checked :
                                       Qt::CheckState::Unchecked );
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(dbg_on_frustum_sector_enumeration(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_dbg_raycast_sector_enumeration(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Enable raycast sector enumeration")
                {
                    setCheckState( wnd->ptree().get("dbg.camera.raycast_sector_enumeration", false) ?
                                       Qt::CheckState::Checked :
                                       Qt::CheckState::Unchecked );
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(dbg_on_raycast_sector_enumeration(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )


{

}

program_window*  widgets::wnd() const noexcept
{
    return m_wnd;
}

QLineEdit*widgets::camera_pos_x() const noexcept
{
    return m_camera_pos_x;
}

QLineEdit* widgets::camera_pos_y() const noexcept
{
    return m_camera_pos_y;
}

QLineEdit* widgets::camera_pos_z() const noexcept
{
    return m_camera_pos_z;
}

QLineEdit* widgets::camera_rot_w() const noexcept
{
    return m_camera_rot_w;
}

QLineEdit* widgets::camera_rot_x() const noexcept
{
    return m_camera_rot_x;
}

QLineEdit* widgets::camera_rot_y() const noexcept
{
    return m_camera_rot_y;
}

QLineEdit* widgets::camera_rot_z() const noexcept
{
    return m_camera_rot_z;
}

QLineEdit* widgets::camera_yaw() const noexcept
{
    return m_camera_yaw;
}

QLineEdit* widgets::camera_pitch() const noexcept
{
    return m_camera_pitch;
}

QLineEdit* widgets::camera_roll() const noexcept
{
    return m_camera_roll;
}

QCheckBox* widgets::camera_save_pos_rot() const noexcept
{
    return m_camera_save_pos_rot;
}

QLineEdit* widgets::camera_far_plane() const noexcept
{
    return m_camera_far_plane;
}

QLineEdit* widgets::clear_colour_component_red() const noexcept
{
    return m_clear_colour_component_red;
}

QLineEdit* widgets::clear_colour_component_green() const noexcept
{
    return m_clear_colour_component_green;
}

QLineEdit* widgets::clear_colour_component_blue() const noexcept
{
    return m_clear_colour_component_blue;
}

QLineEdit* widgets::dbg_camera_far_plane() const noexcept
{
    return m_dbg_camera_far_plane;
}

QCheckBox* widgets::dbg_camera_synchronised() const noexcept
{
    return m_dbg_camera_synchronised;
}

QCheckBox* widgets::dbg_frustum_sector_enumeration() const noexcept
{
    return m_dbg_frustum_sector_enumeration;
}

QCheckBox* widgets::dbg_raycast_sector_enumeration() const noexcept
{
    return m_dbg_raycast_sector_enumeration;
}

void  widgets::on_camera_pos_changed()
{
    vector3 const  pos(m_camera_pos_x->text().toFloat(),
        m_camera_pos_y->text().toFloat(),
        m_camera_pos_z->text().toFloat());
    wnd()->glwindow().call_later(&simulator::set_camera_position, pos);
}

void  widgets::camera_position_listener()
{
    vector3 const pos = wnd()->glwindow().call_now(&simulator::get_camera_position);
    m_camera_pos_x->setText(QString::number(pos(0)));
    m_camera_pos_y->setText(QString::number(pos(1)));
    m_camera_pos_z->setText(QString::number(pos(2)));
}

void  widgets::on_camera_rot_changed()
{
    quaternion  q(m_camera_rot_w->text().toFloat(),
        m_camera_rot_x->text().toFloat(),
        m_camera_rot_y->text().toFloat(),
        m_camera_rot_z->text().toFloat());
    if (length_squared(q) < 1e-5f)
        q.z() = 1.0f;
    normalise(q);

    update_camera_rot_widgets(q);
    wnd()->glwindow().call_later(&simulator::set_camera_orientation, q);
}

void  widgets::on_camera_rot_tait_bryan_changed()
{
    quaternion  q = rotation_matrix_to_quaternion(yaw_pitch_roll_to_rotation(
        m_camera_yaw->text().toFloat() * PI() / 180.0f,
        m_camera_pitch->text().toFloat() * PI() / 180.0f,
        m_camera_roll->text().toFloat() * PI() / 180.0f
        ));
    normalise(q);
    update_camera_rot_widgets(q);
    wnd()->glwindow().call_later(&simulator::set_camera_orientation, q);
}

void  widgets::camera_rotation_listener()
{
    update_camera_rot_widgets(wnd()->glwindow().call_now(&simulator::get_camera_orientation));
}

void  widgets::update_camera_rot_widgets(quaternion const&  q)
{
    m_camera_rot_w->setText(QString::number(q.w()));
    m_camera_rot_x->setText(QString::number(q.x()));
    m_camera_rot_y->setText(QString::number(q.y()));
    m_camera_rot_z->setText(QString::number(q.z()));

    scalar  yaw, pitch, roll;
    rotation_to_yaw_pitch_roll(quaternion_to_rotation_matrix(q), yaw, pitch, roll);
    m_camera_yaw->setText(QString::number(yaw * 180.0f / PI()));
    m_camera_pitch->setText(QString::number(pitch * 180.0f / PI()));
    m_camera_roll->setText(QString::number(roll * 180.0f / PI()));
}

void  widgets::on_camera_far_changed()
{
    wnd()->glwindow().call_later(&simulator::set_camera_far_plane, m_camera_far_plane->text().toFloat());
}

void widgets::on_clear_colour_changed()
{
    vector3 const  colour(
        (float_32_bit)m_clear_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_blue->text().toInt() / 255.0f
        );
    wnd()->glwindow().call_later(&simulator::set_clear_color, colour);
}

void widgets::on_clear_colour_set(QColor const&  colour)
{
    m_clear_colour_component_red->setText(QString::number(colour.red()));
    m_clear_colour_component_green->setText(QString::number(colour.green()));
    m_clear_colour_component_blue->setText(QString::number(colour.blue()));
    on_clear_colour_changed();
}

void widgets::on_clear_colour_choose()
{
    QColor const  init_colour(m_clear_colour_component_red->text().toInt(),
        m_clear_colour_component_green->text().toInt(),
        m_clear_colour_component_blue->text().toInt());
    QColor const  colour = QColorDialog::getColor(init_colour, wnd(), "Choose clear colour");
    if (!colour.isValid())
        return;
    on_clear_colour_set(colour);
}

void widgets::on_clear_colour_reset()
{
    on_clear_colour_set(QColor(64, 64, 64));
}

void  widgets::dbg_on_camera_far_changed()
{
    wnd()->glwindow().call_later(&simulator::dbg_set_camera_far_plane, m_dbg_camera_far_plane->text().toFloat());
}

void  widgets::dbg_on_camera_sync_changed(int)
{
    wnd()->glwindow().call_later(&simulator::dbg_set_camera_sync_state, m_dbg_camera_synchronised->isChecked());
}

void  widgets::dbg_on_frustum_sector_enumeration(int)
{
    wnd()->glwindow().call_later(&simulator::dbg_enable_frustum_sector_enumeration,m_dbg_frustum_sector_enumeration->isChecked());
}

void  widgets::dbg_on_raycast_sector_enumeration(int)
{
    wnd()->glwindow().call_later(&simulator::dbg_enable_raycast_sector_enumeration,m_dbg_raycast_sector_enumeration->isChecked());
}

void  widgets::save()
{
    if (m_camera_save_pos_rot->isChecked())
    {
        wnd()->ptree().put("camera.pos.x", m_camera_pos_x->text().toFloat());
        wnd()->ptree().put("camera.pos.y", m_camera_pos_y->text().toFloat());
        wnd()->ptree().put("camera.pos.z", m_camera_pos_z->text().toFloat());
        wnd()->ptree().put("camera.rot.w", m_camera_rot_w->text().toFloat());
        wnd()->ptree().put("camera.rot.x", m_camera_rot_x->text().toFloat());
        wnd()->ptree().put("camera.rot.y", m_camera_rot_y->text().toFloat());
        wnd()->ptree().put("camera.rot.z", m_camera_rot_z->text().toFloat());
    }
    wnd()->ptree().put("camera.save_pos_rot", m_camera_save_pos_rot->isChecked());
    wnd()->ptree().put("camera.far_plane", m_camera_far_plane->text().toFloat());

    wnd()->ptree().put("draw.clear_colour.red", m_clear_colour_component_red->text().toInt());
    wnd()->ptree().put("draw.clear_colour.green", m_clear_colour_component_green->text().toInt());
    wnd()->ptree().put("draw.clear_colour.blue", m_clear_colour_component_blue->text().toInt());

    wnd()->ptree().put("dbg.camera.far_plane", m_dbg_camera_far_plane->text().toFloat());
    wnd()->ptree().put("dbg.camera.synchronised", m_dbg_camera_synchronised->isChecked());
    wnd()->ptree().put("dbg.camera.frustum_sector_enumeration", m_dbg_frustum_sector_enumeration->isChecked());
    wnd()->ptree().put("dbg.camera.raycast_sector_enumeration", m_dbg_raycast_sector_enumeration->isChecked());
}

QWidget*  make_draw_tab_content(widgets const&  w)
{
    QWidget* const  draw_tab = new QWidget;
    {
        QVBoxLayout* const draw_tab_layout = new QVBoxLayout;
        {
            QWidget* const camera_group = new QGroupBox("Camera");
            {
                QVBoxLayout* const camera_layout = new QVBoxLayout;
                {
                    QWidget* const position_group = new QGroupBox("Position in meters [xyz]");
                    {
                        QHBoxLayout* const position_layout = new QHBoxLayout;
                        {
                            position_layout->addWidget(w.camera_pos_x());
                            position_layout->addWidget(w.camera_pos_y());
                            position_layout->addWidget(w.camera_pos_z());
                            w.wnd()->on_camera_pos_changed();
                            w.wnd()->glwindow().register_listener(
                                        simulator_notifications::camera_position_updated(),
                                        { &program_window::camera_position_listener,w.wnd() }
                                        );
                        }
                        position_group->setLayout(position_layout);
                    }
                    camera_layout->addWidget(position_group);

                    QWidget* const rotation_group = new QGroupBox("Rotation");
                    {
                        QVBoxLayout* const rotation_layout = new QVBoxLayout;
                        {
                            rotation_layout->addWidget(new QLabel("Quaternion [wxyz]"));
                            QHBoxLayout* const quaternion_layout = new QHBoxLayout;
                            {
                                quaternion_layout->addWidget(w.camera_rot_w());
                                quaternion_layout->addWidget(w.camera_rot_x());
                                quaternion_layout->addWidget(w.camera_rot_y());
                                quaternion_layout->addWidget(w.camera_rot_z());
                            }
                            rotation_layout->addLayout(quaternion_layout);

                            rotation_layout->addWidget(new QLabel("Tait-Bryan angles in degrees [yaw(z)-pitch(y')-roll(x'')]"));
                            QHBoxLayout* const tait_bryan_layout = new QHBoxLayout;
                            {
                                tait_bryan_layout->addWidget(w.camera_yaw());
                                tait_bryan_layout->addWidget(w.camera_pitch());
                                tait_bryan_layout->addWidget(w.camera_roll());
                            }
                            rotation_layout->addLayout(tait_bryan_layout);
                        }
                        rotation_group->setLayout(rotation_layout);

                        w.wnd()->on_camera_rot_changed();
                        w.wnd()->glwindow().register_listener(
                                    simulator_notifications::camera_orientation_updated(),
                                    { &program_window::camera_rotation_listener,w.wnd() }
                                    );
                    }
                    camera_layout->addWidget(rotation_group);

                    w.camera_save_pos_rot()->setCheckState(
                        w.wnd()->ptree().get("camera.save_pos_rot", false) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked
                        );
                    camera_layout->addWidget(w.camera_save_pos_rot());

                    QHBoxLayout* const far_plane_layout = new QHBoxLayout;
                    {
                        far_plane_layout->addWidget(w.camera_far_plane());
                        far_plane_layout->addWidget(new QLabel("Camera's far clip plane."));
                        w.wnd()->on_camera_far_changed();
                    }
                    camera_layout->addLayout(far_plane_layout);
                }
                camera_group->setLayout(camera_layout);
            }
            draw_tab_layout->addWidget(camera_group);

            QWidget* const clear_colour_group = new QGroupBox("Clear colour [rgb]");
            {
                QVBoxLayout* const clear_colour_layout = new QVBoxLayout;
                {
                    QHBoxLayout* const edit_boxes_layout = new QHBoxLayout;
                    {
                        edit_boxes_layout->addWidget(w.clear_colour_component_red());
                        edit_boxes_layout->addWidget(w.clear_colour_component_green());
                        edit_boxes_layout->addWidget(w.clear_colour_component_blue());
                        w.wnd()->on_clear_colour_changed();
                    }
                    clear_colour_layout->addLayout(edit_boxes_layout);

                    QHBoxLayout* const buttons_layout = new QHBoxLayout;
                    {
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Choose")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_clear_colour_choose()));
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Default")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_clear_colour_reset()));
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                    }
                    clear_colour_layout->addLayout(buttons_layout);

                }
                clear_colour_group->setLayout(clear_colour_layout);
            }
            draw_tab_layout->addWidget(clear_colour_group);

            QWidget* const dbg_group = new QGroupBox("Debugging");
            {
                QVBoxLayout* const dbg_layout = new QVBoxLayout;
                {
                    dbg_layout->addWidget(w.dbg_camera_synchronised());
                    w.wnd()->dbg_on_camera_sync_changed(0);

                    QHBoxLayout* const dbg_far_plane_layout = new QHBoxLayout;
                    {
                        dbg_far_plane_layout->addWidget(w.dbg_camera_far_plane());
                        dbg_far_plane_layout->addWidget(new QLabel("Far clip plane overload."));
                        w.wnd()->dbg_on_camera_far_changed();
                    }
                    dbg_layout->addLayout(dbg_far_plane_layout);

                    dbg_layout->addWidget(w.dbg_frustum_sector_enumeration());
                    w.wnd()->dbg_on_frustum_sector_enumeration(0);

                    dbg_layout->addWidget(w.dbg_raycast_sector_enumeration());
                    w.wnd()->dbg_on_raycast_sector_enumeration(0);
                }
                dbg_group->setLayout(dbg_layout);
            }
            draw_tab_layout->addWidget(dbg_group);

            draw_tab_layout->addStretch(1);
        }
        draw_tab->setLayout(draw_tab_layout);
    }
    return draw_tab;
}


}}
