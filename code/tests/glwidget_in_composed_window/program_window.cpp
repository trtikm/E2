#include "./program_window.hpp"
#include "./program_info.hpp"
#include <qtgl/window.hpp>
#include <QIcon>


program_window::program_window()
    : QMainWindow(nullptr)

    , m_glwindow()

    , m_root_splitter( new QSplitter(Qt::Horizontal,this) )
    , m_log_splitter( new QSplitter(Qt::Vertical) )
    , m_console( new QTextEdit )
    , m_text_edit( new QTextEdit )
    , m_tabs( new QTabWidget )
{
    this->setWindowTitle(get_program_name().c_str());
    this->setWindowIcon(QIcon("../data/shared/gfx/icons/E2_icon.png"));
    this->resize(800,600);
    this->setCentralWidget(m_root_splitter);

    m_root_splitter->addWidget(m_log_splitter);
    m_root_splitter->addWidget(m_tabs);
    m_tabs->addTab(m_text_edit,"Tab 1");
    m_log_splitter->addWidget(m_glwindow.create_widget_container());
    m_log_splitter->addWidget(m_console);
}

program_window::~program_window()
{
}
