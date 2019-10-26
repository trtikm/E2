#include <gfxtuner/menu_bar.hpp>
#include <gfxtuner/menu_bar_records_integration.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/program_info.hpp>
#include <gfxtuner/simulation/simulator.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
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
    , m_file_submenu_open_recent_scene(new QMenu(QString("Open r&ecent scene"), wnd))
    , m_file_action_reload_scene(new QAction(QString("Fast &reload scene"), wnd))
    , m_file_action_import_scene(new QAction(QString("I&mport scene"), wnd))
    , m_file_action_save_scene(new QAction(QString("&Save scene"), wnd))
    , m_file_action_save_as_scene(new QAction(QString("Save&As scene"), wnd))
    , m_file_action_exit(new QAction(QString("E&xit"), wnd))
    , m_default_scene_root_dir(boost::filesystem::absolute(get_program_options()->dataRoot()) / get_program_name())
    , m_recent_scenes()
    , m_current_scene_dir()

    , m_menu_edit(new detail::menu("&Edit", wnd))
    , m_edit_action_insert_coord_system(new QAction(QString("Insert &coord system"), wnd))
    , m_record_menu_items()
    , m_edit_action_erase_selected(new QAction(QString("&Erase selected"), wnd))
    , m_edit_action_duplicate_selected(new QAction(QString("&Duplicate selected"), wnd))
    , m_edit_action_change_parent_of_selected(new QAction(QString("C&hange parent of selected"), wnd))
    , m_edit_action_rename_scene_object(new QAction(QString("Rena&me selected"), wnd))
    , m_edit_action_mode_select(new QAction(QString("&Selection"), wnd))
    , m_edit_action_mode_translate(new QAction(QString("&Translation"), wnd))
    , m_edit_action_mode_rotate(new QAction(QString("&Rotation"), wnd))
    , m_edit_action_toggle_pivot_selection(new QAction(QString("To&ggle pivot selection"), wnd))
    , m_edit_action_move_selection_to_pivot(new QAction(QString("Move se&lection to pivot"), wnd))
    , m_edit_action_move_pivot_to_selection(new QAction(QString("Move &pivot to selection"), wnd))
    , m_edit_action_agent_reset_skeleton_pose(new QAction(QString("Reset s&keleton pose"), wnd))
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

    std::multimap<std::string, std::pair<std::string, record_menu_items::record_menu_item_info> >  record_menu_items;
    record_menu_items::register_record_menu_items(record_menu_items);
    for (auto const& record_kind_and_info : record_menu_items)
    {
        QAction* const  action = new QAction(QString(record_kind_and_info.second.second.m_name.c_str()), wnd);
        action->setShortcut(QString(record_kind_and_info.second.second.m_shortcut.c_str()));
        action->setToolTip(QString(record_kind_and_info.second.second.m_tooltip.c_str()));
        m_record_menu_items.insert({record_kind_and_info.first, { action, record_kind_and_info.second.first } });
    }
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
    dialog.setWindowTitle("Open scene");
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

bool  menu_bar::on_file_action_reload_scene()
{
    if (m_current_scene_dir.empty())
    {
        wnd()->print_status_message("ERROR: Cannot reload scene which was not saved on the disk.", 10000);
        return false;
    }
    return true;
}

std::string  menu_bar::on_file_action_import_scene()
{
    QFileDialog  dialog(wnd());
    dialog.setDirectory(get_default_scene_root_dir().string().c_str());
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setWindowTitle("Import scene");
    if (!dialog.exec())
        return "";
    QStringList const  selected = dialog.selectedFiles();
    if (selected.size() != 1)
    {
        wnd()->print_status_message("ERROR: A single load directory must be selected.", 10000);
        return "";
    }
    return qtgl::to_string(selected.at(0));;
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
    dialog.setWindowTitle("Save as scene");
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

void  menu_bar::on_simulation_paused()
{
    toggle_enable_state_of_menu_items_for_simulation_mode(false);
}

void  menu_bar::on_simulation_resumed()
{
    toggle_enable_state_of_menu_items_for_simulation_mode(true);
}


void  menu_bar::toggle_enable_state_of_menu_items_for_simulation_mode(bool const  simulation_resumed)
{
    get_file_action_new_scene()->setDisabled(simulation_resumed);
    get_file_action_open_scene()->setDisabled(simulation_resumed);
    get_file_submenu_open_recent_scene()->setDisabled(simulation_resumed);
    get_file_action_reload_scene()->setDisabled(simulation_resumed);
    get_file_action_import_scene()->setDisabled(simulation_resumed);
    get_file_action_save_scene()->setDisabled(simulation_resumed);
    get_file_action_save_as_scene()->setDisabled(simulation_resumed);
    get_file_action_exit()->setDisabled(false);

    get_edit_action_insert_coord_system()->setDisabled(simulation_resumed);
    for (auto const& record_kind_and_action : get_edit_actions_of_records())
        record_kind_and_action.second.first->setDisabled(simulation_resumed);
    get_edit_action_erase_selected()->setDisabled(simulation_resumed);
    get_edit_action_duplicate_selected()->setDisabled(simulation_resumed);
    get_edit_action_change_parent_of_selected()->setDisabled(simulation_resumed);
    get_edit_action_rename_scene_object()->setDisabled(simulation_resumed);
    get_edit_action_mode_select()->setDisabled(simulation_resumed);
    get_edit_action_mode_translate()->setDisabled(simulation_resumed);
    get_edit_action_mode_rotate()->setDisabled(simulation_resumed);
    get_edit_action_toggle_pivot_selection()->setDisabled(simulation_resumed);
    get_edit_action_move_selection_to_pivot()->setDisabled(simulation_resumed);
    get_edit_action_move_pivot_to_selection()->setDisabled(simulation_resumed);
    get_edit_action_agent_reset_skeleton_pose()->setDisabled(simulation_resumed);
    get_edit_action_undo()->setDisabled(simulation_resumed);
    get_edit_action_redo()->setDisabled(simulation_resumed);

    get_view_action_look_at_selection()->setDisabled(simulation_resumed);
}


void  menu_bar::save()
{
    //wnd()->ptree().put("...",...);
}


void  make_menu_bar_content(menu_bar const&  w)
{
    w.wnd()->glwindow().register_listener(
                simulator_notifications::paused(),
                { &program_window::menu_listener_simulation_paused, w.wnd() }
                );
    w.wnd()->glwindow().register_listener(
                simulator_notifications::resumed(),
                { &program_window::menu_listener_simulation_resumed, w.wnd() }
                );

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

    w.get_menu_file()->addAction(w.get_file_action_reload_scene());
    w.get_file_action_reload_scene()->setShortcut(QString("Ctrl+R"));
    w.get_file_action_reload_scene()->setToolTip("Reloads the current scene, except for graphics data (batches, shaders, buffers, textures, etc.)");
    QObject::connect(w.get_file_action_reload_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_reload_scene);

    w.get_menu_file()->addAction(w.get_file_action_import_scene());
    w.get_file_action_import_scene()->setShortcut(QString("Ctrl+Shift+I"));
    w.get_file_action_import_scene()->setToolTip(
        "Imports a given scene into the current one. If exactly one regular coord. system node is selected, then the scene\n"
        "is imported under the node (i.e. root-level nodes of the imported scene become direct children of the selected node).\n"
        "Otherwise, all root-level nodes of the imported scene become root-level nodes in the final scene and their coordinate\n"
        "system will be located relative to the '@pivot' node of the original scene (i.e. they will be located as if '@pivot'\n"
        "was their parent node). If name of an imported node at root-level in the imported scene is already used in the current\n"
        "scene (at the place where it is imported to), then the imported node is renamed to a unique name by appending it a unique\n"
        "suffix. The selection in the current scene will be cleared and all imported root nodes will be selected in the final\n"
        "scene. The final scene will NOT keep the information about what scene was imported. The final scene will keep the disk\n"
        "pathname of the original scene. Non-regular nodes of the imported scene (i.e. those containing '@' in the name) are NOT\n"
        "imported (i.e. they are skipped)."
        );
    QObject::connect(w.get_file_action_import_scene(), &QAction::triggered, w.wnd(), &program_window::on_menu_import_scene);

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

    w.get_menu_edit()->addSeparator();

    std::string  last_kind = "";
    for (auto const&  record_kind_and_action : w.get_edit_actions_of_records())
    {
        if (!last_kind.empty() && last_kind != record_kind_and_action.first)
            w.get_menu_edit()->addSeparator();
        last_kind = record_kind_and_action.first;

        w.get_menu_edit()->addAction(record_kind_and_action.second.first);
        QObject::connect(
                record_kind_and_action.second.first,
                &QAction::triggered,
                w.wnd(),
                std::bind(
                    &program_window::on_menu_edit_insert_record,
                    w.wnd(),
                    record_kind_and_action.first,
                    record_kind_and_action.second.second
                    )
                );
    }

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_edit_action_erase_selected());
    w.get_edit_action_erase_selected()->setShortcut(QKeySequence::Delete);
    w.get_edit_action_erase_selected()->setToolTip(
        "Erases all selected objects from the scene. If a selected object has children, then they are all erased as well.\n"
        "The '@pivot' coord. system cannot be erased. Also no simulation node (starting with '@') nor any of its direct\n"
        "or indirect children can be erased."
        );
    QObject::connect(w.get_edit_action_erase_selected(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_erase_selected);

    w.get_menu_edit()->addAction(w.get_edit_action_duplicate_selected());
    w.get_edit_action_duplicate_selected()->setShortcut(QString("Ctrl+D"));
    w.get_edit_action_duplicate_selected()->setToolTip(
        "Duplicates a selected coordinate system nodes in the scene, including all its folders, records, and child\n"
        "nodes recursively (if any). The '@pivot' coordinate system cannot be duplicated. You will be asked how many\n"
        "copies of the selected node to create. The i-th copy will be placed at the position 'origin(selected_node) +\n"
        "i * (origin(@pivot) - origin(selected_node)). All copies will have the same orientation as '@pivot'.\n"
        "If the selected node has a parent, then all duplicate nodes will have the same parent. Otherwise, duplicate\n"
        "nodes will be without a parent too. If exactly one coordinate system node is selected for duplication, then\n"
        "you are also asked for the name of the duplicated node. In case of multiple nodes are selected, then each\n"
        "duplicated node has the name of the source node suffixed by a unique number."
        );
    QObject::connect(w.get_edit_action_duplicate_selected(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_duplicate_selected);

    w.get_menu_edit()->addAction(w.get_edit_action_change_parent_of_selected());
    w.get_edit_action_change_parent_of_selected()->setShortcut(QString("Ctrl+Shift+H"));
    w.get_edit_action_change_parent_of_selected()->setToolTip(
        "Changes parent node of each selected coord. system nodes in the scene by the coord. system node corresponding to\n"
        "current (active) item in the scene tree widget. The current tree item must NOT be selected (hint: hold 'Ctrl' key\n"
        "to toggle selection of the current tree item). When the current tree item corresponds to the '@pivot' scene node,\n"
        "then all selected nodes will have no parent (i.e. they will become root-level nodes in the scene)."
        );
    QObject::connect(w.get_edit_action_change_parent_of_selected(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_change_parent_of_selected);

    w.get_menu_edit()->addAction(w.get_edit_action_rename_scene_object());
    w.get_edit_action_rename_scene_object()->setShortcut(QString("Ctrl+Shift+R"));
    w.get_edit_action_rename_scene_object()->setToolTip(
        "TODO"
        );
    QObject::connect(w.get_edit_action_rename_scene_object(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_rename_scene_object);

    w.get_menu_edit()->addSeparator();

    w.get_menu_edit()->addAction(w.get_edit_action_mode_select());
    w.get_edit_action_mode_select()->setShortcut(Qt::Key::Key_C);
    w.get_edit_action_mode_select()->setToolTip(
        "Activates the 'SELECT' edit mode. The active edit mode is shown in the status bar (3rd widget from the right). This mode\n"
        "allows you to select or deselect scene objects (i.e. coord. systems and batches) using mouse buttons. By pressing left mouse\n"
        "button all scene objects are deselected and then a scene object under the mouse is selected, if there is any. By pressing\n"
        "right mouse button the current selection either unchanged, if no scene object is under mouse, or it is updated so that the\n"
        "secne object under the mouse is added, if it was not in the selection, and it is removed from the selection, if it already\n"
        "was in the selection."
        );
    QObject::connect(w.get_edit_action_mode_select(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_selection);

    w.get_menu_edit()->addAction(w.get_edit_action_mode_translate());
    w.get_edit_action_mode_translate()->setShortcut(Qt::Key::Key_T);
    w.get_edit_action_mode_translate()->setToolTip(
        "Activates the 'TRANSLATE' edit mode. The active edit mode is shown in the status bar (3rd widget from the right). This mode\n"
        "allows you to translate selected scene objects using the mouse. The operation is done in the reference coord. system. It is\n"
        "either the 'world' coord. system (default), or '@pivot' coord. system (if selected). Keep left mouse button pressed and move\n"
        "the mouse to perfotm the translation. Holding also any pair of keys X, Y, or Z pressed allows you to specify along what\n"
        "axes/planes the operation will be performed. When none of the keys is pressed then X and Y are active by default."
        );
    QObject::connect(w.get_edit_action_mode_translate(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_mode_translation);

    w.get_menu_edit()->addAction(w.get_edit_action_mode_rotate());
    w.get_edit_action_mode_rotate()->setShortcut(Qt::Key::Key_R);
    w.get_edit_action_mode_rotate()->setToolTip(
        "Activates the 'ROTATE' edit mode. The active edit mode is shown in the status bar (3rd widget from the right). This mode\n"
        "allows you to rotate selected scene objects using the mouse. The operation is done in the reference coord. system. It is either\n"
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

    w.get_menu_edit()->addAction(w.get_edit_action_agent_reset_skeleton_pose());
    w.get_edit_action_agent_reset_skeleton_pose()->setShortcut(QString("Ctrl+Shift+K"));
    w.get_edit_action_agent_reset_skeleton_pose()->setToolTip(
        "For each selected agent the action resets its current pose to the default one (as defined in the 'pose.txt' file).\n"
        "An agent is considered as selected if any of its node in the 'Scene' tab is selected (i.e. any coord. system, forlder,\n"
        "or record can be selected)."
    );
    QObject::connect(w.get_edit_action_agent_reset_skeleton_pose(), &QAction::triggered, w.wnd(), &program_window::on_menu_edit_agent_reset_skeleton_pose);

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

    // "View" menu

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
