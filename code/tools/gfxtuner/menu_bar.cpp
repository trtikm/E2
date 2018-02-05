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
#include <QToolTip>
#include <sstream>

namespace { namespace detail {


char const*  get_none_tooltip_string() { return "<<NONE>>"; }


struct  menu : public QMenu
{
public:
    explicit menu(QWidget* const  parent = Q_NULLPTR) : QMenu(parent) {}
    explicit menu(QString const&  title, QWidget* const  parent = Q_NULLPTR) : QMenu(title, parent) {}

    bool event(QEvent* const  e)
    {
        QHelpEvent const* help_event = static_cast<QHelpEvent*>(e);
        if (help_event->type() == QEvent::ToolTip &&
            activeAction() != 0 &&
            activeAction()->toolTip() != detail::get_none_tooltip_string())
            QToolTip::showText(help_event->globalPos(), activeAction()->toolTip());
        else
            QToolTip::hideText();
        return QMenu::event(e);
    }
};


}}


menu_bar::menu_bar(program_window* const  wnd)
    : m_wnd(wnd)
    , m_menu_bar(new QMenuBar(wnd))

    , m_menu_file(new detail::menu("&File",wnd))
    , m_action_new_scene(new QAction(QString("&New scene"), wnd))
    , m_action_open_scene(new QAction(QString("&Open scene"), wnd))
    , m_menu_open_recent_scene(new QMenu(QString("Open &recent scene"), wnd))
    , m_action_save_scene(new QAction(QString("&Save scene"), wnd))
    , m_action_save_as_scene(new QAction(QString("Save&As scene"), wnd))
    , m_action_exit(new QAction(QString("E&xit"), wnd))
    , m_default_scene_root_dir(boost::filesystem::absolute(get_program_options()->dataRoot()) / get_program_name())
    , m_recent_scenes()
    , m_current_scene_dir()

    , m_menu_edit(new detail::menu("&Edit", wnd))
    , m_action_edit_insert_coord_system(new QAction(QString("Insert &coord system"), wnd))
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
    w.get_action_new_scene()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_action_new_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_new_scene);

    w.get_menu_file()->addSeparator();

    w.get_menu_file()->addAction(w.get_action_open_scene());
    w.get_action_open_scene()->setShortcuts(QKeySequence::Open);
    w.get_action_open_scene()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_action_open_scene(), &QAction::triggered, w.wnd(),&program_window::on_menu_open_scene);

    w.get_menu_file()->addMenu(w.get_menu_open_recent_scene());

    w.get_menu_file()->addSeparator();

    w.get_menu_file()->addAction(w.get_action_save_scene());
    w.get_action_save_scene()->setShortcuts(QKeySequence::Save);
    w.get_action_save_scene()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_action_save_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_save_scene);

    w.get_menu_file()->addAction(w.get_action_save_as_scene());
    w.get_action_save_as_scene()->setShortcuts(QKeySequence::SaveAs);
    w.get_action_save_as_scene()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_action_save_as_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_save_as_scene);

    w.get_menu_file()->addSeparator();

    w.get_menu_file()->addAction(w.get_action_exit());
    w.get_action_exit()->setShortcuts(QKeySequence::Quit);
    w.get_action_exit()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_action_exit(), &QAction::triggered, w.wnd(), &program_window::on_menu_exit);

    w.get_menu_bar()->addMenu(w.get_menu_file());

    // "Edit" menu

    w.get_menu_edit()->addAction(w.get_action_edit_insert_coord_system());
    w.get_action_edit_insert_coord_system()->setToolTip(
        "A coordinate system represent a model space for graphical batches. The new coord. system is placed either to '@pivot' coord.\n"
        "system, if '@pivot' is selected, or to another selected coord. sytem, if any is selected, or to the 'world' system, if no\n"
        "coord. system is selected. Note that besides '@pivot' at most one coord. system can be selected. Otherwise the operation will\n"
        "fail (with an error message to the status bar). When some non-'@pivot' coord. system is selected, then it will be a parent\n"
        "system for the newly created one. Otherwise, the new coord. system will have no parent. Finally, the newly created system\n"
        "will become the only selected object in the scene."
        );
    QObject::connect(w.get_action_edit_insert_coord_system(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_insert_coord_system);

    w.get_menu_edit()->addAction(w.get_action_edit_insert_batch());
    w.get_action_edit_insert_batch()->setToolTip(
        "A batch is an atomic block of grahical data. It can only be placed under a coordinate system. It means that at least one\n"
        "non-'@pivot' coord. system must be selected. Otherwise the operation will fail (with an error message to the status bar).\n"
        "For each selected non-'@pivot' coord. system there will be created a copy of the newly created batch under that system.\n"
        "Batches are available on the disc under models root directory 'E2/dist/data/shared/gfx/models'. Finally, the newly created\n"
        "batch will become the only selected object in the scene."
        );
    QObject::connect(w.get_action_edit_insert_batch(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_insert_batch);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_action_edit_erase_selected());
    w.get_action_edit_erase_selected()->setToolTip(
        "Erases all selected objects from the scene. If a selected object has children, then they are all erased as well.\n"
        "The '@pivot' coord. system cannot be erased. It presence in the selection will lead to failure of the operation."
        );
    QObject::connect(w.get_action_edit_erase_selected(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_erase_selected);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_action_edit_mode_select());
    w.get_action_edit_mode_select()->setShortcut(Qt::Key::Key_C);
    w.get_action_edit_mode_select()->setToolTip(
        "Allows you to select or deselect scene objects (i.e. coord. systems and batches) using mouse buttons. By pressing left mouse\n"
        "button all scene objects are deselected and then a scene object under the mouse is selected, if there is any. By pressing\n"
        "right mouse button the current selection either unchanged, if no scene object is under mouse, or it is updated so that the\n"
        "secne object under the mouse is added, if it was not in the selection, and it is removed from the selection, if it already\n"
        "was in the selection."
        );
    QObject::connect(w.get_action_edit_mode_select(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_selection);

    w.get_menu_edit()->addAction(w.get_action_edit_mode_translate());
    w.get_action_edit_mode_translate()->setShortcut(Qt::Key::Key_T);
    w.get_action_edit_mode_translate()->setToolTip(
        "Allows you to translate selected scene objects using the mouse. The operation is done in the reference coord. system. It is\n"
        "either the 'world' coord. system (default), or '@pivot' coord. system (if selected). Keep left mouse button pressed and move\n"
        "the mouse to perfotm the translation. Holding also any pair of keys X, Y, or Z pressed allows you to specify along what\n"
        "axes/planes the operation will be performed. When none of the keys is pressed then X and Y are active by default."
        );
    QObject::connect(w.get_action_edit_mode_translate(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_translation);

    w.get_menu_edit()->addAction(w.get_action_edit_mode_rotate());
    w.get_action_edit_mode_rotate()->setShortcut(Qt::Key::Key_R);
    w.get_action_edit_mode_rotate()->setToolTip(
        "Allows you to rotate selected scene objects using the mouse. The operation is done in the reference coord. system. It is either\n"
        "the 'world' coord. system (default), or '@pivot' coord. system (if selected). Keep left mouse button pressed and move the mouse\n"
        "to perfotm the rotation. Holding also a key X, Y, or Z pressed allows you to specify along what axis the operation will be\n"
        "performed. When none of the keys is pressed then Z axis is active by default. The right mouse button pressed instead of the left\n"
        "one performs an alternative rotation mode. It applies only when multiple coord. systems are selected. All objects are then\n"
        "rotated around their common geometrical center (and along an axis of your choice, as described above)."
        );
    QObject::connect(w.get_action_edit_mode_rotate(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_rotation);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_action_edit_undo());
    w.get_action_edit_undo()->setShortcut(QKeySequence::Undo);
    w.get_action_edit_undo()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_action_edit_undo(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_undo);

    w.get_menu_edit()->addAction(w.get_action_edit_redo());
    w.get_action_edit_redo()->setShortcut(QKeySequence::Redo);
    w.get_action_edit_redo()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_action_edit_redo(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_redo);

    w.get_menu_bar()->addMenu(w.get_menu_edit());

    // And finally attach the menu to the program window.

    w.wnd()->setMenuBar(w.get_menu_bar());
}
