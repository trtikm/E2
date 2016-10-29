#ifndef QTGL_FREE_FLY_HPP_INCLUDED
#   define QTGL_FREE_FLY_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>
#   include <qtgl/mouse_props.hpp>
#   include <qtgl/keyboard_props.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <functional>
#   include <vector>
#   include <utility>

namespace qtgl { namespace free_fly_controler {


typedef std::function<bool(mouse_props const&,keyboard_props const&)>
        controler_fn;

controler_fn  AND(std::vector<controler_fn> const&  controlers);
controler_fn  OR(std::vector<controler_fn> const&  controlers);
controler_fn  NOT(controler_fn const  controler);

controler_fn  mouse_button_pressed(mouse_button_name const&  button);
controler_fn  keyboard_key_pressed(keyboard_key_name const&  key);


}}

namespace qtgl {


struct  free_fly_action
{
    free_fly_action(
            bool const  do_rotation,  // 'true' for rotation and 'false' for translation
            bool const  use_world_axis, // 'true' for use of world axis and 'false' for local axis
            natural_8_bit const  axis_index, // 0,1,2 with the meaning x,y,z axis respectively
            natural_8_bit const  mouse_move_axis, // 0,1,2 with the meaning x,y,NO mouse axis respectively
            float_32_bit const  action_value, // the value used in the action, e.g. motion/rotation speed/shift
            free_fly_controler::controler_fn  const&  controler // determines when to perform the action, e.g. when a key is pressed
            );
    bool  do_rotation() const { return m_do_rotation; }
    bool  use_world_axis() const { return m_use_world_axis; }
    natural_8_bit  axis_index() const { return m_axis_index; }
    natural_8_bit  mouse_move_axis() const { return m_mouse_move_axis; }
    float_32_bit  action_value() const { return m_action_value; }
    free_fly_controler::controler_fn  const&  controler() const { return m_controler; }
private:
    bool  m_do_rotation;
    bool  m_use_world_axis;
    natural_8_bit  m_axis_index;
    natural_8_bit  m_mouse_move_axis;
    float_32_bit  m_action_value;
    free_fly_controler::controler_fn  m_controler;
};


typedef std::vector<free_fly_action>  free_fly_config;

typedef std::shared_ptr<free_fly_config>  free_fly_config_ptr;
typedef std::shared_ptr<free_fly_config const>  free_fly_config_const_ptr;


std::pair<bool, // was any translation performed ?
          bool  // was any rotation performed ?
>  free_fly(angeo::coordinate_system&  coord_system,
           free_fly_config const&  config,
           float_64_bit const  seconds_from_previous_call,
           mouse_props const&  mouse_info,
           keyboard_props const&  keyboard_info);


}

#endif
