#include <netviewer/program_window.hpp>
#include <netviewer/program_info.hpp>
#include <netviewer/program_options.hpp>
#include <netviewer/simulator.hpp>
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <QString>
#include <QIcon>


namespace tab_names { namespace {


inline std::string  DRAW() noexcept { return "Draw"; }
inline std::string  NETWORK() noexcept { return "Network"; }
inline std::string  SELECTED() noexcept { return "Selected"; }
inline std::string  PERFORMANCE() noexcept { return "Performance"; }


}}

namespace {


std::unique_ptr<boost::property_tree::ptree>  load_ptree(boost::filesystem::path const&  ptree_pathname)
{
    std::unique_ptr<boost::property_tree::ptree>  ptree(new boost::property_tree::ptree);
    if (boost::filesystem::exists(ptree_pathname))
        boost::property_tree::read_info(ptree_pathname.string(), *ptree);
    return std::move(ptree);
}


}


program_window::program_window(boost::filesystem::path const&  ptree_pathname)
    : QMainWindow(nullptr)
    , m_ptree_pathname(ptree_pathname)
    , m_ptree(load_ptree(m_ptree_pathname))
    , m_glwindow(vector3(m_ptree->get("draw.clear_colour.red", 64) / 255.0f,
                         m_ptree->get("draw.clear_colour.green", 64) / 255.0f,
                         m_ptree->get("draw.clear_colour.blue", 64) / 255.0f),
                 m_ptree->get("simulation.paused", false),
                 m_ptree->get("simulation.duration_of_second", 1.0f)
                 )
    , m_has_focus(false)
    , m_focus_just_received(true)
    , m_idleTimerId(-1)
    , m_is_this_first_timer_event(true)

    , m_gl_window_widget(m_glwindow.create_widget_container())

    , m_splitter(new QSplitter(Qt::Horizontal, this))
    , m_tabs(
        [](program_window* wnd) {
            struct s : public QTabWidget {
                s(program_window* wnd) : QTabWidget()
                {
                    connect(this, SIGNAL(currentChanged(int)), wnd, SLOT(on_tab_changed(int)));
                }
            };
            return new s(wnd);
        }(this)
        )

    , m_tab_draw_widgets(this)
    , m_tab_network_widgets(this)
    , m_tab_selected_widgets(this)
    , m_tab_performance_widgets(this)

    , m_menu_bar(this)
    , m_status_bar(this)
{
    this->setWindowTitle(get_program_name().c_str());
    this->setWindowIcon(QIcon((get_program_options()->dataRoot() + "/shared/gfx/icons/E2_icon.png").c_str()));
    this->move({ ptree().get("window.pos.x",0),ptree().get("window.pos.y",0) });
    this->resize(ptree().get("window.width", 1024), ptree().get("window.height", 768));

    make_menu_bar_content(m_menu_bar);

    this->setCentralWidget(m_splitter);
    m_splitter->addWidget(m_gl_window_widget);
    m_splitter->addWidget(m_tabs);

    m_tabs->addTab( window_tabs::tab_draw::make_draw_tab_content(m_tab_draw_widgets),
                    QString(tab_names::DRAW().c_str()) );
    m_tabs->addTab( window_tabs::tab_network::make_network_tab_content(m_tab_network_widgets),
                    QString(tab_names::NETWORK().c_str()) );
    m_tabs->addTab( window_tabs::tab_selected::make_selected_tab_content(m_tab_selected_widgets),
                    QString(tab_names::SELECTED().c_str()) );
    m_tabs->addTab( window_tabs::tab_performance::make_performance_tab_content(m_tab_performance_widgets),
                    QString(tab_names::PERFORMANCE().c_str()) );

    make_status_bar_content(m_status_bar);

    this->show();

    qtgl::set_splitter_sizes(*m_splitter, ptree().get("window.splitter_ratio", 3.0f / 4.0f));

    m_idleTimerId = startTimer(100); // In milliseconds.
}

program_window::~program_window()
{
}

bool program_window::event(QEvent* const event)
{
    switch (event->type())
    {
    case QEvent::WindowActivate:
    case QEvent::FocusIn:
        m_has_focus = true;
        m_focus_just_received = true;
        return QMainWindow::event(event);
    case QEvent::WindowDeactivate:
    case QEvent::FocusOut:
        m_has_focus = false;
        return QMainWindow::event(event);
    default:
        return QMainWindow::event(event);
    }
}

void program_window::timerEvent(QTimerEvent* const event)
{
    if (event->timerId() != m_idleTimerId)
        return;

    if (m_is_this_first_timer_event)
    {
        if (ptree().get("window.show_maximised", false))
            this->showMaximized();
        m_is_this_first_timer_event = false;
    }

    // Here put time-dependent updates...

    m_status_bar.update();

    if (qtgl::to_string(m_tabs->tabText(m_tabs->currentIndex())) == tab_names::NETWORK())
        on_network_info_text_update();
    else if (qtgl::to_string(m_tabs->tabText(m_tabs->currentIndex())) == tab_names::SELECTED())
        on_selection_update();
    else if (qtgl::to_string(m_tabs->tabText(m_tabs->currentIndex())) == tab_names::PERFORMANCE())
        on_performance_update();

    if (m_focus_just_received)
    {
        m_focus_just_received = false;
        m_gl_window_widget->setFocus();
    }
}

void  program_window::closeEvent(QCloseEvent* const  event)
{
    if (!isMaximized())
    {
        ptree().put("window.pos.x", pos().x());
        ptree().put("window.pos.y", pos().y());
        ptree().put("window.width", width());
        ptree().put("window.height", height());
    }
    ptree().put("window.splitter_ratio", qtgl::get_splitter_sizes_ratio(*m_splitter));
    ptree().put("window.show_maximised", isMaximized());

    m_tab_draw_widgets.save();
    m_tab_network_widgets.save();
    m_tab_selected_widgets.save();
    m_tab_performance_widgets.save();

    m_menu_bar.save();

    ptree().put("simulation.paused", m_glwindow.call_now(&simulator::paused));
    ptree().put("simulation.duration_of_second", m_glwindow.call_now(&simulator::desired_network_to_real_time_ratio));

    boost::property_tree::write_info(m_ptree_pathname.string(), ptree());
}

void  program_window::on_tab_changed(int const  tab_index)
{
    std::string const  tab_name = qtgl::to_string(m_tabs->tabText(tab_index));
    if (tab_name == tab_names::DRAW())
    {
        // Nothing to do...
    }
    else if (tab_name == tab_names::NETWORK())
    {
        on_network_info_text_update();
    }
    else if (tab_name == tab_names::SELECTED())
    {
        on_selection_update();
    }
}
