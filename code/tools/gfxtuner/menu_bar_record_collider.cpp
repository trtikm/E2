#include <gfxtuner/menu_bar_record_collider.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_collider {


void  register_record_menu_items(std::multimap<std::string, record_menu_item_info>&  record_menu_items)
{
    record_menu_items.insert({
        scn::get_collider_folder_name(),
        {
            "Insert collision &capsule",
            "Ctrl+Alt+C",
            "TODO!"
        }
    });
    record_menu_items.insert({
        scn::get_collider_folder_name(),
        {
            "Insert collision &sphere",
            "Ctrl+Alt+S",
            "TODO!"
        }
    });
    record_menu_items.insert({
        scn::get_collider_folder_name(),
        {
            "Insert collision triangle &mesh",
            "Ctrl+Alt+M",
            "TODO!"
        }
    });
}


}}
