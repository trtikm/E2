#ifndef QTGL_OFFSCREEN_HPP_INCLUDED
#   define QTGL_OFFSCREEN_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <qtgl/texture.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <array>
#   include <memory>
#   include <type_traits>

namespace qtgl {


struct  offscreen final
{
    template<typename T, bool return_by_value>
    struct  image
    {
        using  element_type = T;

        image(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels)
            : m_width_in_pixels(width_in_pixels)
            , m_height_in_pixels(height_in_pixels)
            , m_data()
        {
            m_data.resize(width_in_pixels * height_in_pixels);
        }

        natural_32_bit  get_width_in_pixels() const { return m_width_in_pixels; }
        natural_32_bit  get_height_in_pixels() const { return m_height_in_pixels; }

        typename std::conditional<return_by_value, element_type, element_type const&>::type
        operator()(natural_32_bit const  row, natural_32_bit const  column) const
        {
            return m_data.at(row * get_width_in_pixels() + column);
        }

        natural_32_bit  num_pixels() const { return (natural_32_bit)m_data.size(); }

        element_type const*  data_ptr() const { return m_data.data(); }
        element_type*  data_ptr() { return m_data.data(); }

    private:
        natural_32_bit  m_width_in_pixels;
        natural_32_bit  m_height_in_pixels;
        std::vector<element_type>  m_data;
    };

    using  depth_image_pixel = float_32_bit;
    struct  colour_image_pixel { float_32_bit  r, g, b, a; };

    using  depth_image = image<depth_image_pixel, true>;
    using  colour_image = image<colour_image_pixel, false>;

    offscreen(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture);
    ~offscreen();

    natural_32_bit  get_width_in_pixels() const { return m_width_in_pixels; }
    natural_32_bit  get_height_in_pixels() const { return m_height_in_pixels; }
    GLuint  get_frame_buffer_object() const { return m_frame_buffer_object; }

    texture  get_depth_texture() const { return m_depth_texture; }
    GLuint  get_depth_texture_pixel_buffer_object() const { return m_depth_texture_pixel_buffer_object; }

    texture  get_colour_texture() const { return m_colour_texture; }
    GLuint  get_colour_texture_pixel_buffer_object() const { return m_colour_texture_pixel_buffer_object; }

private:
    void  release();

    natural_32_bit  m_width_in_pixels;
    natural_32_bit  m_height_in_pixels;
    GLuint  m_frame_buffer_object;
    texture  m_depth_texture;
    GLuint  m_depth_texture_pixel_buffer_object;
    texture  m_colour_texture;
    GLuint  m_colour_texture_pixel_buffer_object;
};


using  offscreen_ptr = std::shared_ptr<offscreen>;
using  offscreen_depth_image_ptr = std::shared_ptr<offscreen::depth_image>;
using  offscreen_colour_image_ptr = std::shared_ptr<offscreen::colour_image>;


struct  make_current_offscreen final
{
    explicit make_current_offscreen(offscreen& ofs);
    ~make_current_offscreen();
private:
    void  release();
    offscreen* m_ofs;
};


offscreen_ptr  make_offscreen(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture);
offscreen_depth_image_ptr  make_offscreen_depth_image(offscreen const&  ofs);
offscreen_depth_image_ptr  make_offscreen_depth_image(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels);
offscreen_colour_image_ptr  make_offscreen_colour_image(offscreen const& ofs);
offscreen_colour_image_ptr  make_offscreen_colour_image(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels);

void  clear_offscreen_depth_image(offscreen::depth_image&  image, offscreen::depth_image_pixel const  value = 0.0f);
void  clear_offscreen_colour_image(offscreen::colour_image&  image, offscreen::colour_image_pixel const&  value = {0.0f, 0.0f, 0.0f, 0.0f});

void  update_offscreen_depth_image(offscreen::depth_image&  image, offscreen const&  ofs);
void  update_offscreen_colour_image(offscreen::colour_image&  image, offscreen const&  ofs);

void  from_screen_space_to_camera_space(offscreen::depth_image&  image, float_32_bit const  near_plane, float_32_bit const  far_plane);
void  from_camera_space_to_interval_01(offscreen::depth_image&  image, float_32_bit const  near_plane, float_32_bit const  far_plane);
void  normalise_offscreen_depth_image_interval_01(offscreen::depth_image& image);

void  modulate_offscreen_colour_image_by_depth_image(offscreen::colour_image&  colour_image, offscreen::depth_image const&  depth_image);


}

namespace qtgl { namespace dbg {


void  draw_offscreen_depth_image(
        offscreen::depth_image const&  image,
        natural_32_bit const  x_screen_pos,
        natural_32_bit const  y_screen_pos,
        natural_32_bit const  screen_width_in_pixels,
        natural_32_bit const  screen_height_in_pixels,
        float_32_bit const  scale = 1.0f
        );


void  draw_offscreen_colour_image(
        offscreen::colour_image const& image,
        natural_32_bit const  x_screen_pos,
        natural_32_bit const  y_screen_pos,
        natural_32_bit const  screen_width_in_pixels,
        natural_32_bit const  screen_height_in_pixels,
        float_32_bit const  scale = 1.0f
        );


}}


#endif
