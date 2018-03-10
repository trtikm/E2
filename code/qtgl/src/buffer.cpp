#include <qtgl/buffer.hpp>
#include <qtgl/detail/read_line.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <functional>
#include <stdexcept>

namespace qtgl { namespace detail { namespace current_draw {


void  set_are_buffers_ready(bool const  are_buffers_ready);
void  set_index_buffer_id(GLuint const id);
void  set_num_components_per_primitive(natural_8_bit const  num_components_per_primitive);
void  set_num_primitives(natural_32_bit const  num_primitives);


}}}

namespace qtgl { namespace detail {


buffer_file_data::buffer_file_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr)
{
    TMPROF_BLOCK();

    if (!boost::filesystem::exists(path))
        throw std::runtime_error(msgstream() << "The buffer file '" << path << "' does not exists.");
    if (!boost::filesystem::is_regular_file(path))
        throw std::runtime_error(msgstream() << "The buffer path '" << path
                                             << "' does not reference a regular file.");

    if (boost::filesystem::file_size(path) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << path
                                             << "' is not a qtgl file (wrong size).");

    std::ifstream  istr(path.string(),std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the buffer file '" << path << "'.");

    std::string  file_type;
    if (!detail::read_line(istr,file_type))
        throw std::runtime_error(msgstream() << "The passed file '" << path
                                             << "' is not a qtgl file (cannot read its type string).");

    std::vector<natural_8_bit>  buffer_data;

    if (file_type == "E2::qtgl/buffer/indices/triangles/text")
    {
        std::string  line;
        if (!detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read number of triangles in the file '" << path
                                                 << "'.");
        natural_32_bit const  num_triangles = std::stoul(line);
        if (num_triangles == 0U)
            throw std::runtime_error(msgstream() << "The triangle index buffer file '" << path
                                                 << "' contains zero triangles.");
        for (natural_32_bit  i = 0U; i < num_triangles; ++i)
            for (natural_32_bit  j = 0U; j < 3U; ++j)
            {
                if (!detail::read_line(istr,line))
                    throw std::runtime_error(
                        msgstream() << "Cannot read the index no." << j << " of the triangle no." << i
                                    << " in the file '" << path << "'.");
                natural_32_bit const index = std::stoul(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&index),
                          reinterpret_cast<natural_8_bit const*>(&index) + sizeof(index),
                          std::back_inserter(buffer_data));
            }
        if (detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "The file '" << path << "' contains more indices than "
                                                 << 3U * num_triangles <<" indices.");

        initialise(
                0U,
                3U, num_triangles, sizeof(natural_32_bit), true,
                buffer_data.data(), buffer_data.data() + buffer_data.size(),
                nullptr
                );
    }
    else if (file_type == "E2::qtgl/buffer/vertices/3d/text")
    {
        std::string  line;
        if (!detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read number of vertices in the file '" << path
                                                 << "'.");
        natural_32_bit const  num_vertices = std::stoul(line);
        if (num_vertices == 0U)
            throw std::runtime_error(msgstream() << "The vertex buffer file '" << path
                                                 << "' contains zero vertices.");
        float_32_bit  radius_squared = 0.0f;
        vector3  lo_corner{ 0.0f, 0.0f, 0.0f };
        vector3  hi_corner{ 0.0f, 0.0f, 0.0f };
        for (natural_32_bit i = 0U; i < num_vertices; ++i)
        {
            vector3  point;
            for (natural_32_bit j = 0U; j < 3U; ++j)
            {
                if (!detail::read_line(istr,line))
                    throw std::runtime_error(msgstream() << "Cannot read the coordinate no." << j
                                                         << " of the vertex no." << i
                                                         << " in the file '" << path << "'.");
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
            throw std::runtime_error(msgstream() << "The file '" << path << "' contains more than "
                                                 << 3U * num_vertices << " coordinates.");
        spatial_boundary const  boundary{ std::sqrtf(radius_squared),lo_corner,hi_corner };

        initialise(
                0U,
                3U, num_vertices, sizeof(float_32_bit), false,
                buffer_data.data(), buffer_data.data() + buffer_data.size(),
                &boundary
                );
    }
    else if (file_type == "E2::qtgl/buffer/diffuse_colours/text")
    {
        std::string  line;
        if (!detail::read_line(istr, line))
            throw std::runtime_error(msgstream() << "Cannot read number of diffuse colours in the file '"
                                                 << path << "'.");
        natural_32_bit const  num_colours = std::stoul(line);
        if (num_colours == 0U)
            throw std::runtime_error(msgstream() << "The diffuse colours buffer file '" << path
                                                 << "' contains zero diffuse colours.");
        for (natural_32_bit i = 0U; i < num_colours; ++i)
            for (natural_32_bit j = 0U; j < 4U; ++j)
            {
                if (!detail::read_line(istr, line))
                    throw std::runtime_error(msgstream() << "Cannot read the colour component no." << j
                                                         << " of the diffuse colour no." << i
                                                         << " in the file '" << path << "'.");
                float_32_bit const coord = std::stof(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&coord),
                    reinterpret_cast<natural_8_bit const*>(&coord) + sizeof(coord),
                    std::back_inserter(buffer_data));
            }
        if (detail::read_line(istr, line))
            throw std::runtime_error(msgstream() << "The file '" << path << "' contains more than "
                                                 << 4U * num_colours << " colour components.");

        initialise(
                0U,
                4U, num_colours, sizeof(float_32_bit), false,
                buffer_data.data(), buffer_data.data() + buffer_data.size(),
                nullptr
                );
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
            throw std::runtime_error(msgstream() << "Unknown bind location in the file type string of the file '" 
                                                 << path << "'.");

        std::string  line;
        if (!detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read number of texture coordinatres in the file '"
                                                 << path << "'.");
        natural_32_bit const  num_vertices = std::stoul(line);
        if (num_vertices == 0U)
            throw std::runtime_error(msgstream() << "The texture coordinates buffer file '" << path
                                                 << "' is empty.");
        for (natural_32_bit i = 0U; i < num_vertices; ++i)
            for (natural_32_bit j = 0U; j < 2U; ++j)
            {
                if (!detail::read_line(istr,line))
                    throw std::runtime_error(msgstream() << "Cannot read the coordinate no." << j
                                                         << " of the texture vertex no." << i
                                                         << " in the file '" << path << "'.");
                float_32_bit const coord = std::stof(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&coord),
                          reinterpret_cast<natural_8_bit const*>(&coord) + sizeof(coord),
                          std::back_inserter(buffer_data));
            }
        if (detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "The file '" << path << "' contains more than "
                                                 << 2U * num_vertices << " coordinates.");

        initialise(
                0U,
                2U, num_vertices, sizeof(float_32_bit), false,
                buffer_data.data(), buffer_data.data() + buffer_data.size(),
                nullptr
                );
    }
    else if (file_type == "E2::qtgl/buffer/indices_of_matrices/2/text" ||
             file_type == "E2::qtgl/buffer/indices_of_matrices/3/text" ||
             file_type == "E2::qtgl/buffer/indices_of_matrices/4/text" )
    {
        natural_32_bit const  num_matrices_per_vertex =
                file_type == "E2::qtgl/buffer/indices_of_matrices/2/text" ? 2U :
                file_type == "E2::qtgl/buffer/indices_of_matrices/3/text" ? 3U :
                                                                            4U ;

        std::string  line;
        if (!detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read indices of matrices in the file '" << path
                                                 << "'.");
        natural_32_bit const  num_elements = std::stoul(line);
        if (num_elements == 0U)
            throw std::runtime_error(msgstream() << "The buffer file '" << path
                                                 << "' contains zero indices of matrices.");
        for (natural_32_bit  i = 0U; i < num_elements; ++i)
            for (natural_32_bit  j = 0U; j < num_matrices_per_vertex; ++j)
            {
                if (!detail::read_line(istr,line))
                    throw std::runtime_error(msgstream() << "Cannot read the index no." << j
                                                         << " of the element no." << i
                                                         << " in the file '" << path << "'.");
                natural_32_bit const index = std::stoul(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&index),
                          reinterpret_cast<natural_8_bit const*>(&index) + sizeof(index),
                          std::back_inserter(buffer_data));
            }
        if (detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "The file '" << path << "' contains more indices than "
                                                 << num_matrices_per_vertex * num_elements <<" indices.");

        initialise(
                0U,
                num_matrices_per_vertex, num_elements, sizeof(natural_32_bit), true,
                buffer_data.data(), buffer_data.data() + buffer_data.size(),
                nullptr
                );
    }
    else if (file_type == "E2::qtgl/buffer/weights_of_matrices/2/text" ||
             file_type == "E2::qtgl/buffer/weights_of_matrices/3/text" ||
             file_type == "E2::qtgl/buffer/weights_of_matrices/4/text" )
    {
        natural_32_bit const  num_matrices_per_vertex =
                file_type == "E2::qtgl/buffer/weights_of_matrices/2/text" ? 2U :
                file_type == "E2::qtgl/buffer/weights_of_matrices/3/text" ? 3U :
                                                                            4U ;

        std::string  line;
        if (!detail::read_line(istr, line))
            throw std::runtime_error(msgstream() << "Cannot read number of weights in the file '" << path << "'.");
        natural_32_bit const  num_weights = std::stoul(line);
        if (num_weights == 0U)
            throw std::runtime_error(msgstream() << "The buffer file '" << path
                                                 << "' contains zero weights of matrices.");
        for (natural_32_bit i = 0U; i < num_weights; ++i)
            for (natural_32_bit j = 0U; j < num_matrices_per_vertex; ++j)
            {
                if (!detail::read_line(istr, line))
                    throw std::runtime_error(msgstream() << "Cannot read the weight no." << i+j
                                                         << " in the file '" << path << "'.");
                float_32_bit const coord = std::stof(line);
                std::copy(reinterpret_cast<natural_8_bit const*>(&coord),
                    reinterpret_cast<natural_8_bit const*>(&coord) + sizeof(coord),
                    std::back_inserter(buffer_data));
            }
        if (detail::read_line(istr, line))
            throw std::runtime_error(msgstream() << "The file '" << path << "' contains more than "
                                                 << 1U * num_weights << " weights of matrices.");

        initialise(
                0U,
                num_matrices_per_vertex, num_weights, sizeof(float_32_bit), false,
                buffer_data.data(), buffer_data.data() + buffer_data.size(),
                nullptr
                );
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
        throw std::runtime_error(msgstream() << "The passed file '" << path
                                             << "' is a qtgl file of unknown type '" << file_type << "'.");
    }
}


buffer_file_data::buffer_file_data(
        async::finalise_load_on_destroy_ptr,
        GLuint const  id, 
        natural_8_bit const  num_components_per_primitive,
        natural_32_bit const  num_primitives,
        natural_8_bit const  num_bytes_per_component,
        bool const   has_integral_components,
        natural_8_bit const* const  data_begin,
        natural_8_bit const* const  data_end,
        spatial_boundary const* const  boundary
        )
{
    initialise(
        id,
        num_components_per_primitive, num_primitives, num_bytes_per_component, has_integral_components,
        data_begin, data_end,
        boundary
        );
}


buffer_file_data::buffer_file_data(
        async::finalise_load_on_destroy_ptr,
        std::vector< std::array<float_32_bit,2> > const&  data
        )
{
    initialise(
        0U,
        2U, (natural_32_bit)data.size(), (natural_8_bit)sizeof(float_32_bit), false,
        (natural_8_bit const*)data.data(), (natural_8_bit const*)data.data() + data.size() * sizeof(data.at(0)),
        nullptr
        );
}


buffer_file_data::buffer_file_data(
        async::finalise_load_on_destroy_ptr,
        std::vector< std::array<float_32_bit,3> > const&  data,
        bool const  do_compute_boundary
        )
{
    float_32_bit  radius_squared = 0.0f;
    vector3  lo_corner{ 0.0f, 0.0f, 0.0f };
    vector3  hi_corner{ 0.0f, 0.0f, 0.0f };
    if (do_compute_boundary)
        for (auto const& point : data)
        {
            for (natural_32_bit j = 0U; j < 3U; ++j)
            {
                if (point.at(j) < lo_corner(j))
                    lo_corner(j) = point.at(j);
                if (point.at(j) > hi_corner(j))
                    hi_corner(j) = point.at(j);
            }
            float_32_bit const  len2 = point.at(0U) * point.at(0U) +
                                       point.at(1U) * point.at(1U) +
                                       point.at(2U) * point.at(2U);
            if (len2 > radius_squared)
                radius_squared = len2;
        }
    spatial_boundary const  boundary{ std::sqrtf(radius_squared),lo_corner,hi_corner };
    initialise(
        0U,
        3U, (natural_32_bit)data.size(), sizeof(float_32_bit), false,
        (natural_8_bit const*)data.data(), (natural_8_bit const*)data.data() + data.size() * sizeof(data.at(0)),
        do_compute_boundary ? &boundary : nullptr
        );
}


buffer_file_data::buffer_file_data(
        async::finalise_load_on_destroy_ptr,
        std::vector< std::array<float_32_bit,4> > const&  data
        )
{
    initialise(
        0U,
        4U, (natural_32_bit)data.size(), (natural_8_bit)sizeof(float_32_bit), false,
        (natural_8_bit const*)data.data(), (natural_8_bit const*)data.data() + data.size() * sizeof(data.at(0)),
        nullptr
        );
}


buffer_file_data::buffer_file_data(
        async::finalise_load_on_destroy_ptr,
        std::vector< natural_32_bit > const&  data
        )
{
    initialise(
        0U,
        1U, (natural_32_bit)data.size(), (natural_8_bit)sizeof(natural_32_bit), true,
        (natural_8_bit const*)data.data(), (natural_8_bit const*)data.data() + data.size() * sizeof(data.at(0)),
        nullptr
        );
}


buffer_file_data::buffer_file_data(
        async::finalise_load_on_destroy_ptr,
        std::vector< std::array<natural_32_bit,2> > const&  data
        )
{
    initialise(
        0U,
        2U, (natural_32_bit)data.size(), (natural_8_bit)sizeof(natural_32_bit), true,
        (natural_8_bit const*)data.data(), (natural_8_bit const*)data.data() + data.size() * sizeof(data.at(0)),
        nullptr
        );
}


buffer_file_data::buffer_file_data(
        async::finalise_load_on_destroy_ptr,
        std::vector< std::array<natural_32_bit,3> > const&  data
        )
{
    initialise(
        0U,
        3U, (natural_32_bit)data.size(), (natural_8_bit)sizeof(natural_32_bit), true,
        (natural_8_bit const*)data.data(), (natural_8_bit const*)data.data() + data.size() * sizeof(data.at(0)),
        nullptr
        );
}


buffer_file_data::~buffer_file_data()
{
    TMPROF_BLOCK();

    destroy_gl_buffer();
}



void  buffer_file_data::initialise(
        GLuint const  id,
        natural_8_bit const  num_components_per_primitive,
        natural_32_bit const  num_primitives,
        natural_8_bit const  num_bytes_per_component,
        bool const   has_integral_components,
        natural_8_bit const* const  data_begin,
        natural_8_bit const* const  data_end,
        spatial_boundary const* const  boundary
        )
{
    ASSUMPTION(
        (data_begin == nullptr && data_end == nullptr && id != 0U) ||
        (data_begin != nullptr && data_end != nullptr && data_begin < data_end)
        );

    m_id = id;
    m_num_primitives = num_primitives;
    m_num_components_per_primitive = num_components_per_primitive;
    m_num_bytes_per_component = num_bytes_per_component;
    m_has_integral_components = has_integral_components;
    m_data_ptr = data_begin == nullptr ? nullptr :
                                         std::unique_ptr< std::vector<natural_8_bit> >(
                                                new std::vector<natural_8_bit>(data_begin, data_end)
                                                );
    m_boundary = boundary == nullptr ? nullptr :
                                       std::unique_ptr<spatial_boundary>(new spatial_boundary(*boundary));

    ASSUMPTION(m_num_components_per_primitive != 0U);
    ASSUMPTION(m_num_primitives > 0U);
    ASSUMPTION((m_has_integral_components && m_num_bytes_per_component == (natural_8_bit)sizeof(natural_32_bit)) ||
               (!m_has_integral_components && m_num_bytes_per_component == sizeof(float_32_bit)));
    ASSUMPTION(m_data_ptr->size() == m_num_bytes_per_component * m_num_components_per_primitive * m_num_primitives);
}


void  buffer_file_data::create_gl_buffer()
{
    if (id() != 0U)
        return;

    TMPROF_BLOCK();

    glapi().glGenBuffers(1U, &m_id);
    ASSUMPTION(m_id != 0U);

    GLenum const  target = has_integral_components() ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
    glapi().glBindBuffer(target, m_id);
    glapi().glBufferData(
            target,
            (GLsizeiptr)m_data_ptr->size(),
            (GLvoid const*)m_data_ptr->data(),
            GL_STATIC_DRAW
            );
}


void  buffer_file_data::destroy_gl_buffer()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glapi().glDeleteBuffers(1U, &m_id);
}



}}

namespace qtgl { namespace detail {


static void  load_buffers_map(
        std::unordered_map<vertex_shader_input_buffer_binding_location, boost::filesystem::path> const&  paths,
        buffers_binding_data::buffers_map_type&  buffers,
        std::function<void()> const&  initialiser,
        async::finalise_load_on_destroy_ptr  finaliser
        )
{
    TMPROF_BLOCK();

    struct local
    {
        static void  on_buffer_loaded(
                std::unordered_map<vertex_shader_input_buffer_binding_location, boost::filesystem::path> const&  paths,
                buffers_binding_data::buffers_map_type::iterator  it,
                buffers_binding_data::buffers_map_type&  buffers,
                std::function<void()> const&  initialiser,
                async::finalise_load_on_destroy_ptr  finaliser
                )
        {
            ASSUMPTION(it != buffers.end());

            if (it->second.get_load_state() != async::LOAD_STATE::FINISHED_SUCCESSFULLY)
            {
                // Oh no! We failed to load the buffer at position 'it'!
                // So, let's also fail the load of the whole 'buffers_binding_data' resource.

                finaliser->force_finalisation_as_failure(
                        "Load of buffer '" + paths.at(it->first).string() + "' has FAILED!"
                        );
                return;
            }

            // Let's load the next buffer (if any remains)

            ++it;
            if (it == buffers.end())
            {
                // All buffers are loaded! So, let's initialise the 'buffers_binding_data' structure.
                initialiser();
                return;
            }
    
            // Let's load the next buffer.

            it->second.insert_load_request(
                    paths.at(it->first).string(),
                    1UL,
                    std::bind(&local::on_buffer_loaded, paths, it, std::ref(buffers), initialiser, finaliser)
                    );
        }
    };

    ASSUMPTION(!paths.empty());
    ASSUMPTION(buffers.empty());
    for (auto const& elem : paths)
        buffers.insert({ elem.first, buffer() });

    // We load individual buffers one by one, starting from the first one in the map (at cbegin()).
    // NOTE: All remaining buffers are loaded in function 'local::on_buffer_loaded'.

    auto const  it = buffers.begin();

    it->second.insert_load_request(
            paths.at(it->first).string(),
            1UL,
            std::bind(&local::on_buffer_loaded, paths, it, std::ref(buffers), initialiser, finaliser)
            );
}


buffers_binding_data::buffers_binding_data(
        async::finalise_load_on_destroy_ptr  finaliser,
        boost::filesystem::path const&  index_buffer_path,
        std::unordered_map<vertex_shader_input_buffer_binding_location, boost::filesystem::path> const&  paths
        )
    : m_id(0U)
    , m_index_buffer()
    , m_num_indices_per_primitive(0U)
    , m_buffers()
    , m_ready(false)
{
    m_index_buffer.insert_load_request(
        index_buffer_path.string(),
        1UL,
        std::bind(
            &load_buffers_map,
            paths,
            std::ref(m_buffers),
            [this]() { initialise( 0U, m_index_buffer, 0U, m_buffers); },
            finaliser
            )
        );
}


buffers_binding_data::buffers_binding_data(
        async::finalise_load_on_destroy_ptr  finaliser,
        natural_8_bit const  num_indices_per_primitive, // 1 (points), 2 (lines), or 3 (triangles)
        std::unordered_map<vertex_shader_input_buffer_binding_location, boost::filesystem::path> const&  paths
        )
    : m_id(0U)
    , m_index_buffer()
    , m_num_indices_per_primitive(0U)
    , m_buffers()
    , m_ready(false)
{
    load_buffers_map(
        paths,
        m_buffers,
        [this, num_indices_per_primitive]() { initialise(0U, buffer(), num_indices_per_primitive, m_buffers); },
        finaliser
        );
}


buffers_binding_data::~buffers_binding_data()
{
    TMPROF_BLOCK();

    destroy_gl_binding();
}


void  buffers_binding_data::initialise(
        GLuint const  id,
        buffer const  index_buffer,
        natural_8_bit const  num_indices_per_primitive, // 1 (points), 2 (lines), or 3 (triangles)
        buffers_map_type const&  buffers
        )
{
    TMPROF_BLOCK();

    m_id = id;
    m_index_buffer = index_buffer;
    m_num_indices_per_primitive = num_indices_per_primitive;
    m_buffers = buffers;
    m_ready = false;

    ASSUMPTION(m_num_indices_per_primitive < 4);
    ASSUMPTION(
        (m_index_buffer.empty() && m_num_indices_per_primitive != 0U) ||
        (!m_index_buffer.empty() && m_num_indices_per_primitive == 0U)
        );
    ASSUMPTION(m_buffers.size() <= (natural_64_bit)GL_MAX_VERTEX_ATTRIBS);
    ASSUMPTION(m_buffers.count(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION) == 1UL);
    ASSUMPTION(m_buffers.at(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION).has_boundary());
    ASSUMPTION(
        [this]() -> bool {
                for (auto const& elem : m_buffers)
                {
                    if (!elem.second.loaded_successfully())
                        return false;
                    if (elem.second.num_primitives() != m_buffers.cbegin()->second.num_primitives())
                        return false;
                }
                return true;
            }()
        );
}


void  buffers_binding_data::create_gl_binding()
{
    if (id() != 0U)
        return;

    TMPROF_BLOCK();

    glapi().glGenVertexArrays(1U, &m_id);
    ASSUMPTION(m_id != 0U);
}


void  buffers_binding_data::destroy_gl_binding()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glapi().glDeleteVertexArrays(1U, &m_id);
}


}}

namespace qtgl {


bool  buffers_binding::ready() const
{
    TMPROF_BLOCK();

    if (!loaded_successfully())
        return false;
    if (!resource().ready())
    {
        detail::current_draw::set_are_buffers_ready(false);
        if (!get_index_buffer().empty() && !get_index_buffer().loaded_successfully())
            return false;
        for (auto const& location_and_buffer : get_buffers())
            if (!location_and_buffer.second.loaded_successfully())
                return false;

        buffers_binding* const  mutable_this = const_cast<buffers_binding*>(this);

        mutable_this->create_gl_binding();
        glapi().glBindVertexArray(id());

        if (get_index_buffer().loaded_successfully())
            get_index_buffer().create_gl_buffer();

        for (auto const& location_and_buffer : get_buffers())
        {
            location_and_buffer.second.create_gl_buffer();

            glapi().glBindBuffer(GL_ARRAY_BUFFER, location_and_buffer.second.id());
            glapi().glEnableVertexAttribArray(value(location_and_buffer.first));
            glapi().glVertexAttribPointer(
                        value(location_and_buffer.first),
                        location_and_buffer.second.num_components_per_primitive(),
                        GL_FLOAT,
                        GL_FALSE,
                        0U,
                        nullptr
                        );
        }

        mutable_this->set_ready();
    }

    return true;
}


bool  buffers_binding::make_current() const
{
    TMPROF_BLOCK();

    if (!ready())
        return false;

    glapi().glBindVertexArray(id());

    if (get_index_buffer().empty())
    {
        detail::current_draw::set_index_buffer_id(0U);
        detail::current_draw::set_num_components_per_primitive(get_num_indices_per_primitive());
        detail::current_draw::set_num_primitives(
            get_buffers().at(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION).num_primitives()
            );
    }
    else
    {
        detail::current_draw::set_index_buffer_id(get_index_buffer().id());
        detail::current_draw::set_num_components_per_primitive(get_index_buffer().num_components_per_primitive());
        detail::current_draw::set_num_primitives(get_index_buffer().num_primitives());
    }
    detail::current_draw::set_are_buffers_ready(true);

    return true;
}


bool  make_current(buffers_binding const&  binding)
{
    return binding.make_current();
}


}
