#ifndef E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_PERFORMANCE_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_PERFORMANCE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QTextEdit>
#   include <QCheckBox>
#   include <string>


struct  program_window;


namespace window_tabs { namespace tab_performance {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window* wnd() const noexcept;

    QTextEdit*  text() const noexcept { return m_text; }
    QCheckBox*  use_update_queues_of_ships_in_network() const noexcept { return m_use_update_queues_of_ships_in_network; }

    void  on_performance_update();
    void  on_use_update_queues_of_ships_in_network_changed(int const  value);

    void  save();

private:
    program_window*  m_wnd;

    QTextEdit*  m_text;
    QCheckBox*  m_use_update_queues_of_ships_in_network;
    std::string  m_last_experiment_name;
    natural_64_bit  m_update_id;
};


QWidget*  make_performance_tab_content(widgets const&  w);


}}

#endif
