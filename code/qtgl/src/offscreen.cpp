#include <qtgl/offscreen.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl { namespace detail {


texture  make_offscreen_texture(
        natural_32_bit const  width_in_pixels,
        natural_32_bit const  height_in_pixels,
        natural_32_bit const  pixel_format,
        natural_32_bit const  pixel_components_type
        )
{
    GLuint  colour_texture_id;
    glapi().glGenTextures(1, &colour_texture_id);
    ASSUMPTION(colour_texture_id != 0U);
    glapi().glBindTexture(GL_TEXTURE_2D, colour_texture_id);
    glapi().glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width_in_pixels, height_in_pixels, 0, pixel_format, pixel_components_type, NULL);
    glapi().glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glapi().glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glapi().glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glapi().glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    texture const texture(
        colour_texture_id,
        texture_file(
            "",
            pixel_format,
            GL_REPEAT,
            GL_REPEAT,
            GL_NEAREST,
            GL_NEAREST
            ),
        texture_image(
            width_in_pixels,
            height_in_pixels,
            nullptr,
            nullptr,
            3U,
            pixel_components_type
            )
        );

    INVARIANT(glapi().glGetError() == 0U);

    return texture;
}


}}

namespace qtgl {


offscreen::offscreen(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture)
    : m_width_in_pixels(width_in_pixels)
    , m_height_in_pixels(height_in_pixels)
    , m_frame_buffer_object(0U)
    , m_depth_texture()
    , m_colour_texture()
{
    ASSUMPTION(m_width_in_pixels > 0U && m_height_in_pixels > 0U);

    glapi().glGenFramebuffers(1, &m_frame_buffer_object);
    ASSUMPTION(m_frame_buffer_object != 0U);

    m_depth_texture = detail::make_offscreen_texture(m_width_in_pixels, m_height_in_pixels, GL_DEPTH_COMPONENT, GL_FLOAT);

    if (use_colour_texture)
        m_colour_texture = detail::make_offscreen_texture(m_width_in_pixels, m_height_in_pixels, GL_RGB, GL_UNSIGNED_BYTE);

    glapi().glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_object);
    glapi().glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture.id(), 0);

    if (use_colour_texture)
        glapi().glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour_texture.id(), 0);
    else
    {
        glapi().glDrawBuffer(GL_NONE);
        glapi().glReadBuffer(GL_NONE);
    }

    INVARIANT(glapi().glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glapi().glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glapi().glBindTexture(GL_TEXTURE_2D, 0);

    INVARIANT(glapi().glGetError() == 0U);
}


offscreen::~offscreen()
{
    release();
}


void offscreen::release()
{
    glapi().glDeleteFramebuffers(1, &m_frame_buffer_object);
    INVARIANT(glapi().glGetError() == 0U);
}


offscreen_ptr  make_offscreen(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture)
{
    return std::make_shared<offscreen>(width_in_pixels, height_in_pixels, use_colour_texture);
}


offscreen_depth_image_ptr  make_offscreen_depth_image(offscreen const& ofs)
{
    return make_offscreen_depth_image(ofs.get_width_in_pixels(), ofs.get_height_in_pixels());
}


offscreen_depth_image_ptr  make_offscreen_depth_image(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels)
{
    return std::make_shared<offscreen::depth_image>(width_in_pixels, height_in_pixels);
}


offscreen_colour_image_ptr  make_offscreen_colour_image(offscreen const& ofs)
{
    return make_offscreen_colour_image(ofs.get_width_in_pixels(), ofs.get_height_in_pixels());
}


offscreen_colour_image_ptr  make_offscreen_colour_image(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels)
{
    return std::make_shared<offscreen::colour_image>(width_in_pixels, height_in_pixels);
}


texture  make_offscreen_compatible_depth_texture(offscreen const&  ofs)
{
    return detail::make_offscreen_texture(ofs.get_width_in_pixels(), ofs.get_height_in_pixels(), GL_DEPTH_COMPONENT, GL_FLOAT);
}


texture  make_offscreen_compatible_colour_texture(offscreen const&  ofs)
{
    return detail::make_offscreen_texture(ofs.get_width_in_pixels(), ofs.get_height_in_pixels(), GL_RGB, GL_UNSIGNED_BYTE);
}


make_current_offsceen::make_current_offsceen(offscreen const&  ofs)
    : m_offscreen_ptr(&ofs)
{
    ASSUMPTION(ofs.get_frame_buffer_object() != 0U);
    glapi().glBindFramebuffer(GL_FRAMEBUFFER, ofs.get_frame_buffer_object());
    INVARIANT(glapi().glGetError() == 0U);
}


make_current_offsceen::~make_current_offsceen()
{
    release();
}


void  make_current_offsceen::release()
{
    glapi().glBindFramebuffer(GL_FRAMEBUFFER, 0);
    INVARIANT(glapi().glGetError() == 0U);
}


void  clear_offscreen_depth_image(offscreen::depth_image& image)
{
    for (float_32_bit*  ptr = image.data_ptr(), *  end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        *ptr = 0.0f;
}


void  clear_offscreen_colour_image(offscreen::colour_image&  image)
{
    for (offscreen::rgb*  ptr = image.data_ptr(), *  end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        *ptr = {0U, 0U, 0U};
}


void  update_offscreen_depth_image(offscreen::depth_image& image, make_current_offsceen const& mofs)
{
    TMPROF_BLOCK();

    texture const  txt = mofs.get_offscreen().get_depth_texture();

    ASSUMPTION(image.get_width_in_pixels() == txt.width() && image.get_height_in_pixels() == txt.height());

    glapi().glReadPixels(
        0, 0,
        image.get_width_in_pixels(), image.get_height_in_pixels(),
        txt.pixel_format(),
        txt.pixel_components_type(),
        image.data_ptr()
        );

    INVARIANT(qtgl::glapi().glGetError() == 0U);
}


void  update_offscreen_colour_image(offscreen::colour_image& image, make_current_offsceen const& mofs)
{
    TMPROF_BLOCK();

    texture const  txt = mofs.get_offscreen().get_colour_texture();

    ASSUMPTION(txt.loaded_successfully() && image.get_width_in_pixels() == txt.width() && image.get_height_in_pixels() == txt.height());

    glapi().glReadPixels(
        0, 0,
        image.get_width_in_pixels(), image.get_height_in_pixels(),
        txt.pixel_format(),
        txt.pixel_components_type(),
        image.data_ptr()
        );

    INVARIANT(qtgl::glapi().glGetError() == 0U);
}


void  from_screen_space_to_camera_space(offscreen::depth_image&  image, float_32_bit const  near_plane, float_32_bit const  far_plane)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        near_plane > 0.0f && far_plane > near_plane &&
        image.num_pixels() == image.get_width_in_pixels() * image.get_height_in_pixels()
        );

    float_32_bit const  a = 2.0f * near_plane * far_plane;
    float_32_bit const  b = near_plane + far_plane;
    float_32_bit const  c = far_plane - near_plane;
    for (float_32_bit*  ptr = image.data_ptr(), *  end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        *ptr = a / (b - c * (2.0f * (*ptr) - 1.0f));
}


void  from_camera_space_to_interval_01(offscreen::depth_image& image, float_32_bit const  near_plane, float_32_bit const  far_plane)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        near_plane > 0.0f && far_plane > near_plane &&
        image.num_pixels() == image.get_width_in_pixels() * image.get_height_in_pixels()
        );

    float_32_bit const  a = far_plane - near_plane;
    for (float_32_bit*  ptr = image.data_ptr(), *  end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        *ptr = (*ptr) / a;
}


void  copy_offscreen_depth_image_to_texture(offscreen::depth_image const& image, texture const  txt)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        txt.loaded_successfully() && txt.id() != 0 &&
        image.get_width_in_pixels() == txt.width() &&
        image.get_height_in_pixels() == txt.height() &&
        txt.pixel_format() == GL_DEPTH_COMPONENT &&
        txt.pixel_components_type() == GL_FLOAT
        );

    qtgl::glapi().glBindTexture(GL_TEXTURE_2D, txt.id());

    INVARIANT(qtgl::glapi().glGetError() == 0U);

    qtgl::glapi().glTexImage2D(GL_TEXTURE_2D, 0,
        txt.pixel_format(),
        txt.width(), txt.height(),
        0,
        txt.pixel_format(), txt.pixel_components_type(),
        image.data_ptr()
        );

    INVARIANT(qtgl::glapi().glGetError() == 0U);
}


void  copy_offscreen_colour_image_to_texture(offscreen::colour_image const& image, texture const  txt)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        txt.loaded_successfully() && txt.id() != 0 &&
        image.get_width_in_pixels() == txt.width() &&
        image.get_height_in_pixels() == txt.height() &&
        txt.pixel_format() == GL_RGB &&
        txt.pixel_components_type() == GL_UNSIGNED_BYTE
        );

    qtgl::glapi().glBindTexture(GL_TEXTURE_2D, txt.id());

    INVARIANT(qtgl::glapi().glGetError() == 0U);

    qtgl::glapi().glTexImage2D(GL_TEXTURE_2D, 0,
        txt.pixel_format(),
        txt.width(), txt.height(),
        0,
        txt.pixel_format(), txt.pixel_components_type(),
        image.data_ptr()
        );

    INVARIANT(qtgl::glapi().glGetError() == 0U);
}


std::vector< std::array<float_32_bit, 3> > const& get_vertices_of_screen_quad()
{
    static std::vector< std::array<float_32_bit, 3> > const vertices {
        { -1.0f, -1.0f, 0.0f }, {  1.0f, -1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f }, { -1.0f,  1.0f, 0.0f }
    };
    return vertices;
}


std::vector< std::array<float_32_bit, 2> > const& get_texcoords_of_screen_quad()
{
    static std::vector< std::array<float_32_bit, 2> > const texcoords {
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
        { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }
    };
    return texcoords;
}


}
