#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_RESOURCES_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_RESOURCES_HPP_INCLUDED

#   include <QWidget>


struct  program_window;


namespace window_tabs { namespace tab_statistics { namespace tab_resources {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

private:

    program_window*  m_wnd;
};


QWidget*  make_tab_content(widgets const&  w);


}}}

#endif
