#ifndef COM_CONSOLE_RENDER_PROPS_HPP_INCLUDED
#   define COM_CONSOLE_RENDER_PROPS_HPP_INCLUDED

#   include <gfx/batch.hpp>
#   include <gfx/batch_generators.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace com {


struct  console_render_props
{
    struct  text_id
    {
        text_id();
        text_id(std::string const&  text_, float_32_bit const  scale_, float_32_bit const  width_);
        bool  operator==(text_id const&  other) const;
        bool  operator!=(text_id const&  other) const;

        std::string  text;
        float_32_bit  scale;
        float_32_bit  width;
    };

    console_render_props();
    void  update();

    bool  last_show_console;

    text_id  tid;
    gfx::text_info  text_info;
    gfx::batch  text_batch;

    bool  cursor_visible;
    float_32_bit  cursor_countdown_seconds;
    float_32_bit  cursor_scale;
    gfx::batch  cursor_batch;
};


}

#endif
