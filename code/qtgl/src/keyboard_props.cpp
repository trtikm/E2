#include <qtgl/keyboard_props.hpp>
#include <utility/development.hpp>

namespace qtgl {


keyboard_key_name  KEY_A() { return "KEY_A"; }
keyboard_key_name  KEY_D() { return "KEY_D"; }
keyboard_key_name  KEY_S() { return "KEY_S"; }
keyboard_key_name  KEY_W() { return "KEY_W"; }


keyboard_props::keyboard_props()
{}

bool  keyboard_props::is_pressed(keyboard_key_name const&  key_name) const
{
    NOT_IMPLEMENTED_YET();
}

bool  keyboard_props::was_just_pressed(keyboard_key_name const&  key_name) const
{
    NOT_IMPLEMENTED_YET();
}

bool  keyboard_props::was_just_released(keyboard_key_name const&  key_name) const
{
    NOT_IMPLEMENTED_YET();
}


}
