#include <netviewer/window_tabs/tab_camera.hpp>
#include <netviewer/program_window.hpp>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QString>

namespace window_tabs { namespace tab_camera {


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
{

}

program_window* widgets::wnd() const noexcept
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
}


QWidget*  make_camera_tab_content(widgets const&  w)
{
    QWidget* const  camera_tab = new QWidget;
    {
        QVBoxLayout* const camera_tab_layout = new QVBoxLayout;
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
                                notifications::camera_position_updated(),
                                { &program_window::camera_position_listener,w.wnd() }
                                );
                }
                position_group->setLayout(position_layout);
            }
            camera_tab_layout->addWidget(position_group);

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
                            notifications::camera_orientation_updated(),
                            { &program_window::camera_rotation_listener,w.wnd() }
                            );
            }
            camera_tab_layout->addWidget(rotation_group);

            w.camera_save_pos_rot()->setCheckState(
                w.wnd()->ptree().get("camera.save_pos_rot", false) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked
                );
            camera_tab_layout->addWidget(w.camera_save_pos_rot());

            camera_tab_layout->addStretch(1);
        }
        camera_tab->setLayout(camera_tab_layout);
    }
    return camera_tab;
}





}}
