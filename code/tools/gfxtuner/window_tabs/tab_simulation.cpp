#include <gfxtuner/window_tabs/tab_simulation.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <boost/property_tree/ptree.hpp>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QString>
#include <QDoubleValidator>

namespace window_tabs { namespace tab_simulation {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)

    , m_start_paused(new QCheckBox("Pause simulation at program startup."))
    , m_fixed_frequency(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QDoubleValidator(1.0, 5000.0, 1));
                    setText(QString::number(wnd->ptree().get("simulation.fixed_frequency", 30.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_simulation_fixed_frequency_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_min_frequency(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QDoubleValidator(1.0, 5000.0, 1));
                    setText(QString::number(wnd->ptree().get("simulation.min_frequency", 30.0)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_simulation_min_frequency_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_fixed_frequency_never(new QRadioButton("Never"))
    , m_fixed_frequency_always(new QRadioButton("Always"))
    , m_fixed_frequency_single_step_only(new QRadioButton("Single step"))
{
    m_start_paused->setChecked(m_wnd->ptree().get("simulation.start_paused", true));

    std::string const  fixed_step_usage = m_wnd->ptree().get("simulation.fixed_step_usage", "single_step");
    m_fixed_frequency_never->setChecked(fixed_step_usage == "never");
    m_fixed_frequency_always->setChecked(fixed_step_usage == "always");
    m_fixed_frequency_single_step_only->setChecked(fixed_step_usage == "single_step");
    QObject::connect(m_fixed_frequency_never, SIGNAL(toggled(bool)), m_wnd, SLOT(on_simulation_fixed_frequency_use_changed(bool)));
    QObject::connect(m_fixed_frequency_always, SIGNAL(toggled(bool)), m_wnd, SLOT(on_simulation_fixed_frequency_use_changed(bool)));
    QObject::connect(m_fixed_frequency_single_step_only, SIGNAL(toggled(bool)), m_wnd, SLOT(on_simulation_fixed_frequency_use_changed(bool)));
}


void  widgets::on_simulator_started()
{
    on_fixed_frequency_changed();
    on_fixed_frequency_use_changed();
    on_min_frequency_changed();

    wnd()->glwindow().call_now(&simulator::simulation_time_config_ref).set_paused(
            m_start_paused->isChecked() ? true : wnd()->ptree().get("simulation.paused", true)
            );
}


void  widgets::on_fixed_frequency_changed()
{
    wnd()->glwindow().call_now(&simulator::simulation_time_config_ref).set_fixed_time_step_in_seconds(
            1.0f / m_fixed_frequency->text().toFloat()
            );
}

void  widgets::on_fixed_frequency_use_changed()
{
    wnd()->glwindow().call_now(&simulator::simulation_time_config_ref).set_fixed_step_usage(
            m_fixed_frequency_never->isChecked()    ? qtgl::simulation_time_config::FIXED_TIME_STEP_USE::NEVER :
            m_fixed_frequency_always->isChecked()   ? qtgl::simulation_time_config::FIXED_TIME_STEP_USE::ALWAYS :
                                                      qtgl::simulation_time_config::FIXED_TIME_STEP_USE::FOR_SINGLE_STEP_ONLY
            );
}

void  widgets::on_min_frequency_changed()
{
    wnd()->glwindow().call_now(&simulator::simulation_time_config_ref).set_longest_time_step_in_seconds(
            1.0f / m_min_frequency->text().toFloat()
            );
}

void  widgets::save()
{
    wnd()->ptree().put("simulation.start_paused", m_start_paused->isChecked());
    wnd()->ptree().put("simulation.paused", wnd()->glwindow().call_now(&simulator::simulation_time_config_ref).is_paused());
    wnd()->ptree().put("simulation.fixed_frequency", m_fixed_frequency->text().toFloat());
    wnd()->ptree().put(
            "simulation.fixed_step_usage",
            m_fixed_frequency_never->isChecked() ? "never" : m_fixed_frequency_always->isChecked() ? "always" : "single_step"
            );
    wnd()->ptree().put("simulation.min_frequency", m_min_frequency->text().toFloat());
}

QWidget*  make_simulation_tab_content(widgets const&  w)
{
    QWidget* const  simulation_tab = new QWidget;
    {
        QVBoxLayout* const simulation_tab_layout = new QVBoxLayout;
        {
            QWidget* const time_step_control_group = new QGroupBox("Time step control");
            {
                QVBoxLayout* const time_step_control_layout = new QVBoxLayout;
                {
                    QWidget* const fixed_step_group = new QGroupBox("Regular frequency");
                    {
                        QHBoxLayout* const fixed_step_group_layout = new QHBoxLayout;
                        {
                            fixed_step_group_layout->addWidget(new QLabel("Hz"));
                            fixed_step_group_layout->addWidget(w.get_fixed_frequency());
                            fixed_step_group_layout->addWidget(new QLabel("    Usage:"));
                            fixed_step_group_layout->addWidget(w.get_fixed_frequency_never());
                            fixed_step_group_layout->addWidget(w.get_fixed_frequency_always());
                            fixed_step_group_layout->addWidget(w.get_fixed_frequency_single_step_only());
                        }
                        fixed_step_group->setLayout(fixed_step_group_layout);
                    }
                    time_step_control_layout->addWidget(fixed_step_group);

                    QHBoxLayout* const min_frequency_and_paused_startup_layout = new QHBoxLayout;
                    {
                        QWidget* const min_frequency_group = new QGroupBox("Minimal frequency");
                        {
                            QHBoxLayout* const min_frequency_group_layout = new QHBoxLayout;
                            {
                                min_frequency_group_layout->addWidget(new QLabel("Hz"));
                                min_frequency_group_layout->addWidget(w.get_min_frequency());
                            }
                            min_frequency_group->setLayout(min_frequency_group_layout);
                        }
                        min_frequency_and_paused_startup_layout->addWidget(min_frequency_group);

                        min_frequency_and_paused_startup_layout->addWidget(w.get_start_paused());
                    }
                    time_step_control_layout->addLayout(min_frequency_and_paused_startup_layout);
                }
                time_step_control_group->setLayout(time_step_control_layout);
            }
            simulation_tab_layout->addWidget(time_step_control_group);

            simulation_tab_layout->addStretch(1);
        }
        simulation_tab->setLayout(simulation_tab_layout);
    }
    return simulation_tab;
}


}}
