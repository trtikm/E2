#include <gfxtuner/menu_bar_record_rigid_body.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_rigid_body {


void  register_record_menu_items(std::multimap<std::string, record_menu_item_info>&  record_menu_items)
{
    record_menu_items.insert({
        scn::get_rigid_body_folder_name(),
        {
            "Insert physical &rigid body",
            "Ctrl+Alt+R",
            "TODO!"
        }
    });
}


}}
