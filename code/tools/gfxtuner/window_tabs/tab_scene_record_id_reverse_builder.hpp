#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_RECORD_ID_REVERSE_BUILDER_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_RECORD_ID_REVERSE_BUILDER_HPP_INCLUDED

#   include <gfxtuner/window_tabs/tab_scene_tree_widget.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <string>

namespace window_tabs { namespace tab_scene {


struct  scene_record_id_reverse_builder
{
    void next_name(std::string const&  name);
    scn::scene_record_id  get_record_id();
    scn::scene_node_record_id  get_node_record_id();

    static scene_record_id_reverse_builder  run(
            tree_widget_item* const  tree_item,
            tree_widget_item** const  coord_system_item = nullptr);

private:
    std::string  m_node_name;
    std::string  m_folder_name;
    std::string  m_record_name;
};


}}

#endif
