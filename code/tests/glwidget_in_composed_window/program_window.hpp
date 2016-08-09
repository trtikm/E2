#ifndef PROGRAM_WINDOW_HPP_INCLUDED
#   define PROGRAM_WINDOW_HPP_INCLUDED

#   include "./simulator.hpp"
#   include <qtgl/window.hpp>
#   include <QMainWindow>
#   include <QSplitter>
#   include <QWidget>
#   include <QTextEdit>
#   include <QTabWidget>


struct program_window : public QMainWindow
{
    program_window();
    ~program_window();

private:
    Q_OBJECT

    qtgl::window<simulator>  m_glwindow;

    QSplitter*  m_root_splitter;
    QSplitter*  m_log_splitter;
    QTextEdit*  m_console;
    QTextEdit*  m_text_edit;
    QTabWidget*  m_tabs;
};


#endif
