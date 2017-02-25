#include <netviewer/window_tabs/tab_performance.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <netexp/experiment_factory.hpp>
#include <QVBoxLayout>
#include <limits>

namespace {


inline std::string  invalidated_experiment_name() { return "<--!NETVIEWER-TAB-PERFORMANCE-INVALIDATED-EXPERIMENT-NAME-->"; }


}

namespace window_tabs { namespace tab_performance {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
    , m_text(new QTextEdit)
    , m_use_update_queues_of_ships_in_network(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Use update queues of ships in the network")
                {
                    setChecked(wnd->ptree().get("network.use_update_queues_of_ships",true));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_use_update_queues_of_ships_in_network_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_last_experiment_name(invalidated_experiment_name())
    , m_update_id(std::numeric_limits<natural_64_bit>::max())
{}

program_window* widgets::wnd() const noexcept
{
    return m_wnd;
}

void  widgets::on_performance_update()
{
    if (!wnd()->glwindow().call_now(&simulator::has_network))
    {
        text()->setText(QString("No network is loaded."));
        m_last_experiment_name = invalidated_experiment_name();
        m_update_id = std::numeric_limits<natural_64_bit>::max();
        return;
    }
    std::string const  experiment_name = wnd()->glwindow().call_now(&simulator::get_experiment_name);
    if (experiment_name != m_last_experiment_name)
    {
        wnd()->glwindow().call_now(&simulator::enable_usage_of_queues_in_update_of_ships_in_network,
                                   m_use_update_queues_of_ships_in_network->isChecked());
        m_last_experiment_name = experiment_name;
        m_update_id = std::numeric_limits<natural_64_bit>::max();
    }
    natural_64_bit const  update_id = wnd()->glwindow().call_now(&simulator::get_network_update_id);
    if (update_id != m_update_id)
    {
        std::string const  txt = wnd()->glwindow().call_now(&simulator::get_network_performance_text);
        text()->setText(QString(txt.c_str()));

        m_update_id = update_id;
    }
}

void  widgets::on_use_update_queues_of_ships_in_network_changed(int const  value)
{
    m_last_experiment_name = invalidated_experiment_name();
}

void  widgets::save()
{
    wnd()->ptree().put("network.use_update_queues_of_ships",m_use_update_queues_of_ships_in_network->isChecked() ? true : false);
}


QWidget*  make_performance_tab_content(widgets const&  w)
{
    QWidget* const  network_tab = new QWidget;
    {
        QVBoxLayout* const layout = new QVBoxLayout;
        {
            w.text()->setReadOnly(true);
            layout->addWidget(w.text());
            layout->addWidget(w.use_update_queues_of_ships_in_network());
        }
        network_tab->setLayout(layout);
    }

    w.use_update_queues_of_ships_in_network()->setCheckState(
        w.wnd()->ptree().get("network.use_update_queues_of_ships", true) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked
        );

    return network_tab;
}


}}
