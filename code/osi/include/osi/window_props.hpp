#ifndef OSI_WINDOW_PROPS_HPP_INCLUDED
#   define OSI_WINDOW_PROPS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace osi {


struct  window_props
{
    bool  has_focus() const;
    bool  focus_just_received() const;
    bool  focus_just_lost() const;
    bool  minimised() const;
    natural_16_bit  window_width() const;
    natural_16_bit  window_height() const;
    float_32_bit  pixel_width_mm() const;
    float_32_bit  pixel_height_mm() const;
    natural_16_bit  window_frame_size_left() const;
    natural_16_bit  window_frame_size_right() const;
    natural_16_bit  window_frame_size_top() const;
    natural_16_bit  window_frame_size_bottom() const;
};


}

#endif
