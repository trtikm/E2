#include <gfxtuner/window_tabs/tab_scene_record_id_reverse_builder.hpp>
#include <utility/invariants.hpp>
#include <algorithm>

namespace window_tabs { namespace tab_scene {


scene_record_id_reverse_builder  scene_record_id_reverse_builder::run(
        tree_widget_item const*  tree_item,
        tree_widget_item** const  coord_system_item)
{
    ASSUMPTION(tree_item != nullptr);
    scene_record_id_reverse_builder  id_builder;
    if (represents_record(tree_item))
    {
        id_builder.m_record_name = get_tree_widget_item_name(tree_item);
        tree_item = as_tree_widget_item(tree_item->parent());
    }
    if (represents_folder(tree_item))
    {
        id_builder.m_folder_name = get_tree_widget_item_name(tree_item);
        tree_item = as_tree_widget_item(tree_item->parent());
    }
    INVARIANT(represents_coord_system(tree_item));
    if (coord_system_item != nullptr)
        *coord_system_item = const_cast<tree_widget_item*>(tree_item);
    for ( ; tree_item != nullptr; tree_item = as_tree_widget_item(tree_item->parent()))
    {
        INVARIANT(represents_coord_system(tree_item));
        id_builder.m_node_path.push_back(get_tree_widget_item_name(tree_item));
    }
    std::reverse(id_builder.m_node_path.begin(), id_builder.m_node_path.end());
    INVARIANT(!id_builder.m_node_path.empty());
    return id_builder;
}


scene_record_id_reverse_builder  scene_record_id_reverse_builder::run(
        QTreeWidgetItem const* const  tree_item,
        tree_widget_item** const  coord_system_item)
{
    return run(as_tree_widget_item(tree_item), coord_system_item);
}


}}
