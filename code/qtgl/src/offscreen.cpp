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
            pixel_format == GL_DEPTH_COMPONENT ? 1U : 4U,
            pixel_components_type
            )
        );

    INVARIANT(glapi().glGetError() == 0U);

    return texture;
}


GLuint  make_pixel_buffer_object_for_offscreen_texture(texture const t)
{
    GLuint  pbo_id;
    glapi().glGenBuffers(1, &pbo_id);
    INVARIANT(pbo_id != 0U);

    natural_32_bit const  num_bytes = t.width() * t.height() * t.pixel_components() * sizeof(float_32_bit);

    glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id);
    glapi().glBufferData(GL_PIXEL_PACK_BUFFER, num_bytes, 0, GL_STREAM_READ);

    glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    INVARIANT(glapi().glGetError() == 0U);

    return  pbo_id;
}


}}

namespace qtgl {


offscreen::offscreen(natural_32_bit const  width_in_pixels, natural_32_bit const  height_in_pixels, bool const  use_colour_texture)
    : m_width_in_pixels(width_in_pixels)
    , m_height_in_pixels(height_in_pixels)
    , m_frame_buffer_object(0U)
    , m_depth_texture()
    , m_depth_texture_pixel_buffer_object(0U)
    , m_colour_texture()
    , m_colour_texture_pixel_buffer_object(0U)
{
    ASSUMPTION(m_width_in_pixels > 0U && m_height_in_pixels > 0U);

    glapi().glGenFramebuffers(1, &m_frame_buffer_object);
    ASSUMPTION(m_frame_buffer_object != 0U);

    m_depth_texture = detail::make_offscreen_texture(m_width_in_pixels, m_height_in_pixels, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
    m_depth_texture_pixel_buffer_object = detail::make_pixel_buffer_object_for_offscreen_texture(m_depth_texture);

    if (use_colour_texture)
    {
        m_colour_texture = detail::make_offscreen_texture(m_width_in_pixels, m_height_in_pixels, GL_RGBA, GL_FLOAT);
        m_colour_texture_pixel_buffer_object = detail::make_pixel_buffer_object_for_offscreen_texture(m_colour_texture);
    }

    glapi().glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_object);
    glapi().glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture.id(), 0);

    if (use_colour_texture)
        glapi().glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour_texture.id(), 0);

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
    TMPROF_BLOCK();

    glapi().glDeleteBuffers(1, &m_depth_texture_pixel_buffer_object);
    if (!m_colour_texture.empty())
        glapi().glDeleteBuffers(1, &m_colour_texture_pixel_buffer_object);

    glapi().glDeleteFramebuffers(1, &m_frame_buffer_object);

    INVARIANT(glapi().glGetError() == 0U);
}


make_current_offscreen::make_current_offscreen(offscreen&  ofs)
    : m_ofs(&ofs)
{
    TMPROF_BLOCK();

    ASSUMPTION(m_ofs->get_frame_buffer_object() != 0U);

    glapi().glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ofs->get_frame_buffer_object());

    INVARIANT(glapi().glGetError() == 0U);
}


make_current_offscreen::~make_current_offscreen()
{
    TMPROF_BLOCK();
    release();
}


void  make_current_offscreen::release()
{
    glapi().glBindFramebuffer(GL_READ_FRAMEBUFFER, m_ofs->get_frame_buffer_object());

    {
        TMPROF_BLOCK();
        glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, m_ofs->get_depth_texture_pixel_buffer_object());
        glapi().glReadPixels(0, 0, m_ofs->get_width_in_pixels(), m_ofs->get_height_in_pixels(), GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
    }

    if (!m_ofs->get_colour_texture().empty())
    {
        TMPROF_BLOCK();
        glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, m_ofs->get_colour_texture_pixel_buffer_object());
        glapi().glReadPixels(0, 0, m_ofs->get_width_in_pixels(), m_ofs->get_height_in_pixels(), GL_RGBA, GL_FLOAT, 0);
    }

    glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glapi().glBindFramebuffer(GL_FRAMEBUFFER, 0);
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


void  clear_offscreen_depth_image(offscreen::depth_image& image, offscreen::depth_image_pixel const  value)
{
    TMPROF_BLOCK();

    for (offscreen::depth_image_pixel*  ptr = image.data_ptr(), *  end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        *ptr = value;
}


void  clear_offscreen_colour_image(offscreen::colour_image&  image, offscreen::colour_image_pixel const&  value)
{
    TMPROF_BLOCK();

    for (offscreen::colour_image_pixel*  ptr = image.data_ptr(), *  end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        *ptr = value;
}


void  update_offscreen_depth_image(offscreen::depth_image&  image, offscreen const&  ofs)
{
    TMPROF_BLOCK();

    glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, ofs.get_depth_texture_pixel_buffer_object());
    using  raw_depth_image_pixel = natural_32_bit;
    raw_depth_image_pixel const*  src_data_ptr =
            (raw_depth_image_pixel const*)glapi().glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    INVARIANT(qtgl::glapi().glGetError() == 0U && src_data_ptr != nullptr);

    for (offscreen::depth_image_pixel* ptr = image.data_ptr(), *end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr, ++src_data_ptr)
        *ptr = ((float_32_bit)*src_data_ptr) / (float_32_bit)std::numeric_limits<natural_32_bit>::max();

    glapi().glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    INVARIANT(qtgl::glapi().glGetError() == 0U);
}


void  update_offscreen_colour_image(offscreen::colour_image&  image, offscreen const&  ofs)
{
    TMPROF_BLOCK();

    glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, ofs.get_colour_texture_pixel_buffer_object());
    offscreen::colour_image_pixel const* src_data_ptr =
            (offscreen::colour_image_pixel const*)glapi().glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    INVARIANT(qtgl::glapi().glGetError() == 0U && src_data_ptr != nullptr);

    for (offscreen::colour_image_pixel* ptr = image.data_ptr(), *end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr, ++src_data_ptr)
        *ptr = *src_data_ptr;

    glapi().glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glapi().glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

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
    for (offscreen::depth_image_pixel*  ptr = image.data_ptr(), *  end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        *ptr = a / (b - c * (2.0f * (*ptr) - 1.0f));
}


void  from_camera_space_to_interval_01(offscreen::depth_image& image, float_32_bit const  near_plane, float_32_bit const  far_plane)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        near_plane > 0.0f && far_plane > near_plane &&
        image.num_pixels() == image.get_width_in_pixels() * image.get_height_in_pixels()
        );

    float_32_bit const  a = 1.0f / (far_plane - near_plane);
    for (offscreen::depth_image_pixel*  ptr = image.data_ptr(), *  end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        *ptr = std::max(0.0f, std::min(1.0f, 1.0f - (*ptr) * a));
}


void  normalise_offscreen_depth_image_interval_01(offscreen::depth_image&  image)
{
    TMPROF_BLOCK();

    ASSUMPTION(image.num_pixels() == image.get_width_in_pixels() * image.get_height_in_pixels());

    for (offscreen::depth_image_pixel* ptr = image.data_ptr(), *end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr)
        //*ptr = (*ptr) * (*ptr) * (*ptr);
        //*ptr = 1.0f - std::pow((*ptr), 5);
        *ptr = std::sqrt(std::max(0.0f, 1.0f - *ptr));
}


void  modulate_offscreen_colour_image_by_depth_image(offscreen::colour_image&  colour_image, offscreen::depth_image const&  depth_image)
{
    TMPROF_BLOCK();

    ASSUMPTION(colour_image.num_pixels() == depth_image.num_pixels());

    offscreen::depth_image_pixel const*  depth_ptr = depth_image.data_ptr();
    for (offscreen::colour_image_pixel*  ptr = colour_image.data_ptr(), *end = colour_image.data_ptr() + colour_image.num_pixels(); ptr != end; ++ptr, ++depth_ptr)
    {
        offscreen::depth_image_pixel const  d = *depth_ptr;
        ptr->r *= d;
        ptr->g *= d;
        ptr->b *= d;
        ptr->a *= d;
    }
}


}


////////////////////////////////////////////////////////////////////////////////////////////
// DBG CODE
////////////////////////////////////////////////////////////////////////////////////////////


#include <qtgl/batch.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/camera.hpp>
#include <qtgl/draw.hpp>

namespace qtgl { namespace detail {


std::vector< std::array<float_32_bit, 3> > const&  get_vertices_of_screen_quad()
{
    static std::vector< std::array<float_32_bit, 3> > const vertices{
        { -1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f }, { -1.0f,  1.0f, -1.0f }
    };
    return vertices;
}


std::vector< std::array<float_32_bit, 2> > const& get_texcoords_of_screen_quad()
{
    static std::vector< std::array<float_32_bit, 2> > const texcoords{
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
        { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }
    };
    return texcoords;
}


}}

namespace qtgl { namespace dbg {


static void  copy_offscreen_depth_image_to_texture(offscreen::depth_image const&  image, texture const  txt)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        txt.loaded_successfully() && txt.id() != 0 &&
        image.get_width_in_pixels() == txt.width() &&
        image.get_height_in_pixels() == txt.height() &&
        txt.pixel_format() == GL_RGBA &&
        txt.pixel_components_type() == GL_FLOAT
    );

    std::vector<offscreen::colour_image_pixel>  pixels;
    {
        pixels.resize(image.num_pixels());
        offscreen::colour_image_pixel* dst_pixels_ptr = pixels.data();
        for (offscreen::depth_image_pixel const* ptr = image.data_ptr(), *end = image.data_ptr() + image.num_pixels(); ptr != end; ++ptr, ++dst_pixels_ptr)
            dst_pixels_ptr->r = dst_pixels_ptr->g = dst_pixels_ptr->b = dst_pixels_ptr->a = *ptr;
    }

    qtgl::glapi().glBindTexture(GL_TEXTURE_2D, txt.id());
    INVARIANT(qtgl::glapi().glGetError() == 0U);

    qtgl::glapi().glTexImage2D(GL_TEXTURE_2D, 0,
        txt.pixel_format(),
        txt.width(), txt.height(),
        0,
        txt.pixel_format(), txt.pixel_components_type(),
        pixels.data()
        );

    INVARIANT(qtgl::glapi().glGetError() == 0U);
}


static void  copy_offscreen_colour_image_to_texture(offscreen::colour_image const& image, texture const  txt)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        txt.loaded_successfully() && txt.id() != 0 &&
        image.get_width_in_pixels() == txt.width() &&
        image.get_height_in_pixels() == txt.height() &&
        txt.pixel_format() == GL_RGBA &&
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


static void  draw_offscreen_batch(
        batch const  b,
        natural_32_bit const  x_screen_pos,
        natural_32_bit const  y_screen_pos,
        natural_32_bit const  screen_width_in_pixels,
        natural_32_bit const  screen_height_in_pixels,
        float_32_bit const  scale
        )
{
    TMPROF_BLOCK();

    matrix44  matrix_from_world_to_camera;
    {
        texture const  t = b.get_textures_binding().bindings_map().at(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE);

        float_32_bit const sx = scale * ((float_32_bit)t.width()) / ((float_32_bit)screen_width_in_pixels);
        float_32_bit const sy = scale * ((float_32_bit)t.height()) / ((float_32_bit)screen_height_in_pixels);
        float_32_bit const px = 2.0f * ((float_32_bit)x_screen_pos) / ((float_32_bit)screen_width_in_pixels) - 1.0f + sx;
        float_32_bit const py = 2.0f * ((float_32_bit)y_screen_pos) / ((float_32_bit)screen_height_in_pixels) - 1.0f + sy;

        matrix_from_world_to_camera = matrix44_identity();
        matrix_from_world_to_camera(0, 0) = sx;
        matrix_from_world_to_camera(0, 3) = px;
        matrix_from_world_to_camera(1, 1) = sy;
        matrix_from_world_to_camera(1, 3) = py;
    }
    if (qtgl::make_current(b, false))
        qtgl::render_batch(
            b,
            qtgl::vertex_shader_uniform_data_provider(b, { matrix_from_world_to_camera }, matrix44_identity()),
            qtgl::fragment_shader_uniform_data_provider()
            );
}


void  draw_offscreen_depth_image(
        offscreen::depth_image const&  image,
        natural_32_bit const  x_screen_pos,
        natural_32_bit const  y_screen_pos,
        natural_32_bit const  screen_width_in_pixels,
        natural_32_bit const  screen_height_in_pixels,
        float_32_bit const  scale
        )
{
    TMPROF_BLOCK();

    texture const  t =
        detail::make_offscreen_texture(image.get_width_in_pixels(), image.get_height_in_pixels(), GL_RGBA, GL_FLOAT);

    copy_offscreen_depth_image_to_texture(image, t);
    draw_offscreen_batch(
        create_triangle_mesh(detail::get_vertices_of_screen_quad(), detail::get_texcoords_of_screen_quad(), t),
        x_screen_pos,
        y_screen_pos,
        screen_width_in_pixels,
        screen_height_in_pixels,
        scale
        );
}


void  draw_offscreen_colour_image(
        offscreen::colour_image const& image,
        natural_32_bit const  x_screen_pos,
        natural_32_bit const  y_screen_pos,
        natural_32_bit const  screen_width_in_pixels,
        natural_32_bit const  screen_height_in_pixels,
        float_32_bit const  scale
        )
{
    TMPROF_BLOCK();

    texture const  t =
        detail::make_offscreen_texture(image.get_width_in_pixels(), image.get_height_in_pixels(), GL_RGBA, GL_FLOAT);
    copy_offscreen_colour_image_to_texture(image, t);
    draw_offscreen_batch(
        create_triangle_mesh(detail::get_vertices_of_screen_quad(), detail::get_texcoords_of_screen_quad(), t),
        x_screen_pos,
        y_screen_pos,
        screen_width_in_pixels,
        screen_height_in_pixels,
        scale
        );
}


}}
