#include <qtgl/texture.hpp>
#include <qtgl/detail/texture_cache.hpp>
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
                GL_UNSIGNED_INT_8_8_8_8
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
    ASSUMPTION(static_cast<natural_8_bit>(binding) < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    glapi().glActiveTexture(GL_TEXTURE0 + static_cast<natural_8_bit>(binding));
    glapi().glBindTexture(GL_TEXTURE_2D,texture->id());
}


}

namespace qtgl {


textures_binding_ptr  textures_binding::create(
        std::vector< std::pair<fragment_shader_texture_sampler_binding,texture_properties> > const&  data
        )
{
    return std::make_shared<textures_binding const>(data);
}

textures_binding::textures_binding(data_type const&  data)
    : m_data(data)
{
    TMPROF_BLOCK();
    ASSUMPTION(!data.empty());
}

textures_binding::textures_binding(
        std::vector< std::pair<fragment_shader_texture_sampler_binding,texture_properties> > const&  data)
    : m_data()
{
    TMPROF_BLOCK();

    ASSUMPTION(!data.empty());
    for (auto const& elem : data)
        m_data.push_back({elem.first,std::make_shared<texture_properties const>(elem.second)});
    INVARIANT(m_data.size() == data.size());
}

void  insert_load_request(textures_binding const&  binding)
{
    TMPROF_BLOCK();

    for (auto const& elem : binding.data())
        qtgl::insert_load_request(elem.second);
}

void  make_current(textures_binding const&  binding,
                   bool const  use_dummy_texture_if_requested_one_is_not_loaded_yet)
{
    TMPROF_BLOCK();

    for (auto const&  elem : binding.data())
        qtgl::make_current(elem.first,elem.second,use_dummy_texture_if_requested_one_is_not_loaded_yet);
}


}
