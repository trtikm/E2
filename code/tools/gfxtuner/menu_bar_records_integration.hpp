#ifndef E2_TOOL_GFXTUNER_MENU_BAR_RECORDS_INTEGRATION_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_MENU_BAR_RECORDS_INTEGRATION_HPP_INCLUDED

#   include <vector>
#   include <string>

namespace record_menu_items {


struct  record_menu_item_info
{
    bool  is_separator() const { return m_name.empty(); }

    std::string  m_name;
    std::string  m_shortcut;
    std::string  m_tooltip;
    std::string  m_folder_name;
    std::string  m_mode;
};


void  register_record_menu_items(std::vector<record_menu_item_info>&  record_menu_items);


}

#endif
