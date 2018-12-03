#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_RECORD_ID_REVERSE_BUILDER_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_RECORD_ID_REVERSE_BUILDER_HPP_INCLUDED

#   include <gfxtuner/window_tabs/tab_scene_tree_widget.hpp>
#   include <scene/scene_node_id.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <string>

namespace window_tabs { namespace tab_scene {


struct  scene_record_id_reverse_builder
{
    scn::scene_node_id  get_node_id() const { return scn::scene_node_id(m_node_path); }
    scn::scene_record_id  get_record_id() const { return{ get_node_id(), m_folder_name, m_record_name }; }
    scn::scene_node_record_id  get_node_record_id() const { return{ m_folder_name, m_record_name }; }

    static scene_record_id_reverse_builder  run(
            tree_widget_item const*  tree_item,
            tree_widget_item** const  coord_system_item = nullptr);

    static scene_record_id_reverse_builder  run(
            QTreeWidgetItem const* const  tree_item,
            tree_widget_item** const  coord_system_item = nullptr);

private:
    scn::scene_node_id::path_type  m_node_path;
    std::string  m_folder_name;
    std::string  m_record_name;
};


}}

#endif
