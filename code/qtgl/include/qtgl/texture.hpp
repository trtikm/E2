#ifndef QTGL_TEXTURE_HPP_INCLUDED
#   define QTGL_TEXTURE_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <utility>
#   include <memory>

namespace qtgl {


typedef std::vector<natural_8_bit>  image_bytes_sequence;


struct texture;
using  texture_ptr = std::shared_ptr<texture const>;

struct texture
{
    static texture_ptr  create(natural_32_bit const  image_width,
                               natural_32_bit const  image_height,
                               image_bytes_sequence const&  image_data,
                               natural_32_bit const  image_pixel_components = GL_RGBA,
                               natural_32_bit const  image_pixel_components_type = GL_UNSIGNED_INT_8_8_8_8,
                               natural_32_bit const  texture_pixel_format = GL_COMPRESSED_RGBA,
                               natural_32_bit const  texture_x_wrapping_type = GL_REPEAT,
                               natural_32_bit const  texture_y_wrapping_type = GL_REPEAT,
                               natural_32_bit const  texture_min_filtering_type = GL_LINEAR_MIPMAP_LINEAR,
                               natural_32_bit const  texture_mag_filtering_type = GL_LINEAR
                               );

    static texture_ptr  create(boost::filesystem::path const&  image_file,
                               natural_32_bit const  texture_pixel_format = GL_COMPRESSED_RGBA,
                               natural_32_bit const  texture_x_wrapping_type = GL_REPEAT,
                               natural_32_bit const  texture_y_wrapping_type = GL_REPEAT,
                               natural_32_bit const  texture_min_filtering_type = GL_LINEAR_MIPMAP_LINEAR,
                               natural_32_bit const  texture_mag_filtering_type = GL_LINEAR
                               );

    ~texture();

    GLuint  id() const { return m_id; }

private:
    texture(GLuint const  id);

    GLuint  m_id;
};


}

namespace qtgl {


typedef natural_8_bit  texture_binding_location;
typedef std::vector< std::pair<texture_binding_location,texture_ptr> >  texture_binding_data;


inline constexpr texture_binding_location  diffuse_texture_binding_location() noexcept { return 0U; }


struct textures_binding;
using  textures_binding_ptr = std::shared_ptr<textures_binding const>;


struct textures_binding
{
    static textures_binding_ptr  create(texture_binding_data const&  binding);

    texture_binding_data const&  binding() const { return m_binding; }

private:
    textures_binding(texture_binding_data const&  binding)
        : m_binding(binding)
    {}

    texture_binding_data  m_binding;
};


void  make_current(textures_binding_ptr const  binding);


}

#endif
