#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_id_reverse_builder.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <QList>

namespace window_tabs { namespace tab_scene {


bool  correspond(scn::scene_node_id  id, tree_widget_item const*  tree_item)
{
    TMPROF_BLOCK();

    if (!id.valid() || tree_item == nullptr)
        return false;
    for ( ; id.valid(); id = id.get_direct_parent_id(), tree_item = as_tree_widget_item(tree_item->parent()))
        if (tree_item == nullptr ||
            !represents_coord_system(tree_item) ||
            get_tree_widget_item_name(tree_item) != id.path_last_element()
            )
            break;
    // Yes, indeed NOT valid (because we succasfully processed all path elements and so we got an invalid id).
    return !id.valid() && tree_item == nullptr;
}


void  remove_record_from_tree_widget(tree_widget* const  scene_tree, scn::scene_record_id const&  record_id)
{
    TMPROF_BLOCK();

    auto const  record_item = scene_tree->find(record_id);
    ASSUMPTION(record_item != nullptr && represents_record(record_item));
    ASSUMPTION(!record_item->isSelected());

    auto const  folder_item = as_tree_widget_item(record_item->parent());
    ASSUMPTION(folder_item != nullptr && represents_folder(folder_item));

    scene_tree->erase(record_item);
    if (folder_item->childCount() == 0)
        scene_tree->erase(folder_item);
}


tree_widget_item*  insert_coord_system_to_tree_widget(
        tree_widget* const  scene_tree,
        scn::scene_node_id const&  id,
        vector3 const&  origin,
        quaternion const&  orientation,
        QIcon const&  icon,
        tree_widget_item* const  parent_tree_item
        )
{
    ASSUMPTION(parent_tree_item == nullptr || correspond(id.get_direct_parent_id(), parent_tree_item));
    return scene_tree->insert(id.path_last_element(), icon, true, parent_tree_item);
}


tree_widget_item*  insert_record_to_tree_widget(
        tree_widget* const  scene_tree,
        scn::scene_record_id const&  record_id,
        QIcon const&  icon,
        QIcon const&  folder_icon
        )
{
    TMPROF_BLOCK();

    tree_widget_item*  folder_item = scene_tree->find(record_id.get_node_id(), record_id.get_folder_name());
    if (folder_item == nullptr)
    {
        folder_item = scene_tree->insert(record_id.get_folder_name(), folder_icon, false, scene_tree->find(record_id.get_node_id()));
        INVARIANT(folder_item->isSelected() == false);
    }
    return scene_tree->insert(record_id.get_record_name(), icon, false, folder_item);
}


tree_widget_item*  get_active_coord_system_item_in_tree_widget(tree_widget const&  tree_widget)
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


bool  is_active_coord_system_in_tree_widget(tree_widget const&  tree_widget, scn::scene_node_id const&  id)
{
    TMPROF_BLOCK();

    auto const selected_items = tree_widget.selectedItems();
    if (selected_items.size() != 1)
        return false;
    tree_widget_item* const  tree_item = find_nearest_coord_system_item(selected_items.front());
    INVARIANT(tree_item != nullptr && tree_item->represents_coord_system());
    return correspond(id, tree_item);
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
                auto const  id_builder = scene_record_id_reverse_builder::run(item);
                if (represents_coord_system(item))
                    scene_history_ptr->insert<scn::scene_history_coord_system_insert_to_selection>(
                        id_builder.get_node_id(),
                        work_list.at(i).second
                        );
                else if (!represents_folder(item))
                    scene_history_ptr->insert<scn::scene_history_record_insert_to_selection>(
                        id_builder.get_record_id(),
                        work_list.at(i).second
                        );
            }
    if (change == true && apply_commit == true)
        scene_history_ptr->commit();

    return change;
}


}}
