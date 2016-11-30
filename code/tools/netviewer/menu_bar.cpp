#include <netviewer/menu_bar.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <QString>


menu_bar::menu_bar(program_window* const  wnd)
    : m_wnd(wnd)

    , m_menu_bar(new QMenuBar(wnd))
    , m_menu_network(new QMenu("&Network",wnd))
    , m_action_network_open(new QAction(QString("&Open"), wnd))
    , m_action_network_close(new QAction(QString("&Close"), wnd))
{}

void  menu_bar::on_menu_network_open()
{
    std::string const  experiment_name = wnd()->ptree().get("simulation.auto_load_experiment", std::string("calibration"));//performance
    if (!experiment_name.empty())
        wnd()->glwindow().call_later(&simulator::initiate_network_construction, experiment_name);
}

void  menu_bar::on_menu_network_close()
{
    wnd()->glwindow().call_later(&simulator::destroy_network);
}


void  make_menu_bar_content(menu_bar const&  w)
{
    w.get_menu_network()->addAction(w.get_action_network_open());
    QObject::connect(w.get_action_network_open(), &QAction::triggered, w.wnd(),&program_window::on_menu_network_open);

    w.get_menu_network()->addAction(w.get_action_network_close());
    QObject::connect(w.get_action_network_close(), &QAction::triggered, w.wnd(),&program_window::on_menu_network_close);

    w.get_menu_bar()->addMenu(w.get_menu_network());

    w.wnd()->setMenuBar(w.get_menu_bar());
}
