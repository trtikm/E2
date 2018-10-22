#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_id_reverse_builder.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <QList>

namespace window_tabs { namespace tab_scene {


void  remove_record_from_tree_widget(QTreeWidget* const  scene_tree, scn::scene_record_id const&  record_id)
{
    TMPROF_BLOCK();

    auto const  items_list = scene_tree->findItems(
            QString(record_id.get_node_name().c_str()),
            Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive,
            0
            );
    ASSUMPTION(items_list.size() == 1UL);
    auto const  coord_system_item = as_tree_widget_item(items_list.front());
    std::string const  coord_system_name = get_tree_widget_item_name(coord_system_item);
    ASSUMPTION(represents_coord_system(coord_system_item));
    ASSUMPTION(coord_system_name == record_id.get_node_name());
    for (int i = 0, n = coord_system_item->childCount(); i != n; ++i)
    {
        auto const  folder_item = as_tree_widget_item(coord_system_item->child(i));
        if (!represents_folder(folder_item))
        {
            INVARIANT(represents_coord_system(folder_item));
            continue;
        }
        std::string const  folder_item_name = get_tree_widget_item_name(folder_item);
        if (folder_item_name == record_id.get_folder_name())
        {
            for (int j = 0, m = folder_item->childCount(); j != m; ++j)
            {
                auto const  record_item = as_tree_widget_item(folder_item->child(j));
                INVARIANT(represents_record(record_item));
                std::string const  record_name = get_tree_widget_item_name(record_item);
                if (record_name == record_id.get_record_name())
                {
                    ASSUMPTION(!record_item->isSelected());
                    auto  taken_item = folder_item->takeChild(j);
                    INVARIANT(taken_item == record_item); (void)taken_item;
                    delete taken_item;
                    if (folder_item->childCount() == 0)
                    {
                        taken_item = coord_system_item->takeChild(i);
                        INVARIANT(taken_item == folder_item); (void)taken_item;
                        delete taken_item;
                    }
                    return;
                }
            }
            UNREACHABLE();
        }
    }
    UNREACHABLE();
}


QTreeWidgetItem*  insert_coord_system_to_tree_widget(
        QTreeWidget* const  scene_tree,
        scn::scene_node_name const&  name,
        vector3 const&  origin,
        quaternion const&  orientation,
        QIcon const&  icon,
        QTreeWidgetItem* const  parent_tree_item
        )
{
    TMPROF_BLOCK();

    std::unique_ptr<tree_widget_item>  tree_node(new tree_widget_item(true));
    tree_node->setText(0, QString(name.c_str()));
    tree_node->setIcon(0, icon);
    if (parent_tree_item == nullptr)
        scene_tree->addTopLevelItem(tree_node.get());
    else
        parent_tree_item->addChild(tree_node.get());
    return tree_node.release();
}


tree_widget_item*  insert_record_to_tree_widget(
        QTreeWidget* const  scene_tree,
        scn::scene_record_id const&  record_id,
        QIcon const&  icon,
        QIcon const&  folder_icon
        )
{
    TMPROF_BLOCK();

    auto const  items_list = scene_tree->findItems(
            QString(record_id.get_node_name().c_str()),
            Qt::MatchFlag::MatchExactly | Qt::MatchFlag::MatchRecursive,
            0
            );
    ASSUMPTION(items_list.size() == 1UL);
    auto const  coord_system_item = as_tree_widget_item(items_list.front());
    ASSUMPTION(represents_coord_system(coord_system_item));
    std::string const  coord_system_name = get_tree_widget_item_name(coord_system_item);
    ASSUMPTION(coord_system_name == record_id.get_node_name());
    ASSUMPTION(
        ([coord_system_item, &record_id]() -> bool {
            for (int i = 0, n = coord_system_item->childCount(); i != n; ++i)
            {
                auto const  folder_item = as_tree_widget_item(coord_system_item->child(i));
                if (!represents_folder(folder_item))
                {
                    INVARIANT(represents_coord_system(folder_item));
                    continue;
                }
                std::string const  folder_item_name = get_tree_widget_item_name(folder_item);
                if (folder_item_name == record_id.get_folder_name())
                {
                    for (int j = 0, m = folder_item->childCount(); j != m; ++j)
                    {
                        auto const  record_item = as_tree_widget_item(folder_item->child(j));
                        std::string const  record_name = get_tree_widget_item_name(record_item);
                        if (record_name == record_id.get_record_name())
                            return false;
                    }
                    return true;
                }
            }
            return true;
        }())
    );
    tree_widget_item*  folder_item = nullptr;
    for (int i = 0, n = coord_system_item->childCount(); i != n; ++i)
    {
        auto const  item = as_tree_widget_item(coord_system_item->child(i));
        if (!represents_folder(item))
        {
            INVARIANT(represents_coord_system(item));
            continue;
        }
        std::string const  item_name = get_tree_widget_item_name(item);
        if (item_name == record_id.get_folder_name())
        {
            folder_item = item;
            break;
        }
    }
    if (folder_item == nullptr)
    {
        std::unique_ptr<tree_widget_item>  tree_node(new tree_widget_item(false));
        tree_node->setText(0, QString(record_id.get_folder_name().c_str()));
        tree_node->setIcon(0, folder_icon);
        folder_item = tree_node.release();
        coord_system_item->addChild(folder_item);
        INVARIANT(folder_item->isSelected() == false);
    }
    std::unique_ptr<tree_widget_item>  tree_node(new tree_widget_item(false));
    tree_node->setText(0, QString(record_id.get_record_name().c_str()));
    tree_node->setIcon(0, icon);
    folder_item->addChild(tree_node.get());
    INVARIANT(tree_node->isSelected() == false);
    return tree_node.release();
}


tree_widget_item*  get_active_coord_system_item_in_tree_widget(QTreeWidget const&  tree_widget)
{
    TMPROF_BLOCK();

    auto const selected_items = tree_widget.selectedItems();
    INVARIANT(
        !selected_items.empty() &&
        [&selected_items]() -> bool {
            foreach(QTreeWidgetItem* const  item, selected_items)
                if (find_nearest_coord_system_item(item) != find_nearest_coord_system_item(selected_items.front()))
                    return false;
                return true;
            }()
        );
    tree_widget_item* const  tree_item = find_nearest_coord_system_item(selected_items.front());
    INVARIANT(tree_item != nullptr && tree_item->represents_coord_system());
    return tree_item;
}


bool  is_active_coord_system_in_tree_widget(QTreeWidget const&  tree_widget, std::string const&  name)
{
    TMPROF_BLOCK();

    auto const selected_items = tree_widget.selectedItems();
    if (selected_items.size() != 1)
        return false;
    tree_widget_item* const  tree_item = find_nearest_coord_system_item(selected_items.front());
    INVARIANT(tree_item != nullptr && tree_item->represents_coord_system());
    std::string const  active_name = get_tree_widget_item_name(tree_item);
    return name == active_name;
}


bool  update_history_according_to_change_in_selection(
        QList<QTreeWidgetItem*> const&  old_selection,
        QList<QTreeWidgetItem*> const&  new_selection,
        scn::scene_history_ptr const  scene_history_ptr,
        bool  apply_commit
        )
{
    TMPROF_BLOCK();

    std::vector< std::pair<std::unordered_set<QTreeWidgetItem*>, bool> > const  work_list {
        { std::unordered_set<QTreeWidgetItem*>(old_selection.cbegin(), old_selection.cend()), true },
        { std::unordered_set<QTreeWidgetItem*>(new_selection.cbegin(), new_selection.cend()), false },
    };
    bool  change = false;
    for (std::size_t  i = 0UL, j = 1UL; i != work_list.size(); ++i, j = (j + 1) % 2)
        for (auto const  item_ptr : work_list.at(i).first)
            if (work_list.at(j).first.count(item_ptr) == 0UL)
            {
                change = true;
                tree_widget_item* const  item = as_tree_widget_item(item_ptr);
                INVARIANT(item != nullptr);
                std::string const  item_name = get_tree_widget_item_name(item);
                if (represents_coord_system(item))
                    scene_history_ptr->insert<scn::scene_history_coord_system_insert_to_selection>(item_name,work_list.at(i).second);
                else if (!represents_folder(item))
                    scene_history_ptr->insert<scn::scene_history_record_insert_to_selection>(
                        scene_record_id_reverse_builder::run(item).get_record_id(),
                        work_list.at(i).second
                        );
            }
    if (change == true && apply_commit == true)
        scene_history_ptr->commit();

    return change;
}



}}
