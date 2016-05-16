#ifndef QTGL_BUFFER_HPP_INCLUDED
#   define QTGL_BUFFER_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/glapi.hpp>
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


bool  operator==(buffer_properties const&  props0, buffer_properties const&  props1);
inline bool  operator!=(buffer_properties const&  props0, buffer_properties const&  props1) { return !(props0 == props1); }

size_t  hasher_of_buffer_properties(buffer_properties const&  props);


}

namespace qtgl {


struct buffer;
using  buffer_ptr = std::shared_ptr<buffer const>;


struct buffer
{
    static buffer_ptr  create(std::vector< std::array<float_32_bit,2> > const&  data);
    static buffer_ptr  create(std::vector< std::array<float_32_bit,3> > const&  data);
    static buffer_ptr  create(std::vector< std::array<float_32_bit,4> > const&  data);

    static buffer_ptr  create(std::vector< natural_32_bit > const&  data);
    static buffer_ptr  create(std::vector< std::array<natural_32_bit,2> > const&  data);
    static buffer_ptr  create(std::vector< std::array<natural_32_bit,3> > const&  data);

    static buffer_ptr  create(GLuint const  id, buffer_properties const&  buffer_props);
    static buffer_ptr  create(GLuint const  id, buffer_properties_ptr const  buffer_props);

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


}

namespace qtgl {


struct buffers_binding;
using  buffers_binding_ptr = std::shared_ptr<buffers_binding const>;


struct buffers_binding
{
    static buffers_binding_ptr  create(
            buffer_ptr const  index_buffer,
            std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings
            );

    static buffers_binding_ptr  create(
            natural_8_bit const  num_indices_per_primitive,  // 1 (points), 2 (lines), or 3 (triangles)
            std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings
            );

    ~buffers_binding();

    GLuint  id() const noexcept { return m_id; }

    bool  uses_index_buffer() const noexcept { return index_buffer().operator bool(); }
    buffer_ptr  index_buffer() const noexcept { return m_index_buffer; }
    natural_32_bit  num_primitives() const;
    natural_8_bit  num_indices_per_primitive() const noexcept { return m_num_indices_per_primitive; }

private:
    buffers_binding(GLuint const  id, buffer_ptr const  index_buffer, natural_8_bit const  num_indices_per_primitive,
                    std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings);

    buffers_binding(buffers_binding const&) = delete;
    buffers_binding& operator=(buffers_binding const&) = delete;

    buffer_ptr  m_index_buffer;
    GLuint  m_id;
    natural_8_bit  m_num_indices_per_primitive;
    std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr>  m_bindings;
};


void  make_current(buffers_binding const&  binding);


}

#endif
