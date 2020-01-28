#include <gfxtuner/menu_bar_record_agent.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_agent {


void  register_record_menu_items(std::vector<record_menu_item_info>&  record_menu_items)
{
    record_menu_items.push_back({
        "Insert &agent",
        "Ctrl+Alt+A",
        "An 'agent' is an entity in the scene capable of perception and acting in the scene in order to\n"
        "(1) preserve the perception-acting cycle as long as possible and (2) realise its all other desires.\n"
        "An 'agent' record can only be inserted under a coordinate system without a parent node\n.",
        scn::get_agent_folder_name(),
        "default",
        });
}


}}
