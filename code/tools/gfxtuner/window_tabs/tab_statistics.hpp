#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_HPP_INCLUDED

#   include <gfxtuner/window_tabs/tab_statistics_resources.hpp>
#   include <QWidget>
#   include <QTabWidget>


struct  program_window;


namespace window_tabs { namespace tab_statistics {


struct  widgets : public QTabWidget
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

public slots:

    void  on_tab_changed(int const  tab_index);
    void  on_timer_event();

private:

    Q_OBJECT

    program_window*  m_wnd;

    tab_resources::widgets*  m_tab_resources;
};


}}

#endif
