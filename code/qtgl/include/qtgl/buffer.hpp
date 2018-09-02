#ifndef QTGL_BUFFER_HPP_INCLUDED
#   define QTGL_BUFFER_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/glapi.hpp>
#   include <qtgl/spatial_boundary.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/filesystem/path.hpp>
#   include <array>
#   include <vector>
#   include <unordered_map>
#   include <memory>
#   include <utility>


namespace qtgl { namespace detail {


struct  buffer_file_data
{
    buffer_file_data(async::finalise_load_on_destroy_ptr const  finaliser);

    buffer_file_data(
            async::finalise_load_on_destroy_ptr,
            GLuint const  id, 
            natural_8_bit const  num_components_per_primitive,
            natural_32_bit const  num_primitives,
            natural_8_bit const  num_bytes_per_component,
            bool const   has_integral_components,
            natural_8_bit const* const  data_begin,
            natural_8_bit const* const  data_end,
            spatial_boundary const* const  boundary = nullptr
            );

    buffer_file_data(
            async::finalise_load_on_destroy_ptr,
            std::vector< std::array<float_32_bit,2> > const&  data
            );
    buffer_file_data(
            async::finalise_load_on_destroy_ptr,
            std::vector< std::array<float_32_bit,3> > const&  data,
            bool const  do_compute_boundary = false
            );
    buffer_file_data(
            async::finalise_load_on_destroy_ptr,
            std::vector< std::array<float_32_bit,4> > const&  data
            );

    buffer_file_data(
            async::finalise_load_on_destroy_ptr,
            std::vector< natural_32_bit > const&  data
            );
    buffer_file_data(
            async::finalise_load_on_destroy_ptr,
            std::vector< std::array<natural_32_bit,2> > const&  data
            );
    buffer_file_data(
            async::finalise_load_on_destroy_ptr,
            std::vector< std::array<natural_32_bit,3> > const&  data
            );

    ~buffer_file_data();

    GLuint  id() const { return m_id; }
    natural_32_bit  num_primitives() const { return m_num_primitives; }
    natural_8_bit  num_components_per_primitive() const { return m_num_components_per_primitive; }
    natural_8_bit  num_bytes_per_component() const { return m_num_bytes_per_component; }
    bool  has_integral_components() const { return m_has_integral_components; }
    std::vector<natural_8_bit> const&  data() const { return *m_data_ptr; }
    bool  has_boundary() const { return m_boundary.operator bool(); }
    spatial_boundary const&  boundary() const { return *m_boundary; }

    void  create_gl_buffer();
    void  destroy_gl_buffer();

private:
    void  initialise(
            GLuint const  id, 
            natural_8_bit const  num_components_per_primitive,
            natural_32_bit const  num_primitives,
            natural_8_bit const  num_bytes_per_component,
            bool const   has_integral_components,
            natural_8_bit const* const  data_begin,
            natural_8_bit const* const  data_end,
            spatial_boundary const* const  boundary
            );

    GLuint  m_id;
    natural_32_bit  m_num_primitives;
    natural_8_bit  m_num_components_per_primitive;
    natural_8_bit  m_num_bytes_per_component;
    bool  m_has_integral_components;
    std::unique_ptr< std::vector<natural_8_bit> >  m_data_ptr;
    std::unique_ptr<spatial_boundary>  m_boundary;
};


}}

namespace qtgl {


struct buffer : public async::resource_accessor<detail::buffer_file_data>
{
    buffer()
        : async::resource_accessor<detail::buffer_file_data>()
    {}

    explicit buffer(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::buffer_file_data>(
            {"qtgl::buffer",path.string()},
            1U,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr)
    {
        async::resource_accessor<detail::buffer_file_data>::insert_load_request(
            { "qtgl::buffer", path.string() },
            1U,
            parent_finaliser
            );
    }

    buffer( GLuint const  id, 
            natural_8_bit const  num_components_per_primitive,
            natural_32_bit const  num_primitives,
            natural_8_bit const  num_bytes_per_component,
            bool const   has_integral_components,
            natural_8_bit const* const  data_begin,
            natural_8_bit const* const  data_end,
            spatial_boundary const* const  boundary,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::buffer_file_data>(
                key.empty() ? async::key_type("qtgl::buffer") : async::key_type{ "qtgl::buffer", key },
                parent_finaliser,
                id,
                num_components_per_primitive, num_primitives, num_bytes_per_component, has_integral_components,
                data_begin, data_end,
                boundary
                )
    {}

    buffer(std::vector< std::array<float_32_bit,2> > const&  data, std::string const&  key = "")
        : async::resource_accessor<detail::buffer_file_data>(
                key.empty() ? async::key_type("qtgl::buffer") : async::key_type{ "qtgl::buffer", key },
                nullptr,
                data
                )
    {}

    buffer( std::vector< std::array<float_32_bit,3> > const&  data,
            bool const  do_compute_boundary = false,
            std::string const&  key = ""
            )
        : async::resource_accessor<detail::buffer_file_data>(
                key.empty() ? async::key_type("qtgl::buffer") : async::key_type{ "qtgl::buffer", key },
                nullptr,
                data,
                do_compute_boundary
                )
    {}

    buffer(std::vector< std::array<float_32_bit,4> > const&  data, std::string const&  key = "")
        : async::resource_accessor<detail::buffer_file_data>(
                key.empty() ? async::key_type("qtgl::buffer") : async::key_type{ "qtgl::buffer", key },
                nullptr,
                data
                )
    {}

    buffer(std::vector< natural_32_bit > const&  data, std::string const&  key = "")
        : async::resource_accessor<detail::buffer_file_data>(
                key.empty() ? async::key_type("qtgl::buffer") : async::key_type{ "qtgl::buffer", key },
                nullptr,
                data
                )
    {}

    buffer(std::vector< std::array<natural_32_bit,2> > const&  data, std::string const&  key = "")
        : async::resource_accessor<detail::buffer_file_data>(
                key.empty() ? async::key_type("qtgl::buffer") : async::key_type{ "qtgl::buffer", key },
                nullptr,
                data
                )
    {}

    buffer(std::vector< std::array<natural_32_bit,3> > const&  data, std::string const&  key = "")
        : async::resource_accessor<detail::buffer_file_data>(
                key.empty() ? async::key_type("qtgl::buffer") : async::key_type{ "qtgl::buffer", key },
                nullptr,
                data
                )
    {}

    GLuint  id() const { return resource().id(); }
    natural_32_bit  num_primitives() const { return resource().num_primitives(); }
    natural_8_bit  num_components_per_primitive() const { return resource().num_components_per_primitive(); }
    natural_8_bit  num_bytes_per_component() const { return resource().num_bytes_per_component(); }
    bool  has_integral_components() const { return resource().has_integral_components(); }
    std::vector<natural_8_bit> const&  data() const { return resource().data(); }
    bool  has_boundary() const { return resource().has_boundary(); }
    spatial_boundary const&  boundary() const { return resource().boundary(); }

    void  create_gl_buffer() const { resource().create_gl_buffer(); }
    void  destroy_gl_buffer() const { resource().destroy_gl_buffer(); }
};


}

namespace qtgl { namespace detail {


struct buffers_binding_data
{
    using  buffers_map_type = std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, buffer>;

    buffers_binding_data(
            async::finalise_load_on_destroy_ptr  finaliser,
            GLuint const  id,
            buffer const  index_buffer,
            buffers_map_type const&  buffers
            )
    {
        initialise(id, index_buffer, 0U, buffers);
    }

    buffers_binding_data(
            async::finalise_load_on_destroy_ptr const  finaliser,
            GLuint const  id,
            natural_8_bit const  num_indices_per_primitive, // 1 (points), 2 (lines), or 3 (triangles)
            buffers_map_type const&  buffers
            )
    {
        initialise(id, buffer(), num_indices_per_primitive, buffers);
    }

    buffers_binding_data(
            async::finalise_load_on_destroy_ptr const  finaliser,
            boost::filesystem::path const&  index_buffer_path,
            std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path> const&  paths
            );

    buffers_binding_data(
            async::finalise_load_on_destroy_ptr const  finaliser,
            natural_8_bit const  num_indices_per_primitive, // 1 (points), 2 (lines), or 3 (triangles)
            std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path> const&  paths
            );

    ~buffers_binding_data();

    GLuint  id() const { return m_id; }
    buffer  get_index_buffer() const { return m_index_buffer; }    
    natural_8_bit  get_num_indices_per_primitive() const { return m_num_indices_per_primitive; }
    buffers_map_type const& get_buffers() const { return m_buffers; }

    spatial_boundary const& get_boundary() const
    {
        return get_buffers().at(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION).boundary();
    }

    bool  ready() const { return m_ready; }
    void  set_ready() { m_ready = true; }

    void  create_gl_binding();
    void  destroy_gl_binding();

private:

    void  initialise(
            GLuint const  id,
            buffer const  index_buffer,
            natural_8_bit const  num_indices_per_primitive, // 1 (points), 2 (lines), or 3 (triangles)
            buffers_map_type const&  buffers
            );

    GLuint  m_id;
    buffer  m_index_buffer;
    natural_8_bit  m_num_indices_per_primitive; // 1 (points), 2 (lines), or 3 (triangles);
                                                // it is used, only if 'm_index_buffer.empty()'.
    buffers_map_type  m_buffers;
    bool  m_ready;
};


}}

namespace qtgl {


struct  buffers_binding : public async::resource_accessor<detail::buffers_binding_data>
{
    using  buffers_map_type = detail::buffers_binding_data::buffers_map_type;

    buffers_binding()
        : async::resource_accessor<detail::buffers_binding_data>()
    {}

    buffers_binding(
            GLuint const  id,
            buffer const  index_buffer,
            buffers_map_type const&  buffers,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::buffers_binding_data>(
                key.empty() ? async::key_type("qtgl::buffer_binding") : async::key_type{ "qtgl::buffer_binding", key },
                parent_finaliser,
                id,
                index_buffer,
                buffers
                )
    {}

    buffers_binding(
            GLuint const  id,
            natural_8_bit const  num_indices_per_primitive, // 1 (points), 2 (lines), or 3 (triangles)
            buffers_map_type const&  buffers,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::buffers_binding_data>(
                key.empty() ? async::key_type("qtgl::buffer_binding") : async::key_type{ "qtgl::buffer_binding", key },
                parent_finaliser,
                id,
                num_indices_per_primitive,
                buffers
                )
    {}

    buffers_binding(
            boost::filesystem::path const&  index_buffer_path,
            std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path> const&  paths,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::buffers_binding_data>(
                key.empty() ? async::key_type("qtgl::buffer_binding") : async::key_type{ "qtgl::buffer_binding", key },
                parent_finaliser,
                index_buffer_path,
                paths
                )
    {}

    buffers_binding(
            natural_8_bit const  num_indices_per_primitive, // 1 (points), 2 (lines), or 3 (triangles)
            std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path> const&  paths,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::buffers_binding_data>(
                key.empty() ? async::key_type("qtgl::buffer_binding") : async::key_type{ "qtgl::buffer_binding", key },
                parent_finaliser,
                num_indices_per_primitive,
                paths
                )
    {}

    GLuint  id() const { return resource().id(); }
    buffer  get_index_buffer() const { return resource().get_index_buffer(); }

    natural_8_bit  get_num_indices_per_primitive() const
    {
        return get_index_buffer().loaded_successfully() ? get_index_buffer().num_components_per_primitive() :
                                                          resource().get_num_indices_per_primitive();
    }

    buffers_map_type const& get_buffers() const { return resource().get_buffers(); }

    natural_32_bit  num_matrices_per_vertex() const
    {
        auto const  it = get_buffers().find(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_INDICES_OF_MATRICES);
        return it == get_buffers().cend() ? 0U : it->second.num_components_per_primitive();
    }

    spatial_boundary const& get_boundary() const { return resource().get_boundary(); }

    bool  ready() const;
    bool  make_current() const;

private:

    void  set_ready() { resource().set_ready(); }
    void  create_gl_binding() { return resource().create_gl_binding(); }
    void  destroy_gl_binding() { return resource().destroy_gl_binding(); }
};


using  buffers_binding_map_type = buffers_binding::buffers_map_type;


bool  make_current(buffers_binding const&  binding);


}

#endif
