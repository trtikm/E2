#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_RESOURCES_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_RESOURCES_HPP_INCLUDED

#   include <utility/async_resource_load.hpp>
#   include <QWidget>
#   include <QTreeWidget>
#   include <QIcon>


struct  program_window;


namespace window_tabs { namespace tab_statistics { namespace tab_resources {


struct  widgets : public QTreeWidget
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    void  on_time_event();

private:

    program_window*  m_wnd;
    QIcon  m_icon_data_type;
    QIcon  m_icon_wainting_for_load;
    QIcon  m_icon_being_loaded;
    QIcon  m_icon_in_use;

    async::statistics_of_cached_resources  m_records;
    async::key_type  m_just_being_loaded;
};


}}}

#endif
