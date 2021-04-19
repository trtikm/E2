#include <com/detail/console_render_props.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>

namespace com {


console_render_props::text_id::text_id()
    : text_id("", 1.0f, 1.0f)
{}


console_render_props::text_id::text_id(std::string const&  text_, float_32_bit const  scale_, float_32_bit const  width_)
    : text(text_)
    , scale(scale_)
    , width(width_)
{}


bool  console_render_props::text_id::operator==(text_id const&  other) const
{ return scale == other.scale && width == other.width && text == other.text; }


bool  console_render_props::text_id::operator!=(text_id const&  other) const
{ return !(*this == other); }


console_render_props::console_render_props()
    : last_show_console(true)
    , tid()
    , text_info()
    , text_batch()

    , cursor_visible(true)
    , cursor_countdown_seconds(1.0f)
    , cursor_scale(1.0f)
    , cursor_batch()
{}


}
