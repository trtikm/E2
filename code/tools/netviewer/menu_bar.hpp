#ifndef E2_TOOL_NETVIEWER_MENU_BAR_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_MENU_BAR_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QMenuBar>
#   include <QMenu>
#   include <QAction>

struct  program_window;


struct  menu_bar
{
    menu_bar(program_window* const  wnd);

    program_window*  wnd() const noexcept { return m_wnd; }

    QMenuBar*  get_menu_bar() const noexcept { return m_menu_bar; }
    QMenu*  get_menu_network() const noexcept { return m_menu_network; }
    QAction*  get_action_network_open() const noexcept { return m_action_network_open; }
    QAction*  get_action_network_close() const noexcept { return m_action_network_close; }

    void  on_menu_network_open();
    void  on_menu_network_close();

    bool  does_pause_apply_to_network_open() const noexcept { return m_pause_applies_to_network_open; }

    void  save();

private:
    program_window*  m_wnd;

    QMenuBar*  m_menu_bar;
    QMenu*  m_menu_network;
    QAction*  m_action_network_open;
    QAction*  m_action_network_close;
    bool  m_pause_applies_to_network_open;
};


void  make_menu_bar_content(menu_bar const&  w);


#endif
