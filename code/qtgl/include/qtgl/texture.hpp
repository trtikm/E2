#ifndef QTGL_TEXTURE_HPP_INCLUDED
#   define QTGL_TEXTURE_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <unordered_map>
#   include <memory>

namespace qtgl {


struct texture_properties;
using  texture_properties_ptr = std::shared_ptr<texture_properties const>;


struct texture_properties
{
    static texture_properties_ptr  create(
            boost::filesystem::path const&  image_file,
            natural_32_bit const  pixel_format = GL_COMPRESSED_RGBA,
            natural_32_bit const  x_wrapping_type = GL_REPEAT,
            natural_32_bit const  y_wrapping_type = GL_REPEAT,
            natural_32_bit const  min_filtering_type = GL_LINEAR_MIPMAP_LINEAR,
            natural_32_bit const  mag_filtering_type = GL_LINEAR
            );

    texture_properties(
            boost::filesystem::path const&  image_file,
            natural_32_bit const  pixel_format = GL_COMPRESSED_RGBA,
            natural_32_bit const  x_wrapping_type = GL_REPEAT,
            natural_32_bit const  y_wrapping_type = GL_REPEAT,
            natural_32_bit const  min_filtering_type = GL_LINEAR_MIPMAP_LINEAR,
            natural_32_bit const  mag_filtering_type = GL_LINEAR
            );

    boost::filesystem::path const&  image_file() const noexcept { return m_image_file; }
    natural_32_bit  pixel_format() const noexcept { return m_pixel_format; }
    natural_32_bit  x_wrapping_type() const noexcept { return m_x_wrapping_type; }
    natural_32_bit  y_wrapping_type() const noexcept { return m_y_wrapping_type; }
    natural_32_bit  min_filtering_type() const noexcept { return m_min_filtering_type; }
    natural_32_bit  mag_filtering_type() const noexcept { return m_mag_filtering_type; }

private:
    boost::filesystem::path  m_image_file;
    natural_32_bit  m_pixel_format;
    natural_32_bit  m_x_wrapping_type;
    natural_32_bit  m_y_wrapping_type;
    natural_32_bit  m_min_filtering_type;
    natural_32_bit  m_mag_filtering_type;
};


bool  operator==(texture_properties const&  props0, texture_properties const&  props1);
inline bool  operator!=(texture_properties const&  props0, texture_properties const&  props1) { return !(props0 == props1); }

size_t  hasher_of_texture_properties(texture_properties const&  props);


}

namespace qtgl {


struct texture_image_properties
{
    texture_image_properties(
            natural_32_bit const  width,
            natural_32_bit const  height,
            natural_8_bit const* const  data_begin,
            natural_8_bit const* const  data_end,
            natural_32_bit const  pixel_components,
            natural_32_bit const  pixel_components_type
            );

    natural_32_bit  width() const noexcept { return m_width; }
    natural_32_bit  height() const noexcept { return m_height; }
    std::vector<natural_8_bit> const&  data() const noexcept { return *m_data; }
    natural_32_bit  pixel_components() const noexcept { return m_pixel_components; }
    natural_32_bit  pixel_components_type() const noexcept { return m_pixel_components_type; }

private:
    natural_32_bit  m_width;
    natural_32_bit  m_height;
    std::unique_ptr< std::vector<natural_8_bit> >  m_data;
    natural_32_bit  m_pixel_components;
    natural_32_bit  m_pixel_components_type;
};


texture_image_properties  load_texture_image_file(boost::filesystem::path const&  image_file);


}

namespace qtgl {


struct texture;
using  texture_ptr = std::shared_ptr<texture const>;

struct texture
{
    static texture_ptr  create(
            texture_image_properties const&  image_props,
            texture_properties_ptr const  texture_props
            );

    ~texture();

    GLuint  id() const { return m_id; }

    natural_32_bit  width() const noexcept { return m_width; }
    natural_32_bit  height() const noexcept { return m_height; }

    texture_properties_ptr  properties() const noexcept { return m_texture_props; }

    boost::filesystem::path const&  image_file() const { return properties()->image_file(); }
    natural_32_bit  pixel_format() const { return properties()->pixel_format(); }
    natural_32_bit  x_wrapping_type() const { return properties()->x_wrapping_type(); }
    natural_32_bit  y_wrapping_type() const { return properties()->y_wrapping_type(); }
    natural_32_bit  min_filtering_type() const { return properties()->min_filtering_type(); }
    natural_32_bit  mag_filtering_type() const { return properties()->mag_filtering_type(); }

private:
    texture(texture_image_properties const&  image_props, texture_properties_ptr const  texture_props);

    GLuint  m_id;
    natural_32_bit  m_width;
    natural_32_bit  m_height;
    texture_properties_ptr  m_texture_props;
};


}

namespace qtgl {


texture_properties_ptr  load_texture_file(boost::filesystem::path const&  texture_file, //!< This is path to a TEXTURE file,
                                                                                        //!< so it is NOT a path to an IMAGE file!!
                                          std::string&  error_message
                                          );


void  insert_load_request(
        boost::filesystem::path const&  texture_file    //!< This is path to a TEXTURE file,
                                                        //!< so it is NOT a path to an IMAGE file!!
        );

void  insert_load_request(texture_properties_ptr const  props);

inline void  insert_load_request(texture_properties const&  props)
{ insert_load_request(std::make_shared<texture_properties>(props)); }


std::weak_ptr<texture const>  find_texture(texture_properties_ptr const  props);

inline std::weak_ptr<texture const>  find_texture(texture_properties const&  props)
{ return find_texture(std::make_shared<texture_properties>(props)); }


bool  make_current(fragment_shader_texture_sampler_binding const  binding,
                   texture_properties_ptr const  props,
                   bool const  use_dummy_texture_if_requested_one_is_not_loaded_yet = true);

inline bool  make_current(fragment_shader_texture_sampler_binding const  binding,
                          texture_properties const&  props,
                          bool const  use_dummy_texture_if_requested_one_is_not_loaded_yet = true)
{
    return make_current(binding,std::make_shared<texture_properties>(props),
                        use_dummy_texture_if_requested_one_is_not_loaded_yet);
}


void  make_current(fragment_shader_texture_sampler_binding const  binding, texture_ptr const  texture);


void  get_properties_of_cached_textures(std::vector< std::pair<boost::filesystem::path,texture_properties_ptr> >&  output);
void  get_properties_of_failed_textures(std::vector< std::pair<boost::filesystem::path,std::string> >&  output);


}

namespace qtgl {


struct  textures_binding;
using  textures_binding_ptr = std::shared_ptr<textures_binding const>;


struct textures_binding
{
    using  texture_files_map = std::unordered_map<fragment_shader_texture_sampler_binding,
                                                  boost::filesystem::path   //!< This is path to a TEXTURE file,
                                                                            //!< so it is NOT a path to an IMAGE file!!
                                                  >;
    using  data_type = std::unordered_map<fragment_shader_texture_sampler_binding,texture_properties_ptr>;

    static textures_binding_ptr  create(texture_files_map const&  files);
    static textures_binding_ptr  create(
            std::unordered_map<fragment_shader_texture_sampler_binding,texture_properties> const&  data
            );

    textures_binding(texture_files_map const&  files);
    textures_binding(data_type const&  data);
    textures_binding(std::unordered_map<fragment_shader_texture_sampler_binding,texture_properties> const&  data);

    texture_files_map  texture_files() const noexcept { return m_texture_files; }
    data_type const& data() const noexcept { return m_data; }

private:

    texture_files_map  m_texture_files;
    data_type  m_data;
};


void  insert_load_request(textures_binding const&  binding);

texture_properties_ptr  find_properties_of_texture_file(
        boost::filesystem::path const&  texture_file    //!< This is path to a TEXTURE file,
                                                        //!< so it is NOT a path to an IMAGE file!!
        );

bool  make_current(textures_binding const&  binding,
                   bool const  use_dummy_texture_if_requested_one_is_not_loaded_yet = true);


}

#endif
