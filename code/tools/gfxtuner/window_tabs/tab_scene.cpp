#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_bool_lock.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_id_reverse_builder.hpp>
#include <gfxtuner/window_tabs/tab_scene_records_integration.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_agent.hpp>
#include <gfxtuner/dialog/insert_name_dialog.hpp>
#include <gfxtuner/dialog/insert_number_dialog.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <scene/scene_utils.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <scene/scene_editing.hpp>
#include <angeo/axis_aligned_bounding_box.hpp>
#include <ai/skeleton_utils.hpp>
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
#include <locale>

namespace window_tabs { namespace tab_scene {


static natural_64_bit  g_new_coord_system_id_counter = 0ULL;


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)

    , m_scene_tree(new tree_widget(
            std::bind(&widgets::on_scene_hierarchy_item_selected, this),
            std::bind(&widgets::on_scene_hierarchy_item_update_action, this, std::placeholders::_1),
            std::bind(&widgets::on_scene_escape_widget, this)
            ))

    , m_node_icon((boost::filesystem::path{ get_program_options()->dataRoot() } /
                  "shared/gfx/icons/coord_system.png").string().c_str())
    , m_folder_icon((boost::filesystem::path{ get_program_options()->dataRoot() } /
                    "shared/gfx/icons/data_type.png").string().c_str())

    , m_insert_record_handlers()
    , m_update_record_handlers()
    , m_duplicate_record_handlers()
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
    , m_coord_system_axis_x_x(new QLineEdit)
    , m_coord_system_axis_x_y(new QLineEdit)
    , m_coord_system_axis_x_z(new QLineEdit)
    , m_coord_system_axis_y_x(new QLineEdit)
    , m_coord_system_axis_y_y(new QLineEdit)
    , m_coord_system_axis_y_z(new QLineEdit)
    , m_coord_system_axis_z_x(new QLineEdit)
    , m_coord_system_axis_z_y(new QLineEdit)
    , m_coord_system_axis_z_z(new QLineEdit)

    , m_coord_system_location_backup_buffer()
    , m_pending_scene_dir_to_load()
    , m_selected_tree_items_on_simulation_resumed_event()
{
    m_scene_tree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    m_scene_tree->setSortingEnabled(true);
    m_scene_tree->sortItems(0, Qt::SortOrder::AscendingOrder);
    enable_coord_system_location_widgets(false, false);

    scn::scene_history_coord_system_insert::set_undo_processor(
        [this](scn::scene_history_coord_system_insert const&  history_node) {
            std::vector<tree_widget_item*>  items_list;
            tree_widget_item* const  tree_item = m_scene_tree->find(history_node.get_id());
            ASSUMPTION(!tree_item->isSelected());
            erase_coord_system(history_node.get_id(), tree_item);
        });
    scn::scene_history_coord_system_insert::set_redo_processor(
        [this](scn::scene_history_coord_system_insert const& history_node) {
            tree_widget_item*  parent_tree_item = nullptr;
            if (!history_node.get_id().is_root())
            {
                ASSUMPTION(m_scene_tree->find(history_node.get_id()) == nullptr);
                parent_tree_item = m_scene_tree->find(history_node.get_id().get_direct_parent_id());
                ASSUMPTION(represents_coord_system(parent_tree_item));
            }
            auto const  tree_node = insert_coord_system(
                    history_node.get_id(),
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
                    history_node.get_id(),
                    history_node.get_old_origin(),
                    history_node.get_old_orientation()
                    );
            if (is_active_coord_system_in_tree_widget(*m_scene_tree, history_node.get_id()))
                update_coord_system_location_widgets();
        });
    scn::scene_history_coord_system_relocate::set_redo_processor(
        [this](scn::scene_history_coord_system_relocate const&  history_node) {
            m_wnd->glwindow().call_now(
                    &simulator::relocate_scene_node,
                    history_node.get_id(),
                    history_node.get_new_origin(),
                    history_node.get_new_orientation()
                    );
            if (is_active_coord_system_in_tree_widget(*m_scene_tree, history_node.get_id()))
                update_coord_system_location_widgets();
        });

    scn::scene_history_coord_system_insert_to_selection::set_undo_processor(
        [this](scn::scene_history_coord_system_insert_to_selection const&  history_node) {
            tree_widget_item* const  tree_item = m_scene_tree->find(history_node.get_id());
            ASSUMPTION(represents_coord_system(tree_item));
            ASSUMPTION(tree_item->isSelected());
            tree_item->setSelected(false);
            std::unordered_set<scn::scene_node_id>  selected_scene_nodes{ history_node.get_id() };
            std::unordered_set<scn::scene_record_id>  selected_records;
            m_wnd->glwindow().call_now(&simulator::erase_from_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));
            update_coord_system_location_widgets();
        });
    scn::scene_history_coord_system_insert_to_selection::set_redo_processor(
        [this](scn::scene_history_coord_system_insert_to_selection const&  history_node) {
            tree_widget_item* const  tree_item = m_scene_tree->find(history_node.get_id());
            ASSUMPTION(represents_coord_system(tree_item));
            ASSUMPTION(!tree_item->isSelected());
            tree_item->setSelected(true);
            m_scene_tree->scrollToItem(tree_item);
            std::unordered_set<scn::scene_node_id>  selected_scene_nodes{ history_node.get_id() };
            std::unordered_set<scn::scene_record_id>  selected_records;
            m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));
            update_coord_system_location_widgets();
        });

    scn::scene_history_record_insert_to_selection::set_undo_processor(
        [this](scn::scene_history_record_insert_to_selection const&  history_node) {
            auto const  record_item = m_scene_tree->find(history_node.get_id());
            ASSUMPTION(record_item != nullptr && record_item->isSelected());
            record_item->setSelected(false);
            std::unordered_set<scn::scene_node_id>  selected_scene_nodes;
            std::unordered_set<scn::scene_record_id>  selected_records{ history_node.get_id() };
            m_wnd->glwindow().call_now(&simulator::erase_from_scene_selection, std::ref(selected_scene_nodes), std::ref(selected_records));
            update_coord_system_location_widgets();
        });
    scn::scene_history_record_insert_to_selection::set_redo_processor(
        [this](scn::scene_history_record_insert_to_selection const&  history_node) {
            auto const  record_item = m_scene_tree->find(history_node.get_id());
            ASSUMPTION(record_item != nullptr && !record_item->isSelected());
            record_item->setSelected(true);
            m_scene_tree->scrollToItem(record_item);
            std::unordered_set<scn::scene_node_id>  selected_scene_nodes;
            std::unordered_set<scn::scene_record_id>  selected_records{ history_node.get_id() };
            m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::ref(selected_scene_nodes), std::ref(selected_records));
            update_coord_system_location_widgets();
        });

    register_record_icons(m_icons_of_records);
    register_record_undo_redo_processors(this);
    register_record_handler_for_insert_scene_record(m_insert_record_handlers);
    register_record_handler_for_update_scene_record(m_update_record_handlers);
    register_record_handler_for_duplicate_scene_record(m_duplicate_record_handlers);
    register_record_handler_for_erase_scene_record(m_erase_record_handlers);
    register_record_handler_for_load_scene_record(m_load_record_handlers);
    register_record_handler_for_save_scene_record(m_save_record_handlers);
}


void  widgets::add_tree_item_to_selection(tree_widget_item* const  item)
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
        std::unordered_set<scn::scene_node_id>  selected_scene_nodes;
        std::unordered_set<scn::scene_record_id>  selected_records;
        wnd()->glwindow().call_now(&simulator::get_scene_selection, std::ref(selected_scene_nodes), std::ref(selected_records));
        for (auto const& node_name : selected_scene_nodes)
        {
            tree_widget_item* const  item_ptr = m_scene_tree->find(node_name);
            INVARIANT(item_ptr != nullptr && represents_coord_system(item_ptr));
            old_selection.push_back(item_ptr);
        }
        for (scn::scene_record_id const&  record_id : selected_records)
        {
            tree_widget_item* const  item_ptr = m_scene_tree->find(record_id);
            INVARIANT(item_ptr != nullptr && represents_record(item_ptr));
            old_selection.push_back(item_ptr);
        }
    }
    std::unordered_set<scn::scene_node_id>  selected_scene_nodes;
    std::unordered_set<scn::scene_record_id>  selected_records;
    QList<QTreeWidgetItem*>  new_selection;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = as_tree_widget_item(item);
        INVARIANT(tree_item != nullptr);

        auto const  id_builder = scene_record_id_reverse_builder::run(tree_item);
        if (represents_coord_system(tree_item))
            selected_scene_nodes.insert(id_builder.get_node_id());
        else if (represents_record(tree_item))
            selected_records.insert(id_builder.get_record_id());

        if (!represents_folder(tree_item))
            new_selection.push_back(tree_item);
    }
    wnd()->glwindow().call_now(&simulator::set_scene_selection, selected_scene_nodes, selected_records);
    
    update_coord_system_location_widgets();

    update_history_according_to_change_in_selection(old_selection, new_selection, get_scene_history());
    set_window_title();
}

void  widgets::on_simulator_started()
{
    clear_scene();

    if (get_program_options()->has_scene_dir())
        m_pending_scene_dir_to_load = get_program_options()->scene_dir();
}


void  widgets::process_pending_scene_load_requst_if_any()
{
    if (!m_pending_scene_dir_to_load.empty())
    {
        boost::filesystem::path  scene_dir = m_pending_scene_dir_to_load;
        m_pending_scene_dir_to_load.clear();

        if (!boost::filesystem::is_directory(scene_dir))
            scene_dir = get_program_options()->dataRoot() / scene_dir;
        if (boost::filesystem::is_directory(scene_dir) && boost::filesystem::is_regular_file(scene_dir / "hierarchy.info"))
        {
            wnd()->get_current_scene_dir() = canonical_path(scene_dir);
            open_scene(wnd()->get_current_scene_dir());
        }
        else
        {
            clear_scene();
            wnd()->print_status_message("ERROR: Scene directory passed via command-line is wrong.", 10000);
        }
    }
}


void  widgets::on_scene_hierarchy_item_update_action(tree_widget_item* const  item)
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

    std::unordered_set<scn::scene_node_id>  selected_scene_nodes;
    std::unordered_set<scn::scene_record_id>  selected_records;
    wnd()->glwindow().call_now(&simulator::get_scene_selection, std::ref(selected_scene_nodes), std::ref(selected_records));

    auto const  recover_from_failure = [this]() -> void {
        m_scene_tree->clearSelection();
        wnd()->glwindow().call_now(
                &simulator::set_scene_selection,
                std::unordered_set<scn::scene_node_id>(),
                std::unordered_set<scn::scene_record_id>()
                );
        update_coord_system_location_widgets();
        wnd()->print_status_message("ERROR: Detected inconsistency between the simulator and GUI in selection. "
                                    "Clearing the selection.", 10000);
    };

    m_scene_tree->clearSelection();
    for (auto const&  node_name : selected_scene_nodes)
    {
        tree_widget_item* const  item_ptr = m_scene_tree->find(node_name);
        if (item_ptr ==nullptr || !represents_coord_system(item_ptr))
        {
            recover_from_failure();
            return;
        }
        add_tree_item_to_selection(item_ptr);
    }
    for (scn::scene_record_id const&  record_id : selected_records)
    {
        tree_widget_item* const  item_ptr = m_scene_tree->find(record_id);
        if (item_ptr == nullptr || !represents_record(item_ptr))
        {
            recover_from_failure();
            return;
        }
        add_tree_item_to_selection(item_ptr);
    }
    update_coord_system_location_widgets();

    QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
    update_history_according_to_change_in_selection(old_selection, new_selection, get_scene_history());
    set_window_title();
}


tree_widget_item*  widgets::insert_coord_system(
            scn::scene_node_id const&  id,
            vector3 const&  origin,
            quaternion const&  orientation,
            tree_widget_item* const  parent_tree_item
            )
{
    wnd()->glwindow().call_now(&simulator::insert_scene_node_at, id, origin, orientation);
    return insert_coord_system_to_tree_widget(m_scene_tree, id, origin, orientation, m_node_icon, parent_tree_item);
}

void  widgets::erase_coord_system(scn::scene_node_id const&  id, tree_widget_item* const  tree_item)
{
    ASSUMPTION(represents_coord_system(tree_item));
    ASSUMPTION(correspond(id, tree_item));
    m_wnd->glwindow().call_now(&simulator::erase_scene_node, id);
    m_scene_tree->erase(tree_item);
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
    tree_widget_item*  parent_tree_item = nullptr;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item*  tree_item = as_tree_widget_item(item);
        INVARIANT(tree_item != nullptr);
        if (!represents_coord_system(tree_item))
            tree_item = find_nearest_coord_system_item(tree_item);
        if (scene_record_id_reverse_builder::run(tree_item).get_node_id() == scn::get_pivot_node_id())
            use_pivot = true;
        else
        {
            if (parent_tree_item != nullptr && tree_item != parent_tree_item)
            {
                wnd()->print_status_message("ERROR: Insertion has FAILED. Ambiguous non-pivot coord system.", 10000);
                return;
            }
            parent_tree_item = tree_item;
        }
    }

    scn::scene_node_id const  parent_item_id =
        parent_tree_item == nullptr ? scn::scene_node_id() : scene_record_id_reverse_builder::run(parent_tree_item).get_node_id();
    natural_64_bit  old_counter;
    scn::scene_node_id  inserted_item_id;
    std::string  name;
    do
    {
        name = msgstream() << "coord_system_" << g_new_coord_system_id_counter;
        inserted_item_id = parent_item_id / name;
        old_counter = g_new_coord_system_id_counter;
        ++g_new_coord_system_id_counter;
    }
    while (wnd()->glwindow().call_now(&simulator::get_scene_node, inserted_item_id) != nullptr);
    dialog::insert_name_dialog  dlg(wnd(), name,
        [this, &parent_item_id](std::string const&  name) {
            return !dialog::is_scene_forbidden_name(name) &&
                   wnd()->glwindow().call_now(&simulator::get_scene_node, parent_item_id / name) == nullptr;
        });
    dlg.exec();
    if (!dlg.get_name().empty())
    {
        QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();
        m_scene_tree->clearSelection();
        wnd()->glwindow().call_now(
                &simulator::set_scene_selection,
                std::unordered_set<scn::scene_node_id>(),
                std::unordered_set<scn::scene_record_id>()
                );

        vector3  origin = vector3_zero();
        quaternion  orientation = quaternion_identity();
        if (use_pivot)
        {
            scn::scene_node_ptr const  pivot = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_id());
            origin = pivot->get_coord_system()->origin();
            orientation = pivot->get_coord_system()->orientation();
            if (parent_tree_item != nullptr)
                transform_origin_and_orientation_from_world_to_scene_node(
                        wnd()->glwindow().call_now(&simulator::get_scene_node, parent_item_id),
                        origin,
                        orientation
                        );
        }

        scn::scene_node_id const  coord_system_id = parent_item_id / dlg.get_name();

        auto const  tree_item = insert_coord_system(coord_system_id, origin, orientation, parent_tree_item);

        std::unordered_set<scn::scene_node_id>  selected_scene_nodes{ coord_system_id };
        std::unordered_set<scn::scene_record_id>  selected_records;
        m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));

        get_scene_history()->insert<scn::scene_history_coord_system_insert>(
                coord_system_id,
                origin,
                orientation,
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
        scn::scene_node_id const  node_id = scene_record_id_reverse_builder::run(tree_item).get_node_id();
        if (node_id == scn::get_pivot_node_id())
            continue;
        tree_widget_item* const  folder_item = m_scene_tree->find(node_id, record_kind);
        if (folder_item != nullptr)
            for (int j = 0, m = folder_item->childCount(); j != m; ++j)
            {
                auto const  record_item = as_tree_widget_item(folder_item->child(j));
                INVARIANT(represents_record(record_item));
                std::string const  record_item_name = get_tree_widget_item_name(record_item);
                used_names.insert(record_item_name);
            }
        nodes.insert(tree_item);
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
        dialog::insert_name_dialog  dlg(wnd(), record_name,
            [&used_names](std::string const&  name) {
            return !dialog::is_scene_forbidden_name(name) && used_names.count(name) == 0UL;
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
            scn::scene_node_id const  coord_system_id = scene_record_id_reverse_builder::run(tree_item).get_node_id();
            scn::scene_record_id const  record_id{ coord_system_id, record_kind, record_name };

            auto const  record_item =
                    insert_record_to_tree_widget(m_scene_tree, record_id, m_icons_of_records.at(record_kind), m_folder_icon);

            record_name_and_system_inserted.second(record_id);

            std::unordered_set<scn::scene_node_id>  selected_scene_nodes;
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

    std::unordered_set<tree_widget_item*>  to_erase_items;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        if (scene_record_id_reverse_builder::run(item).get_node_id() == scn::get_pivot_node_id())
        {
            wnd()->print_status_message("ERROR: Cannot erase '@pivot' coordinate system.", 10000);
            return;
        }
        if (scene_record_id_reverse_builder::run(item).get_node_id().path().front().front() == '@')
        {
            wnd()->print_status_message("ERROR: Cannot erase simulation node (starting with '@') nor any of its children.", 10000);
            return;
        }
        to_erase_items.insert(as_tree_widget_item(item));
    }

    std::unordered_set<void*>  erased_items;
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

void  widgets::erase_subtree_at_root_item(tree_widget_item* const  root_item, std::unordered_set<void*>&  erased_items, bool const  is_root)
{
    TMPROF_BLOCK();

    tree_widget_item* const  item = as_tree_widget_item(root_item);
    INVARIANT(item != nullptr);
    tree_widget_item*  folder_item = nullptr;
    if (item->represents_coord_system())
    {
        while (item->childCount() > 0)
            erase_subtree_at_root_item(as_tree_widget_item(item->child(0)), erased_items, false);

        scn::scene_node_id const  coord_system_id = scene_record_id_reverse_builder::run(item).get_node_id();

        if (item->isSelected())
            get_scene_history()->insert<scn::scene_history_coord_system_insert_to_selection>(coord_system_id, true);
        scn::scene_node_ptr const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
        INVARIANT(node_ptr != nullptr);
        get_scene_history()->insert<scn::scene_history_coord_system_insert>(
                coord_system_id,
                node_ptr->get_coord_system()->origin(),
                node_ptr->get_coord_system()->orientation(),
                true
                );

        std::unordered_set<scn::scene_node_id>  selected_scene_nodes{ coord_system_id };
        std::unordered_set<scn::scene_record_id>  selected_records;
        wnd()->glwindow().call_now(&simulator::erase_from_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));
        wnd()->glwindow().call_now(&simulator::erase_scene_node, coord_system_id);
    }
    else if (represents_folder(item))
    {
        while (item->childCount() > 0)
            erase_subtree_at_root_item(as_tree_widget_item(item->child(0)), erased_items, false);
    }
    else
    {
        scn::scene_record_id const  id = scene_record_id_reverse_builder::run(item).get_record_id();

        if (item->isSelected())
            get_scene_history()->insert<scn::scene_history_record_insert_to_selection>(id, true);

        std::unordered_set<scn::scene_node_id>  selected_scene_nodes;
        std::unordered_set<scn::scene_record_id>  selected_records{ id };
        wnd()->glwindow().call_now(&simulator::erase_from_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));

        erase_scene_record(id);

        folder_item = as_tree_widget_item(item->parent());
        INVARIANT(represents_folder(folder_item));
    }

    m_scene_tree->erase(item);
    erased_items.insert(item);

    if (is_root && folder_item != nullptr && folder_item->childCount() == 0)
    {
        tree_widget_item* const  node_item = as_tree_widget_item(folder_item->parent());
        INVARIANT(represents_coord_system(node_item));

        m_scene_tree->erase(folder_item);
        erased_items.insert(folder_item);
    }
}


void  widgets::duplicate_subtree(
        tree_widget_item const* const  source_item,
        tree_widget_item* const  target_item,
        tree_widgent_items_cache const&  tree_item_children_cache
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(represents_coord_system(source_item) && represents_coord_system(target_item));
    scn::scene_node_id const  source_coord_system_id = scene_record_id_reverse_builder::run(source_item).get_node_id();
    scn::scene_node_id const  target_coord_system_id = scene_record_id_reverse_builder::run(target_item).get_node_id();
    std::string const  source_item_name = get_tree_widget_item_name(source_item);
    std::string const  target_item_name = get_tree_widget_item_name(target_item);
    for (auto const  item : tree_item_children_cache.at(source_item))
    {
        std::string const  item_name = get_tree_widget_item_name(item);
        if (represents_coord_system(item))
        {
            scn::scene_node_const_ptr const  node =
                    wnd()->glwindow().call_now(&simulator::get_scene_node, source_coord_system_id / item_name);
            INVARIANT(correspond(node->get_id(), item));
            auto const  duplicate_item =
                    insert_coord_system(
                            target_coord_system_id / item_name,
                            node->get_coord_system()->origin(),
                            node->get_coord_system()->orientation(),
                            target_item
                            );
            get_scene_history()->insert<scn::scene_history_coord_system_insert>(
                    target_coord_system_id / item_name,
                    node->get_coord_system()->origin(),
                    node->get_coord_system()->orientation(),
                    false
                    );
            duplicate_subtree(item, duplicate_item, tree_item_children_cache);
        }
    }
    for (auto const item : tree_item_children_cache.at(source_item))
    {
        std::string const  item_name = get_tree_widget_item_name(item);

        if (represents_coord_system(item))
            continue;
        INVARIANT(represents_folder(item));

        for (auto const record_item : tree_item_children_cache.at(item))
        {
            INVARIANT(represents_record(record_item));
            std::string const  record_item_name = get_tree_widget_item_name(record_item);
            scn::scene_record_id  src_record_id{ source_coord_system_id, item_name, record_item_name };
            scn::scene_record_id  duplicated_record_id{ target_coord_system_id, item_name, record_item_name };
            m_duplicate_record_handlers.at(item_name)(this, src_record_id, duplicated_record_id);
            insert_record_to_tree_widget(m_scene_tree, duplicated_record_id, m_icons_of_records.at(item_name), m_folder_icon);
        }
    }
}


void  widgets::on_scene_duplicate_selected()
{
    TMPROF_BLOCK();

    ASSUMPTION(!processing_selection_change());
    lock_bool const  _(&m_processing_selection_change);

    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene editing is disabled.", 10000);
        return;
    }

    QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();
    std::unordered_map<tree_widget_item const*, scn::scene_node_const_ptr>  source_nodes;
    vector3  world_source_origin = vector3_zero();
    {
        if (old_selection.size() == 0)
        {
            wnd()->print_status_message("ERROR: No coordinate system node is selected for duplicaation.", 10000);
            return;
        }

        std::unordered_set<tree_widget_item const*>  selection;
        for (QTreeWidgetItem const*  item_ptr : old_selection)
            selection.insert(as_tree_widget_item(item_ptr));

        for (QTreeWidgetItem const* item_ptr : old_selection)
        {
            if (!represents_coord_system(item_ptr))
            {
                wnd()->print_status_message("ERROR: A selected scene object to duplicate is NOT a coordinate system.", 10000);
                return;
            }
            scn::scene_node_id const  item_id = scene_record_id_reverse_builder::run(item_ptr).get_node_id();
            if (scene_record_id_reverse_builder::run(item_ptr).get_node_id() == scn::get_pivot_node_id())
            {
                wnd()->print_status_message("ERROR: Cannot duplicate '@pivot' coordinate system.", 10000);
                return;
            }
            for (QTreeWidgetItem const* ptr = item_ptr->parent(); ptr != nullptr; ptr = ptr->parent())
                if (selection.count(as_tree_widget_item(ptr)) != 0UL)
                {
                    wnd()->print_status_message("ERROR: Sub-trees of selected coord. systems overlap.", 10000);
                    return;
                }
            scn::scene_node_const_ptr const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, item_id);
            source_nodes.insert({ as_tree_widget_item(item_ptr), node_ptr });
            world_source_origin += translation_vector(node_ptr->get_world_matrix());
        }
        world_source_origin /= source_nodes.size();
    }

    scn::scene_node_ptr const  pivot = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_id());
    vector3 const  world_pivot_origin = pivot->get_coord_system()->origin();
    quaternion const  world_pivot_orientation = pivot->get_coord_system()->orientation();

    dialog::insert_number_dialog  dlg(wnd(), "Duplications count", 1, 1, 10000, true);
    if (dlg.exec() == 0)
        return;
    natural_32_bit const  num_copies = (natural_32_bit)dlg.get_value();

    m_scene_tree->clearSelection();
    wnd()->glwindow().call_now(
            &simulator::set_scene_selection,
            std::unordered_set<scn::scene_node_id>(),
            std::unordered_set<scn::scene_record_id>()
            );

    auto const choose_name =
        [this](std::string const&  orig_name, scn::scene_node_id const&  parent_item_id) -> std::string {
            natural_64_bit  counter = 1UL;
            std::string  base = orig_name;
            {
                std::string  counter_text;
                while (!base.empty() && std::isdigit(base.back(), std::locale::classic()))
                {
                    counter_text.push_back(base.back());
                    base.pop_back();
                }
                if (!base.empty() && base.back() != '_' && base.back() != '#')
                    base.push_back('_');
                if (!counter_text.empty())
                {
                    std::reverse(counter_text.begin(), counter_text.end());
                    counter = std::atol(counter_text.c_str());
                }
            }
            std::string  name;
            do
            {
                name = msgstream() << base << counter;
                ++counter;
            } while (wnd()->glwindow().call_now(&simulator::get_scene_node, parent_item_id / name) != nullptr);
            return name;
        };

    for (auto const&  item_and_source_node : source_nodes)
    {
        tree_widget_item const* const  source_item = item_and_source_node.first;
        scn::scene_node_const_ptr const  source_node = item_and_source_node.second;
        
        tree_widget_item*  parent_tree_item = as_tree_widget_item(source_item->parent());
        scn::scene_node_const_ptr const  parent_node = source_node->has_parent() ? source_node->get_parent() : nullptr;
        scn::scene_node_id const  parent_item_id = source_node->get_id().get_direct_parent_id();

        INVARIANT((parent_tree_item == nullptr) == (parent_node == nullptr));

        std::string  base_name = choose_name(source_node->get_name(), parent_item_id);

        if (source_nodes.size() == 1UL)
        {
            dialog::insert_name_dialog  dlg(wnd(), base_name,
                [this, &parent_item_id](std::string const&  name) {
                return !dialog::is_scene_forbidden_name(name) &&
                       wnd()->glwindow().call_now(&simulator::get_scene_node, parent_item_id / name) == nullptr;
            });
            if (dlg.exec() == 0)
                return;
            base_name = dlg.get_name();
            if (base_name.empty())
                return;
        }

        vector3  pivot_origin = world_pivot_origin;
        quaternion  pivot_orientation = world_pivot_orientation;
        vector3  source_origin = world_source_origin;
        if (parent_node != nullptr)
        {
            matrix44 const  to_parent_space = inverse44(parent_node->get_world_matrix());
            scn::transform_origin_and_orientation(to_parent_space, pivot_origin, pivot_orientation);
            source_origin = transform_point(world_source_origin, to_parent_space);
        }
        vector3 const  origin_shift = pivot_origin - source_origin;

        tree_widgent_items_cache  tree_item_children_cache;
        build_cache_from_item_to_its_direct_children(source_item, tree_item_children_cache);

        for (natural_32_bit  i = 0U; i != num_copies; ++i)
        {
            std::string const  name = i == 0U ? base_name : choose_name(base_name, parent_item_id);

            vector3 const  shifted_origin = source_node->get_coord_system()->origin() + (float_32_bit)(i + 1U) * origin_shift;
            auto const  tree_item = insert_coord_system(parent_item_id / name, shifted_origin, pivot_orientation, parent_tree_item);
            get_scene_history()->insert<scn::scene_history_coord_system_insert>(
                    parent_item_id / name,
                    shifted_origin,
                    pivot_orientation,
                    false
                    );
            duplicate_subtree(source_item, tree_item, tree_item_children_cache);

            std::unordered_set<scn::scene_node_id>  selected_scene_nodes{ parent_item_id / name };
            std::unordered_set<scn::scene_record_id>  selected_records;
            m_wnd->glwindow().call_now(&simulator::insert_to_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));

            add_tree_item_to_selection(tree_item);
        }
    }

    update_history_according_to_change_in_selection(old_selection, m_scene_tree->selectedItems(), get_scene_history(), false);

    get_scene_history()->commit();
    set_window_title();

    update_coord_system_location_widgets();
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

    insert_coord_system(scn::get_pivot_node_id(), vector3_zero(), quaternion_identity(), nullptr);

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


tree_widget_item*  widgets::load_scene_node(
        scn::scene_node_id const&  id,
        boost::property_tree::ptree const&  node_tree,
        tree_widget_item*  parent_item
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

    if (id == scn::get_pivot_node_id())
    {
        scn::scene_node_ptr const  pivot = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_id());
        wnd()->glwindow().call_now(&simulator::relocate_scene_node, scn::get_pivot_node_id(), origin, orientation);
        return nullptr;
    }

    tree_widget_item* const  current_node_item = insert_coord_system(id, origin, orientation, parent_item);

    boost::property_tree::ptree const&  children = node_tree.find("children")->second;
    for (auto it = children.begin(); it != children.end(); ++it)
        load_scene_node(id / it->first, it->second, current_node_item);

    boost::property_tree::ptree const&  folders = node_tree.find("folders")->second;
    for (auto folder_it = folders.begin(); folder_it != folders.end(); ++folder_it)
        for (auto record_it = folder_it->second.begin(); record_it != folder_it->second.end(); ++record_it)
            load_scene_record({ id, folder_it->first, record_it->first }, record_it->second);

    return current_node_item;
}

void  widgets::open_scene(boost::filesystem::path const&  scene_root_dir)
{
    TMPROF_BLOCK();

    if (!is_editing_enabled())
    {
        wnd()->print_status_message("ERROR: Scene load is disabled.", 10000);
        return;
    }

    wnd()->print_status_message("Clearing the old scene ...", 10000);

    clear_scene();

    wnd()->print_status_message("Loading scene '" + scene_root_dir.string() + "' ...", 10000);

    try
    {
        boost::property_tree::ptree  load_tree;
        boost::property_tree::read_info((scene_root_dir / "hierarchy.info").string(), load_tree);

        for (auto  it = load_tree.begin(); it != load_tree.end(); ++it)
            load_scene_node(scn::scene_node_id(it->first), it->second, nullptr);
        //m_scene_tree->expandAll();
        wnd()->set_title(scene_root_dir.string());
        wnd()->print_status_message("Loading of the scene has finished.", 10000);
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
        tree_widget_item* const  node_item_ptr,
        boost::property_tree::ptree& save_tree
        )
{
    ASSUMPTION(node_item_ptr != nullptr);

    scn::scene_node_id const  coord_system_id = scene_record_id_reverse_builder::run(node_item_ptr).get_node_id();
    boost::property_tree::path const  node_name_path(get_tree_widget_item_name(node_item_ptr), '/');
    scn::scene_node_ptr const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
    
    {
        boost::property_tree::ptree  origin_tree;
        vector3 const&  origin = node_ptr->get_coord_system()->origin();
        origin_tree.put("x", origin(0));
        origin_tree.put("y", origin(1));
        origin_tree.put("z", origin(2));
        save_tree.put_child(node_name_path / "origin", origin_tree);
    }
    {
        boost::property_tree::ptree  orientation_tree;
        vector4 const&  orientation = quaternion_coefficients_xyzw(node_ptr->get_coord_system()->orientation());
        orientation_tree.put("x", orientation(0));
        orientation_tree.put("y", orientation(1));
        orientation_tree.put("z", orientation(2));
        orientation_tree.put("w", orientation(3));
        save_tree.put_child(node_name_path / "orientation", orientation_tree);
    }

    boost::property_tree::ptree  folders;
    boost::property_tree::ptree  children;
    for (int i = 0, n = node_item_ptr->childCount(); i != n; ++i)
        if (represents_coord_system(node_item_ptr->child(i)))
            save_scene_node(as_tree_widget_item(node_item_ptr->child(i)), children);
        else
        {
            tree_widget_item* const  folder_ptr = as_tree_widget_item(node_item_ptr->child(i));
            INVARIANT(represents_folder(folder_ptr));
            std::string const  folder_name = get_tree_widget_item_name(folder_ptr);
            boost::property_tree::path const  folder_name_path(folder_name, '/');

            boost::property_tree::ptree  records;
            for (int j = 0, m = folder_ptr->childCount(); j != m; ++j)
            {
                tree_widget_item* const  record_ptr = as_tree_widget_item(folder_ptr->child(j));
                INVARIANT(represents_record(record_ptr));
                std::string const  record_name = get_tree_widget_item_name(record_ptr);
                boost::property_tree::path const  record_name_path(record_name, '/');

                boost::property_tree::ptree  record;
                save_scene_record(node_ptr, { folder_name, record_name }, record);
                records.put_child(record_name_path, record);
            }
            folders.put_child(folder_name_path, records);
        }
    save_tree.put_child(node_name_path / "folders", folders);
    save_tree.put_child(node_name_path / "children", children);
}

void  widgets::save_scene(boost::filesystem::path const&  scene_root_dir)
{
    TMPROF_BLOCK();

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

    scn::scene_node_id const  coord_system_id =
            scene_record_id_reverse_builder::run(get_active_coord_system_item_in_tree_widget(*m_scene_tree)).get_node_id();
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
    INVARIANT(node_ptr != nullptr);
    if(length(pos - node_ptr->get_coord_system()->origin()) < 1e-4f)
    {
        wnd()->set_focus_to_glwindow();
        return;
    }

    wnd()->glwindow().call_later(&simulator::set_position_of_scene_node, coord_system_id, pos);

    get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
            coord_system_id,
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

    scn::scene_node_id const  coord_system_id =
            scene_record_id_reverse_builder::run(get_active_coord_system_item_in_tree_widget(*m_scene_tree)).get_node_id();
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
    INVARIANT(node_ptr != nullptr);
    if (length_4d(quaternion_coefficients_xyzw(q) -
                  quaternion_coefficients_xyzw(node_ptr->get_coord_system()->orientation())) < 1e-4f)
    {
        wnd()->set_focus_to_glwindow();
        return;
    }

    wnd()->glwindow().call_later(&simulator::set_orientation_of_scene_node, coord_system_id, q);

    get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
            coord_system_id,
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

    scn::scene_node_id const  coord_system_id =
            scene_record_id_reverse_builder::run(get_active_coord_system_item_in_tree_widget(*m_scene_tree)).get_node_id();
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
    INVARIANT(node_ptr != nullptr);
    if (length_4d(quaternion_coefficients_xyzw(q) -
                  quaternion_coefficients_xyzw(node_ptr->get_coord_system()->orientation())) < 1e-4f)
    {
        wnd()->set_focus_to_glwindow();
        return;
    }

    wnd()->glwindow().call_later(&simulator::set_orientation_of_scene_node, coord_system_id, q);

    get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
            coord_system_id,
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
        scn::scene_node_id const  coord_system_id = scene_record_id_reverse_builder::run(tree_item).get_node_id();
        auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
        INVARIANT(node_ptr != nullptr);
        m_coord_system_location_backup_buffer.insert({ coord_system_id, *node_ptr->get_coord_system() });
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

    tree_widget_item* const  tree_item = m_scene_tree->find(scn::get_pivot_node_id());
    ASSUMPTION(represents_coord_system(tree_item));
    std::unordered_set<scn::scene_node_id>  selected_scene_nodes{ scn::get_pivot_node_id() };
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
        scn::scene_node_id const  coord_system_id = scene_record_id_reverse_builder::run(tree_item).get_node_id();
        if (coord_system_id != scn::get_pivot_node_id())
        {
            auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
            INVARIANT(node_ptr != nullptr);
            nodes.insert(node_ptr);
        }
    }
    if (nodes.empty())
    {
        wnd()->print_status_message("ERROR: There is nothing selected to be translated to the '@pivot' origin.", 10000);
        return;
    }

    auto const  pivot_node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_id());
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
                    node_ptr->get_id(),
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
                node_ptr->get_id(),
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
        scn::scene_node_id const  coord_system_id = scene_record_id_reverse_builder::run(tree_item).get_node_id();
        if (coord_system_id != scn::get_pivot_node_id())
        {
            auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
            INVARIANT(node_ptr != nullptr);
            nodes.insert(node_ptr);
        }
    }
    if (nodes.empty())
    {
        wnd()->print_status_message("ERROR: There is nothing selected to be translated to the '@pivot' origin.", 10000);
        return;
    }

    auto const  pivot_node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, scn::get_pivot_node_id());
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
                pivot_node_ptr->get_id(),
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
                    pivot_node_ptr->get_id(),
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


void  widgets::on_scene_agent_reset_skeleton_pose()
{
    if (!is_editing_enabled())
        return;

    std::unordered_map<scn::scene_record_id, scn::skeleton_props_const_ptr>  agents;
    foreach(QTreeWidgetItem const*  item, m_scene_tree->selectedItems())
    {
        while (item->parent() != nullptr)
            item = item->parent();
        tree_widget_item const* const  tree_item = dynamic_cast<tree_widget_item const*>(item);
        INVARIANT(tree_item != nullptr && represents_coord_system(tree_item));
        scn::scene_node_id const  coord_system_id = scene_record_id_reverse_builder::run(tree_item).get_node_id();
        if (coord_system_id == scn::get_pivot_node_id())
            continue;
        scn::scene_record_id const  agent_id(coord_system_id, "agent", "instance");
        if (scene_tree()->find(agent_id) == nullptr)
            continue;
        scn::skeleton_props_const_ptr const  skeleton_props_ptr =
            wnd()->glwindow().call_now(&simulator::get_agent_info, coord_system_id);
        INVARIANT(skeleton_props_ptr != nullptr);
        agents.insert({ agent_id, skeleton_props_ptr });
    }
    if (agents.empty())
    {
        wnd()->print_status_message("WARNING: There is no agent selected => no skeleton pose was reset.", 10000);
        return;
    }
    for (auto const& id_and_props : agents)
        record_agent::reset_skeleton_joint_nodes_under_agent_node(id_and_props.first, id_and_props.second, this);
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
        if (represents_coord_system(tree_item))
        {
            scn::scene_node_id const  coord_system_id = scene_record_id_reverse_builder::run(tree_item).get_node_id();
            auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
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
            auto const  id_builder = scene_record_id_reverse_builder::run(tree_item, &coord_system_item);
            INVARIANT(represents_coord_system(coord_system_item));
            auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, id_builder.get_node_id());
            INVARIANT(node_ptr != nullptr);

            angeo::axis_aligned_bounding_box  bbox;
            if (scn::get_bbox(*node_ptr, id_builder.get_node_record_id(), bbox))
            {
                if (scn::has_agent(*node_ptr))
                {
                    // Position of the object is perhaps affected by agent's current
                    // skeletal animation. We thus only guess (estimate )a space where
                    // the object might possibly be...
                    float_32_bit const  radius = std::max(length(bbox.min_corner), length(bbox.max_corner));
                    for (int i = 0; i != 3; ++i)
                    {
                        bbox.min_corner(i) = -radius;
                        bbox.max_corner(i) = radius;
                    }
                }

                selection.push_back(angeo::transform_bbox(bbox, node_ptr->get_world_matrix()));
                visited_coord_systems.insert(coord_system_item);
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


void  widgets::rebuild_tree_widget_according_to_scene(
        scn::scene_node_ptr const  node_ptr,
        tree_widget_item* const  parent_tree_item
        )
{
    tree_widget_item* const  tree_item = insert_coord_system_to_tree_widget(
            m_scene_tree,
            node_ptr->get_id(),
            node_ptr->get_coord_system()->origin(),
            node_ptr->get_coord_system()->orientation(),
            m_node_icon,
            parent_tree_item
            );

    for (auto const& folder : node_ptr->get_folders())
        for (auto const&  record : folder.second.get_records())
            insert_record_to_tree_widget(
                    m_scene_tree,
                    scn::scene_record_id(node_ptr->get_id(), folder.first, record.first),
                    get_record_icon(folder.first),
                    m_folder_icon
                    );

    for (auto const& name_and_node : node_ptr->get_children())
        rebuild_tree_widget_according_to_scene(name_and_node.second, tree_item);
}


void  widgets::on_simulation_paused()
{
    TMPROF_BLOCK();

    scene_tree()->setEnabled(true);
    scene_tree()->clear();
    enable_coord_system_location_widgets(true, true);

    get_scene_history()->clear();
    m_save_commit_id = get_scene_history()->get_active_commit_id();

    scn::scene const&  scene_ref = wnd()->glwindow().call_now((scn::scene const& (simulator::*)() const)&simulator::get_scene);
    //insert_coord_system(scn::get_pivot_node_id(), vector3_zero(), quaternion_identity(), nullptr);
    for (auto const&  name_and_node : scene_ref.get_root_nodes())
        rebuild_tree_widget_according_to_scene(name_and_node.second, nullptr);

    set_window_title();

    g_new_coord_system_id_counter = 0UL;

    std::unordered_set<scn::scene_node_id>  selected_scene_nodes;
    std::unordered_set<scn::scene_record_id>  selected_records;
    for (auto const&  record_id : m_selected_tree_items_on_simulation_resumed_event)
        if (auto const  item_ptr = scene_tree()->find(record_id))
        {
            if (represents_coord_system(item_ptr))
                selected_scene_nodes.insert(record_id.get_node_id());
            else if (represents_record(item_ptr))
                selected_records.insert(record_id);
            item_ptr->setSelected(true);
            scene_tree()->scrollToItem(item_ptr);
        }
    m_selected_tree_items_on_simulation_resumed_event.clear();
    wnd()->glwindow().call_now(&simulator::insert_to_scene_selection, std::cref(selected_scene_nodes), std::cref(selected_records));

    update_coord_system_location_widgets();
}


void  widgets::on_simulation_resumed()
{
    for (auto  item_ptr : scene_tree()->selectedItems())
        m_selected_tree_items_on_simulation_resumed_event.push_back(scene_record_id_reverse_builder::run(item_ptr).get_record_id());

    scene_tree()->setEnabled(false);
    enable_coord_system_location_widgets(false, true);
}


void  widgets::simulation_nodes_changed_listener()
{
    if (!wnd()->glwindow().has_simulator())
        return;

    std::vector<tree_widget_item*>  items_to_erase;
    for (int i = 0, n = m_scene_tree->topLevelItemCount(); i != n; ++i)
    {
        tree_widget_item* const  item_ptr = as_tree_widget_item(m_scene_tree->topLevelItem(i));

        if (!represents_coord_system(item_ptr))
            continue;
        scn::scene_node_id const  node_id = scene_record_id_reverse_builder::run(item_ptr).get_node_id();
        if (node_id == scn::get_pivot_node_id() || node_id.path_element(0U).front() != '@')
            continue;
        if (wnd()->glwindow().call_now(&simulator::get_scene_node, std::cref(node_id)) != nullptr)
            continue;

        items_to_erase.push_back(item_ptr);
    }

    for (auto  item_ptr : items_to_erase)
        m_scene_tree->erase(item_ptr);
}


void  widgets::on_scene_escape_widget()
{
    wnd()->set_focus_to_glwindow(false);
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
    scn::scene_node_id const  coord_system_id =
            scene_record_id_reverse_builder::run(get_active_coord_system_item_in_tree_widget(*m_scene_tree)).get_node_id();
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, coord_system_id);
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

    m_coord_system_axis_x_x->setEnabled(state);
    m_coord_system_axis_x_y->setEnabled(state);
    m_coord_system_axis_x_z->setEnabled(state);
    m_coord_system_axis_y_x->setEnabled(state);
    m_coord_system_axis_y_y->setEnabled(state);
    m_coord_system_axis_y_z->setEnabled(state);
    m_coord_system_axis_z_x->setEnabled(state);
    m_coord_system_axis_z_y->setEnabled(state);
    m_coord_system_axis_z_z->setEnabled(state);

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

        m_coord_system_axis_x_x->setText("");
        m_coord_system_axis_x_y->setText("");
        m_coord_system_axis_x_z->setText("");
        m_coord_system_axis_y_x->setText("");
        m_coord_system_axis_y_y->setText("");
        m_coord_system_axis_y_z->setText("");
        m_coord_system_axis_z_x->setText("");
        m_coord_system_axis_z_y->setText("");
        m_coord_system_axis_z_z->setText("");
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

        m_coord_system_axis_x_x->setReadOnly(true);
        m_coord_system_axis_x_y->setReadOnly(true);
        m_coord_system_axis_x_z->setReadOnly(true);
        m_coord_system_axis_y_x->setReadOnly(true);
        m_coord_system_axis_y_y->setReadOnly(true);
        m_coord_system_axis_y_z->setReadOnly(true);
        m_coord_system_axis_z_x->setReadOnly(true);
        m_coord_system_axis_z_y->setReadOnly(true);
        m_coord_system_axis_z_z->setReadOnly(true);
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

    vector3 x, y, z;
    rotation_matrix_to_basis(quaternion_to_rotation_matrix(q), x, y, z);
    normalise(x);
    normalise(y);
    normalise(z);
    m_coord_system_axis_x_x->setText(QString::number(x(0)));
    m_coord_system_axis_x_y->setText(QString::number(x(1)));
    m_coord_system_axis_x_z->setText(QString::number(x(2)));
    m_coord_system_axis_y_x->setText(QString::number(y(0)));
    m_coord_system_axis_y_y->setText(QString::number(y(1)));
    m_coord_system_axis_y_z->setText(QString::number(y(2)));
    m_coord_system_axis_z_x->setText(QString::number(z(0)));
    m_coord_system_axis_z_y->setText(QString::number(z(1)));
    m_coord_system_axis_z_z->setText(QString::number(z(2)));
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
                        { &program_window::scene_listener_simulation_paused, w.wnd() }
                        );
            w.wnd()->glwindow().register_listener(
                        simulator_notifications::resumed(),
                        { &program_window::scene_listener_simulation_resumed, w.wnd() }
                        );

            w.wnd()->glwindow().register_listener(
                        simulator_notifications::scene_simulation_nodes_changed(),
                        { &program_window::scene_listener_simulation_nodes_changed, w.wnd() }
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
                    selected_layout->addWidget(new QLabel("Origin [xyz]"));
                    QHBoxLayout* const position_layout = new QHBoxLayout;
                    {
                        position_layout->addWidget(w.coord_system_pos_x());
                        position_layout->addWidget(w.coord_system_pos_y());
                        position_layout->addWidget(w.coord_system_pos_z());
                    }
                    selected_layout->addLayout(position_layout);

                    selected_layout->addWidget(new QLabel("Quaternion [wxyz]"));
                    QHBoxLayout* const quaternion_layout = new QHBoxLayout;
                    {
                        quaternion_layout->addWidget(w.coord_system_rot_w());
                        quaternion_layout->addWidget(w.coord_system_rot_x());
                        quaternion_layout->addWidget(w.coord_system_rot_y());
                        quaternion_layout->addWidget(w.coord_system_rot_z());
                    }
                    selected_layout->addLayout(quaternion_layout);

                    selected_layout->addWidget(new QLabel("Tait-Bryan angles in degrees [yaw(z)-pitch(y')-roll(x'')]"));
                    QHBoxLayout* const tait_bryan_layout = new QHBoxLayout;
                    {
                        tait_bryan_layout->addWidget(w.coord_system_yaw());
                        tait_bryan_layout->addWidget(w.coord_system_pitch());
                        tait_bryan_layout->addWidget(w.coord_system_roll());
                    }
                    selected_layout->addLayout(tait_bryan_layout);

                    selected_layout->addWidget(new QLabel("Basis vectors X, Y, and Z; for each coords [xyz]"));
                    QHBoxLayout*  axis_layout = new QHBoxLayout;
                    {
                        axis_layout->addWidget(w.get_coord_system_axis_x_x());
                        axis_layout->addWidget(w.get_coord_system_axis_x_y());
                        axis_layout->addWidget(w.get_coord_system_axis_x_z());
                    }
                    selected_layout->addLayout(axis_layout);
                    axis_layout = new QHBoxLayout;
                    {
                        axis_layout->addWidget(w.get_coord_system_axis_y_x());
                        axis_layout->addWidget(w.get_coord_system_axis_y_y());
                        axis_layout->addWidget(w.get_coord_system_axis_y_z());
                    }
                    selected_layout->addLayout(axis_layout);
                    axis_layout = new QHBoxLayout;
                    {
                        axis_layout->addWidget(w.get_coord_system_axis_z_x());
                        axis_layout->addWidget(w.get_coord_system_axis_z_y());
                        axis_layout->addWidget(w.get_coord_system_axis_z_z());
                    }
                    selected_layout->addLayout(axis_layout);

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
