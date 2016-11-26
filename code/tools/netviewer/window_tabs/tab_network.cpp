#include <netviewer/window_tabs/tab_network.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <QVBoxLayout>

namespace window_tabs { namespace tab_network {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
    , m_text(new QTextEdit)
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
    // There is nothing to save.
}


QWidget*  make_network_tab_content(widgets const&  w)
{
    QWidget* const  network_tab = new QWidget;
    {
        QVBoxLayout* const layout = new QVBoxLayout;
        {
            w.text()->setReadOnly(true);
            layout->addWidget(w.text());
        }
        network_tab->setLayout(layout);
    }
    return network_tab;
}


}}
