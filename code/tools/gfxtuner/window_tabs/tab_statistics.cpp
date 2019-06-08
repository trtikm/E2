#include <gfxtuner/window_tabs/tab_statistics.hpp>
#include <gfxtuner/program_window.hpp>
#include <qtgl/gui_utils.hpp>
#include <QVBoxLayout>

namespace { namespace tab_names {


inline std::string  RESOURCES() { return "Resources"; }
inline std::string  DRAW() { return "Draw"; }
inline std::string  COLLISIONS() { return "Collisions"; }
inline std::string  PHYSICS() { return "Physics"; }
inline std::string  AI() { return "AI"; }


}}

namespace window_tabs { namespace tab_statistics {


widgets::widgets(program_window* const  wnd)
    : QTabWidget()
    , m_wnd(wnd)
    , m_tab_resources(new tab_resources::widgets(wnd))
    , m_tab_draw(wnd)
    , m_tab_collisions(wnd)
    , m_tab_physics(wnd)
    , m_tab_ai(wnd)
{
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(on_tab_changed(int)));

    int tab_index;

    tab_index = addTab(m_tab_resources, QString(tab_names::RESOURCES().c_str()));
    setTabToolTip(tab_index, tab_resources::get_tooltip().c_str());

    tab_index = addTab(tab_draw::make_tab_content(m_tab_draw), QString(tab_names::DRAW().c_str()));
    setTabToolTip(tab_index, tab_draw::get_tooltip().c_str());

    tab_index = addTab(tab_collisions::make_tab_content(m_tab_collisions), QString(tab_names::COLLISIONS().c_str()));
    setTabToolTip(tab_index, tab_collisions::get_tooltip().c_str());

    tab_index = addTab(tab_physics::make_tab_content(m_tab_physics), QString(tab_names::PHYSICS().c_str()));
    setTabToolTip(tab_index, tab_physics::get_tooltip().c_str());

    tab_index = addTab(tab_ai::make_tab_content(m_tab_ai), QString(tab_names::AI().c_str()));
    setTabToolTip(tab_index, tab_ai::get_tooltip().c_str());

    for (tab_index = 0; tab_index < count(); ++tab_index)
    {
        std::string const  tab_name = qtgl::to_string(tabText(tab_index));
        if (tab_name == wnd->ptree().get("statistics.active_tab", tab_names::RESOURCES()))
        {
            setCurrentIndex(tab_index);
            break;
        }
    }
}


void  widgets::on_tab_changed(int const  tab_index)
{
    std::string const  tab_name = qtgl::to_string(tabText(tab_index));
    if (tab_name == tab_names::RESOURCES())
    {
        // Nothing to do...
    }
    else if (tab_name == tab_names::DRAW())
    {
        // Nothing to do...
    }
    else if (tab_name == tab_names::COLLISIONS())
    {
        // Nothing to do...
    }
    else if (tab_name == tab_names::PHYSICS())
    {
        // Nothing to do...
    }
    else if (tab_name == tab_names::AI())
    {
        // Nothing to do...
    }
}


void  widgets::on_timer_event()
{
    std::string const  tab_name = qtgl::to_string(tabText(currentIndex()));
    if (tab_name == tab_names::RESOURCES())
    {
        m_tab_resources->on_time_event();
    }
    else if (tab_name == tab_names::DRAW())
    {
        m_tab_draw.on_time_event();
    }
    else if (tab_name == tab_names::COLLISIONS())
    {
        m_tab_collisions.on_time_event();
    }
    else if (tab_name == tab_names::PHYSICS())
    {
        m_tab_physics.on_time_event();
    }
    else if (tab_name == tab_names::AI())
    {
        m_tab_ai.on_time_event();
    }
}


void  widgets::save()
{
    std::string const  tab_name = qtgl::to_string(tabText(currentIndex()));
    wnd()->ptree().put("statistics.active_tab", tab_name);
}


}}
