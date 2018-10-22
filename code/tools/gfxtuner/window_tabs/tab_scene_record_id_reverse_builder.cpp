#include <gfxtuner/window_tabs/tab_scene_record_id_reverse_builder.hpp>

namespace window_tabs { namespace tab_scene {


void scene_record_id_reverse_builder::next_name(std::string const&  name)
{
    if (m_record_name.empty())
        m_record_name = name;
    else
    {
        if (!m_node_name.empty())
        {
            if (m_folder_name.empty())
                m_folder_name = m_node_name;
            else
                m_folder_name = m_node_name + '/' + m_folder_name;
        }
        m_node_name = name;
    }
}

scn::scene_record_id  scene_record_id_reverse_builder::get_record_id()
{
    return { m_node_name, m_folder_name, m_record_name };
}

scn::scene_node_record_id  scene_record_id_reverse_builder::get_node_record_id()
{
    return { m_folder_name, m_record_name };
}

scene_record_id_reverse_builder  scene_record_id_reverse_builder::run(
        tree_widget_item* const  tree_item,
        tree_widget_item** const  coord_system_item)
{
    scene_record_id_reverse_builder  id_builder;
    tree_widget_item* coord_system_tree_item =
        find_nearest_coord_system_item(tree_item, [&id_builder](std::string const& s) { id_builder.next_name(s); });
    if (coord_system_item != nullptr)
        *coord_system_item = coord_system_tree_item;
    return id_builder;
}


}}
