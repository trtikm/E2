#include <gfxtuner/menu_bar_record_agent.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_agent {


void  register_record_menu_items(std::multimap<std::string, std::pair<std::string, record_menu_item_info> >&  record_menu_items)
{
    record_menu_items.insert({
        scn::get_agent_folder_name(),
        {
            "default",
            {
                "Insert &agent",
                "Ctrl+Alt+A",
                "An 'agent' is TODO\n"
            }
        }
    });
}


}}
