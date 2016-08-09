#ifndef PROGRAM_WINDOW_HPP_INCLUDED
#   define PROGRAM_WINDOW_HPP_INCLUDED

#   include "./simulator.hpp"
#   include <qtgl/window.hpp>
#   include <QMainWindow>
#   include <QWidget>


struct program_window : public QMainWindow
{
    program_window();
    ~program_window();

private:
    Q_OBJECT

    qtgl::window<simulator>  m_glwindow;
};


#endif
