#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/simulator_notifications.hpp>
#include <gfxtuner/scene_utils.hpp>
#include <gfxtuner/scene_edit_utils.hpp>
#include <gfxtuner/scene_history.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/std_pair_hash.hpp>
#include <utility/msgstream.hpp>
#include <utility/canonical_path.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <boost/property_tree/ptree.hpp>
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

namespace {


struct  lock_bool
{
    lock_bool(bool* ptr) : m_ptr(ptr) { if (m_ptr != nullptr) *m_ptr = true; }
    ~lock_bool() { if (m_ptr != nullptr) *m_ptr = false; }
private:
    bool* m_ptr;
};


struct  tree_widget_item : public QTreeWidgetItem
{
    explicit tree_widget_item(bool const  represents_coord_system)
        : QTreeWidgetItem()
        , m_represents_coord_system(represents_coord_system)
    {}

    bool  represents_coord_system() const { return m_represents_coord_system; }

private:
    
    bool  m_represents_coord_system;
};


struct  insert_name_dialog : public QDialog
{
    insert_name_dialog(program_window* const  wnd,
                       std::string const&  initial_name,
                       std::function<bool(std::string const&)> const&  is_name_valid)
        : QDialog(wnd)
        , m_wnd(wnd)
        , m_name_edit(
            [](insert_name_dialog* dlg, std::string const&  initial_name) {
                struct s : public QLineEdit {
                    s(insert_name_dialog* dlg, std::string const&  initial_name) : QLineEdit()
                    {
                        setText(QString(initial_name.c_str()));
                        QObject::connect(this, &QLineEdit::textChanged, dlg, &insert_name_dialog::on_name_changed);
                    }
                };
                return new s(dlg, initial_name);
            }(this, initial_name)
            )
        , m_name_taken_indicator(new QLabel("WARNING: The name is already in use."))
        , m_OK_button(
            [](insert_name_dialog* wnd) {
                struct Open : public QPushButton {
                    Open(insert_name_dialog* wnd) : QPushButton("OK")
                    {
                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        setDefault(true);
                        autoDefault();
                    }
                };
                return new Open(wnd);
            }(this)
            )
        , m_is_name_valid_function(is_name_valid)
        , m_name()
    {
        QVBoxLayout* const dlg_layout = new QVBoxLayout;
        {
            dlg_layout->addWidget(m_name_edit);
            dlg_layout->addWidget(m_name_taken_indicator);

            QHBoxLayout* const buttons_layout = new QHBoxLayout;
            {
                buttons_layout->addWidget(m_OK_button);
                buttons_layout->addWidget(
                    [](insert_name_dialog* wnd) {
                        struct Close : public QPushButton {
                            Close(insert_name_dialog* wnd) : QPushButton("Cancel")
                            {
                                QObject::connect(this, SIGNAL(released()), wnd, SLOT(reject()));
                            }
                        };
                        return new Close(wnd);
                    }(this)
                    );
                buttons_layout->addStretch(1);
            }
            dlg_layout->addLayout(buttons_layout);
        }
        this->setLayout(dlg_layout);
        this->setWindowTitle("Insert name");
        this->resize(300,100);

        on_name_changed(QString(initial_name.c_str()));
    }

    std::string const&  get_name() const { return m_name; }

    void  on_name_changed(QString const&  qname)
    {
        if (!m_is_name_valid_function(qtgl::to_string(qname)))
        {
            m_name_taken_indicator->setText(QString("WARNING: the name is already in use."));
            m_OK_button->setEnabled(false);
        }
        else
        {
            m_name_taken_indicator->setText(QString("OK: The name is fresh."));
            m_OK_button->setEnabled(true);
        }
    }

public slots:

    void  accept()
    {
        m_name = qtgl::to_string(m_name_edit->text());
        QDialog::accept();
    }

    void  reject()
    {
        QDialog::reject();
    }
    //
private:
    program_window*  m_wnd;
    QLineEdit*  m_name_edit;
    QLabel*  m_name_taken_indicator;
    QPushButton*  m_OK_button;
    std::function<bool(std::string const&)>  m_is_name_valid_function;
    std::string  m_name;
};


tree_widget_item*  get_active_coord_system_item_in_tree_widget(QTreeWidget const&  tree_widget)
{
    auto const selected_items = tree_widget.selectedItems();
    INVARIANT(
        !selected_items.empty() &&
        [&selected_items]() -> bool {
            foreach(QTreeWidgetItem* const  item, selected_items)
                if (item->parent() != selected_items.front()->parent())
                    return false;
                return true;
            }()
        );
    tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(selected_items.front());
    INVARIANT(tree_item != nullptr);
    if (tree_item->represents_coord_system())
        return tree_item;
    tree_widget_item* const  parent_tree_item = dynamic_cast<tree_widget_item*>(tree_item->parent());
    INVARIANT(parent_tree_item != nullptr && parent_tree_item->represents_coord_system());
    return parent_tree_item;
}

std::string  get_name_of_active_coord_system_in_tree_widget(QTreeWidget const&  tree_widget)
{
    return qtgl::to_string(get_active_coord_system_item_in_tree_widget(tree_widget)->text(0));
}


bool  is_active_coord_system_in_tree_widget(QTreeWidget const&  tree_widget, std::string const&  name)
{
    auto const selected_items = tree_widget.selectedItems();
    if (selected_items.size() != 1)
        return false;
    tree_widget_item*  tree_item = dynamic_cast<tree_widget_item*>(selected_items.front());
    INVARIANT(tree_item != nullptr);
    if (!tree_item->represents_coord_system())
        tree_item = dynamic_cast<tree_widget_item*>(tree_item->parent());
    INVARIANT(tree_item != nullptr && tree_item->represents_coord_system());
    std::string const  active_name = qtgl::to_string(tree_item->text(0));
    return name == active_name;
}


bool  update_history_according_to_change_in_selection(
    QList<QTreeWidgetItem*> const&  old_selection,
    QList<QTreeWidgetItem*> const&  new_selection,
    bool  apply_commit = true
    )
{
    std::unordered_set<QTreeWidgetItem*> const  old_set(old_selection.cbegin(), old_selection.cend());
    std::unordered_set<QTreeWidgetItem*> const  new_set(new_selection.cbegin(), new_selection.cend());
    bool  change = false;
    for (auto const  item_ptr : old_set)
        if (new_set.count(item_ptr) == 0UL)
        {
            change = true;
            tree_widget_item* const  item = dynamic_cast<tree_widget_item*>(item_ptr);
            INVARIANT(item != nullptr);
            std::string const  item_name = qtgl::to_string(item->text(0));
            if (item->represents_coord_system())
                get_scene_history().insert<scene_history_coord_system_insert_to_selection>(item_name,true);
            else
            {
                tree_widget_item* const  parent_item = dynamic_cast<tree_widget_item*>(item->parent());
                INVARIANT(parent_item != nullptr);
                INVARIANT(parent_item->represents_coord_system());
                std::string const  parent_item_name = qtgl::to_string(parent_item->text(0));
                get_scene_history().insert<scene_history_batch_insert_to_selection>(
                    std::pair<std::string, std::string>{ parent_item_name, item_name },
                    true
                    );
            }
        }
    for (auto const item_ptr : new_set)
        if (old_set.count(item_ptr) == 0UL)
        {
            change = true;
            tree_widget_item* const  item = dynamic_cast<tree_widget_item*>(item_ptr);
            INVARIANT(item != nullptr);
            std::string const  item_name = qtgl::to_string(item->text(0));
            if (item->represents_coord_system())
                get_scene_history().insert<scene_history_coord_system_insert_to_selection>(item_name, false);
            else
            {
                tree_widget_item* const  parent_item = dynamic_cast<tree_widget_item*>(item->parent());
                INVARIANT(parent_item != nullptr);
                INVARIANT(parent_item->represents_coord_system());
                std::string const  parent_item_name = qtgl::to_string(parent_item->text(0));
                get_scene_history().insert<scene_history_batch_insert_to_selection>(
                    std::pair<std::string, std::string>{ parent_item_name, item_name },
                    false
                    );
            }
        }
    if (change == true && apply_commit == true)
        get_scene_history().commit();

    return change;
}


natural_64_bit  g_new_coord_system_id_counter = 0ULL;


}

namespace window_tabs { namespace tab_scene {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)

    , m_scene_tree(
        [](program_window* wnd) {
            struct s : public QTreeWidget {
                s(program_window* wnd) : QTreeWidget()
                {
                    QObject::connect(
                        this,
                        SIGNAL(itemSelectionChanged()),
                        wnd,
                        SLOT(on_scene_hierarchy_item_selected())
                        );
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_node_icon((boost::filesystem::path{ get_program_options()->dataRoot() } /
                   "shared/gfx/icons/coord_system.png").string().c_str())
    , m_batch_icon((boost::filesystem::path{ get_program_options()->dataRoot() } /
                   "shared/gfx/icons/batch.png").string().c_str())

    , m_processing_selection_change(false)

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
    enable_coord_system_location_widgets(false);

    scene_history_coord_system_insert::set_undo_processor(
        [this](scene_history_coord_system_insert const&  history_node) {
            auto const  items_list = m_scene_tree->findItems(QString(history_node.get_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
            ASSUMPTION(items_list.size() == 1UL);
            tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(items_list.front());
            ASSUMPTION(!tree_item->isSelected());
            ASSUMPTION(tree_item->represents_coord_system());
            ASSUMPTION((tree_item->parent() == nullptr) == history_node.get_parent_name().empty());
            ASSUMPTION(tree_item->parent() == nullptr ||
                       qtgl::to_string(tree_item->parent()->text(0)) == history_node.get_parent_name());
            std::string const  tree_item_name = qtgl::to_string(tree_item->text(0));
            m_wnd->glwindow().call_now(&simulator::erase_scene_node, tree_item_name);
            auto const  taken_item = tree_item->parent() != nullptr ?
                tree_item->parent()->takeChild(tree_item->parent()->indexOfChild(tree_item)) :
                m_scene_tree->takeTopLevelItem(m_scene_tree->indexOfTopLevelItem(tree_item))
                ;
            INVARIANT(taken_item == tree_item); (void)taken_item;
            delete tree_item;
        });
    scene_history_coord_system_insert::set_redo_processor(
        [this](scene_history_coord_system_insert const& history_node) {
            ASSUMPTION(m_scene_tree->findItems(QString(history_node.get_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0).empty());
            auto const items_list = m_scene_tree->findItems(QString(history_node.get_parent_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
            ASSUMPTION(items_list.size() == 1UL);
            tree_widget_item* const  parent_tree_item = dynamic_cast<tree_widget_item*>(items_list.front());
            ASSUMPTION(parent_tree_item->represents_coord_system());
            insert_coord_system(
                    history_node.get_name(),
                    history_node.get_origin(),
                    history_node.get_orientation(),
                    parent_tree_item
                    );
    });

    scene_history_coord_system_relocate::set_undo_processor(
        [this](scene_history_coord_system_relocate const&  history_node) {
            m_wnd->glwindow().call_later(
                    &simulator::relocate_scene_node,
                    history_node.get_name(),
                    history_node.get_old_origin(),
                    history_node.get_old_orientation()
                    );
            if (is_active_coord_system_in_tree_widget(*m_scene_tree, history_node.get_name()))
                update_coord_system_location_widgets();
        });
    scene_history_coord_system_relocate::set_redo_processor(
        [this](scene_history_coord_system_relocate const&  history_node) {
            m_wnd->glwindow().call_later(
                    &simulator::relocate_scene_node,
                    history_node.get_name(),
                    history_node.get_new_origin(),
                    history_node.get_new_orientation()
                    );
            if (is_active_coord_system_in_tree_widget(*m_scene_tree, history_node.get_name()))
                update_coord_system_location_widgets();
        });

    scene_history_coord_system_insert_to_selection::set_undo_processor(
        [this](scene_history_coord_system_insert_to_selection const&  history_node) {
            auto const items_list = m_scene_tree->findItems(QString(history_node.get_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
            ASSUMPTION(items_list.size() == 1UL);
            tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(items_list.front());
            ASSUMPTION(tree_item->represents_coord_system());
            ASSUMPTION(tree_item->isSelected());
            tree_item->setSelected(false);
            bool const old_processing_selection_change = m_processing_selection_change;
            m_processing_selection_change = false;
            on_scene_hierarchy_item_selected();
            m_processing_selection_change = old_processing_selection_change;
        });
    scene_history_coord_system_insert_to_selection::set_redo_processor(
        [this](scene_history_coord_system_insert_to_selection const&  history_node) {
            auto const items_list = m_scene_tree->findItems(QString(history_node.get_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
            ASSUMPTION(items_list.size() == 1UL);
            tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(items_list.front());
            ASSUMPTION(tree_item->represents_coord_system());
            ASSUMPTION(!tree_item->isSelected());
            tree_item->setSelected(true);
            bool const old_processing_selection_change = m_processing_selection_change;
            m_processing_selection_change = false;
            on_scene_hierarchy_item_selected();
            m_processing_selection_change = old_processing_selection_change;
        });


    scene_history_batch_insert::set_undo_processor(
        [this](scene_history_batch_insert const&  history_node) {
            auto const  items_list = m_scene_tree->findItems(QString(history_node.get_coord_system_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
            ASSUMPTION(items_list.size() == 1UL);
            auto const  parent_item = dynamic_cast<tree_widget_item*>(items_list.front());
            ASSUMPTION(parent_item != nullptr && parent_item->represents_coord_system());
            for (int i = 0, n = parent_item->childCount(); i != n; ++i)
            {
                auto const  item = dynamic_cast<tree_widget_item*>(parent_item->child(i));
                INVARIANT(item != nullptr);
                std::string const  item_name = qtgl::to_string(item->text(0));
                if (!item->represents_coord_system() && item_name == history_node.get_batch_name())
                {
                    std::string const  parent_name = qtgl::to_string(parent_item->text(0));
                    m_wnd->glwindow().call_now(&simulator::erase_batch_from_scene_node, item_name, parent_name);
                    auto const taken_item = parent_item->takeChild(i);
                    INVARIANT(taken_item == item); (void)taken_item;
                    delete item;
                    return;
                }
            }
            UNREACHABLE();
        });
    scene_history_batch_insert::set_redo_processor(
        [this](scene_history_batch_insert const&  history_node) {
            auto const  items_list = m_scene_tree->findItems(QString(history_node.get_coord_system_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
            ASSUMPTION(items_list.size() == 1UL);
            auto const  parent_item = dynamic_cast<tree_widget_item*>(items_list.front());
            ASSUMPTION(parent_item != nullptr && parent_item->represents_coord_system());
            ASSUMPTION((
                [parent_item, &history_node]() -> bool {
                    for (int i = 0, n = parent_item->childCount(); i != n; ++i)
                    {
                        auto const  item = dynamic_cast<tree_widget_item*>(parent_item->child(i));
                        INVARIANT(item != nullptr);
                        std::string const  item_name = qtgl::to_string(item->text(0));
                        if (!item->represents_coord_system() && item_name == history_node.get_batch_name())
                            return false;
                    }
                    return true;
                }
            ()));
            std::string const  parent_name = qtgl::to_string(parent_item->text(0));
            m_wnd->glwindow().call_now(&simulator::insert_batch_to_scene_node, history_node.get_batch_name(), history_node.get_batch_pathname(), parent_name);
            std::unique_ptr<tree_widget_item>  tree_node(new tree_widget_item(false));
            tree_node->setText(0, QString(history_node.get_batch_name().c_str()));
            tree_node->setIcon(0, m_batch_icon);
            parent_item->addChild(tree_node.get());
            tree_node.release();
        });

    scene_history_batch_insert_to_selection::set_undo_processor(
        [this](scene_history_batch_insert_to_selection const&  history_node) {
            auto const  items_list = m_scene_tree->findItems(QString(history_node.get_coord_system_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
            ASSUMPTION(items_list.size() == 1UL);
            auto const  parent_item = dynamic_cast<tree_widget_item*>(items_list.front());
            ASSUMPTION(parent_item != nullptr && parent_item->represents_coord_system());
            std::string const  parent_name = qtgl::to_string(parent_item->text(0));
            for (int i = 0, n = parent_item->childCount(); i != n; ++i)
            {
                auto const  item = dynamic_cast<tree_widget_item*>(parent_item->child(i));
                INVARIANT(item != nullptr);
                std::string const  item_name = qtgl::to_string(item->text(0));
                if (!item->represents_coord_system() && item_name == history_node.get_batch_name())
                {
                    ASSUMPTION(item->isSelected());
                    item->setSelected(false);
                    bool const old_processing_selection_change = m_processing_selection_change;
                    m_processing_selection_change = false;
                    on_scene_hierarchy_item_selected();
                    m_processing_selection_change = old_processing_selection_change;
                    return;
                }
            }
            UNREACHABLE();
        });
    scene_history_batch_insert_to_selection::set_redo_processor(
        [this](scene_history_batch_insert_to_selection const&  history_node) {
            auto const  items_list = m_scene_tree->findItems(QString(history_node.get_coord_system_name().c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
            ASSUMPTION(items_list.size() == 1UL);
            auto const  parent_item = dynamic_cast<tree_widget_item*>(items_list.front());
            ASSUMPTION(parent_item != nullptr && parent_item->represents_coord_system());
            std::string const  parent_name = qtgl::to_string(parent_item->text(0));
            for (int i = 0, n = parent_item->childCount(); i != n; ++i)
            {
                auto const  item = dynamic_cast<tree_widget_item*>(parent_item->child(i));
                INVARIANT(item != nullptr);
                std::string const  item_name = qtgl::to_string(item->text(0));
                if (!item->represents_coord_system() && item_name == history_node.get_batch_name())
                {
                    ASSUMPTION(!item->isSelected());
                    item->setSelected(true);
                    bool const old_processing_selection_change = m_processing_selection_change;
                    m_processing_selection_change = false;
                    on_scene_hierarchy_item_selected();
                    m_processing_selection_change = old_processing_selection_change;
                    return;
                }
            }
            UNREACHABLE();
        });
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

    QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();

    std::unordered_set<std::string>  selected_scene_nodes;
    std::unordered_set<std::pair<std::string, std::string> >  selected_batches;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(item);
        INVARIANT(tree_item != nullptr);
        std::string const  tree_item_name = qtgl::to_string(tree_item->text(0));
        if (tree_item->represents_coord_system())
            selected_scene_nodes.insert(tree_item_name);
        else
        {
            tree_widget_item* const  parent_tree_item = dynamic_cast<tree_widget_item*>(tree_item->parent());
            INVARIANT(parent_tree_item != nullptr);
            INVARIANT(parent_tree_item->represents_coord_system());
            std::string const  parent_tree_item_name = qtgl::to_string(parent_tree_item->text(0));
            selected_batches.insert({ parent_tree_item_name, tree_item_name });
        }
    }
    wnd()->glwindow().call_now(&simulator::update_scene_selection, selected_scene_nodes, selected_batches);
    
    update_coord_system_location_widgets();

    QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
    update_history_according_to_change_in_selection(old_selection, new_selection);
}

void  widgets::selection_changed_listener()
{
    lock_bool const  _(&m_processing_selection_change);

    QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();

    std::unordered_set<std::string>  selected_scene_nodes;
    std::unordered_set<std::pair<std::string, std::string> >  selected_batches;
    wnd()->glwindow().call_now(&simulator::get_scene_selection_data, std::ref(selected_scene_nodes), std::ref(selected_batches));

    auto const  recover_from_failure = [this]() -> void {
        m_scene_tree->clearSelection();
        wnd()->glwindow().call_now(
                &simulator::update_scene_selection,
                std::unordered_set<std::string>(),
                std::unordered_set<std::pair<std::string, std::string> >()
                );
        update_coord_system_location_widgets();
        wnd()->print_status_message("ERROR: Detected inconsistency between the simulator and GUI in selection. "
                                    "Clearing the selection.", 10000);
    };
    auto const  is_coord_system_node = [](QTreeWidgetItem* const  item) -> bool {
        if (auto const ptr = dynamic_cast<tree_widget_item*>(item))
            return ptr->represents_coord_system();
        return false;
    };

    m_scene_tree->clearSelection();
    for (auto const&  node_name : selected_scene_nodes)
    {
        auto const  items_list = m_scene_tree->findItems(QString(node_name.c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
        if (items_list.size() != 1 || !is_coord_system_node(items_list.front()))
        {
            recover_from_failure();
            return;
        }
        add_tree_item_to_selection(items_list.front());
    }
    for (auto const& node_and_batch : selected_batches)
    {
        auto const  items_list = m_scene_tree->findItems(QString(node_and_batch.first.c_str()), Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive, 0);
        if (items_list.size() != 1 || !is_coord_system_node(items_list.front()))
        {
            recover_from_failure();
            return;
        }
        bool  batch_found = false;
        for (int i = 0, n = items_list.front()->childCount(); i != n; ++i)
        {
            auto const  item = dynamic_cast<tree_widget_item*>(items_list.front()->child(i));
            INVARIANT(item != nullptr);
            if (!item->represents_coord_system() && qtgl::to_string(item->text(0)) == node_and_batch.second)
            {
                add_tree_item_to_selection(item);
                batch_found = true;
                break;
            }
        }
        if (batch_found == false)
        {
            recover_from_failure();
            return;
        }
    }
    update_coord_system_location_widgets();

    QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
    update_history_according_to_change_in_selection(old_selection, new_selection);
}


QTreeWidgetItem*  widgets::insert_coord_system(
            std::string const&  name,
            vector3 const&  origin,
            quaternion const&  orientation,
            QTreeWidgetItem* const  parent_tree_item
            )
{
    std::unique_ptr<tree_widget_item>  tree_node(new tree_widget_item(true));
    tree_node->setText(0, QString(name.c_str()));
    tree_node->setIcon(0, m_node_icon);
    if (parent_tree_item == nullptr)
    {
        wnd()->glwindow().call_now(&simulator::insert_scene_node_at, name, origin, orientation);
        m_scene_tree->addTopLevelItem(tree_node.get());
    }
    else
    {
        wnd()->glwindow().call_now(
                &simulator::insert_child_scene_node_at,
                name,
                origin,
                orientation,
                qtgl::to_string(parent_tree_item->text(0))
                );
        parent_tree_item->addChild(tree_node.get());
    }
    return tree_node.release();
}

void  widgets::on_scene_insert_coord_system()
{
    bool  use_pivot = false;
    QTreeWidgetItem*  parent_tree_item = nullptr;
    std::string  parent_tree_item_name;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(item);
        INVARIANT(tree_item != nullptr);
        if (!tree_item->represents_coord_system())
        {
            wnd()->print_status_message("ERROR: Insertion has FAILED. A batch is selected.", 10000);
            return;
        }
        std::string const  tree_item_name = qtgl::to_string(tree_item->text(0));
        if (tree_item_name == get_pivot_node_name())
            use_pivot = true;
        else
        {
            if (parent_tree_item != nullptr)
            {
                wnd()->print_status_message("ERROR: Insertion has FAILED. At most one non-pivot coord system can be selected.", 10000);
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

        vector3  origin = vector3_zero();
        quaternion  orientation = quaternion_identity();
        if (use_pivot)
        {
            scene_node_ptr const  pivot = wnd()->glwindow().call_now(&simulator::get_scene_node, get_pivot_node_name());
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
        add_tree_item_to_selection(tree_item);
        get_scene_history().insert<scene_history_coord_system_insert>(
                dlg.get_name(),
                origin,
                orientation,
                parent_tree_item == nullptr ? "" : qtgl::to_string(parent_tree_item->text(0)),
                false
                );

        QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
        update_history_according_to_change_in_selection(old_selection, new_selection, false);
        get_scene_history().commit();
    }
    else
        g_new_coord_system_id_counter = old_counter;
}


QTreeWidgetItem*  widgets::insert_batch(
            QTreeWidgetItem* const  node_item,
            std::string const&  batch_name,
            boost::filesystem::path const  batch_pathname
            )
{
    std::string const  node_name = qtgl::to_string(node_item->text(0));
    wnd()->glwindow().call_now(&simulator::insert_batch_to_scene_node, batch_name, batch_pathname, node_name);
    std::unique_ptr<tree_widget_item>  tree_node(new tree_widget_item(false));
    tree_node->setText(0, QString(batch_name.c_str()));
    tree_node->setIcon(0, m_batch_icon);
    node_item->addChild(tree_node.get());
    return tree_node.release();
}

void  widgets::on_scene_insert_batch()
{
    std::unordered_set<tree_widget_item*>  nodes;
    std::unordered_set<std::string>  used_names;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(item);
        INVARIANT(tree_item != nullptr);
        std::string const  tree_item_name = qtgl::to_string(tree_item->text(0));
        if (tree_item->represents_coord_system())
        {
            nodes.insert(tree_item);
            auto const  node = wnd()->glwindow().call_now(&simulator::get_scene_node, tree_item_name);
            INVARIANT(node != nullptr);
            for (auto const&  name_batch : node->get_batches())
                used_names.insert(name_batch.first);
        }
    }
    if (nodes.empty())
    {
        wnd()->print_status_message("ERROR: No coordinate system is selected.", 10000);
        return;
    }

    boost::filesystem::path const  batches_root_dir = canonical_path(boost::filesystem::absolute(
            boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "models"
            ));

    QFileDialog  dialog(wnd());
    dialog.setDirectory(batches_root_dir.string().c_str());
    //dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (!dialog.exec())
        return;
    QStringList const  selected = dialog.selectedFiles();
    if (selected.size() != 1)
    {
        wnd()->print_status_message("ERROR: No coordinate system is selected.", 10000);
        return;
    }
    boost::filesystem::path const  batch_pathname = qtgl::to_string(selected.at(0));
    boost::filesystem::path const  batch_dir = batch_pathname.parent_path().filename();
    boost::filesystem::path  batch_name = batch_pathname.filename();
    if (batch_name.has_extension())
        batch_name.replace_extension("");
    std::string const  raw_name = batch_dir.string() + "/" + batch_name.string();
    static natural_64_bit  counter = 0ULL;
    std::string  name = raw_name;
    while (used_names.count(name) != 0UL)
    {
        name = msgstream() << raw_name << "_" << counter;
        ++counter;
    }
    insert_name_dialog  dlg(wnd(), name,
        [&used_names](std::string const&  name) {
        return used_names.count(name) == 0UL;
    });
    dlg.exec();
    if (!dlg.get_name().empty())
    {
        QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();
        m_scene_tree->clearSelection();

        for (auto const&  tree_item : nodes)
        {
            auto const  batch_item = insert_batch(tree_item, dlg.get_name(), batch_pathname);
            add_tree_item_to_selection(batch_item);
            get_scene_history().insert<scene_history_batch_insert>(
                    std::pair<std::string, std::string>{ qtgl::to_string(tree_item->text(0)), dlg.get_name() },
                    batch_pathname,
                    false
                    );
        }
        
        QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
        update_history_according_to_change_in_selection(old_selection, new_selection, false);

        get_scene_history().commit();
    }
}

void  widgets::on_scene_erase_selected()
{
    if (m_scene_tree->selectedItems().empty())
        return;

    std::unordered_set<QTreeWidgetItem*>  to_erase_items;
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        if (item->parent() == nullptr && qtgl::to_string(item->text(0)) == get_pivot_node_name())
        {
            wnd()->print_status_message("ERROR: Cannot erase 'pivot' coordinate system.", 10000);
            return;
        }
        to_erase_items.insert(item);
    }

    QList<QTreeWidgetItem*> const  old_selection = m_scene_tree->selectedItems();
    m_scene_tree->clearSelection();
    QList<QTreeWidgetItem*> const  new_selection = m_scene_tree->selectedItems();
    update_history_according_to_change_in_selection(old_selection, new_selection, false);

    std::unordered_set<QTreeWidgetItem*>  erased_items;
    for (auto const  item : to_erase_items)
        if (erased_items.count(item) == 0UL)
            erase_subtree_at_root_item(item, erased_items);

    INVARIANT(!erased_items.empty());
    get_scene_history().commit();
}

void  widgets::erase_subtree_at_root_item(QTreeWidgetItem* const  root_item, std::unordered_set<QTreeWidgetItem*>&  erased_items)
{
    tree_widget_item* const  item = dynamic_cast<tree_widget_item*>(root_item);
    INVARIANT(item != nullptr);
    std::string const  item_name = qtgl::to_string(item->text(0));
    if (item->represents_coord_system())
    {
        while (item->childCount() > 0)
            erase_subtree_at_root_item(item->child(0), erased_items);

        get_scene_history().insert<scene_history_coord_system_insert_to_selection>(item_name, true);
        scene_node_ptr const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, item_name);
        INVARIANT(node_ptr != nullptr);
        get_scene_history().insert<scene_history_coord_system_insert>(
                item_name,
                node_ptr->get_coord_system()->origin(),
                node_ptr->get_coord_system()->orientation(),
                item->parent() == nullptr ? "" : qtgl::to_string(item->parent()->text(0)),
                true
                );

        wnd()->glwindow().call_now(&simulator::erase_scene_node, item_name);
    }
    else
    {
        tree_widget_item* const  parent_item = dynamic_cast<tree_widget_item*>(item->parent());
        INVARIANT(parent_item != nullptr);
        INVARIANT(parent_item->represents_coord_system());
        std::string const  parent_item_name = qtgl::to_string(parent_item->text(0));

        std::pair<std::string, std::string> const  name{ parent_item_name, item_name };
        get_scene_history().insert<scene_history_batch_insert_to_selection>(name, true);
        scene_node_ptr const  parent_node_ptr =
            wnd()->glwindow().call_now(&simulator::get_scene_node, parent_item_name);
        INVARIANT(parent_node_ptr != nullptr);
        qtgl::batch_ptr const  batch_ptr = parent_node_ptr->get_batch(item_name);
        INVARIANT(batch_ptr != nullptr);
        get_scene_history().insert<scene_history_batch_insert>(name, batch_ptr->path(), true);

        wnd()->glwindow().call_now(&simulator::erase_batch_from_scene_node, item_name, parent_item_name);
    }

    auto const  taken_item = item->parent() != nullptr ?
        item->parent()->takeChild(item->parent()->indexOfChild(item)) :
        m_scene_tree->takeTopLevelItem(m_scene_tree->indexOfTopLevelItem(item))
        ;
    INVARIANT(taken_item == item); (void)taken_item;

    delete item;
    erased_items.insert(item);
}

void  widgets::clear_scene()
{
    m_scene_tree->clear();
    wnd()->glwindow().call_now(&simulator::clear_scene);
    get_scene_history().clear();

    insert_coord_system(get_pivot_node_name(), vector3_zero(), quaternion_identity(), nullptr);

    g_new_coord_system_id_counter = 0UL;
}


static tree_widget_item*  load_scene_node(
        std::string const&  node_name,
        boost::property_tree::ptree const&  node_tree,
        qtgl::window<simulator>& glwindow,
        std::function<QTreeWidgetItem*(std::string const&, vector3 const&, quaternion const&, QTreeWidgetItem*)> const&  node_inserter,
        std::function<QTreeWidgetItem*(QTreeWidgetItem* const, std::string const&, boost::filesystem::path)> const&  batch_inserter,
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

    if (node_name == get_pivot_node_name())
    {
        scene_node_ptr const  pivot = glwindow.call_now(&simulator::get_scene_node, get_pivot_node_name());
        glwindow.call_now(&simulator::relocate_scene_node, get_pivot_node_name(), origin, orientation);
        return nullptr;
    }

    tree_widget_item* const  current_node_item = dynamic_cast<tree_widget_item*>(
            node_inserter(node_name, origin, orientation, parent_item)
            );

    boost::property_tree::ptree const&  batches = node_tree.find("batches")->second;
    for (auto it = batches.begin(); it != batches.end(); ++it)
        batch_inserter(current_node_item, it->first, it->second.data());

    boost::property_tree::ptree const&  children = node_tree.find("children")->second;
    for (auto it = children.begin(); it != children.end(); ++it)
        load_scene_node(it->first, it->second, glwindow, node_inserter, batch_inserter, current_node_item);

    return current_node_item;
}

void  widgets::open_scene(boost::filesystem::path const&  scene_root_dir)
{
    clear_scene();

    try
    {
        boost::property_tree::ptree  load_tree;
        boost::property_tree::read_info((scene_root_dir / "hierarchy.info").string(), load_tree);

        for (auto  it = load_tree.begin(); it != load_tree.end(); ++it)
            load_scene_node(
                it->first,
                it->second,
                wnd()->glwindow(),
                std::bind(
                    &window_tabs::tab_scene::widgets::insert_coord_system,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4
                    ),
                std::bind(
                    &window_tabs::tab_scene::widgets::insert_batch,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3
                    ),
                nullptr
                );
        m_scene_tree->expandAll();
        wnd()->print_status_message(std::string("SUCCESS: Scene loaded from: ") + scene_root_dir.string(), 5000);
    }
    catch (boost::property_tree::ptree_error const&  e)
    {
        wnd()->print_status_message(std::string("ERROR: Load of scene has FAILED. ") + e.what(), 10000);
        clear_scene();
    }
    catch (...)
    {
        wnd()->print_status_message("ERROR: Load of scene has FAILED. Reason is unknown.", 10000);
        clear_scene();
    }
}

static void  save_scene_item(
        tree_widget_item* const  item_ptr,
        qtgl::window<simulator>& glwindow,
        boost::property_tree::ptree& save_tree
        )
{
    ASSUMPTION(item_ptr != nullptr);

    std::string const  item_name = qtgl::to_string(item_ptr->text(0));
    scene_node_ptr const  node_ptr = glwindow.call_now(&simulator::get_scene_node, item_name);
    
    {
        boost::property_tree::ptree  origin_tree;
        vector3 const&  origin = node_ptr->get_coord_system()->origin();
        origin_tree.put("x", origin(0));
        origin_tree.put("y", origin(1));
        origin_tree.put("z", origin(2));
        save_tree.put_child(item_name + ".origin", origin_tree);
    }
    {
        boost::property_tree::ptree  orientation_tree;
        vector4 const&  orientation = quaternion_coefficients_xyzw(node_ptr->get_coord_system()->orientation());
        orientation_tree.put("x", orientation(0));
        orientation_tree.put("y", orientation(1));
        orientation_tree.put("z", orientation(2));
        orientation_tree.put("w", orientation(3));
        save_tree.put_child(item_name + ".orientation", orientation_tree);
    }

    boost::property_tree::ptree  batches;
    for (auto const& name_batch : node_ptr->get_batches())
        batches.put(name_batch.first, name_batch.second->path().string());
    save_tree.put_child(item_name + ".batches", batches);

    boost::property_tree::ptree  children;
    for (int i = 0, n = item_ptr->childCount(); i != n; ++i)
    {
        tree_widget_item* const  child = dynamic_cast<tree_widget_item*>(item_ptr->child(i));
        if (child->represents_coord_system())
            save_scene_item(child, glwindow, children);
    }
    save_tree.put_child(item_name + ".children", children);
}

void  widgets::save_scene(boost::filesystem::path const&  scene_root_dir)
{
    boost::property_tree::ptree save_tree;
    for (int i = 0, n = m_scene_tree->topLevelItemCount(); i != n; ++i)
        save_scene_item(
                dynamic_cast<tree_widget_item*>(m_scene_tree->topLevelItem(i)),
                wnd()->glwindow(),
                save_tree
                );
    boost::filesystem::create_directories(scene_root_dir);
    boost::property_tree::write_info((scene_root_dir / "hierarchy.info").string(), save_tree);
    wnd()->print_status_message(std::string("SUCCESS: Scene saved to: ") + scene_root_dir.string(), 5000);
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
    wnd()->glwindow().call_later(&simulator::set_position_of_scene_node, name, pos);

    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, name);
    get_scene_history().insert<scene_history_coord_system_relocate>(
            name,
            node_ptr->get_coord_system()->origin(),
            node_ptr->get_coord_system()->orientation(),
            pos,
            node_ptr->get_coord_system()->orientation()
            );
    get_scene_history().commit();
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
    wnd()->glwindow().call_later(&simulator::set_orientation_of_scene_node, name, q);

    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, name);
    get_scene_history().insert<scene_history_coord_system_relocate>(
            name,
            node_ptr->get_coord_system()->origin(),
            node_ptr->get_coord_system()->orientation(),
            node_ptr->get_coord_system()->origin(),
            q
            );
    get_scene_history().commit();
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
    wnd()->glwindow().call_later(&simulator::set_orientation_of_scene_node, name, q);

    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, name);
    get_scene_history().insert<scene_history_coord_system_relocate>(
            name,
            node_ptr->get_coord_system()->origin(),
            node_ptr->get_coord_system()->orientation(),
            node_ptr->get_coord_system()->origin(),
            q
            );
    get_scene_history().commit();
}


void  widgets::on_coord_system_position_started()
{
    m_coord_system_location_backup_buffer.clear();
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(item);
        INVARIANT(tree_item != nullptr);
        std::string  node_name;
        {
            if (tree_item->represents_coord_system())
                node_name = qtgl::to_string(tree_item->text(0));
            else
            {
                tree_widget_item* const  parent_item = dynamic_cast<tree_widget_item*>(item->parent());
                INVARIANT(parent_item != nullptr);
                INVARIANT(parent_item->represents_coord_system());
                node_name = qtgl::to_string(parent_item->text(0));
            }
        }
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
        get_scene_history().insert<scene_history_coord_system_relocate>(
                name_and_system.first,
                name_and_system.second.origin(),
                name_and_system.second.orientation(),
                node_ptr->get_coord_system()->origin(),
                node_ptr->get_coord_system()->orientation()
                );
    }
    if (!m_coord_system_location_backup_buffer.empty())
        get_scene_history().commit();
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
    wnd()->glwindow().call_later(&simulator::set_scene_edit_mode, SCENE_EDIT_MODE::SELECT_SCENE_OBJECT);
}

void  widgets::on_scene_mode_translation()
{
    wnd()->glwindow().call_later(&simulator::set_scene_edit_mode, SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES);
}

void  widgets::on_scene_mode_rotation()
{
    wnd()->glwindow().call_later(&simulator::set_scene_edit_mode, SCENE_EDIT_MODE::ROTATE_SELECTED_NODES);
}

void  widgets::on_scene_undo()
{

}

void  widgets::on_scene_redo()
{

}


void  widgets::update_coord_system_location_widgets()
{
    auto const selected_items = m_scene_tree->selectedItems();
    if (![&selected_items]() -> bool {
            if (selected_items.empty())
                return false;
            foreach(QTreeWidgetItem* const  item, selected_items)
                if (item->parent() != selected_items.front()->parent())
                    return false;
            return true;
            }())
    {
        enable_coord_system_location_widgets(false);
        return;
    }

    tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(selected_items.front());
    INVARIANT(tree_item != nullptr);

    enable_coord_system_location_widgets(true);
    std::string const  tree_item_name = get_name_of_active_coord_system_in_tree_widget(*m_scene_tree);
    auto const  node_ptr = wnd()->glwindow().call_now(&simulator::get_scene_node, tree_item_name);
    refresh_text_in_coord_system_location_widgets(node_ptr);
}

void  widgets::enable_coord_system_location_widgets(bool const  state)
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
}

void  widgets::refresh_text_in_coord_system_location_widgets(scene_node_ptr const  node_ptr)
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


QWidget*  make_scene_tab_content(widgets const&  w)
{
    QWidget* const  scene_tab = new QWidget;
    {
        QVBoxLayout* const scene_tab_layout = new QVBoxLayout;
        {
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

            QWidget* const operation_group = new QGroupBox("Operation");
            {
                QVBoxLayout* const buttons_layout = new QVBoxLayout;
                {
                    QHBoxLayout* const insertion_layout = new QHBoxLayout;
                    {
                        insertion_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Insert node")
                                        {
                                            QObject::connect(
                                                this,
                                                SIGNAL(released()),
                                                wnd,
                                                SLOT(on_scene_insert_coord_system())
                                                );
                                        }
                                    };
                                    return new choose(wnd);
                                }(w.wnd())
                                );
                        insertion_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Insert batch")
                                        {
                                            QObject::connect(
                                                this,
                                                SIGNAL(released()),
                                                wnd,
                                                SLOT(on_scene_insert_batch())
                                                );
                                        }
                                    };
                                    return new choose(wnd);
                                }(w.wnd())
                                );
                    }
                    buttons_layout->addLayout(insertion_layout);

                    buttons_layout->addWidget(
                        [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Erase selected")
                                    {
                                        QObject::connect(
                                            this,
                                            SIGNAL(released()),
                                            wnd,
                                            SLOT(on_scene_erase_selected())
                                            );
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                }
                operation_group->setLayout(buttons_layout);
            }
            scene_tab_layout->addWidget(operation_group);

            //QWidget* const search_group = new QGroupBox("Search");
            //{
            //}
            //scene_tab_layout->addWidget(search_group);

            QWidget* const selected_group = new QGroupBox("Properties of selection");
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

            scene_tab_layout->addStretch(1);
        }
        scene_tab->setLayout(scene_tab_layout);
    }
    return scene_tab;
}


}}
