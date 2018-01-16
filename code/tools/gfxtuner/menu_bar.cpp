#include <gfxtuner/menu_bar.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/program_info.hpp>
#include <gfxtuner/simulator.hpp>
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <utility/canonical_path.hpp>
#include <boost/property_tree/ptree.hpp>
#include <QString>
#include <QFileDialog>
#include <sstream>


menu_bar::menu_bar(program_window* const  wnd)
    : m_wnd(wnd)
    , m_menu_bar(new QMenuBar(wnd))

    , m_menu_file(new QMenu("&File",wnd))
    , m_action_new_scene(new QAction(QString("&New scene"), wnd))
    , m_action_open_scene(new QAction(QString("&Open scene"), wnd))
    , m_menu_open_recent_scene(new QMenu(QString("Open &recent scene"), wnd))
    , m_action_save_scene(new QAction(QString("&Save scene"), wnd))
    , m_action_save_as_scene(new QAction(QString("Save&As scene"), wnd))
    , m_action_exit(new QAction(QString("E&xit"), wnd))
    , m_default_scene_root_dir(boost::filesystem::absolute(get_program_options()->dataRoot()) / get_program_name())
    , m_recent_scenes()
    , m_current_scene_dir()
{
    if(!boost::filesystem::is_directory(m_default_scene_root_dir))
        boost::filesystem::create_directories(m_default_scene_root_dir);
    m_default_scene_root_dir = canonical_path(m_default_scene_root_dir);
}

void  menu_bar::on_new_scene()
{
    m_current_scene_dir.clear();
}

bool  menu_bar::on_open_scene()
{
    QFileDialog  dialog(wnd());
    dialog.setDirectory(get_default_scene_root_dir().string().c_str());
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (!dialog.exec())
        return false;
    QStringList const  selected = dialog.selectedFiles();
    if (selected.size() != 1)
    {
        wnd()->print_status_message("ERROR: A single load directory must be selected.", 10000);
        return false;
    }
    get_current_scene_dir() = qtgl::to_string(selected.at(0));
    return true;
}

void  menu_bar::on_open_recent_scene()
{

}

bool  menu_bar::on_save_scene()
{
    if (get_current_scene_dir().empty())
        return on_save_as_scene();
    return true;
}

bool  menu_bar::on_save_as_scene()
{
    boost::filesystem::path  output_dir =
        get_current_scene_dir().empty() ? get_default_scene_root_dir() : get_current_scene_dir();
    QFileDialog  dialog(wnd());
    dialog.setDirectory(output_dir.string().c_str());
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (!dialog.exec())
        return false;
    QStringList const  selected = dialog.selectedFiles();
    if (selected.size() != 1)
    {
        wnd()->print_status_message("ERROR: A single output directory must be selected.", 10000);
        return false;
    }
    get_current_scene_dir() = qtgl::to_string(selected.at(0));
    return true;
}

bool  menu_bar::on_exit()
{
    return true;
}


void  menu_bar::save()
{
    //wnd()->ptree().put("...",...);
}


void  make_menu_bar_content(menu_bar const&  w)
{
    // "File" menu

    w.get_menu_file()->addAction(w.get_action_new_scene());
    w.get_action_new_scene()->setShortcuts(QKeySequence::New);
    QObject::connect(w.get_action_new_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_new_scene);

    w.get_menu_file()->addAction(w.get_action_open_scene());
    w.get_action_open_scene()->setShortcuts(QKeySequence::Open);
    QObject::connect(w.get_action_open_scene(), &QAction::triggered, w.wnd(),&program_window::on_menu_open_scene);

    w.get_menu_file()->addMenu(w.get_menu_open_recent_scene());

    w.get_menu_file()->addAction(w.get_action_save_scene());
    w.get_action_save_scene()->setShortcuts(QKeySequence::Save);
    QObject::connect(w.get_action_save_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_save_scene);

    w.get_menu_file()->addAction(w.get_action_save_as_scene());
    w.get_action_save_as_scene()->setShortcuts(QKeySequence::SaveAs);
    QObject::connect(w.get_action_save_as_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_save_as_scene);

    w.get_menu_file()->addSeparator();

    w.get_menu_file()->addAction(w.get_action_exit());
    w.get_action_exit()->setShortcuts(QKeySequence::Quit);
    QObject::connect(w.get_action_exit(), &QAction::triggered, w.wnd(), &program_window::on_menu_exit);

    w.get_menu_bar()->addMenu(w.get_menu_file());

    // And finally attach the menu to the program window.

    w.wnd()->setMenuBar(w.get_menu_bar());
}
