#include <gfxtuner/window_tabs/tab_scene_tree_widget.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <memory>

namespace window_tabs { namespace tab_scene {


tree_widget_item*  tree_widget::insert(std::string const&  text, QIcon const&  icon, bool const  represents_coord_system, tree_widget_item* const  parent_tree_item)
{
    std::unique_ptr<tree_widget_item>  tree_node(new tree_widget_item(represents_coord_system));
    tree_node->setText(0, text.c_str());
    tree_node->setIcon(0, icon);
    if (parent_tree_item == nullptr)
        addTopLevelItem(tree_node.get());
    else
        parent_tree_item->addChild(tree_node.get());
    return tree_node.release();
}


void  tree_widget::erase(tree_widget_item* const  item_ptr)
{
    auto const  taken_item = item_ptr->parent() != nullptr ?
            item_ptr->parent()->takeChild(item_ptr->parent()->indexOfChild(item_ptr)) :
            takeTopLevelItem(indexOfTopLevelItem(item_ptr))
            ;
    INVARIANT(taken_item == item_ptr); (void)taken_item;
    delete taken_item;
}


tree_widget_item*  find_nearest_coord_system_item(
        tree_widget_item const*  tree_item,
        std::function<void(std::string const&)> const&  names_acceptor
        )
{
    TMPROF_BLOCK();

    while (tree_item != nullptr && !tree_item->represents_coord_system())
    {
        names_acceptor(get_tree_widget_item_name(tree_item));
        tree_item = as_tree_widget_item(tree_item->parent());
    }
    if (tree_item != nullptr)
        names_acceptor(get_tree_widget_item_name(tree_item));
    return const_cast<tree_widget_item*>(tree_item);
}


}}
