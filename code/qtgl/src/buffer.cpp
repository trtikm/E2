#include <qtgl/buffer.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <limits>
#include <unordered_set>

namespace qtgl { namespace detail {


void  draw_make_current(GLuint const  id,
                        natural_8_bit const num_components_per_element,
                        natural_32_bit const  num_elements);


}}


namespace qtgl { namespace {


GLuint  create_glbuffer(GLenum const  target, GLvoid const* data, natural_64_bit const size)
{
    TMPROF_BLOCK();

    if (size == 0ULL || size > (natural_64_bit)std::numeric_limits<natural_32_bit>::max())
        return 0U;

    GLuint  id = 0U;
    glapi().glGenBuffers(1U,&id);
    if (id == 0U)
        return 0U;
    glapi().glBindBuffer(target,id);
    glapi().glBufferData(target,(GLsizeiptr)size,data,GL_STATIC_DRAW);
    return id;
}

GLuint  create_vertex_arrays(
        std::vector< std::pair<vertex_shader_input_buffer_binding_location,buffer_ptr> > const&  bindings
        )
{
    TMPROF_BLOCK();

    if (bindings.empty() || bindings.size() > (natural_64_bit)GL_MAX_VERTEX_ATTRIBS)
        return 0U;
    if (![](std::vector< std::pair<vertex_shader_input_buffer_binding_location,buffer_ptr> > const&  bindings)
            {
            std::unordered_set<vertex_shader_input_buffer_binding_location>  visited;
            for (auto const& elem : bindings)
            {
                if (value(elem.first) >= (natural_32_bit)GL_MAX_VERTEX_ATTRIBS ||
                    visited.count(elem.first) != 0 ||
                    !elem.second.operator bool())
                    return false;
                visited.insert(elem.first);
            }
            return true;
            }(bindings) )
        return 0U;

    GLuint  id;
    glapi().glGenVertexArrays(1U,&id);
    if (id == 0U)
        return 0U;
    glapi().glBindVertexArray(id);
    for (auto const& elem : bindings)
    {
        glapi().glBindBuffer(GL_ARRAY_BUFFER,elem.second->id());
        glapi().glEnableVertexAttribArray(value(elem.first));
        glapi().glVertexAttribPointer(value(elem.first),
                                      elem.second->num_components_per_element(),
                                      GL_FLOAT,
                                      GL_FALSE,
                                      0U,
                                      nullptr);
    }

    return id;
}


}}

namespace qtgl {


buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,2> > const&  data)
{
    TMPROF_BLOCK();

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 2ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return buffer_ptr{ new buffer{id,2U,(natural_32_bit)data.size()} };
}

buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,3> > const&  data)
{
    TMPROF_BLOCK();

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 3ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return buffer_ptr{ new buffer{id,3U,(natural_32_bit)data.size()} };
}

buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,4> > const&  data)
{
    TMPROF_BLOCK();

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 4ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return buffer_ptr{ new buffer{id,4U,(natural_32_bit)data.size()} };
}

buffer_ptr  buffer::create(std::vector< natural_32_bit > const&  data)
{
    TMPROF_BLOCK();

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 1ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return buffer_ptr{ new buffer{id,1U,(natural_32_bit)data.size()} };
}

buffer_ptr  buffer::create(std::vector< std::array<natural_32_bit,2> > const&  data)
{
    TMPROF_BLOCK();

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 2ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return buffer_ptr{ new buffer{id,2U,(natural_32_bit)data.size()} };
}

buffer_ptr  buffer::create(std::vector< std::array<natural_32_bit,3> > const&  data)
{
    TMPROF_BLOCK();

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 3ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return buffer_ptr{ new buffer{id,3U,(natural_32_bit)data.size()} };
}

buffer_ptr  buffer::create(GLuint const  id, natural_8_bit  num_components_per_element, natural_32_bit  num_elements)
{
    TMPROF_BLOCK();

    return buffer_ptr{ new buffer{id,num_components_per_element,num_elements} };
}

buffer::buffer(GLuint const  id,
               natural_8_bit const  num_components_per_element,
               natural_32_bit const  num_elements)
    : m_id(id)
    , m_num_components_per_element(num_components_per_element)
    , m_num_elements(num_elements)
{
    // We intentionally allow  id == 0U. We use this as identification of
    // a buffer with invalid/unset content.
    ASSUMPTION(m_num_components_per_element == 2U ||
               m_num_components_per_element == 3U ||
               m_num_components_per_element == 4U );
    ASSUMPTION(m_num_elements > 0U);
}

buffer::~buffer()
{
    if (id() != 0U)
        glapi().glDeleteBuffers(1U,&m_id);
}



}

namespace qtgl {


buffers_binding_ptr  buffers_binding::create(
        buffer_ptr const  index_buffer,
        std::vector< std::pair<vertex_shader_input_buffer_binding_location,buffer_ptr> > const&  bindings
        )
{
    TMPROF_BLOCK();

    GLuint const  id = create_vertex_arrays(bindings);
    if (id == 0U)
        return buffers_binding_ptr{};
    return buffers_binding_ptr( new buffers_binding(id,index_buffer) );
}

buffers_binding_ptr  buffers_binding::create(
        natural_8_bit const  num_indices_per_primitive,
        std::vector< std::pair<vertex_shader_input_buffer_binding_location,buffer_ptr> > const&  bindings
        )
{
    TMPROF_BLOCK();

    buffer_ptr  index_buffer;
    {
        for (auto const& elem : bindings)
            if (elem.first == vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION)
            {
                index_buffer = buffer::create(0U,num_indices_per_primitive,elem.second->num_elements());
                break;
            }
        if (!index_buffer.operator bool())
            return buffers_binding_ptr{};
    }

    GLuint const  id = create_vertex_arrays(bindings);
    if (id == 0U)
        return buffers_binding_ptr{};

    return buffers_binding_ptr( new buffers_binding(id,index_buffer) );
}

buffers_binding::buffers_binding(GLuint const  id, buffer_ptr const  index_buffer)
    : m_id(id)
    , m_index_buffer(index_buffer)
{}

buffers_binding::~buffers_binding()
{
    glapi().glDeleteVertexArrays(1U,&m_id);
}


void  make_current(buffers_binding_ptr const  binding)
{
    TMPROF_BLOCK();

    glapi().glBindVertexArray(binding->id());
    detail::draw_make_current(binding->index_buffer()->id(),
                              binding->index_buffer()->num_components_per_element(),
                              binding->index_buffer()->num_elements());
}


}
