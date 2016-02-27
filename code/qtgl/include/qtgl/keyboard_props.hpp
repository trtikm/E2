#ifndef QTGL_KEYBOARD_PROPS_HPP_INCLUDED
#   define QTGL_KEYBOARD_PROPS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace qtgl {


typedef std::string  keyboard_key_name;

keyboard_key_name  KEY_A();
keyboard_key_name  KEY_D();
keyboard_key_name  KEY_S();
keyboard_key_name  KEY_W();


struct keyboard_props
{
    keyboard_props();

    bool  is_pressed(keyboard_key_name const&  key_name) const;
    bool  was_just_pressed(keyboard_key_name const&  key_name) const;
    bool  was_just_released(keyboard_key_name const&  key_name) const;
};


}

#endif
