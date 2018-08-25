#include <qtgl/buffer.hpp>
#include <utility/read_line.hpp>
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


buffer_file_data::buffer_file_data(async::key_type const&  key, async::finalise_load_on_destroy_ptr)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  path = key.get_unique_id();

    if (!boost::filesystem::exists(path))
        throw std::runtime_error(msgstream() << "The buffer file '" << path << "' does not exists.");
    if (!boost::filesystem::is_regular_file(path))
        throw std::runtime_error(msgstream() << "The buffer path '" << path
                                             << "' does not reference a regular file.");

    std::vector<natural_8_bit> file_content(boost::filesystem::file_size(path));
    {
        TMPROF_BLOCK();

        std::ifstream  istr(path.string(),std::ios_base::binary);
        if (!istr.good())
            throw std::runtime_error(msgstream() << "Cannot open the passed image file: " << path);
        istr.read((char*)&file_content.at(0U), file_content.size());
        if (istr.bad())
            throw std::runtime_error(msgstream() << "Cannot read the passed image file: " << path);
    }

    per_line_buffer_reader  line_reader(file_content.data(), file_content.data() + file_content.size(), path.string());

    natural_8_bit  num_components = 0U;
    natural_32_bit  num_elements = 0U;
    bool  has_integer_components = false;
    bool  compute_boundary = false;
    {
        TMPROF_BLOCK();

        if (path.filename() == "indices.txt")
        {
            num_components = std::stoul(line_reader.get_next_line());
            num_elements = std::stoul(line_reader.get_next_line());
            has_integer_components = true;
            compute_boundary = false;
        }
        else if (path.filename() == "diffuse.txt" || path.filename() == "specular.txt" || path.filename() == "weights.txt")
        {
            num_components = std::stoul(line_reader.get_next_line());
            num_elements = std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = false;
        }
        else if (path.filename() == "vertices.txt")
        {
            num_components = 3U;
            num_elements = std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = true;
        }
        else if (path.filename() == "normals.txt")
        {
            num_components = 3U;
            num_elements = std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = false;
        }
        else if (path.filename().string().find("texcoords") == 0UL && path.extension() == ".txt")
        {
            num_components = 2U;
            num_elements = std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = false;
        }
        else
            throw std::runtime_error(msgstream() << "The passed file '" << path << "' is not of any known buffer type.");
    }

    std::vector<natural_8_bit>  buffer_data(
            (natural_64_bit)num_elements *
            (natural_64_bit)num_components *
            (has_integer_components ? sizeof(natural_32_bit) : sizeof(float_32_bit))
            );
    auto  buffer_data_cursor = buffer_data.begin();
    float_32_bit  radius_squared = 0.0f;
    vector3  lo_corner{ 0.0f, 0.0f, 0.0f };
    vector3  hi_corner{ 0.0f, 0.0f, 0.0f };
    {
        TMPROF_BLOCK();

        for (natural_32_bit  i = 0U; i < num_elements; ++i)
        {
            vector3  point;
            for (natural_8_bit  j = 0U; j < num_components; ++j)
            {
                std::string const  line = line_reader.get_next_line();
                if (has_integer_components)
                {
                    natural_32_bit const  value = std::stoul(line);
                    std::copy(reinterpret_cast<natural_8_bit const*>(&value),
                              reinterpret_cast<natural_8_bit const*>(&value) + sizeof(value),
                              buffer_data_cursor);
                    buffer_data_cursor += sizeof(value);
                }
                else
                {
                    float_32_bit const  value = std::stof(line);
                    std::copy(reinterpret_cast<natural_8_bit const*>(&value),
                              reinterpret_cast<natural_8_bit const*>(&value) + sizeof(value),
                              buffer_data_cursor);
                    buffer_data_cursor += sizeof(value);
                    if (compute_boundary)
                    {
                        point(j) = value;
                        if (value < lo_corner(j))
                            lo_corner(j) = value;
                        if (value > hi_corner(j))
                            hi_corner(j) = value;

                    }
                }
            }
            if (compute_boundary)
            {
                float_32_bit const  len2 = length_squared(point);
                if (len2 > radius_squared)
                    radius_squared = len2;
            }
        }
    }

    spatial_boundary const  boundary{ std::sqrtf(radius_squared), lo_corner, hi_corner };

    initialise(
            0U,
            num_components,
            num_elements,
            has_integer_components ? sizeof(natural_32_bit) : sizeof(float_32_bit),
            has_integer_components,
            buffer_data.data(),
            buffer_data.data() + buffer_data.size(),
            compute_boundary ? &boundary : nullptr
            );
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
    INVARIANT(glapi().glGetError() == 0U);
}


void  buffer_file_data::destroy_gl_buffer()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glapi().glDeleteBuffers(1U, &m_id);
    INVARIANT(glapi().glGetError() == 0U);
}



}}

namespace qtgl { namespace detail {


static void  load_buffers_map(
        std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path> const&  paths,
        buffers_binding_data::buffers_map_type&  buffers,
        std::function<void()> const&  initialiser,
        async::finalise_load_on_destroy_ptr  finaliser
        )
{
    TMPROF_BLOCK();

    struct local
    {
        static void  on_buffer_loaded(
                std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path> const&  paths,
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
            std::bind(&local::on_buffer_loaded, paths, it, std::ref(buffers), initialiser, finaliser)
            );
}


buffers_binding_data::buffers_binding_data(
        async::finalise_load_on_destroy_ptr  finaliser,
        boost::filesystem::path const&  index_buffer_path,
        std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path> const&  paths
        )
    : m_id(0U)
    , m_index_buffer()
    , m_num_indices_per_primitive(0U)
    , m_buffers()
    , m_ready(false)
{
    m_index_buffer.insert_load_request(
        index_buffer_path.string(),
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
        std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path> const&  paths
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
    ASSUMPTION(m_buffers.count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION) == 1UL);
    ASSUMPTION(m_buffers.at(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION).has_boundary());
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
    INVARIANT(glapi().glGetError() == 0U);
}


void  buffers_binding_data::destroy_gl_binding()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glapi().glDeleteVertexArrays(1U, &m_id);
    INVARIANT(glapi().glGetError() == 0U);
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
        INVARIANT(glapi().glGetError() == 0U);

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
            INVARIANT(glapi().glGetError() == 0U);
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
    INVARIANT(glapi().glGetError() == 0U);

    if (get_index_buffer().empty())
    {
        detail::current_draw::set_index_buffer_id(0U);
        detail::current_draw::set_num_components_per_primitive(get_num_indices_per_primitive());
        detail::current_draw::set_num_primitives(
            get_buffers().at(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION).num_primitives()
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
