#ifndef E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_DRAW_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_WINDOW_TABS_TAB_DRAW_HPP_INCLUDED

#   include <utility/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QLineEdit>
#   include <QColor>


struct  program_window;


namespace window_tabs { namespace tab_draw {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const noexcept;

    QLineEdit*  clear_colour_component_red() const noexcept;
    QLineEdit*  clear_colour_component_green() const noexcept;
    QLineEdit*  clear_colour_component_blue() const noexcept;

    void  on_clear_colour_changed();
    void  on_clear_colour_set(QColor const&  colour);
    void  on_clear_colour_choose();
    void  on_clear_colour_reset();

    void  save();

private:
    program_window*  m_wnd;

    QLineEdit*  m_clear_colour_component_red;
    QLineEdit*  m_clear_colour_component_green;
    QLineEdit*  m_clear_colour_component_blue;
};


QWidget*  make_draw_tab_content(widgets const&  w);



}}

#endif
