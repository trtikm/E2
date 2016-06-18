#ifndef QTGL_KEYBOARD_PROPS_HPP_INCLUDED
#   define QTGL_KEYBOARD_PROPS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_set>
#   include <string>

namespace qtgl {


typedef std::string  keyboard_key_name;

keyboard_key_name  KEY_0();
keyboard_key_name  KEY_1();
keyboard_key_name  KEY_2();
keyboard_key_name  KEY_3();
keyboard_key_name  KEY_4();
keyboard_key_name  KEY_5();
keyboard_key_name  KEY_6();
keyboard_key_name  KEY_7();
keyboard_key_name  KEY_8();
keyboard_key_name  KEY_9();

keyboard_key_name  KEY_A();
keyboard_key_name  KEY_B();
keyboard_key_name  KEY_C();
keyboard_key_name  KEY_D();
keyboard_key_name  KEY_E();
keyboard_key_name  KEY_F();
keyboard_key_name  KEY_G();
keyboard_key_name  KEY_H();
keyboard_key_name  KEY_I();
keyboard_key_name  KEY_J();
keyboard_key_name  KEY_K();
keyboard_key_name  KEY_L();
keyboard_key_name  KEY_M();
keyboard_key_name  KEY_N();
keyboard_key_name  KEY_O();
keyboard_key_name  KEY_P();
keyboard_key_name  KEY_Q();
keyboard_key_name  KEY_R();
keyboard_key_name  KEY_S();
keyboard_key_name  KEY_T();
keyboard_key_name  KEY_U();
keyboard_key_name  KEY_V();
keyboard_key_name  KEY_W();
keyboard_key_name  KEY_X();
keyboard_key_name  KEY_Y();
keyboard_key_name  KEY_Z();

keyboard_key_name  KEY_LAPOSTROPH();
keyboard_key_name  KEY_RAPOSTROPH();
keyboard_key_name  KEY_UNDERSCORE();
keyboard_key_name  KEY_EQUAL();
keyboard_key_name  KEY_LBRACKET();
keyboard_key_name  KEY_RBRACKET();
keyboard_key_name  KEY_SEMICOLON();
keyboard_key_name  KEY_BACKSLASH();
keyboard_key_name  KEY_SLASH();
keyboard_key_name  KEY_DOT();
keyboard_key_name  KEY_COMMA();

keyboard_key_name  KEY_NUMLOCK();

keyboard_key_name  KEY_NUMERIC_0();
keyboard_key_name  KEY_NUMERIC_1();
keyboard_key_name  KEY_NUMERIC_2();
keyboard_key_name  KEY_NUMERIC_3();
keyboard_key_name  KEY_NUMERIC_4();
keyboard_key_name  KEY_NUMERIC_5();
keyboard_key_name  KEY_NUMERIC_6();
keyboard_key_name  KEY_NUMERIC_7();
keyboard_key_name  KEY_NUMERIC_8();
keyboard_key_name  KEY_NUMERIC_9();

keyboard_key_name  KEY_NUMERIC_DOT();
keyboard_key_name  KEY_NUMERIC_RETURN();
keyboard_key_name  KEY_NUMERIC_PLUS();
keyboard_key_name  KEY_NUMERIC_MINUS();
keyboard_key_name  KEY_NUMERIC_TIMES();
keyboard_key_name  KEY_NUMERIC_DIVIDE();

keyboard_key_name  KEY_TAB();
keyboard_key_name  KEY_SPACE();
keyboard_key_name  KEY_RETURN();

keyboard_key_name  KEY_LSHIFT();
keyboard_key_name  KEY_RSHIFT();
keyboard_key_name  KEY_CAPSLOCK();

keyboard_key_name  KEY_LEFT();
keyboard_key_name  KEY_RIGHT();
keyboard_key_name  KEY_UP();
keyboard_key_name  KEY_DOWM();

keyboard_key_name  KEY_PAGEUP();
keyboard_key_name  KEY_PAGEDOWN();
keyboard_key_name  KEY_HOME();
keyboard_key_name  KEY_END();

keyboard_key_name  KEY_INSERT();
keyboard_key_name  KEY_DELETE();
keyboard_key_name  KEY_BACKSPACE();

keyboard_key_name  KEY_LALT();
keyboard_key_name  KEY_RALT();
keyboard_key_name  KEY_LCTRL();
keyboard_key_name  KEY_RCTRL();

keyboard_key_name  KEY_ESCAPE();

keyboard_key_name  KEY_PAUSE();
keyboard_key_name  KEY_PRTSCR();

keyboard_key_name  KEY_F1();
keyboard_key_name  KEY_F2();
keyboard_key_name  KEY_F3();
keyboard_key_name  KEY_F4();
keyboard_key_name  KEY_F5();
keyboard_key_name  KEY_F6();
keyboard_key_name  KEY_F7();
keyboard_key_name  KEY_F8();
keyboard_key_name  KEY_F9();
keyboard_key_name  KEY_F10();
keyboard_key_name  KEY_F11();
keyboard_key_name  KEY_F12();


struct keyboard_props
{
    keyboard_props();
    keyboard_props(
            std::vector<std::string> const&  text,
            std::unordered_set<keyboard_key_name> const&  pressed,
            std::unordered_set<keyboard_key_name> const&  just_pressed,
            std::unordered_set<keyboard_key_name> const&  just_released
            );

    bool  is_pressed(keyboard_key_name const&  key_name) const { return m_pressed.count(key_name) != 0ULL; }

    bool  was_just_pressed(keyboard_key_name const&  key_name) const { return m_just_pressed.count(key_name) != 0ULL; }
    bool  was_just_released(keyboard_key_name const&  key_name) const { return m_just_released.count(key_name) != 0ULL; }

    std::vector<std::string> const&  text() const noexcept { return m_text; }

    std::unordered_set<keyboard_key_name> const&  pressed() const noexcept { return m_pressed; }
    std::unordered_set<keyboard_key_name> const&  just_pressed() const noexcept { return m_just_pressed; }
    std::unordered_set<keyboard_key_name> const&  just_released() const noexcept { return m_just_released; }

private:
    std::vector<std::string>  m_text;
    std::unordered_set<keyboard_key_name>  m_pressed;
    std::unordered_set<keyboard_key_name>  m_just_pressed;
    std::unordered_set<keyboard_key_name>  m_just_released;
};


}

#endif
