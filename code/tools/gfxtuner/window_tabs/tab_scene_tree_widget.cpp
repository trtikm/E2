#include <gfxtuner/window_tabs/tab_scene_tree_widget.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_id_reverse_builder.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <memory>

namespace window_tabs { namespace tab_scene {


tree_widget_item*  tree_widget::insert(std::string const&  text, QIcon const&  icon, bool const  represents_coord_system, tree_widget_item* const  parent_tree_item)
{
    TMPROF_BLOCK();

    std::unique_ptr<tree_widget_item>  tree_node(new tree_widget_item(represents_coord_system));
    tree_node->setText(0, text.c_str());
    tree_node->setIcon(0, icon);
    if (parent_tree_item == nullptr)
        addTopLevelItem(tree_node.get());
    else
        parent_tree_item->addChild(tree_node.get());

    {
        scn::scene_record_id const  id = scene_record_id_reverse_builder::run(tree_node.get()).get_record_id();
        auto const it_and_bool = m_from_nodes_to_widgets.insert({ id, tree_node.get() });
        INVARIANT(it_and_bool.second);
        auto const it_and_bool_2 = m_from_widgets_to_nodes.insert({ tree_node.get(), it_and_bool.first });
        INVARIANT(it_and_bool_2.second);
    }

    return tree_node.release();
}


void  tree_widget::erase(tree_widget_item* const  item_ptr)
{
    TMPROF_BLOCK();

    {
        auto const  it = m_from_widgets_to_nodes.find(item_ptr);
        INVARIANT(it != m_from_widgets_to_nodes.end());
        m_from_nodes_to_widgets.erase(it->second);
        m_from_widgets_to_nodes.erase(it);
    }

    auto const  taken_item = item_ptr->parent() != nullptr ?
            item_ptr->parent()->takeChild(item_ptr->parent()->indexOfChild(item_ptr)) :
            takeTopLevelItem(indexOfTopLevelItem(item_ptr))
            ;
    INVARIANT(taken_item == item_ptr); (void)taken_item;
    delete taken_item;
}


void  tree_widget::clear()
{
    QTreeWidget::clear();
    m_from_nodes_to_widgets.clear();
    m_from_widgets_to_nodes.clear();
}


tree_widget_item*  tree_widget::find(scn::scene_record_id const&  id) const
{
    TMPROF_BLOCK();

    auto const  it = m_from_nodes_to_widgets.find(id);
    return it == m_from_nodes_to_widgets.cend() ? nullptr : it->second;
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
