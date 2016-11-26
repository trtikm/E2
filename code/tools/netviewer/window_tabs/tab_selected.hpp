#ifndef E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_SELECTED_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_SELECTED_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QTextEdit>


struct  program_window;


namespace window_tabs { namespace tab_selected {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window* wnd() const noexcept;

    QTextEdit*  selected_text() const noexcept;

    void  on_selection_update();

    void  save();

private:
    program_window*  m_wnd;

    QTextEdit*  m_selected_text;
};


QWidget*  make_selected_tab_content(widgets const&  w);


}}

#endif
