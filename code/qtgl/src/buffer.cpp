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

GLuint  create_vertex_arrays(std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings)
{
    TMPROF_BLOCK();

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
                                      elem.second->properties()->num_components_per_primitive(),
                                      GL_FLOAT,
                                      GL_FALSE,
                                      0U,
                                      nullptr);
    }

    return id;
}


}}

namespace qtgl {


buffer_properties_ptr  buffer_properties::create(
        boost::filesystem::path const&  buffer_file,
        natural_8_bit const  num_components_per_primitive,
        natural_32_bit const  num_primitives,
        natural_8_bit const  num_bytes_per_component,
        bool const   has_integral_components
        )
{
    return std::make_shared<buffer_properties>(buffer_file,num_components_per_primitive,num_primitives,
                                               num_bytes_per_component,has_integral_components);
}

buffer_properties::buffer_properties(
        boost::filesystem::path const&  buffer_file,
        natural_8_bit const  num_components_per_primitive,
        natural_32_bit const  num_primitives,
        natural_8_bit const  num_bytes_per_component,
        bool const   has_integral_components
        )
    : m_buffer_file(buffer_file)
    , m_num_primitives(num_primitives)
    , m_num_components_per_primitive(num_components_per_primitive)
    , m_num_bytes_per_component(num_bytes_per_component)
    , m_has_integral_components(has_integral_components)
{
    ASSUMPTION(m_num_components_per_primitive == 2U ||
               m_num_components_per_primitive == 3U ||
               m_num_components_per_primitive == 4U );
    ASSUMPTION(m_num_primitives > 0U);
    ASSUMPTION((m_has_integral_components && m_num_bytes_per_component == (natural_8_bit)sizeof(natural_32_bit)) ||
               (!m_has_integral_components && m_num_bytes_per_component == sizeof(float_32_bit)));
}

bool  operator==(buffer_properties const&  props0, buffer_properties const&  props1)
{
    return  props0.buffer_file() == props1.buffer_file() &&
            props0.num_components_per_primitive() == props1.num_components_per_primitive() &&
            props0.num_primitives() == props1.num_primitives() &&
            props0.num_bytes_per_component() == props1.num_bytes_per_component() &&
            props0.has_integral_components() == props1.has_integral_components()
            ;
}

size_t  hasher_of_buffer_properties(buffer_properties const&  props)
{
    std::size_t seed = 0ULL;
    boost::hash_combine(seed,props.buffer_file().string());
    boost::hash_combine(seed,props.num_components_per_primitive());
    boost::hash_combine(seed,props.num_primitives());
    boost::hash_combine(seed,props.num_bytes_per_component());
    boost::hash_combine(seed,props.has_integral_components() ? 1U : 0U);
    return seed;
}


}

namespace qtgl {


buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,2> > const&  data)
{
    TMPROF_BLOCK();

    ASSUMPTION(2ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 2ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return create(id,std::make_shared<buffer_properties>("",2U,(natural_32_bit)data.size(),(natural_8_bit)sizeof(float_32_bit),false));
}

buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,3> > const&  data)
{
    TMPROF_BLOCK();

    ASSUMPTION(3ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 3ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return create(id,std::make_shared<buffer_properties>("",3U,(natural_32_bit)data.size(),(natural_8_bit)sizeof(float_32_bit),false));
}

buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,4> > const&  data)
{
    TMPROF_BLOCK();

    ASSUMPTION(4ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 4ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return create(id,std::make_shared<buffer_properties>("",4U,(natural_32_bit)data.size(),(natural_8_bit)sizeof(float_32_bit),false));
}

buffer_ptr  buffer::create(std::vector< natural_32_bit > const&  data)
{
    TMPROF_BLOCK();

    ASSUMPTION(data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 1ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return create(id,std::make_shared<buffer_properties>("",1U,(natural_32_bit)data.size(),(natural_8_bit)sizeof(natural_32_bit),true));
}

buffer_ptr  buffer::create(std::vector< std::array<natural_32_bit,2> > const&  data)
{
    TMPROF_BLOCK();

    ASSUMPTION(2ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 2ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return create(id,std::make_shared<buffer_properties>("",2U,(natural_32_bit)data.size(),(natural_8_bit)sizeof(natural_32_bit),true));
}

buffer_ptr  buffer::create(std::vector< std::array<natural_32_bit,3> > const&  data)
{
    TMPROF_BLOCK();

    ASSUMPTION(3ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 3ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};
    return create(id,std::make_shared<buffer_properties>("",3U,(natural_32_bit)data.size(),(natural_8_bit)sizeof(natural_32_bit),true));
}

buffer_ptr  buffer::create(GLuint const  id, buffer_properties const&  buffer_props)
{
    return create(id,std::make_shared<buffer_properties>(buffer_props));
}

buffer_ptr  buffer::create(GLuint const  id, buffer_properties_ptr const  buffer_props)
{
    return buffer_ptr(new buffer(id,buffer_props));
}

buffer::buffer(GLuint const  id, buffer_properties_ptr const  buffer_props)
    : m_id(id)
    , m_buffer_props(buffer_props)
{
    TMPROF_BLOCK();
    // We intentionally allow  id == 0U. We use this as identification of
    // a buffer with invalid/unset content.
    ASSUMPTION(m_buffer_props.operator bool());
}

buffer::~buffer()
{
    TMPROF_BLOCK();

    if (id() != 0U)
        glapi().glDeleteBuffers(1U,&m_id);
}


}

namespace qtgl {


buffers_binding_ptr  buffers_binding::create(
        buffer_ptr const  index_buffer,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!bindings.empty() && bindings.size() <= (natural_64_bit)GL_MAX_VERTEX_ATTRIBS);
    ASSUMPTION(bindings.count(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION) != 0ULL);
    ASSUMPTION(index_buffer.operator bool() && index_buffer->properties()->has_integral_components());
    ASSUMPTION(
        [](std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings) {
                for (auto const&  elem : bindings)
                    if (elem.second->properties()->num_primitives() != bindings.cbegin()->second->properties()->num_primitives())
                        return false;
                return true;
                }(bindings)
        );

    GLuint const  id = create_vertex_arrays(bindings);
    if (id == 0U)
        return buffers_binding_ptr{};
    return buffers_binding_ptr(new buffers_binding(id,index_buffer,index_buffer->properties()->num_components_per_primitive(),bindings));
}

buffers_binding_ptr  buffers_binding::create(
        natural_8_bit const  num_indices_per_primitive,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!bindings.empty() && bindings.size() <= (natural_64_bit)GL_MAX_VERTEX_ATTRIBS);
    ASSUMPTION(bindings.count(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION) != 0ULL);
    ASSUMPTION(num_indices_per_primitive == 1U || num_indices_per_primitive == 2U || num_indices_per_primitive == 3U);
    ASSUMPTION(
        [](std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings) {
                for (auto const&  elem : bindings)
                    if (elem.second->properties()->num_primitives() != bindings.cbegin()->second->properties()->num_primitives())
                        return false;
                return true;
                }(bindings)
        );

    GLuint const  id = create_vertex_arrays(bindings);
    if (id == 0U)
        return buffers_binding_ptr{};
    return buffers_binding_ptr(new buffers_binding(id,buffer_ptr(),num_indices_per_primitive,bindings));
}

buffers_binding::buffers_binding(GLuint const  id, buffer_ptr const  index_buffer, natural_8_bit const  num_indices_per_primitive,
                                 std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings)
    : m_id(id)
    , m_index_buffer(index_buffer)
    , m_num_indices_per_primitive(num_indices_per_primitive)
    , m_bindings(bindings)
{
    ASSUMPTION(m_id != 0U);
    ASSUMPTION(!m_index_buffer.operator bool() ||
               m_index_buffer->properties()->num_components_per_primitive() == num_indices_per_primitive);
    ASSUMPTION(!m_bindings.empty() && m_bindings.size() <= (natural_64_bit)GL_MAX_VERTEX_ATTRIBS);
    ASSUMPTION(m_bindings.count(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION) != 0ULL);
    ASSUMPTION(
        [](std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings) {
                for (auto const&  elem : bindings)
                    if (elem.second->properties()->num_primitives() != bindings.cbegin()->second->properties()->num_primitives())
                        return false;
                return true;
                }(m_bindings)
        );
}

buffers_binding::~buffers_binding()
{
    glapi().glDeleteVertexArrays(1U,&m_id);
}

natural_32_bit  buffers_binding::num_primitives() const
{
    return m_bindings.at(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION)->properties()->num_primitives();
}

void  make_current(buffers_binding const&  binding)
{
    TMPROF_BLOCK();

    glapi().glBindVertexArray(binding.id());
    detail::draw_make_current(binding.uses_index_buffer() ? binding.index_buffer()->id() : 0U,
                              binding.num_indices_per_primitive(),
                              binding.num_primitives());
}


}
