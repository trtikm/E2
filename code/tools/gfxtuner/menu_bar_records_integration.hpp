#ifndef E2_TOOL_GFXTUNER_MENU_BAR_RECORDS_INTEGRATION_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_MENU_BAR_RECORDS_INTEGRATION_HPP_INCLUDED

#   include <map>
#   include <string>

namespace record_menu_items {


struct  record_menu_item_info
{
    std::string  m_name;
    std::string  m_shortcut;
    std::string  m_tooltip;
};


void  register_record_menu_items(std::multimap<std::string, record_menu_item_info>&  record_menu_items);


}

#endif
