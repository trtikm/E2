#ifndef E2_TOOL_GFXTUNER_MENU_BAR_RECORD_BATCH_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_MENU_BAR_RECORD_BATCH_HPP_INCLUDED

#   include <gfxtuner/menu_bar_records_integration.hpp>

namespace record_menu_items { namespace record_batch {


void  register_record_menu_items(std::multimap<std::string, record_menu_item_info>&  record_menu_items);


}}

#endif
