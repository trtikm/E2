#ifndef QTGL_BUFFER_HPP_INCLUDED
#   define QTGL_BUFFER_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <array>
#   include <vector>
#   include <memory>
#   include <utility>

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

    static buffer_ptr  create(GLuint const  id, natural_8_bit  num_components_per_element, natural_32_bit  num_elements);

    ~buffer();

    GLuint  id() const { return m_id; }
    natural_8_bit  num_components_per_element() const { return m_num_components_per_element; }
    natural_32_bit  num_elements() const { return m_num_elements; }

private:
    buffer(GLuint const  id,
           natural_8_bit const  num_components_per_element,
           natural_32_bit const  num_elements);

    buffer(buffer const&) = delete;
    buffer& operator=(buffer const&) = delete;

    GLuint  m_id;
    natural_8_bit  m_num_components_per_element;
    natural_32_bit  m_num_elements;
};


struct buffers_binding;
using  buffers_binding_ptr = std::shared_ptr<buffers_binding const>;
using  buffer_binding_location = natural_8_bit;


struct buffers_binding
{
    static buffers_binding_ptr  create(
            buffer_ptr const  index_buffer,
            std::vector< std::pair<buffer_binding_location,buffer_ptr> > const&  bindings
            );

    static buffers_binding_ptr  create(
            natural_8_bit const  num_indices_per_primitive,  // 1 (points), 2 (lines), or 3 (triangles)
            std::vector< std::pair<buffer_binding_location,buffer_ptr> > const&  bindings
            );

    ~buffers_binding();

    GLuint  id() const { return m_id; }
    buffer_ptr  index_buffer() const { return m_index_buffer; }

private:
    buffers_binding(GLuint const  id, buffer_ptr const  index_buffer);

    buffers_binding(buffers_binding const&) = delete;
    buffers_binding& operator=(buffers_binding const&) = delete;

    GLuint  m_id;
    buffer_ptr  m_index_buffer;
};


void  make_current(buffers_binding_ptr const  buffers_binding);


inline constexpr buffer_binding_location  vertices_binding_location() noexcept { return 0U; }
inline constexpr buffer_binding_location  colours_binding_location() noexcept { return 1U; }
inline constexpr buffer_binding_location  normals_binding_location() noexcept { return 2U; }
inline constexpr buffer_binding_location  first_texcoords_binding_location() noexcept { return 3U; }


}

#endif
