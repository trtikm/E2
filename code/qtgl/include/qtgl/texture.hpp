#ifndef QTGL_TEXTURE_HPP_INCLUDED
#   define QTGL_TEXTURE_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <unordered_map>
#   include <memory>

namespace qtgl { namespace detail {


struct texture_file_data
{
    texture_file_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr);

    texture_file_data(
            async::finalise_load_on_destroy_ptr,
            boost::filesystem::path const&  image_pathname,
            natural_32_bit const  pixel_format,
            natural_32_bit const  x_wrapping_type,
            natural_32_bit const  y_wrapping_type,
            natural_32_bit const  min_filtering_type,
            natural_32_bit const  mag_filtering_type
            )
    {
        initialise(
                image_pathname,
                pixel_format,
                x_wrapping_type,
                y_wrapping_type,
                min_filtering_type,
                mag_filtering_type
                );
    }

    boost::filesystem::path const&  image_pathname() const { return m_image_pathname; }
    natural_32_bit  pixel_format() const { return m_pixel_format; }
    natural_32_bit  x_wrapping_type() const { return m_x_wrapping_type; }
    natural_32_bit  y_wrapping_type() const { return m_y_wrapping_type; }
    natural_32_bit  min_filtering_type() const { return m_min_filtering_type; }
    natural_32_bit  mag_filtering_type() const { return m_mag_filtering_type; }

private:
    void  initialise(
            boost::filesystem::path const&  image_pathname,
            natural_32_bit const  pixel_format,
            natural_32_bit const  x_wrapping_type,
            natural_32_bit const  y_wrapping_type,
            natural_32_bit const  min_filtering_type,
            natural_32_bit const  mag_filtering_type
            );

    boost::filesystem::path  m_image_pathname;
    natural_32_bit  m_pixel_format;
    natural_32_bit  m_x_wrapping_type;
    natural_32_bit  m_y_wrapping_type;
    natural_32_bit  m_min_filtering_type;
    natural_32_bit  m_mag_filtering_type;
};


}}

namespace qtgl {


struct texture_file : public async::resource_accessor<detail::texture_file_data>
{
    texture_file()
        : async::resource_accessor<detail::texture_file_data>()
    {}

    explicit texture_file(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::texture_file_data>(path.string(), 1U)
    {}

    texture_file(
            boost::filesystem::path const&  image_pathname,
            natural_32_bit const  pixel_format,
            natural_32_bit const  x_wrapping_type,
            natural_32_bit const  y_wrapping_type,
            natural_32_bit const  min_filtering_type,
            natural_32_bit const  mag_filtering_type,
            boost::filesystem::path const&  path
            )
        : async::resource_accessor<detail::texture_file_data>(
                path.string(),
                async::notification_callback_type(),
                image_pathname,
                pixel_format,
                x_wrapping_type,
                y_wrapping_type,
                min_filtering_type,
                mag_filtering_type
                )
    {}

    boost::filesystem::path const&  image_pathname() const { return resource().image_pathname(); }
    natural_32_bit  pixel_format() const { return resource().pixel_format(); }
    natural_32_bit  x_wrapping_type() const { return resource().x_wrapping_type(); }
    natural_32_bit  y_wrapping_type() const { return resource().y_wrapping_type(); }
    natural_32_bit  min_filtering_type() const { return resource().min_filtering_type(); }
    natural_32_bit  mag_filtering_type() const { return resource().mag_filtering_type(); }
};


}

namespace qtgl { namespace detail {


struct texture_image_data
{
    texture_image_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr);

    texture_image_data(
            async::finalise_load_on_destroy_ptr,
            natural_32_bit const  width,
            natural_32_bit const  height,
            natural_8_bit const* const  data_begin,
            natural_8_bit const* const  data_end,
            natural_32_bit const  pixel_components,
            natural_32_bit const  pixel_components_type
            )
    { initialise(width, height, data_begin, data_end, pixel_components, pixel_components_type); }

    natural_32_bit  width() const { return m_width; }
    natural_32_bit  height() const { return m_height; }
    std::vector<natural_8_bit> const&  data() const { return *m_data; }
    natural_32_bit  pixel_components() const { return m_pixel_components; }
    natural_32_bit  pixel_components_type() const { return m_pixel_components_type; }

private:
    void  initialise(
            natural_32_bit const  width,
            natural_32_bit const  height,
            natural_8_bit const* const  data_begin,
            natural_8_bit const* const  data_end,
            natural_32_bit const  pixel_components,
            natural_32_bit const  pixel_components_type
            );

    natural_32_bit  m_width;
    natural_32_bit  m_height;
    std::unique_ptr< std::vector<natural_8_bit> >  m_data;
    natural_32_bit  m_pixel_components;
    natural_32_bit  m_pixel_components_type;
};


}}

namespace qtgl {


struct texture_image : public async::resource_accessor<detail::texture_image_data>
{
    texture_image()
        : async::resource_accessor<detail::texture_image_data>()
    {}

    explicit texture_image(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::texture_image_data>(path.string(), 1U)
    {}

    texture_image(
            natural_32_bit const  width,
            natural_32_bit const  height,
            natural_8_bit const* const  data_begin,
            natural_8_bit const* const  data_end,
            natural_32_bit const  pixel_components,
            natural_32_bit const  pixel_components_type,
            boost::filesystem::path const&  path
            )
        : async::resource_accessor<detail::texture_image_data>(
                path.string(), async::notification_callback_type(),
                width, height, data_begin, data_end, pixel_components, pixel_components_type
                )
    {}

    natural_32_bit  width() const { return resource().width(); }
    natural_32_bit  height() const { return resource().height(); }
    std::vector<natural_8_bit> const&  data() const { return resource().data(); }
    natural_32_bit  pixel_components() const { return resource().pixel_components(); }
    natural_32_bit  pixel_components_type() const { return resource().pixel_components_type(); }
};


}

namespace qtgl { namespace detail {


struct texture_data
{
    texture_data(std::string const&  key, async::finalise_load_on_destroy_ptr  finaliser);

    texture_data(
            async::finalise_load_on_destroy_ptr  finaliser,
            GLuint const  id,
            texture_file const  texture_props,
            texture_image const  image_props
            );

    ~texture_data();

    GLuint  id() const { return m_id; }
    natural_32_bit  width() const { return m_image_props.width(); }
    natural_32_bit  height() const { return m_image_props.height(); }
    natural_32_bit  pixel_format() const { return m_texture_props.pixel_format(); }
    natural_32_bit  x_wrapping_type() const { return m_texture_props.x_wrapping_type(); }
    natural_32_bit  y_wrapping_type() const { return m_texture_props.y_wrapping_type(); }
    natural_32_bit  min_filtering_type() const { return m_texture_props.min_filtering_type(); }
    natural_32_bit  mag_filtering_type() const { return m_texture_props.mag_filtering_type(); }

    void  create_gl_image();

private:

    GLuint  m_id;
    texture_file  m_texture_props;
    texture_image  m_image_props;
};


}}

namespace qtgl {


struct texture : public async::resource_accessor<detail::texture_data>
{
    texture()
        : async::resource_accessor<detail::texture_data>()
    {}

    explicit texture(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::texture_data>("[texture]:" + path.string(), 1U)
    {}

    texture(GLuint const  id,
            texture_file const  texture_props,
            texture_image const  image_props,
            boost::filesystem::path const&  path
            )
        : async::resource_accessor<detail::texture_data>(
                "[texture]:" + path.string(),
                async::notification_callback_type(),
                id,
                texture_props,
                image_props
                )
    {}

    GLuint  id() const { return resource().id(); }
    natural_32_bit  width() const { return resource().width(); }
    natural_32_bit  height() const { return resource().height(); }
    natural_32_bit  pixel_format() const { return resource().pixel_format(); }
    natural_32_bit  x_wrapping_type() const { return resource().x_wrapping_type(); }
    natural_32_bit  y_wrapping_type() const { return resource().y_wrapping_type(); }
    natural_32_bit  min_filtering_type() const { return resource().min_filtering_type(); }
    natural_32_bit  mag_filtering_type() const { return resource().mag_filtering_type(); }

    void  create_gl_image() const { resource().create_gl_image(); }
};


}

namespace qtgl {


struct textures_binding
{
    using  binding_map_type = std::unordered_map<fragment_shader_texture_sampler_binding, texture>;

    explicit textures_binding(bool const  make_ready);
    explicit textures_binding(binding_map_type const&  bindings);

    binding_map_type const&  bindings_map() const { return m_bindings; }
    bool  ready() const { return m_ready; }

    bool  make_current() const;

private:

    binding_map_type  m_bindings;
    mutable bool  m_ready;
};


using  textures_binding_map_type = textures_binding::binding_map_type;


bool  make_current(textures_binding const&  binding);


}

#endif
