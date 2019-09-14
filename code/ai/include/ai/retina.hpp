#ifndef AI_RETINA_HPP_INCLUDED
#   define AI_RETINA_HPP_INCLUDED

#   include <qtgl/offscreen.hpp>
#   include <memory>

namespace ai {


struct  retina
{
    using  depth_image = qtgl::offscreen::depth_image;
    using  depth_image_ptr = qtgl::offscreen_depth_image_ptr;

    using  colour_image = qtgl::offscreen::colour_image;
    using  colour_image_ptr = qtgl::offscreen_colour_image_ptr;

    retina(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture)
        : m_depth_image(qtgl::make_offscreen_depth_image(width_in_pixels, height_in_pixels))
        , m_colour_image(use_colour_texture ? qtgl::make_offscreen_colour_image(width_in_pixels, height_in_pixels) : nullptr)
    {
        qtgl::clear_offscreen_depth_image(*m_depth_image);
        if (m_colour_image != nullptr) qtgl::clear_offscreen_colour_image(*m_colour_image);
    }

    natural_32_bit  get_width_in_pixels() const { return m_depth_image->get_width_in_pixels(); }
    natural_32_bit  get_height_in_pixels() const { return m_depth_image->get_height_in_pixels(); }

    depth_image_ptr  get_depth_image() const { return m_depth_image; }
    colour_image_ptr  get_colour_image() const { return m_colour_image; }

private:
    depth_image_ptr  m_depth_image;
    colour_image_ptr  m_colour_image;
};


using  retina_ptr = std::shared_ptr<retina>;


inline retina_ptr  make_retina(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture)
{
    return std::make_shared<retina>(width_in_pixels, height_in_pixels, use_colour_texture);
}


}

#endif
