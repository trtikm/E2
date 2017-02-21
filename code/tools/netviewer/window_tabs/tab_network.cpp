#include <netviewer/window_tabs/tab_network.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <netexp/experiment_factory.hpp>
#include <QVBoxLayout>

namespace window_tabs { namespace tab_network {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
    , m_text(new QTextEdit)
    , m_auto_open_last(new QCheckBox("Auto-open last opened network"))
    , m_last_experiment_name("<--!NETVIEWER-TAB-NETWORK-NO-EXPERIMENT-AVAILABLE-YET!-->")
{}

program_window* widgets::wnd() const noexcept
{
    return m_wnd;
}

void  widgets::on_text_update()
{
    std::string const  experiment_name = wnd()->glwindow().call_now(&simulator::get_experiment_name);
    if (m_last_experiment_name != experiment_name)
    {
        m_last_experiment_name = experiment_name;
        std::string const  txt = wnd()->glwindow().call_now(&simulator::get_network_info_text);
        text()->setText(QString(txt.c_str()));
    }
}

void  widgets::save()
{
    wnd()->ptree().put("network.do_auto_open",m_auto_open_last->isChecked() ? true : false);
    wnd()->ptree().put("network.auto_open",wnd()->glwindow().call_now(&simulator::get_experiment_name));
}


QWidget*  make_network_tab_content(widgets const&  w)
{
    QWidget* const  network_tab = new QWidget;
    {
        QVBoxLayout* const layout = new QVBoxLayout;
        {
            w.text()->setReadOnly(true);
            layout->addWidget(w.text());
            layout->addWidget(w.auto_open_last());
        }
        network_tab->setLayout(layout);
    }

    w.auto_open_last()->setCheckState(
        w.wnd()->ptree().get("network.do_auto_open", false) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked
        );

    if (w.wnd()->ptree().get("network.do_auto_open", false) &&
        w.wnd()->ptree().get("network.auto_open", "") != "")
    {
        std::string const  experiment_name = w.wnd()->ptree().get("network.auto_open", "");
        std::vector<std::string>  experiments;
        netexp::experiment_factory::instance().get_names_of_registered_experiments(experiments);
        for (auto const& experiment : experiments)
            if (experiment == experiment_name)
            {
                w.wnd()->glwindow().call_later(&simulator::destroy_network);
                w.wnd()->glwindow().call_later(
                        &simulator::initiate_network_construction,
                        experiment_name,
                        w.wnd()->ptree().get("network.pause_construction", false)
                        );
                break;
            }
    }

    return network_tab;
}


}}
