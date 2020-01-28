#include <gfxtuner/menu_bar_records_integration.hpp>
#include <gfxtuner/menu_bar_record_batch.hpp>
#include <gfxtuner/menu_bar_record_collider.hpp>
#include <gfxtuner/menu_bar_record_rigid_body.hpp>
#include <gfxtuner/menu_bar_record_agent.hpp>
#include <gfxtuner/menu_bar_record_device.hpp>
#include <gfxtuner/menu_bar_record_sensor.hpp>

namespace record_menu_items {


void  register_record_menu_items(std::vector<record_menu_item_info>&  record_menu_items)
{
    record_batch::register_record_menu_items(record_menu_items);
    record_menu_items.push_back({}); // Separator
    record_collider::register_record_menu_items(record_menu_items);
    record_menu_items.push_back({}); // Separator
    record_rigid_body::register_record_menu_items(record_menu_items);
    record_menu_items.push_back({}); // Separator
    record_agent::register_record_menu_items(record_menu_items);
    record_device::register_record_menu_items(record_menu_items);
    record_sensor::register_record_menu_items(record_menu_items);
}


}
