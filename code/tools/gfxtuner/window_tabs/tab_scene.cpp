#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_bool_lock.hpp>
#include <gfxtuner/window_tabs/tab_scene_insert_name_dialog.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_id_reverse_builder.hpp>
#include <gfxtuner/window_tabs/tab_scene_records_integration.hpp>
#include <gfxtuner/window_tabs/tab_scene_tree_widget.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <scene/scene_utils.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <scene/scene_editing.hpp>
#include <angeo/axis_aligned_bounding_box.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/std_pair_hash.hpp>
#include <utility/msgstream.hpp>
#include <utility/canonical_path.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QColorDialog>
#include <QString>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QFileDialog>

namespace window_tabs { namespace tab_scene {


static natural_64_bit  g_new_coord_system_id_counter = 0ULL;


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)

    , m_scene_tree(new tree_widget(
            std::bind(&widgets::on_scene_hierarchy_item_selected, this),
            std::bind(&widgets::on_scene_hierarchy_item_update_action, this, std::placeholders::_1)
            ))

    , m_node_icon((boost::filesystem::path{ get_program_options()->dataRoot() } /
                  "shared/gfx/icons/coord_system.png").string().c_str())
    , m_folder_icon((boost::filesystem::path{ get_program_options()->dataRoot() } /
                    "shared/gfx/icons/data_type.png").string().c_str())

    , m_insert_record_handlers()
    , m_update_record_handlers()
    , m_erase_record_handlers()
    , m_load_record_handlers()
    , m_save_record_handlers()

    , m_processing_selection_change(false)

    , m_save_commit_id(scn::get_invalid_scene_history_commit_id())

    , m_coord_system_pos_x(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_coord_system_pos_y(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_coord_system_pos_z(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_coord_system_rot_w(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_coord_system_rot_x(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_coord_system_rot_y(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_coord_system_rot_z(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_coord_system_yaw(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_coord_system_pitch(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_coord_system_roll(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_scene_coord_system_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_coord_system_location_backup_buffer()
{
    m_scene_tree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    m_scene_tree->setSortingEnabled(true);
    m_scene_tree->sortItems(0, Qt::SortOrder::AscendingOrder);
    enable_coord_system_location_widgets(false, false);

    scn::scene_history_coord_system_insert::set_undo_processor(
        [this](scn::scene_history_coord_system_insert const&  history_node) {
            std::vector<tree_widget_item*>  items_list;
            find_all_coord_system_widgets(m_scene_tree, history_node.get_name(), items_list);
            ASSUMPTION(items_list.size() == 1UL);
            tree_widget_item* const  tree_item = as_tree_widget_item(items_list.front());
            ASSUMPTION(!tree_item->isSelected());
            ASSUMPTION(represents_coord_system(tree_item));
            ASSUMPTION((tree_item->parent() == nullptr) == history_node.get_parent_name().empty());
            ASSUMPTION(tree_item->parent() == nullptr ||
                       get_tree_widget_item_name(tree_item->parent()) == history_node.get_parent_name());
            std::string const  tree_item_name = get_tree_widget_item_name(tree_item);
            m_wnd->glwindow().call_now(&simulator::erase_scene_node, tree_item_name);
            auto const  taken_item = tree_item->parent() != nullptr ?
                tree_item->parent()->takeChild(tree_item->parent()->indexOfChild(tree_item)) :
                m_scene_tree->takeTopLevelItem(m_scene_tree->indexOfTopLevelItem(tree_item))
                ;
            INVARIANT(taken_item == tree_item); (void)taken_item;
            delete tree_item;
        });
    scn::scene_history_coord_system_insert::set_redo_processor(
        [this](scn::scene_history_coord_system_insert const& history_node) {
            tree_widget_item*  parent_tree_item = nullptr;
            if (!history_node.get_parent_name().empty())
            {
                std::vector<tree_widget_item*>  items_list;
                find_all_coord_system_widgets(m_scene_tree, history_node.get_name(), items_list);
                ASSUMPTION(items_list.empty());
                find_all_coord_system_widgets(m_scene_tree, history_node.get_parent_name(), items_list);
                ASSUMPTION(items_list.size() == 1UL);
                parent_tree_item = as_tree_widget_item(items_list.front());
                ASSUMPTION(represents_coord_system(parent_tree_item));
            }
            auto const  tree_node = insert_coord_system(
                    history_node.get_name(),
                    history_node.get_origin(),
                    history_node.get_orientation(),
                    parent_tree_item
                    );
            INVARIANT(tree_node->isSelected() == false);
    });

    scn::scene_history_coord_system_relocate::set_undo_processor(
        [this](scn::scene_history_coord_system_relocate const&  history_node) {
            m_wnd->glwindow().call_now(
                    &simulator::relocate_scene_node,
                    history_node.get_name(),
                    history_node.get_old_origin(),
                    history_node.get_old_orientation()
                    );
            if (is_active_coord_system_in_tree_widget(*m_scene_tree, history_node.get_name()))
                update_coord_system_location_widgets();
        });
    scn::scene_history_coord_system_relocate::set_redo_processor(
        [this](scn::scene_history_coord_system_relocate const&  history_node) {
            m_wnd->glwindow().call_now(
                    &simulator::relocate_scene_node,
                    history_node.get_name(),
                    history_node.get_new_origin(),
                    history_node.get_new_orientation()
                    );
            if (is_active_coord_system_in_tree_widget(*m_scene_tree, history_node.get_name()))
                update_coord_system_location_widgets();
        });

    scn::scene_history_coord_system_insert_to_selection::set_undo_processor(
        [this](scn::scene_history_coord_system_insert_to_selection const&  history_node) {
            std::vector<tree_widget_item*>  items_list;
            find_all_coord_system_widgets(m_scene_tree, history_node.get_name(), items_list);
            ASSUMPTION(items_list.size() == 1UL);
            tree_widget_item* const  tree_item = as_tree_widget_item(items_list.front());
            ASSUMPTION(represents_coord_system(tree_item));
            ASSUMPTION(tree_item->isSelected());
            tree_item->setSelected(false);
            std::unordered_set<scn::scene_node_name>  selected_scene_nodes{ history_node.get_name() };
            std::unordered_set<scn::scene_record_id>  selected_records;
            m_wnd->glwindow().call_now(&simulator::erase_from_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));
            update_coord_system_location_widgets();
        });
    scn::scene_history_coord_system_insert_to_selection::set_redo_processor(
        [this](scn::scene_history_coord_system_insert_to_selection const&  history_node) {
            std::vector<tree_widget_item*>  items_list;
            find_all_coord_system_widgets(m_scene_tree, history_node.get_name(), items_list);
            ASSUMPTION(items_list.size() == 1UL);
            tree_widget_item* const  tree_item = as_tree_widget_item(items_list.front());
            ASSUMPTION(represents_coord_system(tree_item));
            ASSUMPTION(!tree_item->isSelected());
            tree_item->setSelected(true);
            m_scene_tree->scrollToItem(tree_item);
            std::unordered_set<scn::scene_node_name>  selected_scene_nodes{ history_node.get_name() };
            std::unordered_set<scn::scene_record_id>  selected_records;
            m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));
            update_coord_system_location_widgets();
        });

    scn::scene_history_record_insert_to_selection::set_undo_processor(
        [this](scn::scene_history_record_insert_to_selection const&  history_node) {
            std::vector<tree_widget_item*>  items_list;
            find_all_coord_system_widgets(m_scene_tree, history_node.get_id().get_node_name(), items_list);
            ASSUMPTION(items_list.size() == 1UL);
            auto const  coord_system_item = as_tree_widget_item(items_list.front());
            ASSUMPTION(represents_coord_system(coord_system_item));
            for (int i = 0, n = coord_system_item->childCount(); i != n; ++i)
            {
                auto const  item = as_tree_widget_item(coord_system_item->child(i));
                if (represents_coord_system(item))
                    continue;
                INVARIANT(represents_folder(item));
                std::string const  item_name = get_tree_widget_item_name(item);
                if (item_name == history_node.get_id().get_folder_name())
                {
                    for (int j = 0, m = item->childCount(); j != m; ++j)
                    {
                        auto const  record_item = as_tree_widget_item(item->child(j));
                        INVARIANT(represents_record(record_item));
                        std::string const  record_item_name = get_tree_widget_item_name(record_item);
                        if (record_item_name == history_node.get_id().get_record_name())
                        {
                            ASSUMPTION(record_item->isSelected());
                            record_item->setSelected(false);
                            std::unordered_set<scn::scene_node_name>  selected_scene_nodes;
                            std::unordered_set<scn::scene_record_id>  selected_records{ history_node.get_id() };
                            m_wnd->glwindow().call_now(&simulator::erase_from_scene_selection, std::ref(selected_scene_nodes), std::ref(selected_records));
                            update_coord_system_location_widgets();
                            return;
                        }
                    }
                    UNREACHABLE();
                }
            }
            UNREACHABLE();
        });
    scn::scene_history_record_insert_to_selection::set_redo_processor(
        [this](scn::scene_history_record_insert_to_selection const&  history_node) {
            std::vector<tree_widget_item*>  items_list;
            find_all_coord_system_widgets(m_scene_tree, history_node.get_id().get_node_name(), items_list);
            ASSUMPTION(items_list.size() == 1UL);
            auto const  coord_system_item = as_tree_widget_item(items_list.front());
            ASSUMPTION(coord_system_item != nullptr && coord_system_item->represents_coord_system());
            for (int i = 0, n = coord_system_item->childCount(); i != n; ++i)
            {
                auto const  item = as_tree_widget_item(coord_system_item->child(i));
                if (represents_coord_system(item))
                    continue;
                INVARIANT(represents_folder(item));
                std::string const  item_name = get_tree_widget_item_name(item);
                if (item_name == history_node.get_id().get_folder_name())
                {
                    for (int j = 0, m = item->childCount(); j != m; ++j)
                    {
                        auto const  record_item = as_tree_widget_item(item->child(j));
                        INVARIANT(represents_record(record_item));
                        std::string const  record_item_name = get_tree_widget_item_name(record_item);
                        if (record_item_name == history_node.get_id().get_record_name())
                        {
                            ASSUMPTION(!record_item->isSelected());
                            record_item->setSelected(true);
                            m_scene_tree->scrollToItem(record_item);
                            std::unordered_set<scn::scene_node_name>  selected_scene_nodes;
                            std::unordered_set<scn::scene_record_id>  selected_records{ history_node.get_id() };
                            m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::ref(selected_scene_nodes), std::ref(selected_records));
                            update_coord_system_location_widgets();
                            return;
                        }
                    }
                    UNREACHABLE();
                }
            }
            UNREACHABLE();
        });

    register_record_icons(m_icons_of_records);
    register_record_undo_redo_processors(this);
    register_record_handler_for_insert_scene_record(m_insert_record_handlers);
    register_record_handler_for_update_scene_record(m_update_record_handlers);
    register_record_handler_for_erase_scene_record(m_erase_record_handlers);
    register_record_handler_for_load_scene_record(m_load_record_handlers);
    register_record_handler_for_save_scene_record(m_save_record_handlers);
}


void  widgets::add_tree_item_to_selection(QTreeWidgetItem* const  item)
{
    m_scene_tree->setItemSelected(item, true);
    m_scene_tree->scrollToItem(item);
}

void  widgets::on_scene_hierarchy_item_selected()
{
    if (processing_selection_change())
        return;
    lock_bool const  _(&m_processing_selection_change);

    QList<QTreeWidgetItem*>  old_selection;
    {
        std::unordered_set<scn::scene_node_name>  selected_scene_nodes;
        std::unordered_set<scn::scene_record_id>  selected_records;
        wnd()->glwindow().call_now(&simulator::get_scene_selection, std::ref(selected_scene_nodes), std::ref(selected_records));
        for (auto const& node_name : selected_scene_nodes)
        {
            std::vector<tree_widget_item*>  items_list;
            find_all_coord_system_widgets(m_scene_tree, node_name, items_list);
            INVARIANT(items_list.size() == 1 && represents_coord_system(items_list.front()));
            old_selection.push_back(items_list.front());
        }
        for (scn::scene_record_id const&  record_id : selected_records)
        {
            std::vector<tree_widget_item*>  items_list;
            find_all_coord_system_widgets(m_scene_tree, record_id.get_node_name(), items_list);
            INVARIANT(items_list.size() == 1 && represents_coord_system(items_list.front()));
            bool  record_found = false;
            for (int i = 0, n = items_list.front()->childCount(); i != n; ++i)
            {
                auto const  item = as_tree_widget_item(items_list.front()->child(i));
                if (represents_coord_system(item))
                    continue;
                INVARIANT(represents_folder(item));
                std::string const  item_name = get_tree_widget_item_name(item);
                if (item_name == record_id.get_folder_name())
                {
                    for (int j = 0, m = item->childCount(); j != m; ++j)
                    {
                        auto const  record_item = as_tree_widget_item(item->child(j));
                        INVARIANT(represents_record(record_item));
                        std::string const  record_item_name = get_tree_widget_item_name(record_item);
                        if (record_item_name == record_id.get_record_name())
                        {
                            old_selection.push_back(record_item);
                            record_found = true;
                            break;
                        }
                    }
                    break;
                }
            }
            INVARIANT(record_found == true);
        }
    }
    std::unordered_set<scn::scene_node_name>  selected_scene_nodes;
    std::unordered_set<scn::scene_record_id>  selected_records;
    QList<QTreeWidgetItem*>  new_selection;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = as_tree_widget_item(item);
        INVARIANT(tree_item != nullptr);

        if (represents_coord_system(tree_item))
            selected_scene_nodes.insert(get_tree_widget_item_name(tree_item));
        else if (represents_record(tree_item))
            selected_records.insert(scene_record_id_reverse_builder::run(tree_item).get_record_id());

        if (!represents_folder(tree_item))
            new_selection.push_back(tree_item);
    }
    wnd()->glwindow().call_now(&simulator::set_scene_selection, selected_scene_nodes, selected_records);
    
    update_coord_system_location_widgets();

    update_history_according_to_change_in_selection(old_selection, new_selection, get_scene_history());
    set_window_title();
    wnd()->set_focus_to_glwindow(false);
}

void  widgets::on_scene_hierarchy_item_update_action(QTreeWidgetItem* const  item)
{
    if (!represents_record(item))
        return;
    scn::scene_record_id const  id = scene_record_id_reverse_builder::run(as_tree_widget_item(item)).get_record_id();
    auto  it = m_update_record_handlers.find(id.get_folder_name());
    if (it == m_update_record_handlers.end())
        return;
    it->second(this, id);
    if (get_scene_history()->has_not_commited_data())
        get_scene_history()->commit();
}

void  widgets::selection_changed_listener()
{
    ASSUMPTION(!processing_selection_change());
    lock_bool const  _(&m_processing_selection_change);

    QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();

    std::unordered_set<scn::scene_node_name>  selected_scene_nodes;
    std::unordered_set<scn::scene_record_id>  selected_records;
    wnd()->glwindow().call_now(&simulator::get_scene_selection, std::ref(selected_scene_nodes), std::ref(selected_records));

    auto const  recover_from_failure = [this]() -> void {
        m_scene_tree->clearSelection();
        wnd()->glwindow().call_now(
                &simulator::set_scene_selection,
                std::unordered_set<scn::scene_node_name>(),
                std::unordered_set<scn::scene_record_id>()
                );
        update_coord_system_location_widgets();
        wnd()->print_status_message("ERROR: Detected inconsistency between the simulator and GUI in selection. "
                                    "Clearing the selection.", 10000);
    };

    m_scene_tree->clearSelection();
    for (auto const&  node_name : selected_scene_nodes)
    {
        std::vector<tree_widget_item*>  items_list;
        find_all_coord_system_widgets(m_scene_tree, node_name, items_list);
        if (items_list.size() != 1 || !represents_coord_system(items_list.front()))
        {
            recover_from_failure();
            return;
        }
        add_tree_item_to_selection(items_list.front());
    }
    for (scn::scene_record_id const&  record_id : selected_records)
    {
        std::vector<tree_widget_item*>  items_list;
        find_all_coord_system_widgets(m_scene_tree, record_id.get_node_name(), items_list);
        if (items_list.size() != 1 || !represents_coord_system(items_list.front()))
        {
            recover_from_failure();
            return;
        }
        bool  record_found = false;
        for (int i = 0, n = items_list.front()->childCount(); i != n; ++i)
        {
            auto const  item = as_tree_widget_item(items_list.front()->child(i));
            if (represents_coord_system(item))
                continue;
            INVARIANT(represents_folder(item));
            std::string const  item_name = get_tree_widget_item_name(item);
            if (item_name == record_id.get_folder_name())
            {
                for (int j = 0, m = item->childCount(); j != m; ++j)
                {
                    auto const  record_item = as_tree_widget_item(item->child(j));
                    INVARIANT(represents_record(record_item));
                    std::string const  record_item_name = get_tree_widget_item_name(record_item);
                    if (record_item_name == record_id.get_record_name())
                    {
                        add_tree_item_to_selection(record_item);
                        record_found = true;
                        break;
                    }
                }
                break;
            }
        }
        if (record_found == false)
        {
            recover_from_failure();
            return;
        }
    }
    update_coord_system_location_widgets();

    QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
    update_history_according_to_change_in_selection(old_selection, new_selection, get_scene_history());
    set_window_title();
}


QTreeWidgetItem*  widgets::insert_coord_system(
            std::string const&  name,
            vector3 const&  origin,
            quaternion const&  orientation,
            QTreeWidgetItem* const  parent_tree_item
            )
{
    auto const item = insert_coord_system_to_tree_widget(m_scene_tree, name, origin, orientation, m_node_icon, parent_tree_item);
    if (parent_tree_item == nullptr)
        wnd()->glwindow().call_now(&simulator::insert_scene_node_at, name, origin, orientation);
    else
        wnd()->glwindow().call_now(
                &simulator::insert_child_scene_node_at,
                name,
                origin,
                orientation,
                get_tree_widget_item_name(parent_tree_item)
                );
    return item;
}

void  widgets::on_scene_insert_coord_system()
{
    ASSUMPTION(!processing_selection_change());
    lock_bool const  _(&m_processing_selection_change);

    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene editing is disabled.", 10000);
        return;
    }

    bool  use_pivot = false;
    QTreeWidgetItem*  parent_tree_item = nullptr;
    std::string  parent_tree_item_name;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item*  tree_item = as_tree_widget_item(item);
        INVARIANT(tree_item != nullptr);
        if (!represents_coord_system(tree_item))
            tree_item = find_nearest_coord_system_item(tree_item);
        std::string const  tree_item_name = get_tree_widget_item_name(tree_item);
        if (tree_item_name == scn::get_pivot_node_name())
            use_pivot = true;
        else
        {
            if (parent_tree_item != nullptr && tree_item != parent_tree_item)
            {
                wnd()->print_status_message("ERROR: Insertion has FAILED. Ambiguous non-pivot coord system.", 10000);
                return;
            }
            parent_tree_item = tree_item;
            parent_tree_item_name = tree_item_name;
        }
    }

    natural_64_bit  old_counter;
    std::string  name;
    do
    {
        name = msgstream() << "coord_system_" << g_new_coord_system_id_counter;
        old_counter = g_new_coord_system_id_counter;
        ++g_new_coord_system_id_counter;
    }
    while (wnd()->glwindow().call_now(&simulator::get_scene_node, name) != nullptr);
    insert_name_dialog  dlg(wnd(), name,
        [this](std::string const&  name) {
            return wnd()->glwindow().call_now(&simulator::get_scene_node, name) == nullptr;
        });
    dlg.exec();
    if (!dlg.get_name().empty())
    {
        QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();
        m_scene_tree->clearSelection();
        wnd()->glwindow().call_now(
                &simulator::set_scene_selection,
                std::unordered_set<scn::scene_node_name>(),
                std::unordered_set<scn::scene_record_id>()
                );

        vector3  origin = vector3_zero();
        quaternion  orientation = quaternion_identity();
        if (use_pivot)
        {
            scn::scene_node_ptr const  pivot = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_name());
            origin = pivot->get_coord_system()->origin();
            orientation = pivot->get_coord_system()->orientation();
            if (parent_tree_item != nullptr)
                transform_origin_and_orientation_from_world_to_scene_node(
                        wnd()->glwindow().call_now(&simulator::get_scene_node, qtgl::to_string(parent_tree_item->text(0))),
                        origin,
                        orientation
                        );
        }

        auto const  tree_item = insert_coord_system(dlg.get_name(), origin, orientation, parent_tree_item);

        std::unordered_set<scn::scene_node_name>  selected_scene_nodes{ dlg.get_name() };
        std::unordered_set<scn::scene_record_id>  selected_records;
        m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));

        get_scene_history()->insert<scn::scene_history_coord_system_insert>(
                dlg.get_name(),
                origin,
                orientation,
                parent_tree_item == nullptr ? "" : qtgl::to_string(parent_tree_item->text(0)),
                false
                );

        add_tree_item_to_selection(tree_item);

        QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
        update_history_according_to_change_in_selection(old_selection, new_selection, get_scene_history(), false);
        get_scene_history()->commit();
        update_coord_system_location_widgets();
        set_window_title();
    }
    else
        g_new_coord_system_id_counter = old_counter;
}


void  widgets::on_scene_insert_record(std::string const&  record_kind, std::string const&  mode)
{
    ASSUMPTION(!processing_selection_change());
    lock_bool const  _(&m_processing_selection_change);

    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene editing is disabled.", 10000);
        return;
    }

    std::unordered_set<tree_widget_item*>  nodes;
    std::unordered_set<std::string>  used_names;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item*  tree_item = as_tree_widget_item(item);
        INVARIANT(tree_item != nullptr);
        if (!represents_coord_system(tree_item))
            tree_item = find_nearest_coord_system_item(tree_item);
        std::string const  tree_item_name = get_tree_widget_item_name(tree_item);
        if (tree_item_name != scn::get_pivot_node_name())
        {
            nodes.insert(tree_item);
            for (int i = 0, n = tree_item->childCount(); i != n; ++i)
            {
                auto const  item = as_tree_widget_item(tree_item->child(i));
                if (represents_coord_system(item))
                    continue;
                INVARIANT(represents_folder(item));
                std::string const  item_name = get_tree_widget_item_name(item);
                if (item_name == record_kind)
                {
                    for (int j = 0, m = item->childCount(); j != m; ++j)
                    {
                        auto const  record_item = as_tree_widget_item(item->child(j));
                        INVARIANT(represents_record(record_item));
                        std::string const  record_item_name = get_tree_widget_item_name(record_item);
                        used_names.insert(record_item_name);
                    }
                    break;
                }
            }
        }
    }
    if (nodes.empty())
    {
        wnd()->print_status_message("ERROR: No coordinate system is selected ('@pivot' is ignored).", 10000);
        return;
    }

    bool allow_multiple_records_in_folder = m_insert_record_handlers.at(record_kind).first;
    if (!allow_multiple_records_in_folder && !used_names.empty())
    {
        wnd()->print_status_message("ERROR: Record of the kind '" + record_kind + "' is aready present.", 10000);
        return;
    }

    std::pair<std::string, std::function<void(scn::scene_record_id const&)> > const  record_name_and_system_inserted =
            m_insert_record_handlers.at(record_kind).second(this, mode, used_names);
    if (record_name_and_system_inserted.first.empty() || !record_name_and_system_inserted.second)
        return;
    std::string  record_name = record_name_and_system_inserted.first;
    if (allow_multiple_records_in_folder)
    {
        natural_64_bit  counter = 0ULL;
        while (used_names.count(record_name) != 0UL)
        {
            record_name = msgstream() << record_name_and_system_inserted.first << "_" << counter;
            ++counter;
        }
        insert_name_dialog  dlg(wnd(), record_name,
            [&used_names](std::string const&  name) {
            return used_names.count(name) == 0UL;
        });
        dlg.exec();
        record_name = dlg.get_name();
    }
    if (!record_name.empty())
    {
        QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();
        m_scene_tree->clearSelection();

        for (auto const& tree_item : nodes)
        {
            scn::scene_record_id const  record_id{ get_tree_widget_item_name(tree_item), record_kind, record_name };

            auto const  record_item =
                    insert_record_to_tree_widget(m_scene_tree, record_id, m_icons_of_records.at(record_kind), m_folder_icon);

            record_name_and_system_inserted.second(record_id);

            std::unordered_set<scn::scene_node_name>  selected_scene_nodes;
            std::unordered_set<scn::scene_record_id>  selected_records{ record_id };
            m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));

            add_tree_item_to_selection(record_item);
        }

        QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
        update_history_according_to_change_in_selection(old_selection, new_selection, get_scene_history(), false);

        get_scene_history()->commit();
        set_window_title();
    }
}


void  widgets::on_scene_erase_selected()
{
    if (m_scene_tree->selectedItems().empty())
        return;

    ASSUMPTION(!processing_selection_change());
    lock_bool const  _(&m_processing_selection_change);

    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene editing is disabled.", 10000);
        return;
    }

    std::unordered_set<QTreeWidgetItem*>  to_erase_items;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        if (item->parent() == nullptr && get_tree_widget_item_name(item) == scn::get_pivot_node_name())
        {
            wnd()->print_status_message("ERROR: Cannot erase 'pivot' coordinate system.", 10000);
            return;
        }
        to_erase_items.insert(item);
    }

    std::unordered_set<QTreeWidgetItem*>  erased_items;
    for (auto const  item : to_erase_items)
        if (erased_items.count(item) == 0UL)
            erase_subtree_at_root_item(item, erased_items);
    INVARIANT(erased_items.size() >= to_erase_items.size());

    m_scene_tree->clearSelection();

    get_scene_history()->commit();
    set_window_title();
}


void  widgets::erase_scene_record(scn::scene_record_id const&  id)
{
    m_erase_record_handlers.at(id.get_folder_name())(this, id);
}

void  widgets::erase_subtree_at_root_item(QTreeWidgetItem* const  root_item, std::unordered_set<QTreeWidgetItem*>&  erased_items, bool const  is_root)
{
    tree_widget_item* const  item = as_tree_widget_item(root_item);
    INVARIANT(item != nullptr);
    std::string const  item_name = get_tree_widget_item_name(item);
    QTreeWidgetItem*  folder_item = nullptr;
    if (item->represents_coord_system())
    {
        while (item->childCount() > 0)
            erase_subtree_at_root_item(item->child(0), erased_items, false);

        if (item->isSelected())
            get_scene_history()->insert<scn::scene_history_coord_system_insert_to_selection>(item_name, true);
        scn::scene_node_ptr const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, item_name);
        INVARIANT(node_ptr != nullptr);
        get_scene_history()->insert<scn::scene_history_coord_system_insert>(
                item_name,
                node_ptr->get_coord_system()->origin(),
                node_ptr->get_coord_system()->orientation(),
                item->parent() == nullptr ? "" : get_tree_widget_item_name(item->parent()),
                true
                );

        std::unordered_set<scn::scene_node_name>  selected_scene_nodes{ item_name };
        std::unordered_set<scn::scene_record_id>  selected_records;
        wnd()->glwindow().call_now(&simulator::erase_from_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));
        wnd()->glwindow().call_now(&simulator::erase_scene_node, item_name);
    }
    else if (represents_folder(item))
    {
        while (item->childCount() > 0)
            erase_subtree_at_root_item(item->child(0), erased_items, false);
    }
    else
    {
        scn::scene_record_id const  id = scene_record_id_reverse_builder::run(item).get_record_id();

        if (item->isSelected())
            get_scene_history()->insert<scn::scene_history_record_insert_to_selection>(id, true);

        std::unordered_set<scn::scene_node_name>  selected_scene_nodes;
        std::unordered_set<scn::scene_record_id>  selected_records{ id };
        wnd()->glwindow().call_now(&simulator::erase_from_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));

        erase_scene_record(id);

        folder_item = item->parent();
        INVARIANT(represents_folder(folder_item));
    }

    auto const  taken_item = item->parent() != nullptr ?
        item->parent()->takeChild(item->parent()->indexOfChild(item)) :
        m_scene_tree->takeTopLevelItem(m_scene_tree->indexOfTopLevelItem(item))
        ;
    INVARIANT(taken_item == item); (void)taken_item;
    delete taken_item;
    erased_items.insert(taken_item);

    if (is_root && folder_item != nullptr && folder_item->childCount() == 0)
    {
        QTreeWidgetItem* const  node_item = folder_item->parent();
        INVARIANT(represents_coord_system(node_item));

        auto const  taken_item = node_item->takeChild(node_item->indexOfChild(folder_item));
        INVARIANT(taken_item == folder_item); (void)taken_item;
        delete taken_item;
        erased_items.insert(taken_item);
    }
}

void  widgets::clear_scene()
{
    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene editing is disabled.", 10000);
        return;
    }

    m_scene_tree->clear();
    wnd()->glwindow().call_now(&simulator::clear_scene);
    get_scene_history()->clear();
    m_save_commit_id = get_scene_history()->get_active_commit_id();
    set_window_title();

    insert_coord_system(scn::get_pivot_node_name(), vector3_zero(), quaternion_identity(), nullptr);

    g_new_coord_system_id_counter = 0UL;
}


void  widgets::load_scene_record(scn::scene_record_id const&  id, boost::property_tree::ptree const&  data)
{
    m_load_record_handlers.at(id.get_folder_name())(this, id, data);
}

void  widgets::save_scene_record(
        scn::scene_node_ptr const  node_ptr,
        scn::scene_node_record_id const&  id,
        boost::property_tree::ptree&  data
        )
{
    m_save_record_handlers.at(id.get_folder_name())(this, node_ptr, id, data);
}


QTreeWidgetItem*  widgets::load_scene_node(
        std::string const&  node_name,
        boost::property_tree::ptree const&  node_tree,
        QTreeWidgetItem*  parent_item
        )
{
    boost::property_tree::ptree const&  origin_tree = node_tree.find("origin")->second;
    vector3 const  origin(
            origin_tree.get<scalar>("x"),
            origin_tree.get<scalar>("y"),
            origin_tree.get<scalar>("z")
            );

    boost::property_tree::ptree const&  orientation_tree = node_tree.find("orientation")->second;
    quaternion const  orientation = make_quaternion_xyzw(
            orientation_tree.get<scalar>("x"),
            orientation_tree.get<scalar>("y"),
            orientation_tree.get<scalar>("z"),
            orientation_tree.get<scalar>("w")
            );

    if (node_name == scn::get_pivot_node_name())
    {
        scn::scene_node_ptr const  pivot = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_name());
        wnd()->glwindow().call_now(&simulator::relocate_scene_node, scn::get_pivot_node_name(), origin, orientation);
        return nullptr;
    }

    QTreeWidgetItem* const  current_node_item = insert_coord_system(node_name, origin, orientation, parent_item);

    boost::property_tree::ptree const&  folders = node_tree.find("folders")->second;
    for (auto folder_it = folders.begin(); folder_it != folders.end(); ++folder_it)
        for (auto record_it = folder_it->second.begin(); record_it != folder_it->second.end(); ++record_it)
            load_scene_record({ node_name, folder_it->first, record_it->first }, record_it->second);

    boost::property_tree::ptree const&  children = node_tree.find("children")->second;
    for (auto it = children.begin(); it != children.end(); ++it)
        load_scene_node(it->first, it->second, current_node_item);

    return current_node_item;
}

void  widgets::open_scene(boost::filesystem::path const&  scene_root_dir)
{
    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene load is disabled.", 10000);
        return;
    }

    clear_scene();

    try
    {
        boost::property_tree::ptree  load_tree;
        boost::property_tree::read_info((scene_root_dir / "hierarchy.info").string(), load_tree);

        for (auto  it = load_tree.begin(); it != load_tree.end(); ++it)
            load_scene_node(it->first, it->second, nullptr);
        //m_scene_tree->expandAll();
        wnd()->set_title(scene_root_dir.string());
    }
    catch (boost::property_tree::ptree_error const&  e)
    {
        wnd()->print_status_message(std::string("ERROR: Load of scene has FAILED. ") + e.what(), 10000);
        wnd()->get_current_scene_dir().clear();
        clear_scene();
    }
    catch (...)
    {
        wnd()->print_status_message("ERROR: Load of scene has FAILED. Reason is unknown.", 10000);
        wnd()->get_current_scene_dir().clear();
        clear_scene();
    }
}

void  widgets::save_scene_node(
        QTreeWidgetItem* const  node_item_ptr,
        boost::property_tree::ptree& save_tree
        )
{
    ASSUMPTION(node_item_ptr != nullptr);

    std::string const  node_name = get_tree_widget_item_name(node_item_ptr);
    scn::scene_node_ptr const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, node_name);
    
    {
        boost::property_tree::ptree  origin_tree;
        vector3 const&  origin = node_ptr->get_coord_system()->origin();
        origin_tree.put("x", origin(0));
        origin_tree.put("y", origin(1));
        origin_tree.put("z", origin(2));
        save_tree.put_child(node_name + ".origin", origin_tree);
    }
    {
        boost::property_tree::ptree  orientation_tree;
        vector4 const&  orientation = quaternion_coefficients_xyzw(node_ptr->get_coord_system()->orientation());
        orientation_tree.put("x", orientation(0));
        orientation_tree.put("y", orientation(1));
        orientation_tree.put("z", orientation(2));
        orientation_tree.put("w", orientation(3));
        save_tree.put_child(node_name + ".orientation", orientation_tree);
    }

    boost::property_tree::ptree  folders;
    boost::property_tree::ptree  children;
    for (int i = 0, n = node_item_ptr->childCount(); i != n; ++i)
        if (represents_coord_system(node_item_ptr->child(i)))
            save_scene_node(node_item_ptr->child(i), children);
        else
        {
            QTreeWidgetItem* const  folder_ptr = node_item_ptr->child(i);
            INVARIANT(represents_folder(folder_ptr));
            std::string const  folder_name = get_tree_widget_item_name(folder_ptr);

            boost::property_tree::ptree  records;
            for (int j = 0, m = folder_ptr->childCount(); j != m; ++j)
            {
                QTreeWidgetItem* const  record_ptr = folder_ptr->child(j);
                INVARIANT(represents_record(record_ptr));
                std::string const  record_name = get_tree_widget_item_name(record_ptr);

                boost::property_tree::ptree  record;
                save_scene_record(node_ptr, { folder_name, record_name }, record);
                records.put_child(record_name, record);
            }
            folders.put_child(folder_name, records);
        }
    save_tree.put_child(node_name + ".folders", folders);
    save_tree.put_child(node_name + ".children", children);
}

void  widgets::save_scene(boost::filesystem::path const&  scene_root_dir)
{
    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene save is disabled.", 10000);
        return;
    }

    boost::property_tree::ptree save_tree;
    for (int i = 0, n = m_scene_tree->topLevelItemCount(); i != n; ++i)
        save_scene_node(as_tree_widget_item(m_scene_tree->topLevelItem(i)), save_tree);

    boost::filesystem::create_directories(scene_root_dir);
    boost::property_tree::write_info((scene_root_dir / "hierarchy.info").string(), save_tree);
    wnd()->print_status_message(std::string("SUCCESS: Scene saved to: ") + scene_root_dir.string(), 5000);
    wnd()->set_title(scene_root_dir.string());
    m_save_commit_id = get_scene_history()->get_active_commit_id();
    set_window_title();
}

void  widgets::save()
{
    //wnd()->ptree().put("draw.show_grid", m_show_grid->isChecked());
}

void  widgets::on_coord_system_pos_changed()
{
    vector3 const  pos(m_coord_system_pos_x->text().toFloat(),
                       m_coord_system_pos_y->text().toFloat(),
                       m_coord_system_pos_z->text().toFloat());

    std::string const&  name = get_name_of_active_coord_system_in_tree_widget(*m_scene_tree);
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, name);
    INVARIANT(node_ptr != nullptr);
    if(length(pos - node_ptr->get_coord_system()->origin()) < 1e-4f)
    {
        wnd()->set_focus_to_glwindow();
        return;
    }

    wnd()->glwindow().call_later(&simulator::set_position_of_scene_node, name, pos);

    get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
            name,
            node_ptr->get_coord_system()->origin(),
            node_ptr->get_coord_system()->orientation(),
            pos,
            node_ptr->get_coord_system()->orientation()
            );
    get_scene_history()->commit();
    set_window_title();
    wnd()->set_focus_to_glwindow();
}

void  widgets::on_coord_system_rot_changed()
{
    quaternion  q(m_coord_system_rot_w->text().toFloat(),
                  m_coord_system_rot_x->text().toFloat(),
                  m_coord_system_rot_y->text().toFloat(),
                  m_coord_system_rot_z->text().toFloat());
    if (length_squared(q) < 1e-5f)
        q.z() = 1.0f;
    normalise(q);

    refresh_text_in_coord_system_rotation_widgets(q);

    std::string const&  name = get_name_of_active_coord_system_in_tree_widget(*m_scene_tree);
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, name);
    INVARIANT(node_ptr != nullptr);
    if (length_4d(quaternion_coefficients_xyzw(q) -
                  quaternion_coefficients_xyzw(node_ptr->get_coord_system()->orientation())) < 1e-4f)
    {
        wnd()->set_focus_to_glwindow();
        return;
    }

    wnd()->glwindow().call_later(&simulator::set_orientation_of_scene_node, name, q);

    get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
            name,
            node_ptr->get_coord_system()->origin(),
            node_ptr->get_coord_system()->orientation(),
            node_ptr->get_coord_system()->origin(),
            q
            );
    get_scene_history()->commit();
    set_window_title();
    wnd()->set_focus_to_glwindow();
}

void  widgets::on_coord_system_rot_tait_bryan_changed()
{
    quaternion  q = rotation_matrix_to_quaternion(yaw_pitch_roll_to_rotation(
        m_coord_system_yaw->text().toFloat() * PI() / 180.0f,
        m_coord_system_pitch->text().toFloat() * PI() / 180.0f,
        m_coord_system_roll->text().toFloat() * PI() / 180.0f
        ));
    normalise(q);

    refresh_text_in_coord_system_rotation_widgets(q);

    std::string const&  name = get_name_of_active_coord_system_in_tree_widget(*m_scene_tree);
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, name);
    INVARIANT(node_ptr != nullptr);
    if (length_4d(quaternion_coefficients_xyzw(q) -
                  quaternion_coefficients_xyzw(node_ptr->get_coord_system()->orientation())) < 1e-4f)
    {
        wnd()->set_focus_to_glwindow();
        return;
    }

    wnd()->glwindow().call_later(&simulator::set_orientation_of_scene_node, name, q);

    get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
            name,
            node_ptr->get_coord_system()->origin(),
            node_ptr->get_coord_system()->orientation(),
            node_ptr->get_coord_system()->origin(),
            q
            );
    get_scene_history()->commit();
    set_window_title();
    wnd()->set_focus_to_glwindow();
}


void  widgets::on_coord_system_position_started()
{
    m_coord_system_location_backup_buffer.clear();
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = find_nearest_coord_system_item(item);
        INVARIANT(represents_coord_system(tree_item));
        std::string  node_name = get_tree_widget_item_name(tree_item);
        auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, node_name);
        INVARIANT(node_ptr != nullptr);
        m_coord_system_location_backup_buffer.insert({ node_name, *node_ptr->get_coord_system() });
    }
}

void  widgets::coord_system_position_listener()
{
    update_coord_system_location_widgets();
}

void  widgets::on_coord_system_position_finished()
{
    for (auto const&  name_and_system : m_coord_system_location_backup_buffer)
    {
        auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, name_and_system.first);
        INVARIANT(node_ptr != nullptr);
        get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
                name_and_system.first,
                name_and_system.second.origin(),
                name_and_system.second.orientation(),
                node_ptr->get_coord_system()->origin(),
                node_ptr->get_coord_system()->orientation()
                );
    }
    if (!m_coord_system_location_backup_buffer.empty())
    {
        get_scene_history()->commit();
        set_window_title();
    }
}


void  widgets::on_coord_system_rotation_started()
{
    on_coord_system_position_started();
}

void  widgets::coord_system_rotation_listener()
{
    update_coord_system_location_widgets();
}

void  widgets::on_coord_system_rotation_finished()
{
    on_coord_system_position_finished();
}

void  widgets::on_scene_mode_selection()
{
    if (is_editing_enabled())
        wnd()->glwindow().call_later(&simulator::set_scene_edit_mode, scn::SCENE_EDIT_MODE::SELECT_SCENE_OBJECT);
}

void  widgets::on_scene_mode_translation()
{
    if (is_editing_enabled())
        wnd()->glwindow().call_later(&simulator::set_scene_edit_mode, scn::SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES);
}

void  widgets::on_scene_mode_rotation()
{
    if (is_editing_enabled())
        wnd()->glwindow().call_later(&simulator::set_scene_edit_mode, scn::SCENE_EDIT_MODE::ROTATE_SELECTED_NODES);
}

void  widgets::on_scene_toggle_pivot_selection()
{
    ASSUMPTION(!processing_selection_change());
    lock_bool const  _(&m_processing_selection_change);

    QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();

    std::vector<tree_widget_item*>  items_list;
    find_all_coord_system_widgets(m_scene_tree, scn::get_pivot_node_name(), items_list);
    ASSUMPTION(items_list.size() == 1UL);
    tree_widget_item* const  tree_item = as_tree_widget_item(items_list.front());
    ASSUMPTION(represents_coord_system(tree_item));
    std::unordered_set<scn::scene_node_name>  selected_scene_nodes{ scn::get_pivot_node_name() };
    std::unordered_set<scn::scene_record_id>  selected_records;
    if (tree_item->isSelected())
    {
        m_wnd->glwindow().call_now(&simulator::erase_from_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));
        tree_item->setSelected(false);
    }
    else
    {
        m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));
        tree_item->setSelected(true);

    }
    update_coord_system_location_widgets();

    QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
    update_history_according_to_change_in_selection(old_selection, new_selection, get_scene_history());
    set_window_title();
}

void  widgets::on_scene_move_selection_to_pivot()
{
    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene editing is disabled.", 10000);
        return;
    }

    std::unordered_set<scn::scene_node_ptr>  nodes;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item*  tree_item = as_tree_widget_item(item);
        INVARIANT(tree_item != nullptr);
        if (!represents_coord_system(tree_item))
        {
            tree_item = find_nearest_coord_system_item(tree_item->parent());
            INVARIANT(tree_item != nullptr && represents_coord_system(tree_item));
        }
        std::string const  tree_item_name = get_tree_widget_item_name(tree_item);
        if (tree_item_name != scn::get_pivot_node_name())
        {
            auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, tree_item_name);
            INVARIANT(node_ptr != nullptr);
            nodes.insert(node_ptr);
        }
    }
    if (nodes.empty())
    {
        wnd()->print_status_message("ERROR: There is nothing selected to be translated to the '@pivot' origin.", 10000);
        return;
    }

    auto const  pivot_node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_name());
    INVARIANT(pivot_node_ptr != nullptr);

    scn::SCENE_EDIT_MODE const  edit_mode = wnd()->glwindow().call_now(&simulator::get_scene_edit_mode);

    if (edit_mode == scn::SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES)
    {
        vector3  shift_vector;
        {
            vector3  source_position;
            {
                if (nodes.size() == 1UL)
                    source_position = transform_point_from_scene_node_to_world(**nodes.cbegin(), vector3_zero());
                else
                    source_position = scn::get_center_of_scene_nodes(nodes);
            }

            shift_vector = pivot_node_ptr->get_coord_system()->origin() - source_position;
        }

        for (auto  node_ptr : nodes)
        {
            vector3 const  local_shift_vector =
                node_ptr->has_parent() ?
                    scn::transform_vector_from_world_to_scene_node(*node_ptr->get_parent(), shift_vector) :
                    shift_vector
                    ;
            get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
                    node_ptr->get_name(),
                    node_ptr->get_coord_system()->origin(),
                    node_ptr->get_coord_system()->orientation(),
                    node_ptr->get_coord_system()->origin() + local_shift_vector,
                    node_ptr->get_coord_system()->orientation()
                    );
            node_ptr->translate(local_shift_vector);
        }
    }
    else if (edit_mode == scn::SCENE_EDIT_MODE::ROTATE_SELECTED_NODES)
        for (auto node_ptr : nodes)
        {
            quaternion const  local_orientation =
                node_ptr->has_parent() ? 
                    transform_orientation_from_world_to_scene_node(
                            *node_ptr->get_parent(),
                            pivot_node_ptr->get_coord_system()->orientation()
                            ) :
                    pivot_node_ptr->get_coord_system()->orientation()
                    ;
            get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
                node_ptr->get_name(),
                node_ptr->get_coord_system()->origin(),
                node_ptr->get_coord_system()->orientation(),
                node_ptr->get_coord_system()->origin(),
                local_orientation
                );
            node_ptr->set_orientation(local_orientation);
        }
    else
    {
        wnd()->print_status_message("ERROR: The '@pivot' can only be relocated to selection, when edit mode is "
                                    "either 'Translation' or 'Rotation'.", 10000);
        return;
    }

    get_scene_history()->commit();
    update_coord_system_location_widgets();
}

void  widgets::on_scene_move_pivot_to_selection()
{
    if (!is_editing_enabled())
        return;

    std::unordered_set<scn::scene_node_ptr>  nodes;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item*  tree_item = dynamic_cast<tree_widget_item*>(item);
        INVARIANT(tree_item != nullptr);
        if (!represents_coord_system(tree_item))
        {
            tree_item = find_nearest_coord_system_item(tree_item->parent());
            INVARIANT(tree_item != nullptr && represents_coord_system(tree_item));
        }
        std::string const  tree_item_name = get_tree_widget_item_name(tree_item);
        if (tree_item_name != scn::get_pivot_node_name())
        {
            auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, tree_item_name);
            INVARIANT(node_ptr != nullptr);
            nodes.insert(node_ptr);
        }
    }
    if (nodes.empty())
    {
        wnd()->print_status_message("ERROR: There is nothing selected to be translated to the '@pivot' origin.", 10000);
        return;
    }

    auto const  pivot_node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_name());
    INVARIANT(pivot_node_ptr != nullptr);

    scn::SCENE_EDIT_MODE const  edit_mode = wnd()->glwindow().call_now(&simulator::get_scene_edit_mode);

    if (edit_mode == scn::SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES)
    {
        vector3  target_position;
        {
            if (nodes.size() == 1UL)
                target_position = transform_point_from_scene_node_to_world(**nodes.cbegin(), vector3_zero());
            else
                target_position = scn::get_center_of_scene_nodes(nodes);
        }

        get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
                pivot_node_ptr->get_name(),
                pivot_node_ptr->get_coord_system()->origin(),
                pivot_node_ptr->get_coord_system()->orientation(),
                target_position,
                pivot_node_ptr->get_coord_system()->orientation()
                );
        pivot_node_ptr->set_origin(target_position);
    }
    else if (edit_mode == scn::SCENE_EDIT_MODE::ROTATE_SELECTED_NODES)
    {
        if (nodes.size() == 1UL)
        {
            quaternion const  new_orientation =
                (*nodes.cbegin())->has_parent() ?
                    transform_orientation_from_scene_node_to_world(
                            *(*nodes.cbegin())->get_parent(),
                            (*nodes.cbegin())->get_coord_system()->orientation()
                            ) :
                    (*nodes.cbegin())->get_coord_system()->orientation()
                    ;
            get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
                    pivot_node_ptr->get_name(),
                    pivot_node_ptr->get_coord_system()->origin(),
                    pivot_node_ptr->get_coord_system()->orientation(),
                    pivot_node_ptr->get_coord_system()->origin(),
                    new_orientation
                    );
            pivot_node_ptr->set_orientation(new_orientation);
        }
        else
        {
            wnd()->print_status_message("ERROR: Multiple coord. systems are selected, i.e. ambiguous target axis vectors "
                                        "the '@pivot' orienation should be set to.", 10000);
            return;
        }
    }
    else
    {
        wnd()->print_status_message("ERROR: The '@pivot' can only be relocated to selection, when edit mode is "
                                    "either 'Translation' or 'Rotation'.", 10000);
        return;
    }

    get_scene_history()->commit();
    update_coord_system_location_widgets();
}

void  widgets::on_scene_undo()
{
    lock_bool const  _(&m_processing_selection_change);
    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene editing is disabled.", 10000);
        return;
    }
    get_scene_history()->undo();
    set_window_title();
}

void  widgets::on_scene_redo()
{
    lock_bool const  _(&m_processing_selection_change);
    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene editing is disabled.", 10000);
        return;
    }
    get_scene_history()->redo();
    set_window_title();
}

void  widgets::on_look_at_selection(std::function<void(vector3 const&, float_32_bit const*)> const&  solver)
{
    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Look at operation is available only in scene editing mode.", 10000);
        return;
    }

    std::vector<angeo::axis_aligned_bounding_box> selection;
    std::unordered_set<tree_widget_item*>  visited_coord_systems;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = as_tree_widget_item(item);
        INVARIANT(tree_item != nullptr);
        std::string const  tree_item_name = get_tree_widget_item_name(tree_item);
        if (represents_coord_system(tree_item))
        {
            auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, tree_item_name);
            INVARIANT(node_ptr != nullptr);
            angeo::axis_aligned_bounding_box  bbox;
            if (scn::get_bbox(*node_ptr, bbox))
            {
                selection.push_back(angeo::transform_bbox(bbox, node_ptr->get_world_matrix()));
                visited_coord_systems.insert(tree_item);
            }
        }
        else if (represents_folder(tree_item))
            continue;
        else
        {
            tree_widget_item*  coord_system_item;
            scn::scene_node_record_id const  id =
                    scene_record_id_reverse_builder::run(tree_item, &coord_system_item).get_node_record_id();
            INVARIANT(represents_coord_system(coord_system_item));
            std::string const  coord_system_item_name = get_tree_widget_item_name(coord_system_item);
            auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_item_name);
            INVARIANT(node_ptr != nullptr);

            angeo::axis_aligned_bounding_box  bbox;
            if (scn::get_bbox(*node_ptr, id, bbox))
            {
                selection.push_back(angeo::transform_bbox(bbox, node_ptr->get_world_matrix()));
                visited_coord_systems.insert(tree_item);
            }
        }
    }

    if (selection.empty())
    {
        wnd()->print_status_message("ERROR: There is nothing selected to be looked at.", 10000);
        return;
    }

    angeo::axis_aligned_bounding_box  union_bbox = selection.front();
    for (std::size_t i = 1UL; i != selection.size(); ++i)
        angeo::extend_union_bbox(union_bbox, selection.at(i));
    vector3 const  centre = angeo::center_of_bbox(union_bbox);
    float_32_bit const  radius = std::max(0.01f, angeo::radius_of_bbox(union_bbox));

    scn::SCENE_EDIT_MODE const  edit_mode = wnd()->glwindow().call_now(&simulator::get_scene_edit_mode);
    if (edit_mode == scn::SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES)
    {
        float_32_bit  distance;
        {
            float_32_bit const  camera_near = wnd()->glwindow().call_now(&simulator::get_camera_near_plane_distance);
            float_32_bit const  camera_side = wnd()->glwindow().call_now(&simulator::get_camera_side_plane_minimal_distance);
            distance = std::max(
                radius + camera_near,
                (radius / camera_side) * std::sqrt(camera_near*camera_near + camera_side*camera_side)
                );
        }
        solver(centre, &distance);
    }
    else if (edit_mode == scn::SCENE_EDIT_MODE::ROTATE_SELECTED_NODES)
    {
        solver(centre, nullptr);
    }
    else
    {
        wnd()->print_status_message("ERROR: The 'look at' operations works only at 'Translation' or 'Rotation' mode.", 10000);
        return;
    }
}

void  widgets::on_pause()
{
    bool const  simulation_paused = wnd()->glwindow().call_now(&simulator::paused);

    scene_tree()->setEnabled(simulation_paused);
    enable_coord_system_location_widgets(simulation_paused, true);
    if (simulation_paused)
        update_coord_system_location_widgets();
}


bool  widgets::is_editing_enabled() const
{
    return wnd()->glwindow().call_now(&simulator::paused);
}


void  widgets::update_coord_system_location_widgets()
{
    auto const selected_items = m_scene_tree->selectedItems();
    if (![&selected_items]() -> bool {
            if (selected_items.empty())
                return false;
            foreach(QTreeWidgetItem* const  item, selected_items)
                if (find_nearest_coord_system_item(item) != find_nearest_coord_system_item(selected_items.front()))
                    return false;
            return true;
            }())
    {
        enable_coord_system_location_widgets(false, false);
        return;
    }

    tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(selected_items.front());
    INVARIANT(tree_item != nullptr);

    enable_coord_system_location_widgets(true, !is_editing_enabled());
    std::string const  tree_item_name = get_name_of_active_coord_system_in_tree_widget(*m_scene_tree);
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, tree_item_name);
    refresh_text_in_coord_system_location_widgets(node_ptr);
}

void  widgets::enable_coord_system_location_widgets(bool const  state, bool const  read_only)
{
    m_coord_system_pos_x->setEnabled(state);
    m_coord_system_pos_y->setEnabled(state);
    m_coord_system_pos_z->setEnabled(state);

    m_coord_system_rot_w->setEnabled(state);
    m_coord_system_rot_x->setEnabled(state);
    m_coord_system_rot_y->setEnabled(state);
    m_coord_system_rot_z->setEnabled(state);

    m_coord_system_yaw->setEnabled(state);
    m_coord_system_pitch->setEnabled(state);
    m_coord_system_roll->setEnabled(state);

    if (state == false)
    {
        m_coord_system_pos_x->setText("");
        m_coord_system_pos_y->setText("");
        m_coord_system_pos_z->setText("");

        m_coord_system_rot_w->setText("");
        m_coord_system_rot_x->setText("");
        m_coord_system_rot_y->setText("");
        m_coord_system_rot_z->setText("");

        m_coord_system_yaw->setText("");
        m_coord_system_pitch->setText("");
        m_coord_system_roll->setText("");
    }
    else
    {
        m_coord_system_pos_x->setReadOnly(read_only);
        m_coord_system_pos_y->setReadOnly(read_only);
        m_coord_system_pos_z->setReadOnly(read_only);

        m_coord_system_rot_w->setReadOnly(read_only);
        m_coord_system_rot_x->setReadOnly(read_only);
        m_coord_system_rot_y->setReadOnly(read_only);
        m_coord_system_rot_z->setReadOnly(read_only);

        m_coord_system_yaw->setReadOnly(read_only);
        m_coord_system_pitch->setReadOnly(read_only);
        m_coord_system_roll->setReadOnly(read_only);
    }
}

void  widgets::refresh_text_in_coord_system_location_widgets(scn::scene_node_ptr const  node_ptr)
{
    ASSUMPTION(node_ptr != nullptr);

    auto const  coord_system_ptr = node_ptr->get_coord_system();
    m_coord_system_pos_x->setText(QString::number(coord_system_ptr->origin()(0)));
    m_coord_system_pos_y->setText(QString::number(coord_system_ptr->origin()(1)));
    m_coord_system_pos_z->setText(QString::number(coord_system_ptr->origin()(2)));

    refresh_text_in_coord_system_rotation_widgets(coord_system_ptr->orientation());
}

void  widgets::refresh_text_in_coord_system_rotation_widgets(quaternion const&  q)
{
    m_coord_system_rot_w->setText(QString::number(q.w()));
    m_coord_system_rot_x->setText(QString::number(q.x()));
    m_coord_system_rot_y->setText(QString::number(q.y()));
    m_coord_system_rot_z->setText(QString::number(q.z()));

    scalar  yaw, pitch, roll;
    rotation_to_yaw_pitch_roll(quaternion_to_rotation_matrix(q), yaw, pitch, roll);
    m_coord_system_yaw->setText(QString::number(yaw * 180.0f / PI()));
    m_coord_system_pitch->setText(QString::number(pitch * 180.0f / PI()));
    m_coord_system_roll->setText(QString::number(roll * 180.0f / PI()));
}


scn::scene_history_ptr  widgets::get_scene_history()
{
    scn::scene_history_ptr const  scene_history_ptr = wnd()->glwindow().call_now(&simulator::get_scene_history);
    if (m_save_commit_id == scn::get_invalid_scene_history_commit_id())
        m_save_commit_id = scene_history_ptr->get_active_commit_id();
    return scene_history_ptr;
}


void  widgets::set_window_title()
{
    wnd()->set_title(
        (!get_scene_history()->is_commit_valid(m_save_commit_id)
            || get_scene_history()->was_applied_mutator_since_commit(m_save_commit_id)
            ? "* " : "") +
        (wnd()->get_current_scene_dir().empty() ? "New scene" : wnd()->get_current_scene_dir().string())
        );
}


QWidget*  make_scene_tab_content(widgets const&  w)
{
    QWidget* const  scene_tab = new QWidget;
    {
        QVBoxLayout* const scene_tab_layout = new QVBoxLayout;
        {
            w.wnd()->glwindow().register_listener(
                        simulator_notifications::paused(),
                        { &program_window::scene_pause_listener, w.wnd() }
                        );

            {
                QTreeWidgetItem* headerItem = new QTreeWidgetItem();
                headerItem->setText(0, QString("Hierarchy"));
                w.scene_tree()->setHeaderItem(headerItem);
                w.wnd()->glwindow().register_listener(
                        simulator_notifications::scene_scene_selection_changed(),
                        { &program_window::scene_selection_listener, w.wnd() }
                        );
            }
            scene_tab_layout->addWidget(w.scene_tree());

            QWidget* const selected_group = new QGroupBox("Local coordinate system");
            {
                QVBoxLayout* const selected_layout = new QVBoxLayout();
                {
                    QWidget* const position_group = new QGroupBox("Position in meters [xyz]");
                    {
                        QHBoxLayout* const position_layout = new QHBoxLayout;
                        {
                            position_layout->addWidget(w.coord_system_pos_x());
                            position_layout->addWidget(w.coord_system_pos_y());
                            position_layout->addWidget(w.coord_system_pos_z());

                            w.wnd()->glwindow().register_listener(
                                        simulator_notifications::scene_node_position_update_started(),
                                        { &program_window::on_scene_coord_system_position_started, w.wnd() }
                                        );
                            w.wnd()->glwindow().register_listener(
                                        simulator_notifications::scene_node_position_updated(),
                                        { &program_window::scene_coord_system_position_listener, w.wnd() }
                                        );
                            w.wnd()->glwindow().register_listener(
                                        simulator_notifications::scene_node_position_update_finished(),
                                        { &program_window::on_scene_coord_system_position_finished, w.wnd() }
                                        );
                        }
                        position_group->setLayout(position_layout);
                    }
                    selected_layout->addWidget(position_group);

                    QWidget* const rotation_group = new QGroupBox("Rotation");
                    {
                        QVBoxLayout* const rotation_layout = new QVBoxLayout;
                        {
                            rotation_layout->addWidget(new QLabel("Quaternion [wxyz]"));
                            QHBoxLayout* const quaternion_layout = new QHBoxLayout;
                            {
                                quaternion_layout->addWidget(w.coord_system_rot_w());
                                quaternion_layout->addWidget(w.coord_system_rot_x());
                                quaternion_layout->addWidget(w.coord_system_rot_y());
                                quaternion_layout->addWidget(w.coord_system_rot_z());
                            }
                            rotation_layout->addLayout(quaternion_layout);

                            rotation_layout->addWidget(new QLabel("Tait-Bryan angles in degrees [yaw(z)-pitch(y')-roll(x'')]"));
                            QHBoxLayout* const tait_bryan_layout = new QHBoxLayout;
                            {
                                tait_bryan_layout->addWidget(w.coord_system_yaw());
                                tait_bryan_layout->addWidget(w.coord_system_pitch());
                                tait_bryan_layout->addWidget(w.coord_system_roll());
                            }
                            rotation_layout->addLayout(tait_bryan_layout);
                        }
                        rotation_group->setLayout(rotation_layout);

                        w.wnd()->glwindow().register_listener(
                                    simulator_notifications::scene_node_orientation_update_started(),
                                    { &program_window::on_scene_coord_system_rotation_started, w.wnd() }
                                    );
                        w.wnd()->glwindow().register_listener(
                                    simulator_notifications::scene_node_orientation_updated(),
                                    { &program_window::scene_coord_system_rotation_listener, w.wnd() }
                                    );
                        w.wnd()->glwindow().register_listener(
                                    simulator_notifications::scene_node_orientation_update_finished(),
                                    { &program_window::on_scene_coord_system_rotation_finished, w.wnd() }
                                    );
                    }
                    selected_layout->addWidget(rotation_group);
                }
                selected_group->setLayout(selected_layout);
            }
            scene_tab_layout->addWidget(selected_group);

            //scene_tab_layout->addStretch(1);
        }
        scene_tab->setLayout(scene_tab_layout);
    }
    return scene_tab;
}


}}
