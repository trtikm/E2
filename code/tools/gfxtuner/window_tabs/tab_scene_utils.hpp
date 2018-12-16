#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_UTILS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_UTILS_HPP_INCLUDED

#   include <gfxtuner/window_tabs/tab_scene_tree_widget.hpp>
#   include <angeo/tensor_math.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <scene/scene_history.hpp>
#   include <QTreeWidget>
#   include <string>
#   include <vector>

namespace window_tabs { namespace tab_scene {


bool  correspond(scn::scene_node_id  id, tree_widget_item const*  tree_item);

void  remove_record_from_tree_widget(tree_widget* const  scene_tree, scn::scene_record_id const&  record_id);

tree_widget_item*  insert_coord_system_to_tree_widget(
        tree_widget* const  scene_tree,
        scn::scene_node_id const&  id,
        vector3 const&  origin,
        quaternion const&  orientation,
        QIcon const&  icon,
        tree_widget_item* const  parent_tree_item
        );

tree_widget_item*  insert_record_to_tree_widget(
        tree_widget* const  scene_tree,
        scn::scene_record_id const&  record_id,
        QIcon const&  icon,
        QIcon const&  folder_icon
        );

tree_widget_item*  get_active_coord_system_item_in_tree_widget(tree_widget const&  tree_widget);

inline std::string  get_name_of_active_coord_system_in_tree_widget(tree_widget const&  tree_widget)
{
    return get_tree_widget_item_name(get_active_coord_system_item_in_tree_widget(tree_widget));
}

bool  is_active_coord_system_in_tree_widget(tree_widget const&  tree_widget, scn::scene_node_id const&  id);


bool  update_history_according_to_change_in_selection(
        QList<QTreeWidgetItem*> const&  old_selection,
        QList<QTreeWidgetItem*> const&  new_selection,
        scn::scene_history_ptr const  scene_history_ptr,
        bool  apply_commit = true
        );


}}

#endif
