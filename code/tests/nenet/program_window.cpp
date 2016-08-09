#include "./program_window.hpp"
#include "./program_info.hpp"
#include <qtgl/window.hpp>
#include <QIcon>


program_window::program_window()
    : QMainWindow(nullptr)
    , m_glwindow()
{
    this->setWindowTitle(get_program_name().c_str());
    this->setWindowIcon(QIcon("../data/shared/gfx/icons/E2_icon.png"));
    this->resize(800,600);
    this->setCentralWidget(m_glwindow.create_widget_container());
}

program_window::~program_window()
{
}
