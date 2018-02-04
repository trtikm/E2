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

    , m_menu_edit(new QMenu("&Edit", wnd))
    , m_action_edit_insert_coord_system(new QAction(QString("&Insert &coord system"), wnd))
    , m_action_edit_insert_batch(new QAction(QString("Insert &batch"), wnd))
    , m_action_edit_erase_selected(new QAction(QString("&Erase selected"), wnd))
    , m_action_edit_mode_select(new QAction(QString("&Selection"), wnd))
    , m_action_edit_mode_translate(new QAction(QString("&Translation"), wnd))
    , m_action_edit_mode_rotate(new QAction(QString("&Rotation"), wnd))
    , m_action_edit_undo(new QAction(QString("&Undo"), wnd))
    , m_action_edit_redo(new QAction(QString("Red&o"), wnd))

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

    w.get_menu_file()->addSeparator();

    w.get_menu_file()->addAction(w.get_action_open_scene());
    w.get_action_open_scene()->setShortcuts(QKeySequence::Open);
    QObject::connect(w.get_action_open_scene(), &QAction::triggered, w.wnd(),&program_window::on_menu_open_scene);

    w.get_menu_file()->addMenu(w.get_menu_open_recent_scene());

    w.get_menu_file()->addSeparator();

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

    // "Edit" menu

    w.get_menu_edit()->addAction(w.get_action_edit_insert_coord_system());
    w.get_action_edit_insert_coord_system()->setWhatsThis("TODO");
    QObject::connect(w.get_action_edit_insert_coord_system(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_insert_coord_system);

    w.get_menu_edit()->addAction(w.get_action_edit_insert_batch());
    w.get_action_edit_insert_batch()->setStatusTip("TODO");
    QObject::connect(w.get_action_edit_insert_batch(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_insert_batch);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_action_edit_erase_selected());
    w.get_action_edit_erase_selected()->setToolTip("TODO");
    QObject::connect(w.get_action_edit_erase_selected(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_erase_selected);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_action_edit_mode_select());
    w.get_action_edit_mode_select()->setShortcut(Qt::Key::Key_C);
    w.get_action_edit_mode_select()->setToolTip("TODO");
    QObject::connect(w.get_action_edit_mode_select(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_selection);

    w.get_menu_edit()->addAction(w.get_action_edit_mode_translate());
    w.get_action_edit_mode_translate()->setShortcut(Qt::Key::Key_T);
    w.get_action_edit_mode_translate()->setToolTip("TODO");
    QObject::connect(w.get_action_edit_mode_translate(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_translation);

    w.get_menu_edit()->addAction(w.get_action_edit_mode_rotate());
    w.get_action_edit_mode_rotate()->setShortcut(Qt::Key::Key_R);
    w.get_action_edit_mode_rotate()->setToolTip("TODO");
    QObject::connect(w.get_action_edit_mode_rotate(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_rotation);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_action_edit_undo());
    w.get_action_edit_undo()->setShortcut(QKeySequence::Undo);
    w.get_action_edit_undo()->setToolTip("TODO");
    QObject::connect(w.get_action_edit_undo(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_undo);

    w.get_menu_edit()->addAction(w.get_action_edit_redo());
    w.get_action_edit_redo()->setShortcut(QKeySequence::Redo);
    w.get_action_edit_redo()->setToolTip("TODO");
    QObject::connect(w.get_action_edit_redo(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_redo);

    w.get_menu_bar()->addMenu(w.get_menu_edit());

    // And finally attach the menu to the program window.

    w.wnd()->setMenuBar(w.get_menu_bar());
}
