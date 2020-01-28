#include <gfxtuner/menu_bar_record_sensor.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_sensor {


void  register_record_menu_items(std::multimap<std::string, std::pair<std::string, record_menu_item_info> >&  record_menu_items)
{
    record_menu_items.insert({
        scn::get_sensors_folder_name(),
        {
            "default",
            {
                "Insert &sensor",
                "Ctrl+Alt+E",
                "An 'sensor' is an entity in the scene transforming collisions of its collision shape with collision\n"
                "shapes of other objects in the scene into events which are sent to an agent or device owning the sensor\n."
                "The owner of a sensor is that agent or device whose scene node is closest parent of sensor's scene node.\n"
                "A sensor must always have exactly one owner. An agent or device may be an owner of any number of sensors."
            }
        }
    });
}


}}
