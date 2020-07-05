#ifndef GFX_FREE_FLY_HPP_INCLUDED
#   define GFX_FREE_FLY_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>
#   include <osi/mouse_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <functional>
#   include <vector>
#   include <utility>

namespace gfx { namespace free_fly_controler {


typedef std::function<bool(osi::mouse_props const&,osi::keyboard_props const&)>
        controler_fn;

controler_fn  AND(std::vector<controler_fn> const&  controlers);
controler_fn  OR(std::vector<controler_fn> const&  controlers);
controler_fn  NOT(controler_fn const  controler);

controler_fn  mouse_button_pressed(osi::mouse_button_name const&  button);
controler_fn  keyboard_key_pressed(osi::keyboard_key_name const&  key);


}}

namespace gfx {


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
    bool  do_rotation() const noexcept { return m_do_rotation; }
    bool  use_world_axis() const noexcept { return m_use_world_axis; }
    natural_8_bit  axis_index() const noexcept { return m_axis_index; }
    natural_8_bit  mouse_move_axis() const noexcept { return m_mouse_move_axis; }
    float_32_bit  action_value() const noexcept { return m_action_value; }
    free_fly_controler::controler_fn  const&  controler() const noexcept { return m_controler; }

    void  set_action_value(float_32_bit const  value) { m_action_value = value; }
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


struct  free_fly_report
{
    bool  translated;
    bool  rotated;

    free_fly_report()
        : translated(false)
        , rotated(false)
    {}

    free_fly_report(bool const  translated_, bool const  rotated_)
        : translated(translated_)
        , rotated(rotated_)
    {}

    free_fly_report&  operator+=(free_fly_report const&  other)
    {
        translated = translated || other.translated;
        rotated = rotated || other.rotated;
        return *this;
    }
};

free_fly_report  free_fly(angeo::coordinate_system&  coord_system,
           free_fly_config const&  config,
           float_64_bit const  seconds_from_previous_call,
           osi::mouse_props const&  mouse_info,
           osi::keyboard_props const&  keyboard_info);


}

#endif
