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
    texture_file_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~texture_file_data();

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

    texture_file(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::texture_file_data>(
            {"qtgl::texture_file",path.string()},
            1U,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr)
    {
        async::resource_accessor<detail::texture_file_data>::insert_load_request(
            { "qtgl::texture_file", path.string() },
            1U,
            parent_finaliser
            );
    }

    texture_file(
            boost::filesystem::path const&  image_pathname,
            natural_32_bit const  pixel_format,
            natural_32_bit const  x_wrapping_type,
            natural_32_bit const  y_wrapping_type,
            natural_32_bit const  min_filtering_type,
            natural_32_bit const  mag_filtering_type,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::texture_file_data>(
                key.empty() ? async::key_type("qtgl::texture_file") : async::key_type{ "qtgl::texture_file", key },
                parent_finaliser,
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
    texture_image_data(async::finalise_load_on_destroy_ptr const  finaliser);

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

    ~texture_image_data();

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

    texture_image(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::texture_image_data>(
            {"qtgl::texture_image",path.string()},
            1U,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr)
    {
        async::resource_accessor<detail::texture_image_data>::insert_load_request(
            { "qtgl::texture_image", path.string() },
            1U,
            parent_finaliser
            );
    }

    texture_image(
            natural_32_bit const  width,
            natural_32_bit const  height,
            natural_8_bit const* const  data_begin,
            natural_8_bit const* const  data_end,
            natural_32_bit const  pixel_components,
            natural_32_bit const  pixel_components_type,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::texture_image_data>(
                key.empty() ? async::key_type("qtgl::texture_image") : async::key_type{ "qtgl::texture_image", key },
                parent_finaliser,
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
    texture_data(async::finalise_load_on_destroy_ptr const  finaliser);

    texture_data(
            async::finalise_load_on_destroy_ptr,
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
    void  destroy_gl_image();

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

    texture(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::texture_data>(
            {"qtgl::texture", path.string()},
            1U,
            parent_finaliser
            )
    {}

    texture(GLuint const  id,
            texture_file const  texture_props,
            texture_image const  image_props,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::texture_data>(
                key.empty() ? async::key_type("qtgl::texture") : async::key_type{ "qtgl::texture", key },
                parent_finaliser,
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
    void  destroy_gl_image() const { resource().destroy_gl_image(); }
};


}

namespace qtgl { namespace detail {


struct textures_binding_data
{
    using  binding_map_type = std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME, texture>;

    textures_binding_data(
            async::finalise_load_on_destroy_ptr,
            binding_map_type const&  bindings
            )
        : m_bindings(bindings)
        , m_ready(false)
    {
        ASSUMPTION(
            [this]() -> bool {
                    for (auto const& sampler_and_texture : m_bindings)
                    {
                        if (!is_texture_sampler(sampler_and_texture.first))
                            return false;
                        if (value(sampler_and_texture.first) >= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
                            return false;
                        if (sampler_and_texture.second.empty())
                            return false;
                    }
                    return true;
                }()
            );    
    }

    textures_binding_data(
            async::finalise_load_on_destroy_ptr finaliser,
            std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME, boost::filesystem::path> const&  texture_paths
            );

    binding_map_type const&  bindings_map() const { return m_bindings; }

    bool  ready() const { return m_ready; }
    void  set_ready() { m_ready = true; }

private:

    binding_map_type  m_bindings;
    bool  m_ready;
};


}}

namespace qtgl {


struct textures_binding : public async::resource_accessor<detail::textures_binding_data>
{
    using  binding_map_type = detail::textures_binding_data::binding_map_type;

    textures_binding()
        : async::resource_accessor<detail::textures_binding_data>()
    {}

    textures_binding(
            binding_map_type const&  bindings,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::textures_binding_data>(
                key.empty() ? async::key_type("qtgl::textures_binding") : async::key_type{ "qtgl::textures_binding", key },
                parent_finaliser,
                bindings
                )
    {}

    textures_binding(
            std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME, boost::filesystem::path> const&  texture_paths,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::textures_binding_data>(
                key.empty() ? async::key_type("qtgl::textures_binding") : async::key_type{ "qtgl::textures_binding", key },
                parent_finaliser,
                texture_paths
                )
    {}

    binding_map_type const&  bindings_map() const { return resource().bindings_map(); }

    bool  ready() const;
    bool  make_current() const;

private:

    void  set_ready() { resource().set_ready(); }
};


using  textures_binding_map_type = textures_binding::binding_map_type;


bool  make_current(textures_binding const&  binding);


}

#endif
