#include <qtgl/texture.hpp>
#include <qtgl/detail/texture_cache.hpp>
#include <qtgl/detail/read_line.hpp>
#include <qtgl/glapi.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/functional/hash.hpp>
#include <QImage>
#include <stdexcept>

namespace qtgl {


texture_properties_ptr  texture_properties::create(
        boost::filesystem::path const&  image_file,
        natural_32_bit const  pixel_format,
        natural_32_bit const  x_wrapping_type,
        natural_32_bit const  y_wrapping_type,
        natural_32_bit const  min_filtering_type,
        natural_32_bit const  mag_filtering_type
        )
{
    return std::make_shared<texture_properties>(image_file,
                                                pixel_format,
                                                x_wrapping_type,
                                                y_wrapping_type,
                                                min_filtering_type,
                                                mag_filtering_type);
}

texture_properties::texture_properties(
        boost::filesystem::path const&  image_file,
        natural_32_bit const  pixel_format,
        natural_32_bit const  x_wrapping_type,
        natural_32_bit const  y_wrapping_type,
        natural_32_bit const  min_filtering_type,
        natural_32_bit const  mag_filtering_type
        )
    : m_image_file(image_file)
    , m_pixel_format(pixel_format)
    , m_x_wrapping_type(x_wrapping_type)
    , m_y_wrapping_type(y_wrapping_type)
    , m_min_filtering_type(min_filtering_type)
    , m_mag_filtering_type(mag_filtering_type)
{}

bool  operator==(texture_properties const&  props0, texture_properties const&  props1)
{
    return props0.image_file() == props1.image_file() &&
           props0.pixel_format() == props1.pixel_format() &&
           props0.x_wrapping_type() == props1.x_wrapping_type() &&
           props0.y_wrapping_type() == props1.y_wrapping_type() &&
           props0.min_filtering_type() == props1.min_filtering_type() &&
           props0.mag_filtering_type() == props1.mag_filtering_type()
           ;
}

size_t  hasher_of_texture_properties(texture_properties const&  props)
{
    std::size_t seed = 0ULL;
    boost::hash_combine(seed,props.image_file().string());
    boost::hash_combine(seed,props.pixel_format());
    boost::hash_combine(seed,props.x_wrapping_type());
    boost::hash_combine(seed,props.y_wrapping_type());
    boost::hash_combine(seed,props.min_filtering_type());
    boost::hash_combine(seed,props.mag_filtering_type());

    return seed;
}


}

namespace qtgl {


texture_image_properties::texture_image_properties(
        natural_32_bit const  width,
        natural_32_bit const  height,
        natural_8_bit const* const  data_begin,
        natural_8_bit const* const  data_end,
        natural_32_bit const  pixel_components = GL_RGBA,
        natural_32_bit const  pixel_components_type = GL_UNSIGNED_INT_8_8_8_8
        )
    : m_width(width)
    , m_height(height)
    , m_data{new std::vector<natural_8_bit>(data_begin,data_end)}
    , m_pixel_components(pixel_components)
    , m_pixel_components_type(pixel_components_type)
{
    ASSUMPTION(m_width > 0U && m_height > 0U);
    ASSUMPTION(m_data->size() % ((natural_64_bit)m_width * (natural_64_bit)m_height) == 0ULL);
    ASSUMPTION(m_data->size() / ((natural_64_bit)m_width * (natural_64_bit)m_height) > 0ULL &&
               m_data->size() / ((natural_64_bit)m_width * (natural_64_bit)m_height) < 17ULL );
}


texture_image_properties  load_texture_image_file(boost::filesystem::path const&  image_file)
{
    TMPROF_BLOCK();

    ASSUMPTION(boost::filesystem::exists(image_file));
    ASSUMPTION(boost::filesystem::is_regular_file(image_file));

    QImage  qimage;
    {
        QImage  qtmp_image;
        {
            std::vector<natural_8_bit> buffer(boost::filesystem::file_size(image_file),0U);
            {
                boost::filesystem::ifstream  istr(image_file,std::ios_base::binary);
                if (!istr.good())
                    throw std::runtime_error(msgstream() << "Cannot open the passed image file: " << image_file);
                istr.read((char*)&buffer.at(0U),buffer.size());
                if (istr.bad())
                    throw std::runtime_error(msgstream() << "Cannot read the passed image file: " << image_file);
            }
            if (!qtmp_image.loadFromData(&buffer.at(0),(int)buffer.size()))
                throw std::runtime_error(msgstream() << "Qt function QImage::loadFromData() has failed for "
                                                               "the passed image file: " << image_file);
        }
        if (qtmp_image.format() != QImage::Format_RGBA8888)
        {
            struct format_convertor : public QImage
            {
                format_convertor(QImage const& orig)
                    : QImage(orig)
                {}
                QImage  operator()() const
                {
                    return convertToFormat_helper(QImage::Format_RGBA8888,Qt::AutoColor);
                }
            };
            qtmp_image = format_convertor(qtmp_image)();
        }
        qimage = qtmp_image.transformed(QMatrix().scale(1,-1));
    }

    return texture_image_properties(
                qimage.width(),
                qimage.height(),
                qimage.bits(),
                qimage.bits()+qimage.byteCount(),
                GL_RGBA,
                GL_UNSIGNED_BYTE
                );
}


}

namespace qtgl {


texture::texture(texture_image_properties const&  image_props, texture_properties_ptr const  texture_props)
    : m_id(0U)
    , m_width(image_props.width())
    , m_height(image_props.height())
    , m_texture_props(texture_props)
{
    TMPROF_BLOCK();

    ASSUMPTION(m_texture_props.operator bool());

    glapi().glGenTextures(1,&m_id);
    ASSUMPTION(m_id != 0U);

    glapi().glBindTexture(GL_TEXTURE_2D,m_id);
    glapi().glTexImage2D(GL_TEXTURE_2D, 0,
                         m_texture_props->pixel_format(),
                         m_width, m_height,
                         0,
                         image_props.pixel_components(), image_props.pixel_components_type(),
                         (GLvoid const*)image_props.data().data()
                         );
    glapi().glGenerateMipmap(GL_TEXTURE_2D);
    glapi().glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,m_texture_props->x_wrapping_type());
    glapi().glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,m_texture_props->y_wrapping_type());
    glapi().glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,m_texture_props->min_filtering_type());
    glapi().glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,m_texture_props->mag_filtering_type());
}

texture::~texture()
{
    glapi().glDeleteTextures(1U,&m_id);
}


texture_ptr  texture::create(
        texture_image_properties const&  image_props,
        texture_properties_ptr const  texture_props
        )
{
    return texture_ptr{ new texture{image_props,texture_props} };
}


}

namespace qtgl {


texture_properties_ptr  load_texture_file(boost::filesystem::path const&  texture_file, std::string&  error_message)
{
    ASSUMPTION(error_message.empty());

    if (!boost::filesystem::exists(texture_file))
    {
        error_message = msgstream() << "The texture file '" << texture_file << "' does not exist.";
        return {};
    }
    if (!boost::filesystem::is_regular_file(texture_file))
    {
        error_message = msgstream() << "The texture path '" << texture_file << "' does not reference a regular file.";
        return {};
    }

    if (boost::filesystem::file_size(texture_file) < 4ULL)
    {
        error_message = msgstream() << "The passed file '" << texture_file << "' is not a qtgl file (wrong size).";
        return {};
    }

    std::ifstream  istr(texture_file.string(),std::ios_base::binary);
    if (!istr.good())
    {
        error_message = msgstream() << "Cannot open the texture file '" << texture_file << "'.";
        return {};
    }

    std::string  file_type;
    if (!detail::read_line(istr,file_type))
    {
        error_message = msgstream() << "The passed file '" << texture_file << "' is not a qtgl file (cannot read its type string).";
        return {};
    }

    if (file_type == "E2::qtgl/texture/text")
    {
        std::string  line;

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read a path to an image file in the file '" << texture_file << "'.";
            return {};
        }
        boost::filesystem::path const  image_file = boost::filesystem::absolute(texture_file.parent_path() / line);
        if (!boost::filesystem::exists(image_file) || !boost::filesystem::is_regular_file(image_file))
        {
            error_message = msgstream() << "The image file '" << image_file.string()
                                        << "' referenced from the texture file '" << texture_file << "' does not exist.";
            return {};
        }

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read 'pixel format' in the texture file '" << texture_file << "'.";
            return {};
        }
        natural_32_bit  pixel_format;
        if (line == "COMPRESSED_RGB")
            pixel_format= GL_COMPRESSED_RGB;
        else if (line == "COMPRESSED_RGBA")
            pixel_format= GL_COMPRESSED_RGBA;
        else
        {
            error_message = msgstream() << "Unknown pixel format '" << line << "' in the texture file '" << texture_file << "'.";
            return {};
        }

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read 'x-wrapping type' in the texture file '" << texture_file << "'.";
            return {};
        }
        natural_32_bit  x_wrapping_type;
        if (line == "REPEAT")
            x_wrapping_type= GL_REPEAT;
        else if (line == "CLAMP")
            x_wrapping_type= GL_CLAMP;
        else
        {
            error_message = msgstream() << "Unknown x-wrapping type '" << line << "' in the texture file '" << texture_file << "'.";
            return {};
        }

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read 'y-wrapping type' in the texture file '" << texture_file << "'.";
            return {};
        }
        natural_32_bit  y_wrapping_type;
        if (line == "REPEAT")
            y_wrapping_type= GL_REPEAT;
        else if (line == "CLAMP")
            y_wrapping_type= GL_CLAMP;
        else
        {
            error_message = msgstream() << "Unknown y-wrapping type '" << line << "' in the texture file '" << texture_file << "'.";
            return {};
        }

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read 'min filtering type' in the texture file '" << texture_file << "'.";
            return {};
        }
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
        {
            error_message = msgstream() << "Unknown min filtering type '" << line << "' in the texture file '" << texture_file << "'.";
            return {};
        }

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read 'mag filtering type' in the texture file '" << texture_file << "'.";
            return {};
        }
        natural_32_bit  mag_filtering_type;
        if (line == "NEAREST")
            mag_filtering_type= GL_NEAREST;
        else if (line == "LINEAR")
            mag_filtering_type= GL_LINEAR;
        else
        {
            error_message = msgstream() << "Unknown mag filtering type '" << line << "' in the texture file '" << texture_file << "'.";
            return {};
        }

        return texture_properties::create(image_file,
                                          pixel_format,
                                          x_wrapping_type,
                                          y_wrapping_type,
                                          min_filtering_type,
                                          mag_filtering_type
                                          );
    }
    else
    {
        error_message = msgstream() << "The passed texture file '" << texture_file
                                    << "' is of an unknown type '" << file_type << "'.";
        return {};
    }
}


void  insert_load_request(boost::filesystem::path const&  texture_file)
{
    detail::texture_cache::instance().insert_load_request(texture_file);
}

void  insert_load_request(texture_properties_ptr const  props)
{
    detail::texture_cache::instance().insert_load_request(props);
}

std::weak_ptr<texture const>  find_texture(texture_properties_ptr const  props)
{
    return detail::texture_cache::instance().find(props);
}

bool  make_current(fragment_shader_texture_sampler_binding const  binding,
                   texture_properties_ptr const  props,
                   bool const  use_dummy_texture_if_requested_one_is_not_loaded_yet)
{
    std::weak_ptr<texture const> const  wptr = detail::texture_cache::instance().find(props);
    std::shared_ptr<texture const>  ptr = wptr.lock();
    bool  result = true;
    if (!ptr.operator bool())
    {
        if (!use_dummy_texture_if_requested_one_is_not_loaded_yet)
            return false;
        detail::texture_cache::instance().insert_load_request(props);
        ptr = detail::texture_cache::instance().get_dummy_texture().lock();
        INVARIANT(ptr.operator bool());
        result = false;
    }
    make_current(binding,ptr);
    return result;
}

void  make_current(fragment_shader_texture_sampler_binding const  binding, texture_ptr const  texture)
{
    ASSUMPTION(value(binding) < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    glapi().glActiveTexture(GL_TEXTURE0 + value(binding));
    glapi().glBindTexture(GL_TEXTURE_2D,texture->id());
}


}

namespace qtgl {


textures_binding_ptr  textures_binding::create(texture_files_map const&  files)
{
    return std::make_shared<textures_binding const>(files);
}

textures_binding_ptr  textures_binding::create(
        std::unordered_map<fragment_shader_texture_sampler_binding,texture_properties> const&  data
        )
{
    return std::make_shared<textures_binding const>(data);
}

textures_binding::textures_binding(texture_files_map const&  files)
    : m_texture_files(files)
    , m_data()
{
    TMPROF_BLOCK();

    ASSUMPTION(!m_texture_files.empty());
    insert_load_request(*this);
}

textures_binding::textures_binding(data_type const&  data)
    : m_texture_files()
    , m_data(data)
{
    TMPROF_BLOCK();
    ASSUMPTION(!m_data.empty());
    insert_load_request(*this);
}

textures_binding::textures_binding(
        std::unordered_map<fragment_shader_texture_sampler_binding,texture_properties> const&  data)
    : m_texture_files()
    , m_data()
{
    TMPROF_BLOCK();

    ASSUMPTION(!data.empty());
    for (auto const& elem : data)
        m_data.insert({elem.first,std::make_shared<texture_properties const>(elem.second)});
    INVARIANT(m_data.size() == data.size());
    insert_load_request(*this);
}

void  insert_load_request(textures_binding const&  binding)
{
    TMPROF_BLOCK();

    for (auto const& elem : binding.texture_files())
        qtgl::insert_load_request(elem.second);
    for (auto const& elem : binding.data())
        qtgl::insert_load_request(elem.second);
}

texture_properties_ptr  find_properties_of_texture_file(boost::filesystem::path const&  texture_file)
{
    return detail::texture_cache::instance().find(texture_file);
}

bool  make_current(textures_binding const&  binding,
                   bool const  use_dummy_texture_if_requested_one_is_not_loaded_yet)
{
    TMPROF_BLOCK();

    detail::texture_cache::instance().process_pending_textures();

    bool result = true;
    for (auto const&  elem : binding.texture_files())
    {
        texture_properties_ptr const  props = find_properties_of_texture_file(elem.second);
        if (!props.operator bool())
        {
            qtgl::insert_load_request(elem.second);
            if (use_dummy_texture_if_requested_one_is_not_loaded_yet == false)
                result = false;
        }
        else if (!qtgl::make_current(elem.first,props,use_dummy_texture_if_requested_one_is_not_loaded_yet))
            result = false;
    }
    for (auto const&  elem : binding.data())
        if (!qtgl::make_current(elem.first,elem.second,use_dummy_texture_if_requested_one_is_not_loaded_yet))
        {
            qtgl::insert_load_request(elem.second);
            result = false;
        }
    return result;
}


}
