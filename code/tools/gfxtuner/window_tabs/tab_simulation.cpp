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
    , m_fixed_frequency_never(nullptr)//new QRadioButton("Never"))
{
    m_start_paused->setChecked(m_wnd->ptree().get("simulation.start_paused", true));    
}


void  widgets::on_simulator_started()
{
    on_fixed_frequency_changed();
    on_min_frequency_changed();
    wnd()->glwindow().call_now(&simulator::simulation_time_config_ref).set_paused(
            m_start_paused->isChecked() ? true : wnd()->ptree().get("simulation.paused", true)
            );
}


void widgets::on_fixed_frequency_changed()
{
    wnd()->glwindow().call_now(&simulator::simulation_time_config_ref).set_fixed_time_step_in_seconds(
            1.0f / m_fixed_frequency->text().toFloat()
            );
}

void widgets::on_min_frequency_changed()
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
    wnd()->ptree().put("simulation.min_frequency", m_min_frequency->text().toFloat());
}

QWidget*  make_simulation_tab_content(widgets const&  w)
{
    QWidget* const  simulation_tab = new QWidget;
    {
        QVBoxLayout* const simulation_tab_layout = new QVBoxLayout;
        {
            simulation_tab_layout->addWidget(w.get_start_paused());
            QWidget* const time_step_control_group = new QGroupBox("Time step control");
            {
                QVBoxLayout* const time_step_control_layout = new QVBoxLayout;
                {
                    QHBoxLayout* const fixed_frequency_layout = new QHBoxLayout;
                    {
                        fixed_frequency_layout->addWidget(new QLabel("Regular frequency [Hz]"));
                        fixed_frequency_layout->addWidget(w.get_fixed_frequency());
                    }
                    time_step_control_layout->addLayout(fixed_frequency_layout);

                    QHBoxLayout* const min_frequency_layout = new QHBoxLayout;
                    {
                        min_frequency_layout->addWidget(new QLabel("Minimal frequency [Hz]"));
                        min_frequency_layout->addWidget(w.get_min_frequency());
                    }
                    time_step_control_layout->addLayout(min_frequency_layout);
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
