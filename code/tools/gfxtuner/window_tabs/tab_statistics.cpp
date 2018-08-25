#include <gfxtuner/window_tabs/tab_statistics.hpp>
#include <gfxtuner/program_window.hpp>
#include <qtgl/gui_utils.hpp>
#include <QVBoxLayout>

namespace { namespace tab_names {


inline std::string  RESOURCES() { return "Resources"; }


}}

namespace window_tabs { namespace tab_statistics {


widgets::widgets(program_window* const  wnd)
    : QTabWidget()
    , m_wnd(wnd)
    , m_tab_resources(new tab_resources::widgets(wnd))
{
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(on_tab_changed(int)));

    addTab(m_tab_resources, QString(tab_names::RESOURCES().c_str()));
}


void  widgets::on_tab_changed(int const  tab_index)
{
    std::string const  tab_name = qtgl::to_string(tabText(tab_index));
    if (tab_name == tab_names::RESOURCES())
    {
        // Nothing to do...
    }
}


void  widgets::on_timer_event()
{
    std::string const  tab_name = qtgl::to_string(tabText(currentIndex()));
    if (tab_name == tab_names::RESOURCES())
    {
        // Nothing to do...
    }
}


}}
