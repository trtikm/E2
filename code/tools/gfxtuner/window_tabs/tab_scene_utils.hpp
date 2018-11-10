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


void  find_all_coord_system_widgets(
        QTreeWidget* const  scene_tree,
        scn::scene_node_name const&  node_name,
        std::vector<tree_widget_item*>&  output
        );

void  remove_record_from_tree_widget(QTreeWidget* const  scene_tree, scn::scene_record_id const&  record_id);

QTreeWidgetItem*  insert_coord_system_to_tree_widget(
        QTreeWidget* const  scene_tree,
        scn::scene_node_name const&  name,
        vector3 const&  origin,
        quaternion const&  orientation,
        QIcon const&  icon,
        QTreeWidgetItem* const  parent_tree_item
        );

tree_widget_item*  insert_record_to_tree_widget(
        QTreeWidget* const  scene_tree,
        scn::scene_record_id const&  record_id,
        QIcon const&  icon,
        QIcon const&  folder_icon
        );

tree_widget_item*  get_active_coord_system_item_in_tree_widget(QTreeWidget const&  tree_widget);

inline std::string  get_name_of_active_coord_system_in_tree_widget(QTreeWidget const&  tree_widget)
{
    return get_tree_widget_item_name(get_active_coord_system_item_in_tree_widget(tree_widget));
}

bool  is_active_coord_system_in_tree_widget(QTreeWidget const&  tree_widget, std::string const&  name);


bool  update_history_according_to_change_in_selection(
        QList<QTreeWidgetItem*> const&  old_selection,
        QList<QTreeWidgetItem*> const&  new_selection,
        scn::scene_history_ptr const  scene_history_ptr,
        bool  apply_commit = true
        );


}}

#endif
