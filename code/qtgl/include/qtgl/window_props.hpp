#ifndef QTGL_WINDOW_PROPS_HPP_INCLUDED
#   define QTGL_WINDOW_PROPS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace qtgl {


struct window_props
{
    window_props(
            natural_32_bit const  width_in_pixels,
            natural_32_bit const  height_in_pixels,
            float_32_bit const  pixel_width_in_milimeters,
            float_32_bit const  pixel_height_in_milimeters,
            bool const  is_whole_program_active,
            bool const  has_focus
            );

    natural_32_bit  width_in_pixels() const { return m_width_in_pixels; }
    natural_32_bit  height_in_pixels() const { return m_height_in_pixels; }

    float_32_bit  pixel_width_in_milimeters() const { return m_pixel_width_in_milimeters; }
    float_32_bit  pixel_height_in_milimeters() const { return m_pixel_height_in_milimeters; }

    bool  is_whole_program_active() const { return m_is_whole_program_active; }

    bool  has_focus() const { return m_has_focus; }

private:
    natural_32_bit  m_width_in_pixels;
    natural_32_bit  m_height_in_pixels;
    float_32_bit  m_pixel_width_in_milimeters;
    float_32_bit  m_pixel_height_in_milimeters;
    bool  m_is_whole_program_active;
    bool  m_has_focus;
};


float_32_bit  window_width_in_meters(window_props const&  window_info);
float_32_bit  window_height_in_meters(window_props const&  window_info);


}

#endif
