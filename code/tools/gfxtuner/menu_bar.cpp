#include <gfxtuner/menu_bar.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/program_info.hpp>
#include <gfxtuner/simulation/simulator.hpp>
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

namespace detail {


static char const*  get_none_tooltip_string() { return "<<NONE>>"; }


struct  menu : public QMenu
{
public:
    explicit menu(QWidget* const  parent = Q_NULLPTR) : QMenu(parent) {}
    explicit menu(QString const&  title, QWidget* const  parent = Q_NULLPTR) : QMenu(title, parent) {}

    bool event(QEvent* const  e)
    {
        QHelpEvent const* help_event = static_cast<QHelpEvent*>(e);
        if (help_event->type() == QEvent::ToolTip &&
            activeAction() != nullptr &&
            activeAction()->toolTip() != detail::get_none_tooltip_string())
            QToolTip::showText(help_event->globalPos(), activeAction()->toolTip());
        else
            QToolTip::hideText();
        return QMenu::event(e);
    }
};


}


menu_bar::menu_bar(program_window* const  wnd)
    : m_wnd(wnd)
    , m_menu_bar(new QMenuBar(wnd))

    , m_menu_file(new detail::menu("&File",wnd))
    , m_file_action_new_scene(new QAction(QString("&New scene"), wnd))
    , m_file_action_open_scene(new QAction(QString("&Open scene"), wnd))
    , m_file_submenu_open_recent_scene(new QMenu(QString("Open &recent scene"), wnd))
    , m_file_action_save_scene(new QAction(QString("&Save scene"), wnd))
    , m_file_action_save_as_scene(new QAction(QString("Save&As scene"), wnd))
    , m_file_action_exit(new QAction(QString("E&xit"), wnd))
    , m_default_scene_root_dir(boost::filesystem::absolute(get_program_options()->dataRoot()) / get_program_name())
    , m_recent_scenes()
    , m_current_scene_dir()

    , m_menu_edit(new detail::menu("&Edit", wnd))
    , m_edit_action_insert_coord_system(new QAction(QString("Insert &coord system"), wnd))
    , m_edit_action_insert_batch(new QAction(QString("Insert &batch"), wnd))
    , m_edit_action_erase_selected(new QAction(QString("&Erase selected"), wnd))
    , m_edit_action_mode_select(new QAction(QString("&Selection"), wnd))
    , m_edit_action_mode_translate(new QAction(QString("&Translation"), wnd))
    , m_edit_action_mode_rotate(new QAction(QString("&Rotation"), wnd))
    , m_edit_action_toggle_pivot_selection(new QAction(QString("To&ggle pivot selection"), wnd))
    , m_edit_action_move_selection_to_pivot(new QAction(QString("Move se&lection to pivot"), wnd))
    , m_edit_action_move_pivot_to_selection(new QAction(QString("Move &pivot to selection"), wnd))
    , m_edit_action_undo(new QAction(QString("&Undo"), wnd))
    , m_edit_action_redo(new QAction(QString("Red&o"), wnd))

    , m_menu_view(new detail::menu("&View", wnd))
    , m_view_action_double_camera_speed(new QAction(QString("&Double camera speed"), wnd))
    , m_view_action_half_camera_speed(new QAction(QString("&Half camera speed"), wnd))
    , m_view_action_look_at_selection(new QAction(QString("&Look at selection"), wnd))
{
    if(!boost::filesystem::is_directory(m_default_scene_root_dir))
        boost::filesystem::create_directories(m_default_scene_root_dir);
    m_default_scene_root_dir = canonical_path(m_default_scene_root_dir);
}

void  menu_bar::on_file_action_new_scene()
{
    m_current_scene_dir.clear();
}

bool  menu_bar::on_file_action_open_scene()
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

void  menu_bar::on_file_action_open_recent_scene()
{

}

bool  menu_bar::on_file_action_save_scene()
{
    if (get_current_scene_dir().empty())
        return on_file_action_save_as_scene();
    return true;
}

bool  menu_bar::on_file_action_save_as_scene()
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

bool  menu_bar::on_file_action_exit()
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

    w.get_menu_file()->addAction(w.get_file_action_new_scene());
    w.get_file_action_new_scene()->setShortcuts(QKeySequence::New);
    w.get_file_action_new_scene()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_file_action_new_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_new_scene);

    w.get_menu_file()->addSeparator();

    w.get_menu_file()->addAction(w.get_file_action_open_scene());
    w.get_file_action_open_scene()->setShortcuts(QKeySequence::Open);
    w.get_file_action_open_scene()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_file_action_open_scene(), &QAction::triggered, w.wnd(),&program_window::on_menu_open_scene);

    w.get_menu_file()->addMenu(w.get_file_submenu_open_recent_scene());

    w.get_menu_file()->addSeparator();

    w.get_menu_file()->addAction(w.get_file_action_save_scene());
    w.get_file_action_save_scene()->setShortcuts(QKeySequence::Save);
    w.get_file_action_save_scene()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_file_action_save_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_save_scene);

    w.get_menu_file()->addAction(w.get_file_action_save_as_scene());
    w.get_file_action_save_as_scene()->setShortcuts(QKeySequence::SaveAs);
    w.get_file_action_save_as_scene()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_file_action_save_as_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_save_as_scene);

    w.get_menu_file()->addSeparator();

    w.get_menu_file()->addAction(w.get_file_action_exit());
    w.get_file_action_exit()->setShortcuts(QKeySequence::Quit);
    w.get_file_action_exit()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_file_action_exit(), &QAction::triggered, w.wnd(), &program_window::on_menu_exit);

    w.get_menu_bar()->addMenu(w.get_menu_file());

    // "Edit" menu

    w.get_menu_edit()->addAction(w.get_edit_action_insert_coord_system());
    w.get_edit_action_insert_coord_system()->setShortcut(QString("Ctrl+Alt+I"));
    w.get_edit_action_insert_coord_system()->setToolTip(
        "A coordinate system represent a model space for graphical batches. The new coord. system is placed either to '@pivot' coord.\n"
        "system, if '@pivot' is selected, or to another selected coord. sytem, if any is selected, or to the 'world' system, if no\n"
        "coord. system is selected. Note that besides '@pivot' at most one coord. system can be selected. Otherwise the operation will\n"
        "fail (with an error message to the status bar). When some non-'@pivot' coord. system is selected, then it will be a parent\n"
        "system for the newly created one. Otherwise, the new coord. system will have no parent. Finally, the newly created system\n"
        "will become the only selected object in the scene."
        );
    QObject::connect(w.get_edit_action_insert_coord_system(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_insert_coord_system);

    w.get_menu_edit()->addAction(w.get_edit_action_insert_batch());
    w.get_edit_action_insert_batch()->setShortcut(QString("Ctrl+Alt+B"));
    w.get_edit_action_insert_batch()->setToolTip(
        "A batch is an atomic block of grahical data. It can only be placed under a coordinate system. It means that at least one\n"
        "non-'@pivot' coord. system must be selected. Otherwise the operation will fail (with an error message to the status bar).\n"
        "For each selected non-'@pivot' coord. system there will be created a copy of the newly created batch under that system.\n"
        "Batches are available on the disc under models root directory 'E2/dist/data/shared/gfx/models'. Finally, the newly created\n"
        "batch will become the only selected object in the scene."
        );
    QObject::connect(w.get_edit_action_insert_batch(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_insert_batch);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_edit_action_erase_selected());
    w.get_edit_action_erase_selected()->setShortcut(QKeySequence::Delete);
    w.get_edit_action_erase_selected()->setToolTip(
        "Erases all selected objects from the scene. If a selected object has children, then they are all erased as well.\n"
        "The '@pivot' coord. system cannot be erased. It presence in the selection will lead to failure of the operation."
        );
    QObject::connect(w.get_edit_action_erase_selected(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_erase_selected);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_edit_action_mode_select());
    w.get_edit_action_mode_select()->setShortcut(Qt::Key::Key_C);
    w.get_edit_action_mode_select()->setToolTip(
        "Allows you to select or deselect scene objects (i.e. coord. systems and batches) using mouse buttons. By pressing left mouse\n"
        "button all scene objects are deselected and then a scene object under the mouse is selected, if there is any. By pressing\n"
        "right mouse button the current selection either unchanged, if no scene object is under mouse, or it is updated so that the\n"
        "secne object under the mouse is added, if it was not in the selection, and it is removed from the selection, if it already\n"
        "was in the selection."
        );
    QObject::connect(w.get_edit_action_mode_select(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_selection);

    w.get_menu_edit()->addAction(w.get_edit_action_mode_translate());
    w.get_edit_action_mode_translate()->setShortcut(Qt::Key::Key_T);
    w.get_edit_action_mode_translate()->setToolTip(
        "Allows you to translate selected scene objects using the mouse. The operation is done in the reference coord. system. It is\n"
        "either the 'world' coord. system (default), or '@pivot' coord. system (if selected). Keep left mouse button pressed and move\n"
        "the mouse to perfotm the translation. Holding also any pair of keys X, Y, or Z pressed allows you to specify along what\n"
        "axes/planes the operation will be performed. When none of the keys is pressed then X and Y are active by default."
        );
    QObject::connect(w.get_edit_action_mode_translate(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_translation);

    w.get_menu_edit()->addAction(w.get_edit_action_mode_rotate());
    w.get_edit_action_mode_rotate()->setShortcut(Qt::Key::Key_R);
    w.get_edit_action_mode_rotate()->setToolTip(
        "Allows you to rotate selected scene objects using the mouse. The operation is done in the reference coord. system. It is either\n"
        "the 'world' coord. system (default), or '@pivot' coord. system (if selected). Keep left mouse button pressed and move the mouse\n"
        "to perfotm the rotation. Holding also a key X, Y, or Z pressed allows you to specify along what axis the operation will be\n"
        "performed. When none of the keys is pressed then Z axis is active by default. The right mouse button pressed instead of the left\n"
        "one performs an alternative rotation mode. It applies only when multiple coord. systems are selected. All objects are then\n"
        "rotated around their common geometrical center (and along an axis of your choice, as described above)."
        );
    QObject::connect(w.get_edit_action_mode_rotate(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_rotation);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_edit_action_toggle_pivot_selection());
    w.get_edit_action_toggle_pivot_selection()->setShortcut(QString(Qt::Key::Key_P));
    w.get_edit_action_toggle_pivot_selection()->setToolTip(
        "Toggles the presence of the '@pivot' coordinate system in the list of selected scene objects."
        );
    QObject::connect(w.get_edit_action_toggle_pivot_selection(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_toggle_pivot_selection);

    w.get_menu_edit()->addAction(w.get_edit_action_move_selection_to_pivot());
    w.get_edit_action_move_selection_to_pivot()->setShortcut(QString("Ctrl+P"));
    w.get_edit_action_move_selection_to_pivot()->setToolTip(
        "If the edit mode is 'Translation' and there is selected at least one non-'@pivot' coordinate system (including those of\n"
        "selected batches), then the operation translates the common centre of all selected non-'@pivot' coordinate systems to the\n"
        "origin of the '@pivot' coord. system. If the edit mode is 'Rotation' and there is selected at least one non-'@pivot'\n"
        "coordinate system (including those of selected batches), then the operation rotates axis vectors of all selected non-'@pivot'\n"
        "coord. systems so that they all equals to the corresponding axis vectors of the '@pivot' coord. system. It does not matter\n"
        "whether '@pivot' is selected or not."
        );
    QObject::connect(w.get_edit_action_move_selection_to_pivot(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_move_selection_to_pivot);

    w.get_menu_edit()->addAction(w.get_edit_action_move_pivot_to_selection());
    w.get_edit_action_move_pivot_to_selection()->setShortcut(QString("Ctrl+Shift+P"));
    w.get_edit_action_move_pivot_to_selection()->setToolTip(
        "If the edit mode is 'Translation' and there is selected at least one non-'@pivot' coordinate system (including those of\n"
        "selected batches), then the operation translates the '@pivot' coord. system to the common centre of all selected non-'@pivot'\n"
        "coordinate systems. If the edit mode is 'Rotation' and there is selected exactly one non-'@pivot' coordinate system (including\n"
        "those of selected batches), then the operation rotates the '@pivot' coord. system so that its axis vectors equals to the\n"
        "corresponding axis vectors of the selected non-'@pivot' coord. system. It does not matter, whether '@pivot' is selected or not."
        );
    QObject::connect(w.get_edit_action_move_pivot_to_selection(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_move_pivot_to_selection);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_edit_action_undo());
    w.get_edit_action_undo()->setShortcut(QKeySequence::Undo);
    w.get_edit_action_undo()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_edit_action_undo(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_undo);

    w.get_menu_edit()->addAction(w.get_edit_action_redo());
    w.get_edit_action_redo()->setShortcut(QKeySequence::Redo);
    w.get_edit_action_redo()->setToolTip(detail::get_none_tooltip_string());
    QObject::connect(w.get_edit_action_redo(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_redo);

    w.get_menu_bar()->addMenu(w.get_menu_edit());

    // "Edit" menu

    w.get_menu_view()->addAction(w.get_view_action_double_camera_speed());
    w.get_view_action_double_camera_speed()->setShortcut(QString("Ctrl++"));
    w.get_view_action_double_camera_speed()->setToolTip(
        "Makes the camera movement two times faster. You can also set exact camera speed in the tab 'Draw'."
        );
    QObject::connect(w.get_view_action_double_camera_speed(), &QAction::triggered, w.wnd(), &program_window::on_menu_view_double_camera_speed);

    w.get_menu_view()->addAction(w.get_view_action_half_camera_speed());
    w.get_view_action_half_camera_speed()->setShortcut(QString("Ctrl+-"));
    w.get_view_action_half_camera_speed()->setToolTip(
        "Makes the camera movement two times slower. You can also set exact camera speed in the tab 'Draw'."
        );
    QObject::connect(w.get_view_action_half_camera_speed(), &QAction::triggered, w.wnd(), &program_window::on_menu_view_half_camera_speed);

    w.get_menu_view()->addSeparator();

    w.get_menu_view()->addAction(w.get_view_action_look_at_selection());
    w.get_view_action_look_at_selection()->setShortcut(QString("Ctrl+L"));
    w.get_view_action_look_at_selection()->setToolTip(
        "In edit mode 'Translation' the operation rotates and moves the camera so that selected scene coordinate\n"
        "systems and batches are visible. In edit mode 'Rotation' the operation rotates the camera so that it\n"
        "looks at the selected scene coordinate systems and batches (but it stays at the current position)."
        );
    QObject::connect(w.get_view_action_look_at_selection(), &QAction::triggered, w.wnd(), &program_window::on_menu_view_look_at_selection);

    w.get_menu_bar()->addMenu(w.get_menu_view());

    // And finally attach the menu to the program window.

    w.wnd()->setMenuBar(w.get_menu_bar());
}
