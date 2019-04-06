#include <qtgl/window_props.hpp>
#include <utility/assumptions.hpp>

namespace qtgl {


window_props::window_props()
    : m_width_in_pixels(1)
    , m_height_in_pixels(1)
    , m_pixel_width_in_milimeters(1.0f)
    , m_pixel_height_in_milimeters(1.0f)
    , m_is_whole_program_active(true)
    , m_has_focus(true)
    , m_just_resized(false)
{}


window_props::window_props(
        natural_32_bit const  width_in_pixels,
        natural_32_bit const  height_in_pixels,
        float_32_bit const  pixel_width_in_milimeters,
        float_32_bit const  pixel_height_in_milimeters,
        bool const  is_whole_program_active,
        bool const  has_focus,
        bool const  just_resized
        )
    : m_width_in_pixels(width_in_pixels)
    , m_height_in_pixels(height_in_pixels)
    , m_pixel_width_in_milimeters(pixel_width_in_milimeters)
    , m_pixel_height_in_milimeters(pixel_height_in_milimeters)
    , m_is_whole_program_active(is_whole_program_active)
    , m_has_focus(has_focus)
    , m_just_resized(just_resized)
{
    ASSUMPTION(m_pixel_width_in_milimeters > 0.0f);
    ASSUMPTION(m_pixel_height_in_milimeters > 0.0f);
}


float_32_bit  window_width_in_meters(window_props const&  window_info)
{
    return (window_info.width_in_pixels() * window_info.pixel_width_in_milimeters()) / 1000.0f;
}

float_32_bit  window_height_in_meters(window_props const&  window_info)
{
    return (window_info.height_in_pixels() * window_info.pixel_height_in_milimeters()) / 1000.0f;
}


}
