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

    void  insert_record_to_the_tree(
            async::key_type const&  key,
            async::cached_resource_info const&  info,
            bool const  is_just_being_loaded
            );

    void  erase_record_from_the_tree(
            async::key_type const&  key,
            async::cached_resource_info const&  info
            );

    void  update_record_in_the_tree(
            async::key_type const&  key,
            async::cached_resource_info const&  old_info,
            async::cached_resource_info const&  new_info,
            bool const  is_just_being_loaded
            );

    QIcon const*  choose_icon(async::LOAD_STATE const  load_state, bool const  is_just_being_loaded) const;

    program_window*  m_wnd;
    QIcon  m_icon_data_type;
    QIcon  m_icon_wainting_for_load;
    QIcon  m_icon_being_loaded;
    QIcon  m_icon_in_use;
    QIcon  m_icon_failed_to_load;

    async::statistics_of_cached_resources  m_records;
    async::key_type  m_just_being_loaded;
};


}}}

#endif
