#include <qtgl/texture.hpp>
#include <qtgl/glapi.hpp>
#include <utility/read_line.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <QImage>
#include <stdexcept>

namespace qtgl { namespace detail {


texture_file_data::texture_file_data(async::key_type const&  key, async::finalise_load_on_destroy_ptr)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  path = key.get_unique_id();

    ASSUMPTION(boost::filesystem::exists(path));
    ASSUMPTION(boost::filesystem::is_regular_file(path));

    if (boost::filesystem::file_size(path) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << path
                                             << "' is not a qtgl file (wrong size).");

    std::ifstream  istr(path.string(),std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the texture file '" << path << "'.");

    std::string  line;

    if (!read_line(istr,line))
        throw std::runtime_error(msgstream() << "Cannot read a path to an image file in the file '"
                                                << path << "'.");

    boost::filesystem::path const  image_file = canonical_path(path.parent_path() / line);

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
        pixel_format = GL_COMPRESSED_RGB;
    else if (line == "COMPRESSED_RGBA")
        pixel_format = GL_COMPRESSED_RGBA;
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
        x_wrapping_type= GL_CLAMP;
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
        y_wrapping_type= GL_CLAMP;
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
        boost::filesystem::path const&  image_pathname,
        natural_32_bit const  pixel_format,
        natural_32_bit const  x_wrapping_type,
        natural_32_bit const  y_wrapping_type,
        natural_32_bit const  min_filtering_type,
        natural_32_bit const  mag_filtering_type
        )
{
    TMPROF_BLOCK();

    if (!boost::filesystem::exists(image_pathname) || !boost::filesystem::is_regular_file(image_pathname))
        throw std::runtime_error(msgstream() << "The image file '" << image_pathname << "' does not exist.");
    if (pixel_format != GL_RGB
        && pixel_format != GL_RGBA
        && pixel_format != GL_COMPRESSED_RGB
        && pixel_format != GL_COMPRESSED_RGBA)
        throw std::runtime_error(msgstream() << "Unknown pixel format '" << pixel_format << "'.");
    if (x_wrapping_type != GL_REPEAT && x_wrapping_type != GL_CLAMP)
        throw std::runtime_error(msgstream() << "Unknown x-wrapping type '" << x_wrapping_type << "'.");
    if (y_wrapping_type != GL_REPEAT && y_wrapping_type != GL_CLAMP)
        throw std::runtime_error(msgstream() << "Unknown y-wrapping type '" << y_wrapping_type << "'.");
    if (min_filtering_type != GL_NEAREST_MIPMAP_NEAREST &&
            min_filtering_type != GL_NEAREST_MIPMAP_LINEAR &&
            min_filtering_type != GL_LINEAR_MIPMAP_NEAREST &&
            min_filtering_type != GL_LINEAR_MIPMAP_LINEAR
            )
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

namespace qtgl { namespace detail {


texture_image_data::texture_image_data(async::key_type const&  key, async::finalise_load_on_destroy_ptr)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  path = key.get_unique_id();

    ASSUMPTION(boost::filesystem::exists(path));
    ASSUMPTION(boost::filesystem::is_regular_file(path));

    QImage  qimage;
    {
        QImage  qtmp_image;
        {
            std::vector<natural_8_bit> buffer(boost::filesystem::file_size(path),0U);
            {
                boost::filesystem::ifstream  istr(path,std::ios_base::binary);
                if (!istr.good())
                    throw std::runtime_error(msgstream() << "Cannot open the passed image file: " << path);
                istr.read((char*)&buffer.at(0U),buffer.size());
                if (istr.bad())
                    throw std::runtime_error(msgstream() << "Cannot read the passed image file: " << path);
            }
            if (!qtmp_image.loadFromData(&buffer.at(0),(int)buffer.size()))
                throw std::runtime_error(msgstream() << "Qt function QImage::loadFromData() has failed for "
                                                        "the passed image file: " << path);
        }
        QImage::Format const  desired_format = qtmp_image.hasAlphaChannel() ? QImage::Format_RGBA8888 :
                                                                              QImage::Format_RGB888;
        if (qtmp_image.format() != desired_format)
        {
            // We subclass QImage, because its method 'convertToFormat_helper' is protected.
            struct format_convertor : public QImage
            {
                format_convertor(QImage const& orig, QImage::Format const  desired_format)
                    : QImage(orig)
                    , m_desired_format(desired_format)
                {}
                QImage  operator()() const
                {
                    return convertToFormat_helper(m_desired_format,Qt::AutoColor);
                }
            private:
                QImage::Format  m_desired_format;
            };
            qtmp_image = format_convertor(qtmp_image, desired_format)();
        }

        // We have to flip the image vertically for OpenGL (because its origin is at bottom).
        qimage = qtmp_image.transformed(QMatrix().scale(1,-1));
    }

    initialise(
        qimage.width(),
        qimage.height(),
        qimage.bits(),
        qimage.bits()+qimage.byteCount(),
        qimage.hasAlphaChannel() ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE
        );
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
    ASSUMPTION(data_begin != nullptr && data_end != nullptr);
    ASSUMPTION((
        [width, height, data_begin, data_end]() -> bool {
            integer_64_bit  size = data_end - data_begin;
            return size % ((natural_64_bit)width * (natural_64_bit)height) == 0ULL &&
                   size / ((natural_64_bit)width * (natural_64_bit)height) > 0ULL &&
                   size / ((natural_64_bit)width * (natural_64_bit)height) < 17ULL;
            }()
        ));

    m_width = width;
    m_height = height;
    m_data = std::unique_ptr< std::vector<natural_8_bit> >(new std::vector<natural_8_bit>(data_begin, data_end));
    m_pixel_components = pixel_components;
    m_pixel_components_type = pixel_components_type;
}


}}

namespace qtgl { namespace detail {


texture_data::texture_data(async::key_type const&  key, async::finalise_load_on_destroy_ptr  finaliser)
    : m_id(0U)
    , m_texture_props()
    , m_image_props()
{
    TMPROF_BLOCK();

    std::string const  texture_file_pathname = key.get_unique_id();
    m_texture_props.insert_load_request(
            texture_file_pathname,
            [this, texture_file_pathname, finaliser]() -> void {
                    if (m_texture_props.get_load_state() != async::LOAD_STATE::FINISHED_SUCCESSFULLY)
                    {
                        finaliser->force_finalisation_as_failure(
                                "Load of texture file '" + texture_file_pathname + "' has FAILED!"
                                );
                        return;
                    }
                    std::string const  image_file_pathname = m_texture_props.image_pathname().string();
                    m_image_props.insert_load_request(
                        image_file_pathname,
                        [this, image_file_pathname, finaliser]() -> void {
                                if (m_image_props.get_load_state() != async::LOAD_STATE::FINISHED_SUCCESSFULLY)
                                {
                                    finaliser->force_finalisation_as_failure(
                                            "Load of texture image file '" + image_file_pathname + "' has FAILED!"
                                            );
                                    return;
                                }
                            }
                        );

                }
            );
}


texture_data::texture_data(
        async::finalise_load_on_destroy_ptr  finaliser,
        GLuint const  id,
        texture_file const  texture_props,
        texture_image const  image_props
        )
    : m_id(id)
    , m_texture_props(texture_props)
    , m_image_props(image_props)
{
    ASSUMPTION(m_texture_props.loaded_successfully() && m_image_props.loaded_successfully());
}


void  texture_data::create_gl_image()
{
    if (m_id != 0U)
        return;

    TMPROF_BLOCK();

    ASSUMPTION(m_texture_props.loaded_successfully() && m_image_props.loaded_successfully());

    glapi().glGenTextures(1, &m_id);
    ASSUMPTION(m_id != 0U);

    glapi().glBindTexture(GL_TEXTURE_2D, m_id);
    glapi().glTexImage2D(GL_TEXTURE_2D, 0,
        pixel_format(),
        width(), height(),
        0,
        m_image_props.pixel_components(), m_image_props.pixel_components_type(),
        (GLvoid const*)m_image_props.data().data()
        );
    glapi().glGenerateMipmap(GL_TEXTURE_2D);
    glapi().glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, x_wrapping_type());
    glapi().glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, y_wrapping_type());
    glapi().glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filtering_type());
    glapi().glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filtering_type());
    INVARIANT(glapi().glGetError() == 0U);
}


void  texture_data::destroy_gl_image()
{
    if (m_id == 0U)
        return;

    TMPROF_BLOCK();

    glapi().glDeleteTextures(1U, &m_id);
    INVARIANT(glapi().glGetError() == 0U);
}


texture_data::~texture_data()
{
    TMPROF_BLOCK();

    destroy_gl_image();
}


}}

namespace qtgl {


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
            glapi().glActiveTexture(GL_TEXTURE0 + value(sampler_and_texture.first));
            glapi().glBindTexture(GL_TEXTURE_2D, sampler_and_texture.second.id());
            INVARIANT(glapi().glGetError() == 0U);
        }
    return true;
}


bool  make_current(textures_binding const&  binding)
{
    return binding.make_current();
}


}
