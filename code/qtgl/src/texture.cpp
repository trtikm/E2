#include <qtgl/texture.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <QImage>

namespace qtgl { namespace detail { namespace {


GLuint  create_texture(
            natural_32_bit const  image_width,
            natural_32_bit const  image_height,
            image_bytes_sequence const&  image_data,
            natural_32_bit const  image_pixel_components,
            natural_32_bit const  image_pixel_components_type,
            natural_32_bit const  texture_pixel_format,
            natural_32_bit const  texture_x_wrapping_type,
            natural_32_bit const  texture_y_wrapping_type,
            natural_32_bit const  texture_min_filtering_type,
            natural_32_bit const  texture_mag_filtering_type
            )
{
    TMPROF_BLOCK();

    GLuint  id;
    glapi().glGenTextures(1,&id);
    if (id == 0U)
        return 0U;
    glapi().glBindTexture(GL_TEXTURE_2D,id);
    glapi().glTexImage2D(GL_TEXTURE_2D, 0,
                         texture_pixel_format,
                         image_width, image_height,
                         0,
                         image_pixel_components, image_pixel_components_type,
                         (GLvoid const*)&image_data.at(0));
    glapi().glGenerateMipmap(GL_TEXTURE_2D);
    glapi().glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,texture_x_wrapping_type);
    glapi().glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,texture_y_wrapping_type);
    glapi().glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,texture_min_filtering_type);
    glapi().glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,texture_mag_filtering_type);

    return id;
}


}}}

namespace qtgl {


texture::texture(GLuint const  id)
    : m_id(id)
{
    ASSUMPTION(m_id != 0U);
}

texture::~texture()
{
    glapi().glDeleteTextures(1U,&m_id);
}


texture_ptr  create_texture(natural_32_bit const  image_width,
                            natural_32_bit const  image_height,
                            image_bytes_sequence const&  image_data,
                            natural_32_bit const  image_pixel_components,
                            natural_32_bit const  image_pixel_components_type,
                            natural_32_bit const  texture_pixel_format,
                            natural_32_bit const  texture_x_wrapping_type,
                            natural_32_bit const  texture_y_wrapping_type,
                            natural_32_bit const  texture_min_filtering_type,
                            natural_32_bit const  texture_mag_filtering_type
                            )
{
    GLuint const  id =
            detail::create_texture(
                    image_width,
                    image_height,
                    image_data,
                    image_pixel_components,
                    image_pixel_components_type,
                    texture_pixel_format,
                    texture_x_wrapping_type,
                    texture_y_wrapping_type,
                    texture_min_filtering_type,
                    texture_mag_filtering_type
                    );
    if (id == 0U)
        return texture_ptr{};
    return texture_ptr{ new texture{id} };
}

texture_ptr  create_texture(boost::filesystem::path const&  image_file,
                            natural_32_bit const  texture_pixel_format,
                            natural_32_bit const  texture_x_wrapping_type,
                            natural_32_bit const  texture_y_wrapping_type,
                            natural_32_bit const  texture_min_filtering_type,
                            natural_32_bit const  texture_mag_filtering_type
                            )
{
    TMPROF_BLOCK();

    natural_32_bit   image_width;
    natural_32_bit   image_height;
    image_bytes_sequence  image_data;
    {
        QImage  qimage;
        {
            QImage  qtmp_image;
            {
                std::vector<natural_8_bit> buffer(boost::filesystem::file_size(image_file),0U);
                {
                    boost::filesystem::ifstream  istr(image_file,std::ios_base::binary);
                    if (!istr.good())
                        return texture_ptr{};
                    istr.read((char*)&buffer.at(0U),buffer.size());
                    if (istr.bad())
                        return texture_ptr{};
                }
                if (!qtmp_image.loadFromData(&buffer.at(0),(int)buffer.size()))
                    return texture_ptr{};
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
        image_width = qimage.width();
        image_height = qimage.height();
        image_data.resize(qimage.byteCount());
        image_data = image_bytes_sequence(qimage.bits(),qimage.bits()+qimage.byteCount());
    }
    return create_texture(image_width,
                          image_height,
                          image_data,
                          GL_RGBA,
                          GL_UNSIGNED_INT_8_8_8_8,
                          texture_pixel_format,
                          texture_x_wrapping_type,
                          texture_y_wrapping_type,
                          texture_min_filtering_type,
                          texture_mag_filtering_type
                          );
}

texture_ptr  create_chessboard_texture(
                            natural_32_bit const  texture_pixel_format,
                            natural_32_bit const  texture_x_wrapping_type,
                            natural_32_bit const  texture_y_wrapping_type,
                            natural_32_bit const  texture_min_filtering_type,
                            natural_32_bit const  texture_mag_filtering_type
                            )
{
    static float const  image_data[2U*2U*3U] = {
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
    };
    natural_8_bit const* const  begin = (natural_8_bit const*)image_data;
    natural_8_bit const* const  end = begin + sizeof(image_data);
    return create_texture(2U,2U,image_bytes_sequence(begin,end),GL_RGB,GL_FLOAT,
                          texture_pixel_format,
                          texture_x_wrapping_type,
                          texture_y_wrapping_type,
                          texture_min_filtering_type,
                          texture_mag_filtering_type
                          );
}


}

namespace qtgl {


textures_binding_ptr  create_textures_binding(texture_binding_data const&  binding
        )
{
    for (auto const& elem : binding)
        if (!elem.second.operator bool())
            return textures_binding_ptr();
    return textures_binding_ptr{ new textures_binding{binding} };
}

void  make_current(textures_binding_ptr const  binding)
{
    for (auto const&  elem : binding->binding())
    {
        ASSUMPTION(elem.first < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
        glapi().glActiveTexture(GL_TEXTURE0 + elem.first);
        glapi().glBindTexture(GL_TEXTURE_2D,elem.second->id());
    }
}


}
