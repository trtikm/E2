#include <qtgl/buffer.hpp>
#include <qtgl/detail/buffer_cache.hpp>
#include <qtgl/detail/read_line.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <limits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <unordered_set>

namespace qtgl { namespace detail { namespace current_draw {


void  set_are_buffers_ready(bool const  are_buffers_ready);
void  set_index_buffer_id(GLuint const id);
void  set_num_components_per_primitive(natural_8_bit const  num_components_per_primitive);
void  set_num_primitives(natural_32_bit const  num_primitives);


}}}


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

GLuint  create_vertex_arrays()
{
    TMPROF_BLOCK();

    GLuint  id;
    glapi().glGenVertexArrays(1U,&id);
    if (id == 0U)
        return 0U;
    glapi().glBindVertexArray(id);
    return id;
}

void  fill_vertex_arrays(std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings)
{
    TMPROF_BLOCK();

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
}

bool  check_consistency(std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings)
{
    for (auto const&  elem : direct_bindings)
    {
        if (!elem.second.operator bool())
            return false;
        if (elem.second->properties()->num_primitives() != direct_bindings.cbegin()->second->properties()->num_primitives())
            return false;
    }
    return true;
}

bool  check_consistency(
        std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings
        )
{
    if (buffer_paths.size() + direct_bindings.size() == 0U ||
        buffer_paths.size() + direct_bindings.size() > (natural_64_bit)GL_MAX_VERTEX_ATTRIBS)
        return false;

    if (buffer_paths.count(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION) == 0ULL &&
        direct_bindings.count(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION) == 0ULL)
        return false;

    for (auto const&  elem : buffer_paths)
    {
        if (elem.second.empty())
            return false;
        if (direct_bindings.count(elem.first) != 0ULL)
            return false;
    }

    return check_consistency(direct_bindings);
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

bool  buffer_properties::operator==(buffer_properties const&  other) const
{
    return  buffer_file() == other.buffer_file() &&
            num_components_per_primitive() == other.num_components_per_primitive() &&
            num_primitives() == other.num_primitives() &&
            num_bytes_per_component() == other.num_bytes_per_component() &&
            has_integral_components() == other.has_integral_components()
            ;
}

size_t  buffer_properties::hash() const
{
    std::size_t seed = 0ULL;
    boost::hash_combine(seed,buffer_file().string());
    boost::hash_combine(seed,num_components_per_primitive());
    boost::hash_combine(seed,num_primitives());
    boost::hash_combine(seed,num_bytes_per_component());
    boost::hash_combine(seed,has_integral_components() ? 1U : 0U);
    return seed;
}


//bool  operator==(buffer_properties const&  props0, buffer_properties const&  props1)
//{
//    return  props0.buffer_file() == props1.buffer_file() &&
//            props0.num_components_per_primitive() == props1.num_components_per_primitive() &&
//            props0.num_primitives() == props1.num_primitives() &&
//            props0.num_bytes_per_component() == props1.num_bytes_per_component() &&
//            props0.has_integral_components() == props1.has_integral_components()
//            ;
//}

//size_t  hasher_of_buffer_properties(buffer_properties const&  props)
//{
//    std::size_t seed = 0ULL;
//    boost::hash_combine(seed,props.buffer_file().string());
//    boost::hash_combine(seed,props.num_components_per_primitive());
//    boost::hash_combine(seed,props.num_primitives());
//    boost::hash_combine(seed,props.num_bytes_per_component());
//    boost::hash_combine(seed,props.has_integral_components() ? 1U : 0U);
//    return seed;
//}


vertex_buffer_properties::vertex_buffer_properties(
        boost::filesystem::path const&  buffer_file,
        natural_8_bit const  num_components_per_primitive,
        natural_32_bit const  num_primitives,
        spatial_boundary const&  boundary
        )
    : buffer_properties(
          buffer_file,
          num_components_per_primitive,
          num_primitives,
          sizeof(float_32_bit),
          false
          )
    , m_boundary(boundary)
{}


}

namespace qtgl {


buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,2> > const&  data, std::string const&  buffer_name)
{
    TMPROF_BLOCK();

    ASSUMPTION(2ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 2ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};

    std::size_t  seed = 0ULL;
    for (auto const&  elem : data)
    {
        boost::hash_combine(seed,elem.at(0ULL));
        boost::hash_combine(seed,elem.at(1ULL));
    }
    boost::filesystem::path  buffer_path =
            msgstream() << "/generated_buffer/id_" << seed << "/" << buffer_name << "/" << msgstream::end();

    return create(id,std::make_shared<buffer_properties>(buffer_path,2U,(natural_32_bit)data.size(),
                                                         (natural_8_bit)sizeof(float_32_bit),false));
}

buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,3> > const&  data, std::string const&  buffer_name)
{
    TMPROF_BLOCK();

    ASSUMPTION(3ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 3ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};

    std::size_t  seed = 0ULL;
    for (auto const&  elem : data)
    {
        boost::hash_combine(seed,elem.at(0ULL));
        boost::hash_combine(seed,elem.at(1ULL));
        boost::hash_combine(seed,elem.at(2ULL));
    }
    boost::filesystem::path  buffer_path =
            msgstream() << "/generated_buffer/id_" << seed << "/" << buffer_name << "/" << msgstream::end();

    return create(id,std::make_shared<buffer_properties>(buffer_path,3U,(natural_32_bit)data.size(),
                                                         (natural_8_bit)sizeof(float_32_bit),false));
}

buffer_ptr  buffer::create(std::vector< std::array<float_32_bit,4> > const&  data, std::string const&  buffer_name)
{
    TMPROF_BLOCK();

    ASSUMPTION(4ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 4ULL * sizeof(float_32_bit));
    if (id == 0U)
        return buffer_ptr{};

    std::size_t  seed = 0ULL;
    for (auto const&  elem : data)
    {
        boost::hash_combine(seed,elem.at(0ULL));
        boost::hash_combine(seed,elem.at(1ULL));
        boost::hash_combine(seed,elem.at(2ULL));
        boost::hash_combine(seed,elem.at(3ULL));
    }
    boost::filesystem::path  buffer_path =
            msgstream() << "/generated_buffer/id_" << seed << "/" << buffer_name << "/" << msgstream::end();

    return create(id,std::make_shared<buffer_properties>(buffer_path,4U,(natural_32_bit)data.size(),
                                                         (natural_8_bit)sizeof(float_32_bit),false));
}

buffer_ptr  buffer::create(std::vector< natural_32_bit > const&  data, std::string const&  buffer_name)
{
    TMPROF_BLOCK();

    ASSUMPTION(data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 1ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};

    std::size_t  seed = 0ULL;
    for (auto const  elem : data)
        boost::hash_combine(seed,elem);
    boost::filesystem::path  buffer_path =
            msgstream() << "/generated_buffer/id_" << seed << "/" << buffer_name << "/" << msgstream::end();

    return create(id,std::make_shared<buffer_properties>(buffer_path,1U,(natural_32_bit)data.size(),
                                                         (natural_8_bit)sizeof(natural_32_bit),true));
}

buffer_ptr  buffer::create(std::vector< std::array<natural_32_bit,2> > const&  data, std::string const&  buffer_name)
{
    TMPROF_BLOCK();

    ASSUMPTION(2ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 2ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};

    std::size_t  seed = 0ULL;
    for (auto const&  elem : data)
    {
        boost::hash_combine(seed,elem.at(0ULL));
        boost::hash_combine(seed,elem.at(1ULL));
    }
    boost::filesystem::path  buffer_path =
            msgstream() << "/generated_buffer/id_" << seed << "/" << buffer_name << "/" << msgstream::end();


    return create(id,std::make_shared<buffer_properties>(buffer_path,2U,(natural_32_bit)data.size(),
                                                         (natural_8_bit)sizeof(natural_32_bit),true));
}

buffer_ptr  buffer::create(std::vector< std::array<natural_32_bit,3> > const&  data, std::string const&  buffer_name)
{
    TMPROF_BLOCK();

    ASSUMPTION(3ULL * data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());

    GLuint const  id =
            create_glbuffer(GL_ELEMENT_ARRAY_BUFFER,(GLvoid const*)&data.at(0),data.size() * 3ULL * sizeof(natural_32_bit));
    if (id == 0U)
        return buffer_ptr{};

    std::size_t  seed = 0ULL;
    for (auto const&  elem : data)
    {
        boost::hash_combine(seed,elem.at(0ULL));
        boost::hash_combine(seed,elem.at(1ULL));
        boost::hash_combine(seed,elem.at(2ULL));
    }
    boost::filesystem::path  buffer_path =
            msgstream() << "/generated_buffer/id_" << seed << "/" << buffer_name << "/" << msgstream::end();

    return create(id,std::make_shared<buffer_properties>(buffer_path,3U,(natural_32_bit)data.size(),
                                                         (natural_8_bit)sizeof(natural_32_bit),true));
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

    ASSUMPTION(m_id != 0U);
    ASSUMPTION(m_buffer_props.operator bool());
}

buffer_ptr  buffer::create(std::vector<natural_8_bit> const&  data,
                           buffer_properties_ptr const  buffer_props,
                           std::string& error_message)
{
    ASSUMPTION(buffer_props.operator bool());
    ASSUMPTION(error_message.empty());
    ASSUMPTION(data.size() <= (natural_64_bit)std::numeric_limits<natural_32_bit>::max());
    ASSUMPTION(data.size() == buffer_props->num_bytes_per_component() * buffer_props->num_components_per_primitive()
                                                                      * buffer_props->num_primitives());

    GLuint const  id =
            create_glbuffer(buffer_props->has_integral_components() ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER,
                            (GLvoid const*)data.data(),data.size());
    if (id == 0U)
    {
        error_message = "Construction of the buffer in function 'qtgl::create_glbuffer' has failed.";
        return buffer_ptr{};
    }
    return create(id,buffer_props);


}

buffer::~buffer()
{
    TMPROF_BLOCK();

    if (id() != 0U)
        glapi().glDeleteBuffers(1U,&m_id);
}


buffer_properties_ptr  load_buffer_file(boost::filesystem::path const&  buffer_file,
                                        std::vector<natural_8_bit>&  buffer_data,
                                        std::string&  error_message)
{
    ASSUMPTION(buffer_data.empty());
    ASSUMPTION(error_message.empty());

    if (!boost::filesystem::exists(buffer_file))
    {
        error_message = msgstream() << "The buffer file '" << buffer_file << "' does not exists.";
        return buffer_properties_ptr();
    }
    if (!boost::filesystem::is_regular_file(buffer_file))
    {
        error_message = msgstream() << "The buffer path '" << buffer_file << "' does not reference a regular file.";
        return buffer_properties_ptr();
    }

    if (boost::filesystem::file_size(buffer_file) < 4ULL)
    {
        error_message = msgstream() << "The passed file '" << buffer_file << "' is not a qtgl file (wrong size).";
        return buffer_properties_ptr();
    }

    std::ifstream  istr(buffer_file.string(),std::ios_base::binary);
    if (!istr.good())
    {
        error_message = msgstream() << "Cannot open the buffer file '" << buffer_file << "'.";
        return buffer_properties_ptr();
    }

    std::string  file_type;
    if (!detail::read_line(istr,file_type))
    {
        error_message = msgstream() << "The passed file '" << buffer_file << "' is not a qtgl file (cannot read its type string).";
        return buffer_properties_ptr();
    }

    if (file_type == "E2::qtgl/buffer/indices/triangles/text")
    {
        std::string  line;
        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read number of triangles in the file '" << buffer_file << "'.";
            return buffer_properties_ptr();
        }
        natural_32_bit const  num_triangles = std::stoul(line);
        if (num_triangles == 0U)
        {
            error_message = msgstream() << "The triangle index buffer file '" << buffer_file << "' contains zero triangles.";
            return buffer_properties_ptr();
        }
        for (natural_32_bit  i = 0U; i < num_triangles; ++i)
            for (natural_32_bit  j = 0U; j < 3U; ++j)
            {
                if (!detail::read_line(istr,line))
                {
                    error_message = msgstream() << "Cannot read the index no." << j << " of the triangle no." << i
                                                << " in the file '" << buffer_file << "'.";
                    return buffer_properties_ptr();
                }
                natural_32_bit const index = std::stoul(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&index),
                          reinterpret_cast<natural_8_bit const*>(&index) + sizeof(index),
                          std::back_inserter(buffer_data));
            }
        if (detail::read_line(istr,line))
        {
            error_message = msgstream() << "The file '" << buffer_file << "' contains more indices than "
                                        << 3U * num_triangles <<" indices.";
            return buffer_properties_ptr();
        }
        return buffer_properties::create(buffer_file,3U,num_triangles,sizeof(natural_32_bit),true);
    }
    else if (file_type == "E2::qtgl/buffer/vertices/3d/text")
    {
        std::string  line;
        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read number of vertices in the file '" << buffer_file << "'.";
            return buffer_properties_ptr();
        }
        natural_32_bit const  num_vertices = std::stoul(line);
        if (num_vertices == 0U)
        {
            error_message = msgstream() << "The vertex buffer file '" << buffer_file << "' contains zero vertices.";
            return buffer_properties_ptr();
        }
        float_32_bit  radius_squared = 0.0f;
        vector3  lo_corner{ 0.0f, 0.0f, 0.0f };
        vector3  hi_corner{ 0.0f, 0.0f, 0.0f };
        for (natural_32_bit i = 0U; i < num_vertices; ++i)
        {
            vector3  point;
            for (natural_32_bit j = 0U; j < 3U; ++j)
            {
                if (!detail::read_line(istr,line))
                {
                    error_message = msgstream() << "Cannot read the coordinate no." << j << " of the vertex no." << i
                                                << " in the file '" << buffer_file << "'.";
                    return buffer_properties_ptr();
                }
                float_32_bit const coord = std::stof(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&coord),
                          reinterpret_cast<natural_8_bit const*>(&coord) + sizeof(coord),
                          std::back_inserter(buffer_data));
                point(j) = coord;
                if (coord < lo_corner(j))
                    lo_corner(j) = coord;
                if (coord > hi_corner(j))
                    hi_corner(j) = coord;
            }
            float_32_bit const  len2 = length_squared(point);
            if (len2 > radius_squared)
                radius_squared = len2;
        }
        if (detail::read_line(istr,line))
        {
            error_message = msgstream() << "The file '" << buffer_file << "' contains more than "
                << 3U * num_vertices << " coordinates.";
            return buffer_properties_ptr();
        }
        return std::make_shared<vertex_buffer_properties const>(
                    buffer_file,3U,num_vertices,
                    spatial_boundary{ std::sqrtf(radius_squared),lo_corner,hi_corner }
                    );
    }
    else if (file_type == "E2::qtgl/buffer/diffuse_colours/text")
    {
        std::string  line;
        if (!detail::read_line(istr, line))
        {
            error_message = msgstream() << "Cannot read number of diffuse colours in the file '" << buffer_file << "'.";
            return buffer_properties_ptr();
        }
        natural_32_bit const  num_colours = std::stoul(line);
        if (num_colours == 0U)
        {
            error_message = msgstream() << "The diffuse colours buffer file '" << buffer_file << "' contains zero diffuse colours.";
            return buffer_properties_ptr();
        }
        for (natural_32_bit i = 0U; i < num_colours; ++i)
            for (natural_32_bit j = 0U; j < 4U; ++j)
            {
                if (!detail::read_line(istr, line))
                {
                    error_message = msgstream() << "Cannot read the colour component no." << j << " of the diffuse colour no." << i
                        << " in the file '" << buffer_file << "'.";
                    return buffer_properties_ptr();
                }
                float_32_bit const coord = std::stof(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&coord),
                    reinterpret_cast<natural_8_bit const*>(&coord) + sizeof(coord),
                    std::back_inserter(buffer_data));
            }
        if (detail::read_line(istr, line))
        {
            error_message = msgstream() << "The file '" << buffer_file << "' contains more than "
                << 4U * num_colours << " colour components.";
            return buffer_properties_ptr();
        }
        return buffer_properties::create(buffer_file, 4U, num_colours, sizeof(float_32_bit), false);
    }
    else if (file_type == "E2::qtgl/buffer/specular_colours/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/normals/3d/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type.find("E2::qtgl/buffer/texcoords/2d/") == 0ULL)
    {
        natural_8_bit  location = 0U;
        bool  found = true;
        for ( ; location < 10U; ++location)
            if (file_type == (msgstream() << "E2::qtgl/buffer/texcoords/2d/" << (int)location << "/text").get())
            {
                found = true;
                break;
            }
        if (!found)
        {
            error_message = msgstream() << "Unknown bind location in the file type string of the file '" << buffer_file << "'.";
            return buffer_properties_ptr();
        }

        std::string  line;
        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read number of texture coordinatres in the file '" << buffer_file << "'.";
            return buffer_properties_ptr();
        }
        natural_32_bit const  num_vertices = std::stoul(line);
        if (num_vertices == 0U)
        {
            error_message = msgstream() << "The texture coordinates buffer file '" << buffer_file << "' is empty.";
            return buffer_properties_ptr();
        }
        for (natural_32_bit i = 0U; i < num_vertices; ++i)
            for (natural_32_bit j = 0U; j < 2U; ++j)
            {
                if (!detail::read_line(istr,line))
                {
                    error_message = msgstream() << "Cannot read the coordinate no." << j << " of the texture vertex no." << i
                                                << " in the file '" << buffer_file << "'.";
                    return buffer_properties_ptr();
                }
                float_32_bit const coord = std::stof(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&coord),
                          reinterpret_cast<natural_8_bit const*>(&coord) + sizeof(coord),
                          std::back_inserter(buffer_data));
            }
        if (detail::read_line(istr,line))
        {
            error_message = msgstream() << "The file '" << buffer_file << "' contains more than "
                << 2U * num_vertices << " coordinates.";
            return buffer_properties_ptr();
        }
        return buffer_properties::create(buffer_file,2U,num_vertices,sizeof(float_32_bit),false);
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/1/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/2/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/3/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/4/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/5/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/6/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/7/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/8/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/buffer/texcoords/2d/9/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else
    {
        error_message = msgstream() << "The passed file '" << buffer_file
                                    << "' is a qtgl file of unknown type '" << file_type << "'.";
        return buffer_properties_ptr();
    }
}

void  send_buffer_load_request(boost::filesystem::path const&  buffer_file)
{
    detail::buffer_cache::instance().insert_load_request(buffer_file);
}

void  get_properties_of_cached_buffers(std::vector<buffer_properties_ptr>&  output)
{
    detail::buffer_cache::instance().cached(output);
}

void  get_properties_of_failed_buffers(std::vector< std::pair<buffer_properties_ptr,std::string> >&  output)
{
    detail::buffer_cache::instance().failed(output);
}


}

namespace qtgl {


buffers_binding_ptr  buffers_binding::create(
        boost::filesystem::path const&  index_buffer_path,
        std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings
        )
{
    ASSUMPTION(!index_buffer_path.empty());
    return buffers_binding_ptr(new buffers_binding(index_buffer_path,buffer_ptr(),0U,buffer_paths,direct_bindings));
}

buffers_binding_ptr  buffers_binding::create(
        buffer_ptr const  index_buffer,
        std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(index_buffer.operator bool() && index_buffer->properties()->has_integral_components());
    return buffers_binding_ptr(new buffers_binding(boost::filesystem::path(),index_buffer,
                                                   index_buffer->properties()->num_components_per_primitive(),
                                                   buffer_paths,direct_bindings));
}

buffers_binding_ptr  buffers_binding::create(
        natural_8_bit const  num_indices_per_primitive,
        std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(num_indices_per_primitive == 1U || num_indices_per_primitive == 2U || num_indices_per_primitive == 3U);
    return buffers_binding_ptr(new buffers_binding(boost::filesystem::path(),buffer_ptr(),
                                                   num_indices_per_primitive,
                                                   buffer_paths,direct_bindings));
}

buffers_binding::buffers_binding(
        boost::filesystem::path const&  index_buffer_path,
        buffer_ptr const  direct_index_buffer,
        natural_8_bit const  num_indices_per_primitive,
        std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path> const&  buffer_paths,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings)
    : m_index_buffer_path(index_buffer_path)
    , m_buffer_paths(buffer_paths)
    , m_direct_index_buffer(direct_index_buffer)
    , m_direct_bindings(direct_bindings)
    , m_num_indices_per_primitive(num_indices_per_primitive)
    , m_binding_data(new binding_data_type)
{
    ASSUMPTION(check_consistency(buffer_paths,direct_bindings));

    if (!m_index_buffer_path.empty())
        detail::buffer_cache::instance().insert_load_request(m_index_buffer_path);
    for (auto const&  location_path : buffer_paths)
        detail::buffer_cache::instance().insert_load_request(location_path.second);
}


vertex_buffer_properties_ptr  buffers_binding::find_vertex_buffer_properties() const
{
    auto const  path_it = m_buffer_paths.find(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION);
    if (path_it != m_buffer_paths.cend())
    {
        std::weak_ptr<buffer const> const  wptr = detail::buffer_cache::instance().find(path_it->second);
        buffer_ptr const  ptr = wptr.lock();
        if (ptr.operator bool())
            return std::dynamic_pointer_cast<vertex_buffer_properties const>(ptr->properties());
    }
    else
    {
        auto const  buf_it = m_direct_bindings.find(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION);
        if (buf_it != m_direct_bindings.cend())
            return std::dynamic_pointer_cast<vertex_buffer_properties const>(buf_it->second->properties());
    }
    return vertex_buffer_properties_ptr{};
}


bool  buffers_binding::make_current() const
{
    TMPROF_BLOCK();

    detail::buffer_cache::instance().process_pending_buffers();

    bool  is_ready = true;
    bool  need_num_primitives = false;
    natural_8_bit  num_indices_per_primitive = 0U;
    natural_32_bit  num_primitives = 0U;
    GLuint  index_buffer_id = 0U;
    std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr>   bindings;

    if (m_direct_index_buffer.operator bool())
    {
        num_indices_per_primitive = m_direct_index_buffer->properties()->num_components_per_primitive();
        num_primitives = m_direct_index_buffer->properties()->num_primitives();
        index_buffer_id = m_direct_index_buffer->id();
    }
    else if (!m_index_buffer_path.empty())
    {
        std::weak_ptr<buffer const> const  wptr = detail::buffer_cache::instance().find(m_index_buffer_path);
        buffer_ptr const  ptr = wptr.lock();
        if (ptr.operator bool())
        {
            num_indices_per_primitive = ptr->properties()->num_components_per_primitive();
            num_primitives = ptr->properties()->num_primitives();
            index_buffer_id = ptr->id();
        }
        else
        {
            detail::buffer_cache::instance().insert_load_request(m_index_buffer_path);
            is_ready = false;
        }
    }
    else
    {
        num_indices_per_primitive = m_num_indices_per_primitive;
        auto const  it = m_direct_bindings.find(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION);
        if (it != m_direct_bindings.cend())
            num_primitives = it->second->properties()->num_primitives();
        else
            need_num_primitives = true;
    }

    for (auto const&  location_path : m_buffer_paths)
    {
        std::weak_ptr<buffer const> const  wptr = detail::buffer_cache::instance().find(location_path.second);
        buffer_ptr const  ptr = wptr.lock();
        if (ptr.operator bool())
        {
            bindings.insert({location_path.first,ptr});
            if (need_num_primitives && location_path.first == vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION)
            {
                num_primitives = ptr->properties()->num_primitives();
                need_num_primitives = false;
            }
        }
        else
        {
            detail::buffer_cache::instance().insert_load_request(location_path.second);
            is_ready = false;
        }
    }

    if (is_ready)
    {
        INVARIANT(num_primitives > 0U);
        INVARIANT(need_num_primitives == false);
        INVARIANT(bindings.size() == m_buffer_paths.size());

        bool  recreate_id = false;
        if (id() == 0U)
            recreate_id = true;
        else
        {
            for (auto const&  location_buffer : bindings)
            {
                INVARIANT(m_binding_data->bindings().count(location_buffer.first) != 0ULL);
                if (location_buffer.second->id() != m_binding_data->bindings().at(location_buffer.first))
                {
                    recreate_id = true;
                    break;
                }
            }
        }
        if (recreate_id)
            is_ready = m_binding_data->reset(index_buffer_id,bindings,m_direct_bindings);

        if (is_ready)
        {
            INVARIANT(id() != 0U);

            glapi().glBindVertexArray(id());

            detail::current_draw::set_index_buffer_id(index_buffer_id);
            detail::current_draw::set_num_components_per_primitive(num_indices_per_primitive);
            detail::current_draw::set_num_primitives(num_primitives);
        }
    }

    detail::current_draw::set_are_buffers_ready(is_ready);
    return is_ready;
}


bool  buffers_binding::binding_data_type::reset(
        GLuint const  index_buffer_id,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  bindings,
        std::unordered_map<vertex_shader_input_buffer_binding_location,buffer_ptr> const&  direct_bindings)
{
    TMPROF_BLOCK();

    destroy_ID();
    m_id = create_vertex_arrays();
    if (m_id == 0U)
        return false;
    fill_vertex_arrays(bindings);
    fill_vertex_arrays(direct_bindings);
    m_index_buffer_id = index_buffer_id;
    for (auto const&  location_buffer : bindings)
        m_bindings.insert({location_buffer.first,location_buffer.second->id()});
    return true;
}

void  buffers_binding::binding_data_type::destroy_ID()
{
    TMPROF_BLOCK();

    if (m_id != 0U)
    {
        glapi().glDeleteVertexArrays(1U,&m_id);
        m_id = 0U;
        m_index_buffer_id = 0U;
        m_bindings.clear();
    }
}


}
