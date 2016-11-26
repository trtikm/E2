#ifndef E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_NETWORK_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_NETWORK_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QTextEdit>
#   include <string>


struct  program_window;


namespace window_tabs { namespace tab_network {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window* wnd() const noexcept;

    QTextEdit*  text() const noexcept { return m_text; }

    void  on_text_update();

    void  save();

private:
    program_window*  m_wnd;

    QTextEdit*  m_text;
    std::string  m_last_experiment_name;

};


QWidget*  make_network_tab_content(widgets const&  w);


}}

#endif
