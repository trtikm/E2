#include <gfxtuner/menu_bar.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/simulator.hpp>
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <QString>


menu_bar::menu_bar(program_window* const  wnd)
    : m_wnd(wnd)

    , m_menu_bar(new QMenuBar(wnd))
    , m_menu_file(new QMenu("&File",wnd))
    , m_action_open(new QAction(QString("&Open"), wnd))
{}

void  menu_bar::on_open()
{
}

void  menu_bar::save()
{
    //wnd()->ptree().put("...",...);
}


void  make_menu_bar_content(menu_bar const&  w)
{
    w.get_menu_file()->addAction(w.get_action_open());
    QObject::connect(w.get_action_open(), &QAction::triggered, w.wnd(),&program_window::on_menu_open);

    w.get_menu_bar()->addMenu(w.get_menu_file());

    w.wnd()->setMenuBar(w.get_menu_bar());
}
