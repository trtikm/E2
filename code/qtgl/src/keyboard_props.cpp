#include <qtgl/keyboard_props.hpp>
#include <utility/development.hpp>

namespace qtgl {


keyboard_key_name  KEY_0() { return "0"; }
keyboard_key_name  KEY_1() { return "1"; }
keyboard_key_name  KEY_2() { return "2"; }
keyboard_key_name  KEY_3() { return "3"; }
keyboard_key_name  KEY_4() { return "4"; }
keyboard_key_name  KEY_5() { return "5"; }
keyboard_key_name  KEY_6() { return "6"; }
keyboard_key_name  KEY_7() { return "7"; }
keyboard_key_name  KEY_8() { return "8"; }
keyboard_key_name  KEY_9() { return "9"; }

keyboard_key_name  KEY_A() { return "A"; }
keyboard_key_name  KEY_B() { return "B"; }
keyboard_key_name  KEY_C() { return "C"; }
keyboard_key_name  KEY_D() { return "D"; }
keyboard_key_name  KEY_E() { return "E"; }
keyboard_key_name  KEY_F() { return "F"; }
keyboard_key_name  KEY_G() { return "G"; }
keyboard_key_name  KEY_H() { return "H"; }
keyboard_key_name  KEY_I() { return "I"; }
keyboard_key_name  KEY_J() { return "J"; }
keyboard_key_name  KEY_K() { return "K"; }
keyboard_key_name  KEY_L() { return "L"; }
keyboard_key_name  KEY_M() { return "M"; }
keyboard_key_name  KEY_N() { return "N"; }
keyboard_key_name  KEY_O() { return "O"; }
keyboard_key_name  KEY_P() { return "P"; }
keyboard_key_name  KEY_Q() { return "Q"; }
keyboard_key_name  KEY_R() { return "R"; }
keyboard_key_name  KEY_S() { return "S"; }
keyboard_key_name  KEY_T() { return "T"; }
keyboard_key_name  KEY_U() { return "U"; }
keyboard_key_name  KEY_V() { return "V"; }
keyboard_key_name  KEY_W() { return "W"; }
keyboard_key_name  KEY_X() { return "X"; }
keyboard_key_name  KEY_Y() { return "Y"; }
keyboard_key_name  KEY_Z() { return "Z"; }

keyboard_key_name  KEY_LAPOSTROPH() { return "`"; }
keyboard_key_name  KEY_RAPOSTROPH() { return "'"; }
keyboard_key_name  KEY_UNDERSCORE() { return "_"; }
keyboard_key_name  KEY_EQUAL() { return "="; }
keyboard_key_name  KEY_LBRACKET() { return "["; }
keyboard_key_name  KEY_RBRACKET() { return "]"; }
keyboard_key_name  KEY_SEMICOLON() { return ";"; }
keyboard_key_name  KEY_BACKSLASH() { return "\\"; }
keyboard_key_name  KEY_SLASH() { return "/"; }
keyboard_key_name  KEY_DOT() { return "."; }
keyboard_key_name  KEY_COMMA() { return ","; }

keyboard_key_name  KEY_NUMLOCK() { return "NUMLOCK"; }

keyboard_key_name  KEY_NUMERIC_0() { return "NUM 0"; }
keyboard_key_name  KEY_NUMERIC_1() { return "NUM 1"; }
keyboard_key_name  KEY_NUMERIC_2() { return "NUM 2"; }
keyboard_key_name  KEY_NUMERIC_3() { return "NUM 3"; }
keyboard_key_name  KEY_NUMERIC_4() { return "NUM 4"; }
keyboard_key_name  KEY_NUMERIC_5() { return "NUM 5"; }
keyboard_key_name  KEY_NUMERIC_6() { return "NUM 6"; }
keyboard_key_name  KEY_NUMERIC_7() { return "NUM 7"; }
keyboard_key_name  KEY_NUMERIC_8() { return "NUM 8"; }
keyboard_key_name  KEY_NUMERIC_9() { return "NUM 9"; }

keyboard_key_name  KEY_NUMERIC_DOT() { return "NUM ."; }
keyboard_key_name  KEY_NUMERIC_RETURN() { return "NUM RETURN"; }
keyboard_key_name  KEY_NUMERIC_PLUS() { return "NUM +"; }
keyboard_key_name  KEY_NUMERIC_MINUS() { return "NUM -"; }
keyboard_key_name  KEY_NUMERIC_TIMES() { return "NUM *"; }
keyboard_key_name  KEY_NUMERIC_DIVIDE() { return "NUM /"; }

keyboard_key_name  KEY_TAB() { return "TAB"; }
keyboard_key_name  KEY_SPACE() { return "SPACE"; }
keyboard_key_name  KEY_RETURN() { return "RETURN"; }

keyboard_key_name  KEY_LSHIFT() { return "LSHIFT"; }
keyboard_key_name  KEY_RSHIFT() { return "RSHIFT"; }
keyboard_key_name  KEY_CAPSLOCK() { return "CAPSLOCK"; }

keyboard_key_name  KEY_LEFT() { return "LEFT"; }
keyboard_key_name  KEY_RIGHT() { return "RIGHT"; }
keyboard_key_name  KEY_UP() { return "UP"; }
keyboard_key_name  KEY_DOWN() { return "DOWN"; }

keyboard_key_name  KEY_PAGEUP() { return "PAGEUP"; }
keyboard_key_name  KEY_PAGEDOWN() { return "PAGEDOWN"; }
keyboard_key_name  KEY_HOME() { return "HOME"; }
keyboard_key_name  KEY_END() { return "END"; }

keyboard_key_name  KEY_INSERT() { return "INSERT"; }
keyboard_key_name  KEY_DELETE() { return "DELETE"; }
keyboard_key_name  KEY_BACKSPACE() { return "BACKSPACE"; }

keyboard_key_name  KEY_LALT() { return "LALT"; }
keyboard_key_name  KEY_RALT() { return "RALT"; }
keyboard_key_name  KEY_LCTRL() { return "LCTRL"; }
keyboard_key_name  KEY_RCTRL() { return "RCTRL"; }

keyboard_key_name  KEY_ESCAPE() { return "ESCAPE"; }

keyboard_key_name  KEY_PAUSE() { return "PAUSE"; }
keyboard_key_name  KEY_PRTSCR() { return "PRTSCR"; }

keyboard_key_name  KEY_F1() { return "F1"; }
keyboard_key_name  KEY_F2() { return "F2"; }
keyboard_key_name  KEY_F3() { return "F3"; }
keyboard_key_name  KEY_F4() { return "F4"; }
keyboard_key_name  KEY_F5() { return "F5"; }
keyboard_key_name  KEY_F6() { return "F6"; }
keyboard_key_name  KEY_F7() { return "F7"; }
keyboard_key_name  KEY_F8() { return "F8"; }
keyboard_key_name  KEY_F9() { return "F9"; }
keyboard_key_name  KEY_F10() { return "F10"; }
keyboard_key_name  KEY_F11() { return "F11"; }
keyboard_key_name  KEY_F12() { return "F12"; }


keyboard_props::keyboard_props()
    : m_text()
    , m_pressed()
    , m_just_pressed()
    , m_just_released()
{}

keyboard_props::keyboard_props(
        std::vector<std::string> const&  text,
        std::unordered_set<keyboard_key_name> const&  pressed,
        std::unordered_set<keyboard_key_name> const&  just_pressed,
        std::unordered_set<keyboard_key_name> const&  just_released
        )
    : m_text(text)
    , m_pressed(pressed)
    , m_just_pressed(just_pressed)
    , m_just_released(just_released)
{}


}
