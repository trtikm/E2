#include <gfxtuner/window_tabs/tab_draw.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <qtgl/camera_utils.hpp>
#include <boost/property_tree/ptree.hpp>
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_pos_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_pos_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_pos_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_tait_bryan_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_tait_bryan_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_save_pos_rot(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Save position and rotation")
                {
                    setChecked(wnd->ptree().get("camera.save_pos_rot", true));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_save_pos_rot_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_far_plane(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    //setValidator(new QDoubleValidator(1.0, 5000.0, 1));
                    setText(QString::number(wnd->ptree().get("camera.far_plane", 200.0)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_far_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_speed(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    //setValidator(new QDoubleValidator(0.1, 500.0, 1));
                    setText(QString::number(wnd->ptree().get("camera.speed", 15.0)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_speed_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_clear_colour_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_clear_colour_changed()));
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
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_show_grid(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Show grid")
                {
                    setChecked(wnd->ptree().get("draw.show_grid", true));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_show_grid_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
{
    m_camera_rot_w->setEnabled(false);
    m_camera_rot_x->setEnabled(false);
    m_camera_rot_y->setEnabled(false);
    m_camera_rot_z->setEnabled(false);
    m_camera_pitch->setEnabled(false);
}


void  widgets::on_camera_pos_changed()
{
    vector3 const  pos(m_camera_pos_x->text().toFloat(),
        m_camera_pos_y->text().toFloat(),
        m_camera_pos_z->text().toFloat());
    wnd()->glwindow().call_later(&simulator::set_camera_position, pos);
    wnd()->set_focus_to_glwindow();
}

void  widgets::camera_position_listener()
{
    vector3 const pos = wnd()->glwindow().call_now(&simulator::get_camera_position);
    update_camera_pos_widgets(pos);
}

void  widgets::update_camera_pos_widgets(vector3 const&  pos)
{
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
    wnd()->set_focus_to_glwindow();
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
    wnd()->set_focus_to_glwindow();
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
    float_32_bit  far_plane = m_camera_far_plane->text().toFloat();
    if (far_plane < 1.0f)
    {
        far_plane = 1.0f;
        m_camera_far_plane->setText(QString("1"));
    }
    else if (far_plane > 5000.0f)
    {
        far_plane = 5000.0f;
        m_camera_far_plane->setText(QString("5000"));
    }
    wnd()->glwindow().call_later(&simulator::set_camera_far_plane, far_plane);
    wnd()->set_focus_to_glwindow();
}

void  widgets::on_camera_speed_changed()
{
    set_camera_speed(m_camera_speed->text().toFloat());
    wnd()->set_focus_to_glwindow();
}

void widgets::on_clear_colour_changed()
{
    vector3 const  colour(
        (float_32_bit)m_clear_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_blue->text().toInt() / 255.0f
        );
    wnd()->glwindow().call_later(&simulator::set_clear_color, colour);
    wnd()->set_focus_to_glwindow();
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
    m_clear_colour_component_red->setText(QString::number(colour.red()));
    m_clear_colour_component_green->setText(QString::number(colour.green()));
    m_clear_colour_component_blue->setText(QString::number(colour.blue()));
    vector3 const  normalised_colour(
        (float_32_bit)m_clear_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_blue->text().toInt() / 255.0f
        );
    wnd()->glwindow().call_later(&simulator::set_clear_color, normalised_colour);
    wnd()->set_focus_to_glwindow(false);
}

void widgets::on_clear_colour_reset()
{
    on_clear_colour_set(QColor(64, 64, 64));
}

void widgets::on_show_grid_changed(int const  value)
{
    wnd()->glwindow().call_later(&simulator::set_show_grid_state, m_show_grid->isChecked());
    wnd()->set_focus_to_glwindow();
}

void  widgets::on_save_pos_rot_changed(int const  value)
{
    wnd()->set_focus_to_glwindow();
}

void  widgets::on_double_camera_speed()
{
    set_camera_speed(m_camera_speed->text().toFloat() * 2.0f);
}

void  widgets::on_half_camera_speed()
{
    set_camera_speed(m_camera_speed->text().toFloat() / 2.0f);
}

void  widgets::on_look_at(vector3 const&  target, float_32_bit const* const  distance_ptr)
{
    angeo::coordinate_system  coord_system{
        wnd()->glwindow().call_now(&simulator::get_camera_position),
        wnd()->glwindow().call_now(&simulator::get_camera_orientation)
    };

    qtgl::look_at(coord_system, target, distance_ptr == nullptr ? length(target - coord_system.origin()) : *distance_ptr);

    wnd()->glwindow().call_later(&simulator::set_camera_position, coord_system.origin());
    wnd()->glwindow().call_later(&simulator::set_camera_orientation, coord_system.orientation());

    update_camera_pos_widgets(coord_system.origin());
    update_camera_rot_widgets(coord_system.orientation());
}

void  widgets::set_camera_speed(float_32_bit  speed)
{
    if (speed < 0.1f)
        speed = 0.1f;
    else if (speed > 500.0f)
        speed = 500.0f;

    //if (std::fabs(speed - m_camera_speed->text().toFloat()) > 1e-4f)
    {
        std::stringstream  sstr;
        sstr << speed;
        m_camera_speed->setText(QString(sstr.str().c_str()));
    }

    wnd()->glwindow().call_later(&simulator::set_camera_speed, speed);
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
    wnd()->ptree().put("camera.speed", m_camera_speed->text().toFloat());

    wnd()->ptree().put("draw.clear_colour.red", m_clear_colour_component_red->text().toInt());
    wnd()->ptree().put("draw.clear_colour.green", m_clear_colour_component_green->text().toInt());
    wnd()->ptree().put("draw.clear_colour.blue", m_clear_colour_component_blue->text().toInt());

    wnd()->ptree().put("draw.show_grid", m_show_grid->isChecked());
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
                            w.wnd()->on_draw_camera_pos_changed();
                            w.wnd()->glwindow().register_listener(
                                        simulator_notifications::camera_position_updated(),
                                        { &program_window::draw_camera_position_listener,w.wnd() }
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

                        w.wnd()->on_draw_camera_rot_changed();
                        w.wnd()->glwindow().register_listener(
                                    simulator_notifications::camera_orientation_updated(),
                                    { &program_window::draw_camera_rotation_listener,w.wnd() }
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
                        far_plane_layout->addWidget(new QLabel("Far clip plane [m]"));
                        w.wnd()->on_draw_camera_far_changed();
                    }
                    camera_layout->addLayout(far_plane_layout);

                    QHBoxLayout* const speed_layout = new QHBoxLayout;
                    {
                        speed_layout->addWidget(w.camera_speed());
                        speed_layout->addWidget(new QLabel("Speed [m/s]"));
                        w.wnd()->on_draw_camera_speed_changed();
                    }
                    camera_layout->addLayout(speed_layout);
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
                        w.wnd()->on_draw_clear_colour_changed();
                    }
                    clear_colour_layout->addLayout(edit_boxes_layout);

                    QHBoxLayout* const buttons_layout = new QHBoxLayout;
                    {
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Choose")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_draw_clear_colour_choose()));
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
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_draw_clear_colour_reset()));
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

            QHBoxLayout* const render_options_layout = new QHBoxLayout;
            {
                render_options_layout->addWidget(w.show_grid());
                w.wnd()->on_draw_show_grid_changed(0);
            }
            draw_tab_layout->addLayout(render_options_layout);

            draw_tab_layout->addStretch(1);
        }
        draw_tab->setLayout(draw_tab_layout);
    }
    return draw_tab;
}


}}
