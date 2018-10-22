#include <gfxtuner/window_tabs/tab_scene_tree_widget.hpp>
#include <utility/timeprof.hpp>

namespace window_tabs { namespace tab_scene {


tree_widget_item*  find_nearest_coord_system_item(
        tree_widget_item*  tree_item,
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
    return tree_item;
}


}}
