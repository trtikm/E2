#ifndef E2_TOOL_GFXTUNER_MENU_BAR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_MENU_BAR_HPP_INCLUDED

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

    QMenu*  get_menu_file() const noexcept { return m_menu_file; }
    QAction*  get_action_open() const noexcept { return m_action_open; }

    void  on_open();

    void  save();

private:
    program_window*  m_wnd;

    QMenuBar*  m_menu_bar;
    QMenu*  m_menu_file;
    QAction*  m_action_open;
};


void  make_menu_bar_content(menu_bar const&  w);


#endif
