#include <gfx/texture.hpp>
#include <gfx/image.hpp>
#include <utility/read_line.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/canonical_path.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace gfx { namespace detail {


texture_file_data::texture_file_data(async::finalise_load_on_destroy_ptr const  finaliser)
{
    TMPROF_BLOCK();

    std::filesystem::path const  path = finaliser->get_key().get_unique_id();

    ASSUMPTION(std::filesystem::exists(path));
    ASSUMPTION(std::filesystem::is_regular_file(path));

    if (std::filesystem::file_size(path) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << path
                                             << "' is not a gfx file (wrong size).");

    std::ifstream  istr(path.string(),std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the texture file '" << path << "'.");

    std::string  line;

    if (!read_line(istr,line))
        throw std::runtime_error(msgstream() << "Cannot read a path to an image file in the file '"
                                                << path << "'.");

    std::filesystem::path const  image_file = canonical_path(path.parent_path() / line);

    if (!read_line(istr,line))
        throw std::runtime_error(
                msgstream() << "Cannot read 'pixel format' in the texture file '" << path << "'."
                );
    natural_32_bit  pixel_format;
    if (line == "RGB")
        pixel_format = GL_RGB;
    else if (line == "RGBA")
        pixel_format = GL_RGBA;
    else if (line == "COMPRESSED_RGB")
        pixel_format = GL_RGB; //GL_COMPRESSED_RGB;
    else if (line == "COMPRESSED_RGBA")
        pixel_format = GL_RGBA; //GL_COMPRESSED_RGBA;
    else
        throw std::runtime_error(
                msgstream() << "Unknown pixel format '" << line << "' in the texture file '" << path << "'."
                );

    if (!read_line(istr,line))
        throw std::runtime_error(
                msgstream() << "Cannot read 'x-wrapping type' in the texture file '" << path << "'."
                );
    natural_32_bit  x_wrapping_type;
    if (line == "REPEAT")
        x_wrapping_type= GL_REPEAT;
    else if (line == "CLAMP")
        x_wrapping_type= GL_CLAMP_TO_EDGE;
    else
        throw std::runtime_error(
                msgstream() << "Unknown x-wrapping type '" << line << "' in the texture file '" << path << "'."
                );

    if (!read_line(istr,line))
        throw std::runtime_error(
                msgstream() << "Cannot read 'y-wrapping type' in the texture file '" << path << "'."
                );
    natural_32_bit  y_wrapping_type;
    if (line == "REPEAT")
        y_wrapping_type= GL_REPEAT;
    else if (line == "CLAMP")
        y_wrapping_type= GL_CLAMP_TO_EDGE;
    else
        throw std::runtime_error(
                msgstream() << "Unknown y-wrapping type '" << line << "' in the texture file '" << path << "'."
                );

    if (!read_line(istr,line))
        throw std::runtime_error(
                msgstream() << "Cannot read 'min filtering type' in the texture file '" << path << "'."
                );
    natural_32_bit  min_filtering_type;
    if (line == "NEAREST_MIPMAP_NEAREST")
        min_filtering_type= GL_NEAREST_MIPMAP_NEAREST;
    else if (line == "NEAREST_MIPMAP_LINEAR")
        min_filtering_type= GL_NEAREST_MIPMAP_LINEAR;
    else if (line == "LINEAR_MIPMAP_NEAREST")
        min_filtering_type= GL_LINEAR_MIPMAP_NEAREST;
    else if (line == "LINEAR_MIPMAP_LINEAR")
        min_filtering_type= GL_LINEAR_MIPMAP_LINEAR;
    else
        throw std::runtime_error(
                msgstream() << "Unknown min filtering type '" << line << "' in the texture file '"
                            << path << "'."
                );

    if (!read_line(istr,line))
        throw std::runtime_error(
                msgstream() << "Cannot read 'mag filtering type' in the texture file '" << path << "'."
                );
    natural_32_bit  mag_filtering_type;
    if (line == "NEAREST")
        mag_filtering_type= GL_NEAREST;
    else if (line == "LINEAR")
        mag_filtering_type= GL_LINEAR;
    else
        throw std::runtime_error(
                msgstream() << "Unknown mag filtering type '" << line << "' in the texture file '"
                            << path << "'."
                );

    initialise(
            image_file,
            pixel_format,
            x_wrapping_type,
            y_wrapping_type,
            min_filtering_type,
            mag_filtering_type
            );
}


texture_file_data::~texture_file_data()
{
    TMPROF_BLOCK();
}


void  texture_file_data::initialise(
        std::filesystem::path const&  image_pathname,
        natural_32_bit const  pixel_format,
        natural_32_bit const  x_wrapping_type,
        natural_32_bit const  y_wrapping_type,
        natural_32_bit const  min_filtering_type,
        natural_32_bit const  mag_filtering_type
        )
{
    TMPROF_BLOCK();

    if (!image_pathname.empty() && (!std::filesystem::exists(image_pathname) || !std::filesystem::is_regular_file(image_pathname)))
        throw std::runtime_error(msgstream() << "The image file '" << image_pathname << "' does not exist.");
    if (pixel_format != GL_RGB
        && pixel_format != GL_RGBA
        //&& pixel_format != GL_COMPRESSED_RGB
        //&& pixel_format != GL_COMPRESSED_RGBA
        && pixel_format != GL_DEPTH_COMPONENT)
        throw std::runtime_error(msgstream() << "Unknown pixel format '" << pixel_format << "'.");
    if (x_wrapping_type != GL_REPEAT && x_wrapping_type != GL_CLAMP_TO_EDGE)
        throw std::runtime_error(msgstream() << "Unknown x-wrapping type '" << x_wrapping_type << "'.");
    if (y_wrapping_type != GL_REPEAT && y_wrapping_type != GL_CLAMP_TO_EDGE)
        throw std::runtime_error(msgstream() << "Unknown y-wrapping type '" << y_wrapping_type << "'.");
    if (min_filtering_type != GL_NEAREST_MIPMAP_NEAREST &&
            min_filtering_type != GL_NEAREST_MIPMAP_LINEAR &&
            min_filtering_type != GL_LINEAR_MIPMAP_NEAREST &&
            min_filtering_type != GL_LINEAR_MIPMAP_LINEAR &&
            min_filtering_type != GL_NEAREST &&
            min_filtering_type != GL_LINEAR)
        throw std::runtime_error(msgstream() << "Unknown min filtering type '" << min_filtering_type << "'.");
    if (mag_filtering_type != GL_NEAREST && mag_filtering_type != GL_LINEAR)
        throw std::runtime_error(msgstream() << "Unknown mag filtering type '" << mag_filtering_type << "'.");

    m_image_pathname = image_pathname;
    m_pixel_format = pixel_format;
    m_x_wrapping_type = x_wrapping_type;
    m_y_wrapping_type = y_wrapping_type;
    m_min_filtering_type = min_filtering_type;
    m_mag_filtering_type = mag_filtering_type;
}


}}

namespace gfx { namespace detail {


texture_image_data::texture_image_data(async::finalise_load_on_destroy_ptr const  finaliser)
{
    TMPROF_BLOCK();

    std::filesystem::path const  path = finaliser->get_key().get_unique_id();

    ASSUMPTION(std::filesystem::exists(path));
    ASSUMPTION(std::filesystem::is_regular_file(path));

    image_rgba_8888  img;
    load_png_image(path, img);
    flip_image_vertically(img);

    initialise(img.width, img.height, img.data.data(), img.data.data() + img.data.size(), GL_RGBA, GL_UNSIGNED_BYTE);
}


texture_image_data::~texture_image_data()
{
    TMPROF_BLOCK();
}


void  texture_image_data::initialise(
        natural_32_bit const  width,
        natural_32_bit const  height,
        natural_8_bit const* const  data_begin,
        natural_8_bit const* const  data_end,
        natural_32_bit const  pixel_components,
        natural_32_bit const  pixel_components_type
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(width > 0U && height > 0U);
    ASSUMPTION(((data_begin == nullptr && data_end == nullptr) ||
        [width, height, data_begin, data_end]() -> bool {
            integer_64_bit  size = data_end - data_begin;
            return size % ((natural_64_bit)width * (natural_64_bit)height) == 0ULL &&
                   size / ((natural_64_bit)width * (natural_64_bit)height) > 0ULL &&
                   size / ((natural_64_bit)width * (natural_64_bit)height) < 17ULL;
            }()
        ));

    m_width = width;
    m_height = height;
    m_data = data_begin == nullptr ? nullptr : std::unique_ptr< std::vector<natural_8_bit> >(new std::vector<natural_8_bit>(data_begin, data_end));
    m_pixel_components = pixel_components;
    m_pixel_components_type = pixel_components_type;
}


}}

namespace gfx { namespace detail {


texture_data::texture_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_id(0U)
    , m_texture_props()
    , m_image_props()
{
    TMPROF_BLOCK();

    std::string const  texture_file_pathname = finaliser->get_key().get_unique_id();

    async::finalise_load_on_destroy_ptr const  texture_file_finaliser =
        async::finalise_load_on_destroy::create(
                [this](async::finalise_load_on_destroy_ptr const  finaliser) {
                    std::string const  image_file_pathname = m_texture_props.image_pathname().string();
                    m_image_props.insert_load_request(image_file_pathname, finaliser);
                    },
                finaliser
                );
    m_texture_props.insert_load_request(texture_file_pathname, texture_file_finaliser);
}


texture_data::texture_data(
        async::finalise_load_on_destroy_ptr,
        GLuint const  id,
        texture_file const  texture_props,
        texture_image const  image_props
        )
    : m_id(id)
    , m_texture_props(texture_props)
    , m_image_props(image_props)
{
    if (!m_texture_props.loaded_successfully())
        throw std::runtime_error(msgstream() << "Load of texture file '" << m_texture_props.key() << "' has FAILED.");
    if (!m_image_props.loaded_successfully())
        throw std::runtime_error(msgstream() << "Load of texture image '" << m_image_props.key() << "' has FAILED.");
}


void  texture_data::create_gl_image() const
{
    if (m_id != 0U)
        return;

    TMPROF_BLOCK();

    ASSUMPTION(m_texture_props.loaded_successfully() && m_image_props.loaded_successfully());

    glGenTextures(1, &m_id);
    ASSUMPTION(m_id != 0U);

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0,
        pixel_format(),
        width(), height(),
        0,
        m_image_props.pixel_components(), m_image_props.pixel_components_type(),
        (GLvoid const*)m_image_props.data().data()
        );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, x_wrapping_type());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, y_wrapping_type());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filtering_type());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filtering_type());
    INVARIANT(glGetError() == 0U);
}


void  texture_data::destroy_gl_image() const
{
    if (m_id == 0U)
        return;

    TMPROF_BLOCK();

    glDeleteTextures(1U, &m_id);
    INVARIANT(glGetError() == 0U);
}


texture_data::~texture_data()
{
    TMPROF_BLOCK();

    destroy_gl_image();
}


}}

namespace gfx { namespace detail {


textures_binding_data::textures_binding_data(
        async::finalise_load_on_destroy_ptr finaliser,
        std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME, std::filesystem::path> const&  texture_paths
        )
    : m_bindings()
    , m_ready(false)
{
    for (auto const& elem : texture_paths)
        m_bindings.insert({ elem.first, texture(elem.second, finaliser) });
}


}}

namespace gfx {


bool  textures_binding::ready() const
{
    TMPROF_BLOCK();

    if (empty())
        return true;
    if (!loaded_successfully())
        return false;
    if (!resource().ready())
    {
        for (auto const& sampler_and_texture : bindings_map())
        {
            ASSUMPTION(value(sampler_and_texture.first) < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
            if (!sampler_and_texture.second.loaded_successfully())
                return false;
            sampler_and_texture.second.create_gl_image();
            if (sampler_and_texture.second.id() == 0U)
                return false;
        }

        const_cast<textures_binding*>(this)->set_ready();
    }

    return true;
}


bool  textures_binding::make_current() const
{
    TMPROF_BLOCK();

    if (!ready())
        return false;
    if (!empty())
        for (auto const&  sampler_and_texture : bindings_map())
        {
            glActiveTexture(GL_TEXTURE0 + value(sampler_and_texture.first));
            glBindTexture(GL_TEXTURE_2D, sampler_and_texture.second.id());
            INVARIANT(glGetError() == 0U);
        }
    return true;
}


bool  make_current(textures_binding const&  binding)
{
    return binding.make_current();
}


}
