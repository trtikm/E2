#ifndef QTGL_OFFSCREEN_HPP_INCLUDED
#   define QTGL_OFFSCREEN_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <qtgl/texture.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <array>
#   include <memory>

namespace qtgl {


struct  offscreen final
{
    struct  depth_image
    {
        depth_image(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels)
            : m_width_in_pixels(width_in_pixels)
            , m_height_in_pixels(height_in_pixels)
            , m_data()
        {
            m_data.resize(width_in_pixels * height_in_pixels);
        }

        natural_32_bit  get_width_in_pixels() const { return m_width_in_pixels; }
        natural_32_bit  get_height_in_pixels() const { return m_height_in_pixels; }

        float_32_bit  operator()(natural_32_bit const  row, natural_32_bit const  column) const
        {
            return m_data.at(row * get_width_in_pixels() + column);
        }

        natural_32_bit  num_pixels() const { return (natural_32_bit)m_data.size(); }

        float_32_bit const*  data_ptr() const { return m_data.data(); }
        float_32_bit*  data_ptr() { return m_data.data(); }

    private:
        natural_32_bit  m_width_in_pixels;
        natural_32_bit  m_height_in_pixels;
        std::vector<float_32_bit>  m_data;
    };

    struct  colour_image
    {
        struct  rgb { natural_8_bit  r, g, b; };

        colour_image(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels)
            : m_width_in_pixels(width_in_pixels)
            , m_height_in_pixels(height_in_pixels)
            , m_data()
        {
            m_data.resize(width_in_pixels * height_in_pixels);
        }

        natural_32_bit  get_width_in_pixels() const { return m_width_in_pixels; }
        natural_32_bit  get_height_in_pixels() const { return m_height_in_pixels; }

        rgb const&  operator()(natural_32_bit const  row, natural_32_bit const  column) const
        {
            return m_data.at(row * get_width_in_pixels() + column);
        }

        natural_32_bit  num_pixels() const { return (natural_32_bit)m_data.size(); }

        rgb const*  data_ptr() const { return m_data.data(); }
        rgb*  data_ptr() { return m_data.data(); }

    private:
        natural_32_bit  m_width_in_pixels;
        natural_32_bit  m_height_in_pixels;
        std::vector<rgb>  m_data;
    };

    offscreen(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture);
    ~offscreen();

    natural_32_bit  get_width_in_pixels() const { return m_width_in_pixels; }
    natural_32_bit  get_height_in_pixels() const { return m_height_in_pixels; }
    GLuint  get_frame_buffer_object() const { return m_frame_buffer_object; }
    texture  get_depth_texture() const { return m_depth_texture; }
    texture  get_colour_texture() const { return m_colour_texture; }

private:
    void  release();

    natural_32_bit  m_width_in_pixels;
    natural_32_bit  m_height_in_pixels;
    GLuint  m_frame_buffer_object;
    texture  m_depth_texture;
    texture  m_colour_texture;
};


using  offscreen_ptr = std::shared_ptr<offscreen>;
using  offscreen_depth_image_ptr = std::shared_ptr<offscreen::depth_image>;
using  offscreen_colour_image_ptr = std::shared_ptr<offscreen::colour_image>;

offscreen_ptr  make_offscreen(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture);
offscreen_depth_image_ptr  make_offscreen_depth_image(offscreen const&  ofs);
offscreen_colour_image_ptr  make_offscreen_colour_image(offscreen const&  ofs);

texture  make_offscreen_compatible_depth_texture(offscreen const&  ofs);
texture  make_offscreen_compatible_colour_texture(offscreen const&  ofs);


struct  make_current_offsceen final
{
    explicit make_current_offsceen(offscreen const& ofs);
    ~make_current_offsceen();

    offscreen const&  get_offscreen() const { return *m_offscreen_ptr; }

private:
    void  release();
    offscreen const*  m_offscreen_ptr;
};


void  update_offscreen_depth_image(offscreen::depth_image&  image, make_current_offsceen const&  mofs);
void  update_offscreen_colour_image(offscreen::colour_image&  image, make_current_offsceen const&  mofs);


void  from_screen_space_to_camera_space(offscreen::depth_image&  image, float_32_bit const  near_plane, float_32_bit const  far_plane);
void  from_camera_space_to_interval_01(offscreen::depth_image& image, float_32_bit const  near_plane, float_32_bit const  far_plane);


void  copy_offscreen_depth_image_to_texture(offscreen::depth_image const& image, texture const  txt);
void  copy_offscreen_colour_image_to_texture(offscreen::colour_image const& image, texture const  txt);


std::vector< std::array<float_32_bit, 3> > const&  get_vertices_of_screen_quad();
std::vector< std::array<float_32_bit, 2> > const&  get_texcoords_of_screen_quad();


}

#endif
