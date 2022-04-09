#include <osi/provider.hpp>
#include <GLFW/glfw3.h>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/config.hpp>
#include <utility/log.hpp>
#include <array>
#include <algorithm>
#include <iostream>
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
#   include <emscripten.h>
#endif
namespace osi { namespace {


GLFWwindow*  window_ptr = nullptr;

struct  round_state
{
    round_state()
        : round_number(0ULL)
        , round_start_time(0.0f)
        , round_time_delta(0.0f)
        , has_focus(true)
        , focus_just_received(true)
        , focus_just_lost(false)
        , close_requested(false)
        , width(1U)
        , height(1U)
        , text()
        , keys_pressed()
        , keys_just_pressed()
        , keys_just_released()
        , cursor_x(0.0f)
        , cursor_y(0.0f)
        , cursor_x_delta(0.0f)
        , cursor_y_delta(0.0f)
        , wheel_delta_x(0.0f)
        , wheel_delta_y(0.0f)
        , buttons_pressed()
        , buttons_just_pressed()
        , buttons_just_released()
        , error_msg()
    {}

    natural_64_bit  round_number;
    float_64_bit  round_start_time;
    float_32_bit  round_time_delta;
    bool  has_focus;
    bool  focus_just_received;
    bool  focus_just_lost;
    bool  close_requested;
    natural_16_bit  width;
    natural_16_bit  height;
    std::string  text;
    std::unordered_set<keyboard_key_name>  keys_pressed;
    std::unordered_set<keyboard_key_name>  keys_just_pressed;
    std::unordered_set<keyboard_key_name>  keys_just_released;
    float_32_bit  cursor_x;
    float_32_bit  cursor_y;
    float_32_bit  cursor_x_delta;
    float_32_bit  cursor_y_delta;
    float_32_bit  wheel_delta_x;
    float_32_bit  wheel_delta_y;
    std::unordered_set<mouse_button_name>  buttons_pressed;
    std::unordered_set<mouse_button_name>  buttons_just_pressed;
    std::unordered_set<mouse_button_name>  buttons_just_released;
    std::string  error_msg;
};

std::array<round_state, 2>  states;

round_state&  current_state() { return states.at(0); }
round_state&  other_state() { return states.at(1); }

void swap_states()
{
    std::swap(current_state(), other_state());

    current_state().round_number = other_state().round_number + 1ULL;
    current_state().round_start_time = seconds_openned();
    current_state().round_time_delta = (float_32_bit)(current_state().round_start_time - other_state().round_start_time);

    current_state().focus_just_received = current_state().has_focus && !other_state().has_focus;
    current_state().focus_just_lost = !current_state().has_focus && other_state().has_focus;

    current_state().close_requested = current_state().close_requested || other_state().close_requested;

    int  w, h;
    glfwGetWindowSize(window_ptr, &w, &h);
    current_state().width = (natural_16_bit)w;
    current_state().height = (natural_16_bit)h;

    double  cx, cy;
    glfwGetCursorPos(window_ptr, &cx, &cy);
    current_state().cursor_x_delta = (float_32_bit)cx - other_state().cursor_x;
    current_state().cursor_y_delta = (float_32_bit)cy - other_state().cursor_y;
    current_state().cursor_x = (float_32_bit)cx;
    current_state().cursor_y = (float_32_bit)cy;

    other_state().text.clear();
    other_state().keys_pressed = current_state().keys_pressed;
    other_state().keys_just_pressed.clear();
    other_state().keys_just_released.clear();
    other_state().buttons_pressed = current_state().buttons_pressed;
    other_state().buttons_just_pressed.clear();
    other_state().buttons_just_released.clear();
    other_state().error_msg.clear();
    other_state().has_focus = current_state().has_focus;
    other_state().wheel_delta_x = 0.0f;
    other_state().wheel_delta_y = 0.0f;
}


void  on_error(int error, const char*  description)
{
    other_state().error_msg = description;
}


keyboard_key_name  to_key_name(int const  key)
{
    switch (key)
    {
    case GLFW_KEY_SPACE              : return KEY_SPACE();
    case GLFW_KEY_APOSTROPHE         : return KEY_RAPOSTROPH();
    case GLFW_KEY_COMMA              : return KEY_COMMA();
    case GLFW_KEY_MINUS              : return KEY_MINUS();
    case GLFW_KEY_PERIOD             : return KEY_DOT();
    case GLFW_KEY_SLASH              : return KEY_SLASH();
    case GLFW_KEY_0                  : return KEY_0();
    case GLFW_KEY_1                  : return KEY_1();
    case GLFW_KEY_2                  : return KEY_2();
    case GLFW_KEY_3                  : return KEY_3();
    case GLFW_KEY_4                  : return KEY_4();
    case GLFW_KEY_5                  : return KEY_5();
    case GLFW_KEY_6                  : return KEY_6();
    case GLFW_KEY_7                  : return KEY_7();
    case GLFW_KEY_8                  : return KEY_8();
    case GLFW_KEY_9                  : return KEY_9();
    case GLFW_KEY_SEMICOLON          : return KEY_SEMICOLON();
    case GLFW_KEY_EQUAL              : return KEY_EQUAL();
    case GLFW_KEY_A                  : return KEY_A();
    case GLFW_KEY_B                  : return KEY_B();
    case GLFW_KEY_C                  : return KEY_C();
    case GLFW_KEY_D                  : return KEY_D();
    case GLFW_KEY_E                  : return KEY_E();
    case GLFW_KEY_F                  : return KEY_F();
    case GLFW_KEY_G                  : return KEY_G();
    case GLFW_KEY_H                  : return KEY_H();
    case GLFW_KEY_I                  : return KEY_I();
    case GLFW_KEY_J                  : return KEY_J();
    case GLFW_KEY_K                  : return KEY_K();
    case GLFW_KEY_L                  : return KEY_L();
    case GLFW_KEY_M                  : return KEY_M();
    case GLFW_KEY_N                  : return KEY_N();
    case GLFW_KEY_O                  : return KEY_O();
    case GLFW_KEY_P                  : return KEY_P();
    case GLFW_KEY_Q                  : return KEY_Q();
    case GLFW_KEY_R                  : return KEY_R();
    case GLFW_KEY_S                  : return KEY_S();
    case GLFW_KEY_T                  : return KEY_T();
    case GLFW_KEY_U                  : return KEY_U();
    case GLFW_KEY_V                  : return KEY_V();
    case GLFW_KEY_W                  : return KEY_W();
    case GLFW_KEY_X                  : return KEY_X();
    case GLFW_KEY_Y                  : return KEY_Y();
    case GLFW_KEY_Z                  : return KEY_Z();
    case GLFW_KEY_LEFT_BRACKET       : return KEY_LBRACKET();
    case GLFW_KEY_BACKSLASH          : return KEY_BACKSLASH();
    case GLFW_KEY_RIGHT_BRACKET      : return KEY_RBRACKET();
    case GLFW_KEY_GRAVE_ACCENT       : return KEY_LAPOSTROPH();
    //case GLFW_KEY_WORLD_1            : return KEY_();
    //case GLFW_KEY_WORLD_2            : return KEY_();
    case GLFW_KEY_ESCAPE             : return KEY_ESCAPE();
    case GLFW_KEY_ENTER              : return KEY_RETURN();
    case GLFW_KEY_TAB                : return KEY_TAB();
    case GLFW_KEY_BACKSPACE          : return KEY_BACKSPACE();
    case GLFW_KEY_INSERT             : return KEY_INSERT();
    case GLFW_KEY_DELETE             : return KEY_DELETE();
    case GLFW_KEY_RIGHT              : return KEY_RIGHT();
    case GLFW_KEY_LEFT               : return KEY_LEFT();
    case GLFW_KEY_DOWN               : return KEY_DOWN();
    case GLFW_KEY_UP                 : return KEY_UP();
    case GLFW_KEY_PAGE_UP            : return KEY_PAGEUP();
    case GLFW_KEY_PAGE_DOWN          : return KEY_PAGEDOWN();
    case GLFW_KEY_HOME               : return KEY_HOME();
    case GLFW_KEY_END                : return KEY_END();
    case GLFW_KEY_CAPS_LOCK          : return KEY_CAPSLOCK();
    case GLFW_KEY_SCROLL_LOCK        : return KEY_SCROLLLOCK();
    case GLFW_KEY_NUM_LOCK           : return KEY_NUMLOCK();
    case GLFW_KEY_PRINT_SCREEN       : return KEY_PRINTSCR();
    case GLFW_KEY_PAUSE              : return KEY_PAUSE();
    case GLFW_KEY_F1                 : return KEY_F1();
    case GLFW_KEY_F2                 : return KEY_F2();
    case GLFW_KEY_F3                 : return KEY_F3();
    case GLFW_KEY_F4                 : return KEY_F4();
    case GLFW_KEY_F5                 : return KEY_F5();
    case GLFW_KEY_F6                 : return KEY_F6();
    case GLFW_KEY_F7                 : return KEY_F7();
    case GLFW_KEY_F8                 : return KEY_F8();
    case GLFW_KEY_F9                 : return KEY_F9();
    case GLFW_KEY_F10                : return KEY_F10();
    case GLFW_KEY_F11                : return KEY_F11();
    case GLFW_KEY_F12                : return KEY_F12();
    //case GLFW_KEY_F13                : return KEY_();
    //case GLFW_KEY_F14                : return KEY_();
    //case GLFW_KEY_F15                : return KEY_();
    //case GLFW_KEY_F16                : return KEY_();
    //case GLFW_KEY_F17                : return KEY_();
    //case GLFW_KEY_F18                : return KEY_();
    //case GLFW_KEY_F19                : return KEY_();
    //case GLFW_KEY_F20                : return KEY_();
    //case GLFW_KEY_F21                : return KEY_();
    //case GLFW_KEY_F22                : return KEY_();
    //case GLFW_KEY_F23                : return KEY_();
    //case GLFW_KEY_F24                : return KEY_();
    //case GLFW_KEY_F25                : return KEY_();
    case GLFW_KEY_KP_0               : return KEY_NUMERIC_0();
    case GLFW_KEY_KP_1               : return KEY_NUMERIC_1();
    case GLFW_KEY_KP_2               : return KEY_NUMERIC_2();
    case GLFW_KEY_KP_3               : return KEY_NUMERIC_3();
    case GLFW_KEY_KP_4               : return KEY_NUMERIC_4();
    case GLFW_KEY_KP_5               : return KEY_NUMERIC_5();
    case GLFW_KEY_KP_6               : return KEY_NUMERIC_6();
    case GLFW_KEY_KP_7               : return KEY_NUMERIC_7();
    case GLFW_KEY_KP_8               : return KEY_NUMERIC_8();
    case GLFW_KEY_KP_9               : return KEY_NUMERIC_9();
    case GLFW_KEY_KP_DECIMAL         : return KEY_NUMERIC_DOT();
    case GLFW_KEY_KP_DIVIDE          : return KEY_NUMERIC_DIVIDE();
    case GLFW_KEY_KP_MULTIPLY        : return KEY_NUMERIC_TIMES();
    case GLFW_KEY_KP_SUBTRACT        : return KEY_NUMERIC_MINUS();
    case GLFW_KEY_KP_ADD             : return KEY_NUMERIC_PLUS();
    case GLFW_KEY_KP_ENTER           : return KEY_NUMERIC_RETURN();
    //case GLFW_KEY_KP_EQUAL           : return KEY_();
    case GLFW_KEY_LEFT_SHIFT         : return KEY_LSHIFT();
    case GLFW_KEY_LEFT_CONTROL       : return KEY_LCTRL();
    case GLFW_KEY_LEFT_ALT           : return KEY_LALT();
    //case GLFW_KEY_LEFT_SUPER         : return KEY_();
    case GLFW_KEY_RIGHT_SHIFT        : return KEY_RSHIFT();
    case GLFW_KEY_RIGHT_CONTROL      : return KEY_RCTRL();
    case GLFW_KEY_RIGHT_ALT          : return KEY_RALT();
    //case GLFW_KEY_RIGHT_SUPER        : return KEY_();
    //case GLFW_KEY_MENU               : return KEY_();
    default: return "";
    }
}


void  on_keyboard_event(GLFWwindow* const  window, int const  key, int const  scancode, int const  action, int const  mods)
{
    keyboard_key_name const  name = to_key_name(key);
    if (name.empty())
        return;
    if (action == GLFW_RELEASE)
    {
        auto const  it = other_state().keys_just_pressed.find(name);
        if (it == other_state().keys_just_pressed.end())
            other_state().keys_just_released.insert(name);
        else
            other_state().keys_just_pressed.erase(name);
        other_state().keys_pressed.erase(name);
    }
    else
    {
        auto const  it = other_state().keys_just_released.find(name);
        if (it == other_state().keys_just_released.end())
            other_state().keys_just_pressed.insert(name);
        else
            other_state().keys_just_released.erase(name);
        other_state().keys_pressed.insert(name);
    }
}


void  on_character_event(GLFWwindow* const  window, unsigned int const  char_unicode)
{
    other_state().text.push_back(char_unicode);
}


mouse_button_name  to_button_name(int const  button)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT: return LEFT_MOUSE_BUTTON();
    case GLFW_MOUSE_BUTTON_RIGHT: return RIGHT_MOUSE_BUTTON();
    case GLFW_MOUSE_BUTTON_MIDDLE: return MIDDLE_MOUSE_BUTTON();
    default: return "";
    }
}


void  on_mouse_button_event(GLFWwindow* const  window, int const  button, int const  action, int const  mods)
{
    mouse_button_name const  name = to_button_name(button);
    if (name.empty())
        return;
    if (action == GLFW_RELEASE)
    {
        auto const  it = other_state().buttons_just_pressed.find(name);
        if (it == other_state().buttons_just_pressed.end())
            other_state().buttons_just_released.insert(name);
        else
            other_state().buttons_just_pressed.erase(name);
        other_state().buttons_pressed.erase(name);
    }
    else
    {
        auto const  it = other_state().buttons_just_released.find(name);
        if (it == other_state().buttons_just_released.end())
            other_state().buttons_just_pressed.insert(name);
        else
            other_state().buttons_just_released.erase(name);
        other_state().buttons_pressed.insert(name);
    }
}


void  on_mouse_wheel_move(GLFWwindow* const  window, double const  xoffset, double const  yoffset)
{
    other_state().wheel_delta_x += (float_32_bit)xoffset;
    other_state().wheel_delta_y += (float_32_bit)yoffset;
}


void  on_focus_changed(GLFWwindow* const  window, int const  state)
{
    other_state().has_focus = (state == GLFW_TRUE);
}


}}

namespace osi {


void  open()
{
    ASSUMPTION(!is_open());

    glfwSetErrorCallback(on_error);

    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window_ptr = glfwCreateWindow(640, 480, "", NULL, NULL);
    if (window_ptr == nullptr)
    {
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window_ptr);

#if PLATFORM() != PLATFORM_WEBASSEMBLY()
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window_ptr);
        glfwTerminate();
        return;
    }

    glfwSwapInterval(0);
#endif

    glfwSetKeyCallback(window_ptr, on_keyboard_event);
    glfwSetCharCallback(window_ptr, on_character_event);

    glfwSetInputMode(window_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window_ptr, on_mouse_button_event);
    glfwSetScrollCallback(window_ptr, on_mouse_wheel_move);

    glfwSetWindowFocusCallback(window_ptr, on_focus_changed);

    glfwSetTime(0.0);
}


void  close()
{
    ASSUMPTION(is_open());

    glfwDestroyWindow(window_ptr);
    glfwTerminate();
}


bool  is_open()
{
    return window_ptr != nullptr;
}


bool  is_close_requested()
{
    return is_open() && (current_state().close_requested || glfwWindowShouldClose(window_ptr) != 0);
}


void  send_close_request()
{
    current_state().close_requested = true;
}


void  set_window_title(std::string const&  title)
{
    glfwSetWindowTitle(window_ptr, title.c_str());
}


void  set_window_icon(natural_8_bit const  width, natural_8_bit const  height, std::vector<natural_8_bit> const&  pixels_rgba_8888)
{
    GLFWimage img;
    img.width = width;
    img.height = height;
    img.pixels = (unsigned char*)pixels_rgba_8888.data();
    glfwSetWindowIcon(window_ptr, 1, &img);
}


void  set_window_pos(natural_16_bit const  x, natural_16_bit const  y)
{
    glfwSetWindowPos(window_ptr, x, y);
}


void  set_window_size(natural_16_bit const  width, natural_16_bit const  height)
{
    glfwSetWindowSize(window_ptr, width, height);
}


void  restore_window()
{
    glfwRestoreWindow(window_ptr);
}


void  maximise_window()
{
    glfwMaximizeWindow(window_ptr);
}


natural_16_bit  window_frame_size_left()
{
    int  left;
#if PLATFORM() != PLATFORM_WEBASSEMBLY()
    glfwGetWindowFrameSize(window_ptr, &left, nullptr, nullptr, nullptr);
#else
    left = 0;
#endif
    return (natural_16_bit)left;
}


natural_16_bit  window_frame_size_right()
{
    int  right;
#if PLATFORM() != PLATFORM_WEBASSEMBLY()
    glfwGetWindowFrameSize(window_ptr, nullptr, nullptr, &right, nullptr);
#else
    int h,f;
    emscripten_get_canvas_size(&right,&h,&f);
#endif
    return (natural_16_bit)right;
}


natural_16_bit  window_frame_size_top()
{
    int  top;
#if PLATFORM() != PLATFORM_WEBASSEMBLY()
    glfwGetWindowFrameSize(window_ptr, nullptr, &top, nullptr, nullptr);
#else
    top = 0;
#endif
    return (natural_16_bit)top;
}


natural_16_bit  window_frame_size_bottom()
{
    int bottom;
#if PLATFORM() != PLATFORM_WEBASSEMBLY()
    glfwGetWindowFrameSize(window_ptr, nullptr, nullptr, nullptr, &bottom);
#else
    int w,f;
    emscripten_get_canvas_size(&w,&bottom,&f);
#endif
    return (natural_16_bit)bottom;
}


void  start_round()
{
    ASSUMPTION(is_open());

    glfwMakeContextCurrent(window_ptr);
}


void  finish_round()
{
    ASSUMPTION(is_open());

    swap_states();

    INVARIANT(glGetError() == 0U);
    glfwSwapBuffers(window_ptr);
    INVARIANT(glGetError() == 0U);
    glfwPollEvents();
    INVARIANT(glGetError() == 0U);
}


natural_64_bit  round_number()
{
    return current_state().round_number;
}


float_64_bit  seconds_openned()
{
    ASSUMPTION(is_open());
    return glfwGetTime();
}


float_64_bit  round_start_time()
{
    return current_state().round_start_time;
}


float_32_bit  round_seconds()
{
    return current_state().round_time_delta;
}


bool  has_focus()
{
    return current_state().has_focus;
}


bool  focus_just_received()
{
    return current_state().focus_just_received;
}


bool  focus_just_lost()
{
    return current_state().focus_just_lost;
}


natural_16_bit  window_width()
{
    return current_state().width;
}


natural_16_bit  window_height()
{
    return current_state().height;
}


std::string const&  typed_text()
{
    return current_state().text;
}


std::unordered_set<keyboard_key_name> const&  keys_pressed()
{
    return  current_state().keys_pressed;
}


std::unordered_set<keyboard_key_name> const&  keys_just_pressed()
{
    return  current_state().keys_just_pressed;
}


std::unordered_set<keyboard_key_name> const&  keys_just_released()
{
    return  current_state().keys_just_released;
}


float_32_bit  cursor_x()
{
    return current_state().cursor_x;
}


float_32_bit  cursor_y()
{
    return current_state().cursor_y;
}


float_32_bit  cursor_x_delta()
{
    return current_state().cursor_x_delta;
}


float_32_bit  cursor_y_delta()
{
    return current_state().cursor_y_delta;
}


float_32_bit  wheel_delta_x()
{
    return current_state().wheel_delta_x;
}


float_32_bit  wheel_delta_y()
{
    return current_state().wheel_delta_y;
}


std::unordered_set<keyboard_key_name> const&  buttons_pressed()
{
    return  current_state().buttons_pressed;
}


std::unordered_set<keyboard_key_name> const&  buttons_just_pressed()
{
    return  current_state().buttons_just_pressed;
}


std::unordered_set<keyboard_key_name> const&  buttons_just_released()
{
    return  current_state().buttons_just_released;
}


std::string const&  error_text()
{
    return current_state().error_msg;
}


}
