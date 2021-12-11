#include <osi/window_props.hpp>
#include <osi/provider.hpp>

namespace osi {


bool  window_props::has_focus() const { return osi::has_focus(); }
bool  window_props::focus_just_received() const { return osi::focus_just_received(); }
bool  window_props::focus_just_lost() const { return osi::focus_just_lost(); }
bool  window_props::minimised() const { return !osi::has_focus() && window_width() == 0U && window_height() == 0U; }
natural_16_bit  window_props::window_width() const { return osi::window_width(); }
natural_16_bit  window_props::window_height() const { return osi::window_height(); }
float_32_bit  window_props::pixel_width_mm() const { return 0.1796875f; }
float_32_bit  window_props::pixel_height_mm() const { return 0.1805555f; }
natural_16_bit  window_props::window_frame_size_left() const { return osi::window_frame_size_left(); }
natural_16_bit  window_props::window_frame_size_right() const { return osi::window_frame_size_right(); }
natural_16_bit  window_props::window_frame_size_top() const { return osi::window_frame_size_top(); }
natural_16_bit  window_props::window_frame_size_bottom() const { return osi::window_frame_size_bottom(); }


}
