#include <netviewer/program_window.hpp>
#include <netviewer/program_info.hpp>
#include <netviewer/simulator.hpp>
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <QString>
#include <QIcon>


namespace tab_names { namespace {


inline std::string  CAMERA() noexcept { return "Camera"; }
inline std::string  DRAW() noexcept { return "Draw"; }
//inline std::string  NENET() noexcept { return "Nenet"; }
//inline std::string  SELECTED() noexcept { return "Selected"; }


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
    , m_glwindow(vector3(m_ptree->get("draw.clear_colour.red", 64) / 255.0f,
                         m_ptree->get("draw.clear_colour.green", 64) / 255.0f,
                         m_ptree->get("draw.clear_colour.blue", 64) / 255.0f),
                 m_ptree->get("simulation.paused", false),
//                 std::make_shared<nenet::params>(
//                     m_ptree->get("nenet.params.time_step", 0.001f),
//                     m_ptree->get("nenet.params.mini_spiking_potential_magnitude", 0.075f),
//                     m_ptree->get("nenet.params.average_mini_spiking_period_in_seconds", 10.0f / 1000.0f),
//                     m_ptree->get("nenet.params.spiking_potential_magnitude", 0.4f),
//                     m_ptree->get("nenet.params.resting_potential", 0.0f),
//                     m_ptree->get("nenet.params.spiking_threshold", 1.0f),
//                     m_ptree->get("nenet.params.after_spike_potential", -1.0f),
//                     m_ptree->get("nenet.params.potential_descend_coef", 0.2f),
//                     m_ptree->get("nenet.params.potential_ascend_coef", 0.01f),
//                     m_ptree->get("nenet.params.max_connection_distance", 0.25f),
//                     m_ptree->get("nenet.params.output_terminal_velocity_max_magnitude", 0.01f),
//                     m_ptree->get("nenet.params.output_terminal_velocity_min_magnitude", 0.003f)
//                     ),
                 m_ptree->get("simulation.duration_of_second", 1.0f)
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

    , m_tab_draw_widgets(this)
    , m_tab_camera_widgets(this)

    , m_status_bar(this)

//    , m_nenet_param_time_step(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.time_step", 0.001f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_time_step_changed()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_simulation_speed(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.speed", 1.0f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_simulation_speed_changed()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_mini_spiking_potential_magnitude(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.mini_spiking_potential_magnitude", 0.075f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_mini_spiking_potential_magnitude()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_average_mini_spiking_period_in_seconds(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.m_nenet_param_average_mini_spiking_period_in_seconds", 10.0f / 1000.0f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_average_mini_spiking_period_in_seconds()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_spiking_potential_magnitude(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.spiking_potential_magnitude", 0.4f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_spiking_potential_magnitude()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_resting_potential(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.resting_potential", 0.0f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_resting_potential()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_spiking_threshold(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.spiking_threshold", 1.0f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_spiking_threshold()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_after_spike_potential(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.after_spike_potential", -1.0f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_after_spike_potential()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_potential_descend_coef(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.potential_descend_coef", 0.2f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_potential_descend_coef()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_potential_ascend_coef(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.potential_ascend_coef", 0.01f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_potential_ascend_coef()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_max_connection_distance(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.max_connection_distance", 0.25f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_max_connection_distance()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_output_terminal_velocity_max_magnitude(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.output_terminal_velocity_max_magnitude", 0.01f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_output_terminal_velocity_max_magnitude()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )
//    , m_nenet_param_output_terminal_velocity_min_magnitude(
//        [](program_window* wnd) {
//            struct s : public QLineEdit {
//                s(program_window* wnd) : QLineEdit()
//                {
//                    setText(QString::number(wnd->ptree().get("nenet.params.output_terminal_velocity_min_magnitude", 0.003f)));
//                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_nenet_param_output_terminal_velocity_min_magnitude()));
//                }
//            };
//            return new s(wnd);
//        }(this)
//        )

//    , m_selected_props(new QTextEdit)
{
    this->setWindowTitle(get_program_name().c_str());
    this->setWindowIcon(QIcon("../data/shared/gfx/icons/E2_icon.png"));
    this->move({ ptree().get("window.pos.x",0),ptree().get("window.pos.y",0) });
    this->resize(ptree().get("window.width", 1024), ptree().get("window.height", 768));

    this->setCentralWidget(m_splitter);
    m_splitter->addWidget(m_gl_window_widget);
    m_splitter->addWidget(m_tabs);

    m_tabs->addTab( window_tabs::tab_camera::make_camera_tab_content(m_tab_camera_widgets),
                    QString(tab_names::CAMERA().c_str()) );
    m_tabs->addTab( window_tabs::tab_draw::make_draw_tab_content(m_tab_draw_widgets),
                    QString(tab_names::DRAW().c_str()) );

    // Building Nenet tab
//    {
//        QWidget* const  nenet_tab = new QWidget;
//        {
//            QVBoxLayout* const nenet_tab_layout = new QVBoxLayout;
//            {
//                QHBoxLayout* const time_step_layout = new QHBoxLayout;
//                {
//                    time_step_layout->addWidget(new QLabel("Time step in seconds:"));
//                    time_step_layout->addWidget(m_nenet_param_time_step);
//                }
//                nenet_tab_layout->addLayout(time_step_layout);
//                time_step_layout->addStretch(1);

//                QHBoxLayout* const speed_layout = new QHBoxLayout;
//                {
//                    speed_layout->addWidget(new QLabel("Number of simulated seconds per second:"));
//                    speed_layout->addWidget(m_nenet_param_simulation_speed);
//                }
//                nenet_tab_layout->addLayout(speed_layout);
//                speed_layout->addStretch(1);

//                QHBoxLayout* const mini_spiking_potential_magnitude_layout = new QHBoxLayout;
//                {
//                    mini_spiking_potential_magnitude_layout->addWidget(new QLabel("Mini spiking potential magnitude:"));
//                    mini_spiking_potential_magnitude_layout->addWidget(m_nenet_param_mini_spiking_potential_magnitude);
//                }
//                nenet_tab_layout->addLayout(mini_spiking_potential_magnitude_layout);
//                mini_spiking_potential_magnitude_layout->addStretch(1);

//                QHBoxLayout* const average_mini_spiking_period_in_seconds_layout = new QHBoxLayout;
//                {
//                    average_mini_spiking_period_in_seconds_layout->addWidget(new QLabel("Average mini spiking period in seconds:"));
//                    average_mini_spiking_period_in_seconds_layout->addWidget(m_nenet_param_average_mini_spiking_period_in_seconds);
//                }
//                nenet_tab_layout->addLayout(average_mini_spiking_period_in_seconds_layout);
//                average_mini_spiking_period_in_seconds_layout->addStretch(1);

//                QHBoxLayout* const spiking_potential_magnitude_layout = new QHBoxLayout;
//                {
//                    spiking_potential_magnitude_layout->addWidget(new QLabel("Spiking potential magnitude:"));
//                    spiking_potential_magnitude_layout->addWidget(m_nenet_param_spiking_potential_magnitude);
//                }
//                nenet_tab_layout->addLayout(spiking_potential_magnitude_layout);
//                spiking_potential_magnitude_layout->addStretch(1);

//                QHBoxLayout* const resting_potential_layout = new QHBoxLayout;
//                {
//                    resting_potential_layout->addWidget(new QLabel("Resting potential:"));
//                    resting_potential_layout->addWidget(m_nenet_param_resting_potential);
//                }
//                nenet_tab_layout->addLayout(resting_potential_layout);
//                resting_potential_layout->addStretch(1);

//                QHBoxLayout* const after_spike_potential_layout = new QHBoxLayout;
//                {
//                    after_spike_potential_layout->addWidget(new QLabel("After spike potential:"));
//                    after_spike_potential_layout->addWidget(m_nenet_param_after_spike_potential);
//                }
//                nenet_tab_layout->addLayout(after_spike_potential_layout);
//                after_spike_potential_layout->addStretch(1);

//                QHBoxLayout* const potential_descend_coef_layout = new QHBoxLayout;
//                {
//                    potential_descend_coef_layout->addWidget(new QLabel("Potential descend coef:"));
//                    potential_descend_coef_layout->addWidget(m_nenet_param_potential_descend_coef);
//                }
//                nenet_tab_layout->addLayout(potential_descend_coef_layout);
//                potential_descend_coef_layout->addStretch(1);

//                QHBoxLayout* const potential_ascend_coef_layout = new QHBoxLayout;
//                {
//                    potential_ascend_coef_layout->addWidget(new QLabel("Potential ascend coef:"));
//                    potential_ascend_coef_layout->addWidget(m_nenet_param_potential_ascend_coef);
//                }
//                nenet_tab_layout->addLayout(potential_ascend_coef_layout);
//                potential_ascend_coef_layout->addStretch(1);

//                QHBoxLayout* const max_connection_distance_layout = new QHBoxLayout;
//                {
//                    max_connection_distance_layout->addWidget(new QLabel("Max connection distance:"));
//                    max_connection_distance_layout->addWidget(m_nenet_param_max_connection_distance);
//                }
//                nenet_tab_layout->addLayout(max_connection_distance_layout);
//                max_connection_distance_layout->addStretch(1);

//                QHBoxLayout* const output_terminal_velocity_max_magnitude_layout = new QHBoxLayout;
//                {
//                    output_terminal_velocity_max_magnitude_layout->addWidget(new QLabel("Output terminal velocity max magnitude:"));
//                    output_terminal_velocity_max_magnitude_layout->addWidget(m_nenet_param_output_terminal_velocity_max_magnitude);
//                }
//                nenet_tab_layout->addLayout(output_terminal_velocity_max_magnitude_layout);
//                output_terminal_velocity_max_magnitude_layout->addStretch(1);

//                QHBoxLayout* const output_terminal_velocity_min_magnitude_layout = new QHBoxLayout;
//                {
//                    output_terminal_velocity_min_magnitude_layout->addWidget(new QLabel("Output terminal velocity min magnitude:"));
//                    output_terminal_velocity_min_magnitude_layout->addWidget(m_nenet_param_output_terminal_velocity_min_magnitude);
//                }
//                nenet_tab_layout->addLayout(output_terminal_velocity_min_magnitude_layout);
//                output_terminal_velocity_min_magnitude_layout->addStretch(1);

//            }
//            nenet_tab->setLayout(nenet_tab_layout);
//            nenet_tab_layout->addStretch(1);
//        }
//        m_tabs->addTab(nenet_tab, QString(tab_names::NENET().c_str()));
//    }

    // Building Selected tab
//    {
//        QWidget* const  selected_tab = new QWidget;
//        {
//            QVBoxLayout* const layout = new QVBoxLayout;
//            {
//                m_selected_props->setReadOnly(true);
//                layout->addWidget(m_selected_props);
//            }
//            selected_tab->setLayout(layout);
//            //layout->addStretch(1);
//        }
//        m_tabs->addTab(selected_tab, QString(tab_names::SELECTED().c_str()));
//        m_glwindow.register_listener(notifications::selection_changed(), { &program_window::on_selection_changed,this });
//    }

    make_status_bar_content(m_status_bar);

    if (ptree().get("window.show_maximised", false))
        this->showMaximized();
    else
        this->show();

    qtgl::set_splitter_sizes(*m_splitter, ptree().get("window.splitter_ratio", 3.0f / 4.0f));

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

    m_status_bar.update();

//    if (qtgl::to_string(m_tabs->tabText(m_tabs->currentIndex())) == tab_names::SELECTED())
//        on_selection_changed();

    if (m_focus_just_received)
    {
        m_focus_just_received = false;
        m_gl_window_widget->setFocus();
    }
}

void  program_window::closeEvent(QCloseEvent* const  event)
{
    ptree().put("window.pos.x", pos().x());
    ptree().put("window.pos.y", pos().y());
    ptree().put("window.width", width());
    ptree().put("window.height", height());
    ptree().put("window.splitter_ratio", qtgl::get_splitter_sizes_ratio(*m_splitter));
    ptree().put("window.show_maximised", isMaximized());

    m_tab_draw_widgets.save();
    m_tab_camera_widgets.save();

    ptree().put("simulation.paused", m_glwindow.call_now(&simulator::paused));
    ptree().put("simulation.duration_of_second", m_glwindow.call_now(&simulator::desired_number_of_simulated_seconds_per_real_time_second));

//    ptree().put("nenet.params.time_step", m_glwindow.call_now(&simulator::update_time_step_in_seconds));
//    ptree().put("nenet.params.mini_spiking_potential_magnitude", m_glwindow.call_now(&simulator::mini_spiking_potential_magnitude));
//    ptree().put("nenet.params.average_mini_spiking_period_in_seconds", m_glwindow.call_now(&simulator::average_mini_spiking_period_in_seconds));
//    ptree().put("nenet.params.spiking_potential_magnitude", m_glwindow.call_now(&simulator::spiking_potential_magnitude));
//    ptree().put("nenet.params.resting_potential", m_glwindow.call_now(&simulator::resting_potential));
//    ptree().put("nenet.params.spiking_threshold", m_glwindow.call_now(&simulator::spiking_threshold));
//    ptree().put("nenet.params.after_spike_potential", m_glwindow.call_now(&simulator::after_spike_potential));
//    ptree().put("nenet.params.potential_descend_coef", m_glwindow.call_now(&simulator::potential_descend_coef));
//    ptree().put("nenet.params.potential_ascend_coef", m_glwindow.call_now(&simulator::potential_ascend_coef));
//    ptree().put("nenet.params.max_connection_distance", m_glwindow.call_now(&simulator::max_connection_distance));
//    ptree().put("nenet.params.output_terminal_velocity_max_magnitude", m_glwindow.call_now(&simulator::output_terminal_velocity_max_magnitude));
//    ptree().put("nenet.params.output_terminal_velocity_min_magnitude", m_glwindow.call_now(&simulator::output_terminal_velocity_min_magnitude));

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
//    else if (tab_name == tab_names::SELECTED())
//    {
//        on_selection_changed();
//    }
}

//void  program_window::on_nenet_param_simulation_speed_changed()
//{
//    m_glwindow.call_later(&simulator::set_desired_number_of_simulated_seconds_per_real_time_second, m_nenet_param_simulation_speed->text().toFloat());
//}

//void  program_window::on_nenet_param_time_step_changed()
//{
//    m_glwindow.call_later(&simulator::set_update_time_step_in_seconds, m_nenet_param_time_step->text().toFloat());
//}

//void  program_window::on_nenet_param_mini_spiking_potential_magnitude()
//{
//    m_glwindow.call_later(&simulator::set_mini_spiking_potential_magnitude, m_nenet_param_mini_spiking_potential_magnitude->text().toFloat());
//}

//void  program_window::on_nenet_param_average_mini_spiking_period_in_seconds()
//{
//    m_glwindow.call_later(&simulator::set_average_mini_spiking_period_in_seconds, m_nenet_param_average_mini_spiking_period_in_seconds->text().toFloat());
//}

//void  program_window::on_nenet_param_spiking_potential_magnitude()
//{
//    m_glwindow.call_later(&simulator::set_spiking_potential_magnitude, m_nenet_param_spiking_potential_magnitude->text().toFloat());
//}

//void  program_window::on_nenet_param_resting_potential()
//{
//    m_glwindow.call_later(&simulator::set_resting_potential, m_nenet_param_resting_potential->text().toFloat());
//}

//void  program_window::on_nenet_param_spiking_threshold()
//{
//    m_glwindow.call_later(&simulator::set_spiking_threshold, m_nenet_param_spiking_threshold->text().toFloat());
//}

//void  program_window::on_nenet_param_after_spike_potential()
//{
//    m_glwindow.call_later(&simulator::set_after_spike_potential, m_nenet_param_after_spike_potential->text().toFloat());
//}

//void  program_window::on_nenet_param_potential_descend_coef()
//{
//    m_glwindow.call_later(&simulator::set_potential_descend_coef, m_nenet_param_potential_descend_coef->text().toFloat());
//}

//void  program_window::on_nenet_param_potential_ascend_coef()
//{
//    m_glwindow.call_later(&simulator::set_potential_ascend_coef, m_nenet_param_potential_ascend_coef->text().toFloat());
//}

//void  program_window::on_nenet_param_max_connection_distance()
//{
//    m_glwindow.call_later(&simulator::set_max_connection_distance, m_nenet_param_max_connection_distance->text().toFloat());
//}

//void  program_window::on_nenet_param_output_terminal_velocity_max_magnitude()
//{
//    m_glwindow.call_later(&simulator::set_output_terminal_velocity_max_magnitude, m_nenet_param_output_terminal_velocity_max_magnitude->text().toFloat());
//}

//void  program_window::on_nenet_param_output_terminal_velocity_min_magnitude()
//{
//    m_glwindow.call_later(&simulator::set_output_terminal_velocity_min_magnitude, m_nenet_param_output_terminal_velocity_min_magnitude->text().toFloat());
//}

//void program_window::on_selection_changed()
//{
//    std::string const  text = m_glwindow.call_now(&simulator::get_selected_info_text);
//    m_selected_props->setText(QString(text.c_str()));
//}
