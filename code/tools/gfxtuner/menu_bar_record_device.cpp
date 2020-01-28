#include <gfxtuner/menu_bar_record_device.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_device {


void  register_record_menu_items(std::multimap<std::string, std::pair<std::string, record_menu_item_info> >&  record_menu_items)
{
    record_menu_items.insert({
        scn::get_device_folder_name(),
        {
            "default",
            {
                "Insert &device",
                "Ctrl+Alt+D",
                "A 'device' is an entity in the scene capable of acting based on signal received via sensors.\n"
                "An action can for example be: changing the position/rotation, disapearing from the scene,\n"
                "insertion of another device into the scene."
            }
        }
    });
}


}}
