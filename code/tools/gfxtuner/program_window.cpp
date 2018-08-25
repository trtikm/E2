#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_info.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/simulator.hpp>
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <QString>
#include <QIcon>
#include <sstream>


namespace { namespace tab_names {


inline std::string  DRAW() { return "Draw"; }
inline std::string  SCENE() { return "Scene"; }
inline std::string  STATISTICS() { return "Statistics"; }


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
    , m_glwindow()
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
    , m_tab_scene_widgets(this)
    , m_tab_statistics_widgets(new window_tabs::tab_statistics::widgets(this))

    , m_menu_bar(this)
    , m_status_bar(this)
{
    set_title();
    this->setWindowIcon(QIcon((get_program_options()->dataRoot() + "/shared/gfx/icons/E2_icon.png").c_str()));
    this->move({ ptree().get("window.pos.x",0),ptree().get("window.pos.y",0) });
    this->resize(ptree().get("window.width", 1024), ptree().get("window.height", 768));

    make_menu_bar_content(m_menu_bar);

    this->setCentralWidget(m_splitter);
    m_splitter->addWidget(m_gl_window_widget);
    m_splitter->addWidget(m_tabs);

    m_tabs->addTab( window_tabs::tab_draw::make_draw_tab_content(m_tab_draw_widgets),
                    QString(tab_names::DRAW().c_str()) );
    m_tabs->addTab( window_tabs::tab_scene::make_scene_tab_content(m_tab_scene_widgets),
                    QString(tab_names::SCENE().c_str()));
    m_tabs->addTab( m_tab_statistics_widgets,
                    QString(tab_names::STATISTICS().c_str()));

    for (int i = 0; i != m_tabs->count(); ++i)
        if (qtgl::to_string(m_tabs->tabText(i)) == ptree().get("window.active_tab", tab_names::SCENE()))
        {
            m_tabs->setCurrentIndex(i);
            break;
        }

    make_status_bar_content(m_status_bar);

    this->show();

    qtgl::set_splitter_sizes(*m_splitter, ptree().get("window.splitter_ratio", 3.0f / 4.0f));

    this->setFocus();

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

        // Below put calls to methods 'on_simulator_started' of those widgets 
        // requiring synchronisation with the simulator to complete their
        // initialisation.

        m_tab_scene_widgets.on_simulator_started();
    }

    // Here put time-dependent updates...

    m_status_bar.update();

    std::string const  current_tab = qtgl::to_string(m_tabs->tabText(m_tabs->currentIndex()));
    if (current_tab == tab_names::DRAW())
    {
        // Nothing to do...
    }
    else if (current_tab == tab_names::SCENE())
    {
        // Nothing to do...
    }
    else if (current_tab == tab_names::STATISTICS())
    {
        m_tab_statistics_widgets->on_timer_event();
    }

    if (m_focus_just_received)
    {
        m_focus_just_received = false;
        m_gl_window_widget->activateWindow();
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
    ptree().put("window.active_tab", qtgl::to_string(m_tabs->tabText(m_tabs->currentIndex())));

    m_tab_draw_widgets.save();

    m_menu_bar.save();

    ptree().put("simulation.paused", m_glwindow.call_now(&simulator::paused));

    boost::property_tree::write_info(m_ptree_pathname.string(), ptree());
}

void  program_window::on_tab_changed(int const  tab_index)
{
    std::string const  tab_name = qtgl::to_string(m_tabs->tabText(tab_index));
    if (tab_name == tab_names::DRAW())
    {
        // Nothing to do...
    }
    else if (tab_name == tab_names::SCENE())
    {
        // Nothing to do...
    }
    else if (tab_name == tab_names::STATISTICS())
    {
        // Nothing to do...
    }
}

void  program_window::set_title(std::string const&  text)
{
    std::stringstream  sstr;
    sstr << get_program_name();
    if (!text.empty())
        sstr << " [" << text << "]";
    this->setWindowTitle(sstr.str().c_str());
}
