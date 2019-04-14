#include <gfxtuner/menu_bar_records_integration.hpp>
#include <gfxtuner/menu_bar_record_batch.hpp>
#include <gfxtuner/menu_bar_record_collider.hpp>
#include <gfxtuner/menu_bar_record_rigid_body.hpp>
#include <gfxtuner/menu_bar_record_agent.hpp>

namespace record_menu_items {


void  register_record_menu_items(std::multimap<std::string, std::pair<std::string, record_menu_item_info> >&  record_menu_items)
{
    record_batch::register_record_menu_items(record_menu_items);
    record_collider::register_record_menu_items(record_menu_items);
    record_rigid_body::register_record_menu_items(record_menu_items);
    record_agent::register_record_menu_items(record_menu_items);
}


}
