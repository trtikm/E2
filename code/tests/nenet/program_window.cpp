#include "./program_window.hpp"
#include "./program_info.hpp"
#include "./simulator.hpp"
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <QString>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QIcon>
#include <QStatusBar>
#include <QLabel>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <iomanip>


namespace tab_names { namespace {


inline std::string  CAMERA() noexcept { return "Camera"; }
inline std::string  DRAW() noexcept { return "Draw"; }


}}

namespace {


std::unique_ptr<boost::property_tree::ptree>  load_ptree(boost::filesystem::path const&  ptree_pathname)
{
    std::unique_ptr<boost::property_tree::ptree>  ptree(new boost::property_tree::ptree);
    if (boost::filesystem::exists(ptree_pathname))
        boost::property_tree::read_info(ptree_pathname.string(), *ptree);
    return std::move(ptree);
}


}


program_window::program_window(boost::filesystem::path const&  ptree_pathname)
    : QMainWindow(nullptr)
    , m_ptree_pathname(ptree_pathname)
    , m_ptree(load_ptree(m_ptree_pathname))
    , m_glwindow(vector3(m_ptree->get("tabs.draw.clear_colour.red", 64) / 255.0f,
                         m_ptree->get("tabs.draw.clear_colour.green", 64) / 255.0f,
                         m_ptree->get("tabs.draw.clear_colour.blue", 64) / 255.0f),
                 m_ptree->get("nenet.paused", false)
                 )
    , m_has_focus(false)
    , m_focus_just_received(true)
    , m_idleTimerId(-1)

    , m_gl_window_widget(m_glwindow.create_widget_container())

    , m_splitter(new QSplitter(Qt::Horizontal, this))
    , m_tabs(
        [](program_window* wnd) {
            struct s : public QTabWidget {
                s(program_window* wnd) : QTabWidget()
                {
                    connect(this, SIGNAL(currentChanged(int)), wnd, SLOT(on_tab_changed(int)));
                }
            };
            return new s(wnd);
        }(this)
        )

    , m_clear_colour_component_red(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("tabs.draw.clear_colour.red", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )
    , m_clear_colour_component_green(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("tabs.draw.clear_colour.green", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )
    , m_clear_colour_component_blue(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("tabs.draw.clear_colour.blue", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )

    , m_camera_pos_x(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("tabs.camera.pos.x", 0.5f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )
    , m_camera_pos_y(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("tabs.camera.pos.y", 0.5f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )
    , m_camera_pos_z(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("tabs.camera.pos.z", 2.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )

    , m_camera_rot_w(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("tabs.camera.rot.w", 1.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )
    , m_camera_rot_x(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("tabs.camera.rot.x", 0.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )
    , m_camera_rot_y(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("tabs.camera.rot.y", 0.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(this)
        )
    , m_camera_rot_z(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("tabs.camera.rot.z", 0.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(this)
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
        }(this)
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
        }(this)
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
        }(this)
        )

    , m_camera_save_pos_rot(new QCheckBox("Save position and rotation"))

    , m_spent_real_time(new QLabel("0.0s"))
    , m_spent_simulation_time(new QLabel("0.0s"))
    , m_spent_times_ratio(new QLabel("1.0"))
    , m_num_passed_simulation_steps(new QLabel("#Steps: 0"))
{
    this->setWindowTitle(get_program_name().c_str());
    this->setWindowIcon(QIcon("../data/shared/gfx/icons/E2_icon.png"));
    this->move({ ptree().get("program_window.pos.x",0),ptree().get("program_window.pos.y",0) });
    this->resize(ptree().get("program_window.width", 1024), ptree().get("program_window.height", 768));

    this->setCentralWidget(m_splitter);
    m_splitter->addWidget(m_gl_window_widget);
    m_splitter->addWidget(m_tabs);

    // Building Camera tab
    {
        QWidget* const  camera_tab = new QWidget;
        {
            QVBoxLayout* const camera_tab_layout = new QVBoxLayout;
            {
                QWidget* const position_group = new QGroupBox("Position in meters [xyz]");
                {
                    QHBoxLayout* const position_layout = new QHBoxLayout;
                    {
                        position_layout->addWidget(m_camera_pos_x);
                        position_layout->addWidget(m_camera_pos_y);
                        position_layout->addWidget(m_camera_pos_z);
                        on_camera_pos_changed();
                        m_glwindow.register_listener(notifications::camera_position_updated(),
                        { &program_window::camera_position_listener,this });
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
                            quaternion_layout->addWidget(m_camera_rot_w);
                            quaternion_layout->addWidget(m_camera_rot_x);
                            quaternion_layout->addWidget(m_camera_rot_y);
                            quaternion_layout->addWidget(m_camera_rot_z);
                        }
                        rotation_layout->addLayout(quaternion_layout);

                        rotation_layout->addWidget(new QLabel("Tait-Bryan angles in degrees [yaw(z)-pitch(y')-roll(x'')]"));
                        QHBoxLayout* const tait_bryan_layout = new QHBoxLayout;
                        {
                            tait_bryan_layout->addWidget(m_camera_yaw);
                            tait_bryan_layout->addWidget(m_camera_pitch);
                            tait_bryan_layout->addWidget(m_camera_roll);
                        }
                        rotation_layout->addLayout(tait_bryan_layout);
                    }
                    rotation_group->setLayout(rotation_layout);

                    on_camera_rot_changed();
                    m_glwindow.register_listener(notifications::camera_orientation_updated(),
                    { &program_window::camera_rotation_listener,this });
                }
                camera_tab_layout->addWidget(rotation_group);

                m_camera_save_pos_rot->setCheckState(
                    ptree().get("tabs.camera.save_pos_rot", true) ? Qt::CheckState::Checked :
                    Qt::CheckState::Unchecked
                    );
                camera_tab_layout->addWidget(m_camera_save_pos_rot);

                camera_tab_layout->addStretch(1);
            }
            camera_tab->setLayout(camera_tab_layout);
        }
        m_tabs->addTab(camera_tab, QString(tab_names::CAMERA().c_str()));
    }

    // Building Draw tab
    {
        QWidget* const  draw_tab = new QWidget;
        {
            QVBoxLayout* const draw_tab_layout = new QVBoxLayout;
            {
                QWidget* const clear_colour_group = new QGroupBox("Clear colour [rgb]");
                {
                    QVBoxLayout* const clear_colour_layout = new QVBoxLayout;
                    {
                        QHBoxLayout* const edit_boxes_layout = new QHBoxLayout;
                        {
                            edit_boxes_layout->addWidget(m_clear_colour_component_red);
                            edit_boxes_layout->addWidget(m_clear_colour_component_green);
                            edit_boxes_layout->addWidget(m_clear_colour_component_blue);
                            on_clear_colour_changed();
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
                            }(this)
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
                            }(this)
                                );
                        }
                        clear_colour_layout->addLayout(buttons_layout);

                    }
                    clear_colour_group->setLayout(clear_colour_layout);
                }
                draw_tab_layout->addWidget(clear_colour_group);
                draw_tab_layout->addStretch(1);
            }
            draw_tab->setLayout(draw_tab_layout);
        }
        m_tabs->addTab(draw_tab, QString(tab_names::DRAW().c_str()));
    }

    statusBar()->addPermanentWidget(m_spent_real_time);
    statusBar()->addPermanentWidget(m_spent_simulation_time);
    statusBar()->addPermanentWidget(m_spent_times_ratio);
    statusBar()->addPermanentWidget(m_num_passed_simulation_steps);
    statusBar()->addPermanentWidget(
            [](qtgl::window<simulator>* const glwindow, bool const  paused) {
                struct s : public qtgl::widget_base<s, qtgl::window<simulator> >, public QLabel {
                    s(qtgl::window<simulator>* const  glwindow, bool const  paused)
                        : qtgl::widget_base<s, qtgl::window<simulator> >(glwindow), QLabel()
                    {
                        setText(paused ? "PAUSED" : "RUNNING");
                        register_listener(notifications::paused(),&s::on_paused_changed);
                    }
                    void  on_paused_changed()
                    {
                        setText(call_now(&simulator::paused) ? "PAUSED" : "RUNNING");
                    }
                };
                return new s(glwindow, paused);
            }(&m_glwindow, m_ptree->get("nenet.paused", false))
            );
    statusBar()->addPermanentWidget(
            [](qtgl::window<simulator>* const glwindow) {
                struct s : public qtgl::widget_base<s, qtgl::window<simulator> >, public QLabel {
                    s(qtgl::window<simulator>* const  glwindow)
                        : qtgl::widget_base<s, qtgl::window<simulator> >(glwindow), QLabel()
                    {
                        setText("FPS: 0");
                        register_listener(qtgl::notifications::fps_updated(),
                            &s::on_fps_changed);
                    }
                    void  on_fps_changed()
                    {
                        std::stringstream  sstr;
                        sstr << "FPS: " << call_now(&qtgl::real_time_simulator::FPS);
                        setText(sstr.str().c_str());
                    }
                };
                return new s(glwindow);
            }(&m_glwindow)
            );
    statusBar()->showMessage("Ready", 2000);

    if (ptree().get("program_window.show_maximised", false))
        this->showMaximized();
    else
        this->show();

    qtgl::set_splitter_sizes(*m_splitter, ptree().get("program_window.splitter_ratio", 3.0f / 4.0f));

    //m_gl_window_widget->setFocus();

    m_idleTimerId = startTimer(100); // In milliseconds.
}

program_window::~program_window()
{
}

bool program_window::event(QEvent* const event)
{
    switch (event->type())
    {
    case QEvent::WindowActivate:
    case QEvent::FocusIn:
        m_has_focus = true;
        m_focus_just_received = true;
        return QMainWindow::event(event);
    case QEvent::WindowDeactivate:
    case QEvent::FocusOut:
        m_has_focus = false;
        return QMainWindow::event(event);
    default:
        return QMainWindow::event(event);
    }
}

void program_window::timerEvent(QTimerEvent* const event)
{
    if (event->timerId() != m_idleTimerId)
        return;

    // Here put time-dependent updates...

    {
        float_64_bit const  real_time = m_glwindow.call_now(&simulator::spent_real_time);
        std::string  msg = msgstream() << "RT: " << std::fixed << std::setprecision(3) << real_time << "s";
        m_spent_real_time->setText(msg.c_str());

        float_64_bit const  simulation_time = m_glwindow.call_now(&simulator::spent_simulation_time);
        msg = msgstream() << "ST: " << std::fixed << std::setprecision(3) << simulation_time  << "s";
        m_spent_simulation_time->setText(msg.c_str());

        float_64_bit const  simulation_time_to_real_time = real_time > 1e-5f ? simulation_time / real_time : 1.0;
        msg = msgstream() << "ST/RT: " << std::fixed << std::setprecision(3) << simulation_time_to_real_time;
        m_spent_times_ratio->setText(msg.c_str());

        natural_64_bit const  num_steps = m_glwindow.call_now(&simulator::nenet_num_updates);
        msg = msgstream() << "#Steps: " << num_steps;
        m_num_passed_simulation_steps->setText(msg.c_str());
    }

    if (m_focus_just_received)
    {
        m_focus_just_received = false;
        m_gl_window_widget->setFocus();
    }
}

void  program_window::closeEvent(QCloseEvent* const  event)
{
    ptree().put("program_window.pos.x", pos().x());
    ptree().put("program_window.pos.y", pos().y());
    ptree().put("program_window.width", width());
    ptree().put("program_window.height", height());
    ptree().put("program_window.splitter_ratio", qtgl::get_splitter_sizes_ratio(*m_splitter));
    ptree().put("program_window.show_maximised", isMaximized());

    ptree().put("tabs.draw.clear_colour.red", m_clear_colour_component_red->text().toInt());
    ptree().put("tabs.draw.clear_colour.green", m_clear_colour_component_green->text().toInt());
    ptree().put("tabs.draw.clear_colour.blue", m_clear_colour_component_blue->text().toInt());

    if (m_camera_save_pos_rot->isChecked())
    {
        ptree().put("tabs.camera.pos.x", m_camera_pos_x->text().toFloat());
        ptree().put("tabs.camera.pos.y", m_camera_pos_y->text().toFloat());
        ptree().put("tabs.camera.pos.z", m_camera_pos_z->text().toFloat());
        ptree().put("tabs.camera.rot.w", m_camera_rot_w->text().toFloat());
        ptree().put("tabs.camera.rot.x", m_camera_rot_x->text().toFloat());
        ptree().put("tabs.camera.rot.y", m_camera_rot_y->text().toFloat());
        ptree().put("tabs.camera.rot.z", m_camera_rot_z->text().toFloat());
    }
    ptree().put("tabs.camera.save_pos_rot", m_camera_save_pos_rot->isChecked());

    ptree().put("nenet.paused", m_glwindow.call_now(&simulator::paused));

    boost::property_tree::write_info(m_ptree_pathname.string(), ptree());
}

void  program_window::on_tab_changed(int const  tab_index)
{
    std::string const  tab_name = qtgl::to_string(m_tabs->tabText(tab_index));
    if (tab_name == tab_names::CAMERA())
    {
        // Nothing to do...
    }
    else if (tab_name == tab_names::DRAW())
    {
        // Nothing to do...
    }
}

void program_window::on_clear_colour_changed()
{
    vector3 const  colour(
        (float_32_bit)m_clear_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_blue->text().toInt() / 255.0f
        );
    m_glwindow.call_later(&simulator::set_clear_color, colour);
}

void program_window::on_clear_colour_set(QColor const&  colour)
{
    m_clear_colour_component_red->setText(QString::number(colour.red()));
    m_clear_colour_component_green->setText(QString::number(colour.green()));
    m_clear_colour_component_blue->setText(QString::number(colour.blue()));
    on_clear_colour_changed();
}

void program_window::on_clear_colour_choose()
{
    QColor const  init_colour(m_clear_colour_component_red->text().toInt(),
        m_clear_colour_component_green->text().toInt(),
        m_clear_colour_component_blue->text().toInt());
    QColor const  colour = QColorDialog::getColor(init_colour, this, "Choose clear colour");
    if (!colour.isValid())
        return;
    on_clear_colour_set(colour);
}

void program_window::on_clear_colour_reset()
{
    on_clear_colour_set(QColor(64, 64, 64));
}

void  program_window::on_camera_pos_changed()
{
    vector3 const  pos(m_camera_pos_x->text().toFloat(),
        m_camera_pos_y->text().toFloat(),
        m_camera_pos_z->text().toFloat());
    m_glwindow.call_later(&simulator::set_camera_position, pos);
}

void  program_window::camera_position_listener()
{
    vector3 const pos = m_glwindow.call_now(&simulator::get_camera_position);
    m_camera_pos_x->setText(QString::number(pos(0)));
    m_camera_pos_y->setText(QString::number(pos(1)));
    m_camera_pos_z->setText(QString::number(pos(2)));
}

void  program_window::on_camera_rot_changed()
{
    quaternion  q(m_camera_rot_w->text().toFloat(),
        m_camera_rot_x->text().toFloat(),
        m_camera_rot_y->text().toFloat(),
        m_camera_rot_z->text().toFloat());
    if (length_squared(q) < 1e-5f)
        q.z() = 1.0f;
    normalise(q);

    update_camera_rot_widgets(q);
    m_glwindow.call_later(&simulator::set_camera_orientation, q);
}

void  program_window::on_camera_rot_tait_bryan_changed()
{
    quaternion  q = rotation_matrix_to_quaternion(yaw_pitch_roll_to_rotation(
        m_camera_yaw->text().toFloat() * PI() / 180.0f,
        m_camera_pitch->text().toFloat() * PI() / 180.0f,
        m_camera_roll->text().toFloat() * PI() / 180.0f
        ));
    normalise(q);
    update_camera_rot_widgets(q);
    m_glwindow.call_later(&simulator::set_camera_orientation, q);
}

void  program_window::camera_rotation_listener()
{
    update_camera_rot_widgets(m_glwindow.call_now(&simulator::get_camera_orientation));
}

void  program_window::update_camera_rot_widgets(quaternion const&  q)
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
