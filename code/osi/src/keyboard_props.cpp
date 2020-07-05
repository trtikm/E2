#include <osi/keyboard_props.hpp>
#include <osi/provider.hpp>

namespace osi {


std::string const&  keyboard_props::typed_text() const { return osi::typed_text(); }
std::unordered_set<keyboard_key_name> const&  keyboard_props::keys_pressed() const { return osi::keys_pressed(); }
std::unordered_set<keyboard_key_name> const&  keyboard_props::keys_just_pressed() const { return osi::keys_just_pressed(); }
std::unordered_set<keyboard_key_name> const&  keyboard_props::keys_just_released() const { return osi::keys_just_released(); }


}
