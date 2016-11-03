#ifndef QTGL_BUFFER_HPP_INCLUDED
#   define QTGL_BUFFER_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/glapi.hpp>
#   include <qtgl/spatial_boundary.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>
#   include <array>
#   include <vector>
#   include <unordered_map>
#   include <memory>
#   include <utility>

namespace qtgl {


struct buffer_properties;
using  buffer_properties_ptr = std::shared_ptr<buffer_properties const>;


struct buffer_properties
{
    static buffer_properties_ptr  create(
            boost::filesystem::path const&  buffer_file,
            natural_8_bit const  num_components_per_primitive,
            natural_32_bit const  num_primitives,
            natural_8_bit const  num_bytes_per_component,
            bool const   has_integral_components
            );

    buffer_properties(
            boost::filesystem::path const&  buffer_file,
            natural_8_bit const  num_components_per_primitive,
            natural_32_bit const  num_primitives,
            natural_8_bit const  num_bytes_per_component,
            bool const   has_integral_components
            );

    virtual ~buffer_properties() {}

    virtual bool  operator==(buffer_properties const&  other) const;
    virtual size_t  hash() const;

    boost::filesystem::path const&  buffer_file() const noexcept { return m_buffer_file; }
    natural_32_bit  num_primitives() const noexcept { return m_num_primitives; }
    natural_8_bit  num_components_per_primitive() const noexcept { return m_num_components_per_primitive; }
    natural_8_bit  num_bytes_per_component() const noexcept { return m_num_bytes_per_component; }
    bool  has_integral_components() const noexcept { return m_has_integral_components; }

private:
    boost::filesystem::path  m_buffer_file;
    natural_32_bit  m_num_primitives;
    natural_8_bit  m_num_components_per_primitive;
    natural_8_bit  m_num_bytes_per_component;
    bool  m_has_integral_components;
};

//bool  operator==(buffer_properties const&  props0, buffer_properties const&  props1);
inline bool  operator!=(buffer_properties const&  props0, buffer_properties const&  props1) { return !(props0 == props1); }

//size_t  hasher_of_buffer_properties(buffer_properties const&  props);
inline size_t  hasher_of_buffer_properties(buffer_properties const&  props) { return props.hash(); }


struct  vertex_buffer_properties : public buffer_properties
{
    vertex_buffer_properties(
            boost::filesystem::path const&  buffer_file,
            natural_8_bit const  num_components_per_primitive,
            natural_32_bit const  num_primitives,
            spatial_boundary const&  boundary
            );

    spatial_boundary const&  boundary() const noexcept { return m_boundary; }

private:
    spatial_boundary  m_boundary;
};

using  vertex_buffer_properties_ptr = std::shared_ptr<vertex_buffer_properties const>;


}

namespace qtgl {


struct buffer;
using  buffer_ptr = std::shared_ptr<buffer const>;


struct buffer
{
    static buffer_ptr  create(std::vector< std::array<float_32_bit,2> > const&  data,
                              std::string const&  buffer_name);
    static buffer_ptr  create(std::vector< std::array<float_32_bit,3> > const&  data,
                              std::string const&  buffer_name);
    static buffer_ptr  create(std::vector< std::array<float_32_bit,4> > const&  data,
                              std::string const&  buffer_name);

    static buffer_ptr  create(std::vector< natural_32_bit > const&  data,
                              std::string const&  buffer_name);
    static buffer_ptr  create(std::vector< std::array<natural_32_bit,2> > const&  data,
                              std::string const&  buffer_name);
    static buffer_ptr  create(std::vector< std::array<natural_32_bit,3> > const&  data,
                              std::string const&  buffer_name);

    static buffer_ptr  create(GLuint const  id, buffer_properties const&  buffer_props);
    static buffer_ptr  create(GLuint const  id, buffer_properties_ptr const  buffer_props);

    static buffer_ptr  create(std::vector<natural_8_bit> const&  data,
                              buffer_properties_ptr const  buffer_props,
                              std::string& error_message);

    ~buffer();

    GLuint  id() const { return m_id; }
    buffer_properties_ptr  properties() const noexcept { return m_buffer_props; }

private:
    buffer(GLuint const  id, buffer_properties_ptr const  buffer_props);

    buffer(buffer const&) = delete;
    buffer& operator=(buffer const&) = delete;

    GLuint  m_id;
    buffer_properties_ptr  m_buffer_props;
};


buffer_properties_ptr  load_buffer_file(boost::filesystem::path const&  buffer_file,
                                        std::vector<natural_8_bit>&  buffer_data,
                                        std::string&  error_message);

void  send_buffer_load_request(boost::filesystem::path const&  buffer_file);
void  get_properties_of_cached_buffers(std::vector<buffer_properties_ptr>&  output);
void  get_properties_of_failed_buffers(std::vector< std::pair<buffer_properties_ptr,std::string> >&  output);


}

namespace qtgl {


struct buffers_binding;
using  buffers_binding_ptr = std::shared_ptr<buffers_binding const>;


struct buffers_binding
{
    static buffers_binding_ptr  create(
            boost::filesystem::path const&  index_buffer_path,
            std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
            std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings = {}
            );

    static buffers_binding_ptr  create(
            buffer_ptr const  index_buffer,
            std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
            std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings = {}
            );

    static buffers_binding_ptr  create(
            natural_8_bit const  num_indices_per_primitive,  // 1 (points), 2 (lines), or 3 (triangles)
            std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
            std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings = {}
            );

    boost::filesystem::path const&  index_buffer_path() const noexcept { return m_index_buffer_path; }
    std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const& buffer_paths() const noexcept
    { return m_buffer_paths; }

    buffer_ptr  direct_index_buffer() const noexcept { return m_direct_index_buffer; }
    std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings() const noexcept
    { return m_direct_bindings; }

    natural_8_bit  num_indices_per_primitive() const noexcept { return m_num_indices_per_primitive; }

    GLuint  id() const noexcept { return m_binding_data->id(); }
    GLuint  index_buffer_id() const noexcept { return m_binding_data->index_buffer_id(); }
    std::unordered_map<vertex_shader_input_buffer_binding_location,GLuint> const&  bindings() const noexcept
    { return m_binding_data->bindings(); }

    vertex_buffer_properties_ptr  find_vertex_buffer_properties() const;

    bool  make_current() const;

private:

    buffers_binding(boost::filesystem::path const&  index_buffer_path,
                    buffer_ptr const  direct_index_buffer,
                    natural_8_bit const  num_indices_per_primitive,
                    std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
                    std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings);

    buffers_binding(buffers_binding const&) = delete;
    buffers_binding& operator=(buffers_binding const&) = delete;

    struct  binding_data_type
    {
        binding_data_type()
            : m_id(0U)
            , m_index_buffer_id(0U)
            , m_bindings()
        {}

        ~binding_data_type()
        {
            destroy_ID();
        }

        GLuint  id() const noexcept { return m_id; }
        GLuint  index_buffer_id() const noexcept { return m_index_buffer_id; }
        std::unordered_map<vertex_shader_input_buffer_binding_location,GLuint> const&  bindings() const noexcept
        { return m_bindings; }

        bool  reset(GLuint const  index_buffer_id,
                    std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings,
                    std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings);

    private:
        void  destroy_ID();

        GLuint  m_id;
        GLuint  m_index_buffer_id;
        std::unordered_map<vertex_shader_input_buffer_binding_location,GLuint>  m_bindings;
    };

    using  binding_data_ptr = std::unique_ptr<binding_data_type>;

    boost::filesystem::path  m_index_buffer_path;
    std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path>  m_buffer_paths;

    buffer_ptr  m_direct_index_buffer;
    std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr>  m_direct_bindings;

    natural_8_bit  m_num_indices_per_primitive;

    binding_data_ptr  m_binding_data;
};


inline bool  make_current(buffers_binding const&  binding) { return binding.make_current(); }


}

#endif
