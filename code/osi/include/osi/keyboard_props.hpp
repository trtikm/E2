#ifndef OSI_KEYBOARD_PROPS_HPP_INCLUDED
#   define OSI_KEYBOARD_PROPS_HPP_INCLUDED

#   include <osi/keyboard_key_names.hpp>
#   include <unordered_set>
#   include <string>

namespace osi {


struct  keyboard_props
{
    std::string const&  typed_text() const;
    std::unordered_set<keyboard_key_name> const&  keys_pressed() const;
    std::unordered_set<keyboard_key_name> const&  keys_just_pressed() const;
    std::unordered_set<keyboard_key_name> const&  keys_just_released() const;
};


}

#endif
