#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_AI_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_AI_HPP_INCLUDED

#   include <QWidget>
#   include <string>


struct  program_window;


namespace window_tabs { namespace tab_statistics { namespace tab_ai {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    void  on_time_event();

private:

    program_window*  m_wnd;
};


std::string  get_tooltip();
QWidget*  make_tab_content(widgets const&  w);


}}}

#endif
