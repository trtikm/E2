#ifndef AI_INPUT_DEVICES_HPP_INCLUDED
#   define AI_INPUT_DEVICES_HPP_INCLUDED

#   include <qtgl/keyboard_props.hpp>
#   include <qtgl/mouse_props.hpp>
#   include <qtgl/window_props.hpp>
#   include <memory>

namespace ai {


struct  input_devices
{
    using  keyboard_props = qtgl::keyboard_props;
    using  mouse_props = qtgl::mouse_props;
    using  window_props = qtgl::window_props;

    keyboard_props  keyboard;
    mouse_props  mouse;
    window_props  window;
};


using  input_devices_ptr = std::shared_ptr<input_devices>;
using  input_devices_const_ptr = std::shared_ptr<input_devices const>;


}

#endif
