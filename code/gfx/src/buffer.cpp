#include <gfx/buffer.hpp>
#include <utility/read_line.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <boost/functional/hash.hpp>
#include <filesystem>
#include <limits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <functional>
#include <stdexcept>

namespace gfx { namespace detail { namespace current_draw {


void  set_are_buffers_ready(bool const  are_buffers_ready);
void  set_index_buffer_id(GLuint const id);
void  set_num_components_per_primitive(natural_8_bit const  num_components_per_primitive);
void  set_num_primitives(natural_32_bit const  num_primitives);


}}}

namespace gfx { namespace detail {


buffer_file_data::buffer_file_data(async::finalise_load_on_destroy_ptr const  finaliser)
{
    TMPROF_BLOCK();

    std::filesystem::path const  path = finaliser->get_key().get_unique_id();

    if (!std::filesystem::exists(path))
        throw std::runtime_error(msgstream() << "The buffer file '" << path << "' does not exists.");
    if (!std::filesystem::is_regular_file(path))
        throw std::runtime_error(msgstream() << "The buffer path '" << path
                                             << "' does not reference a regular file.");

    std::vector<natural_8_bit> file_content(std::filesystem::file_size(path) + 1UL);
    {
        TMPROF_BLOCK();

        std::ifstream  istr(path.string(),std::ios_base::binary);
        if (!istr.good())
            throw std::runtime_error(msgstream() << "Cannot open the passed image file: " << path);
        istr.read((char*)&file_content.at(0U), file_content.size());
        if (istr.bad())
            throw std::runtime_error(msgstream() << "Cannot read the passed image file: " << path);
        file_content.back() = 0U;
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
            num_components = (natural_8_bit)std::stoul(line_reader.get_next_line());
            num_elements = (natural_32_bit)std::stoul(line_reader.get_next_line());
            has_integer_components = true;
            compute_boundary = false;
        }
        else if (path.filename() == "diffuse.txt" || path.filename() == "specular.txt" || path.filename() == "weights.txt")
        {
            num_components = (natural_8_bit)std::stoul(line_reader.get_next_line());
            num_elements = (natural_32_bit)std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = false;
        }
        else if (path.filename() == "vertices.txt")
        {
            num_components = 3U;
            num_elements = (natural_32_bit)std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = true;
        }
        else if (path.filename() == "normals.txt")
        {
            num_components = 3U;
            num_elements = (natural_32_bit)std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = false;
        }
        else if (path.filename() == "tangents.txt")
        {
            num_components = 3U;
            num_elements = (natural_32_bit)std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = false;
        }
        else if (path.filename() == "bitangents.txt")
        {
            num_components = 3U;
            num_elements = (natural_32_bit)std::stoul(line_reader.get_next_line());
            has_integer_components = false;
            compute_boundary = false;
        }
        else if (path.filename().string().find("texcoords") == 0UL && path.extension() == ".txt")
        {
            num_components = 2U;
            num_elements = (natural_32_bit)std::stoul(line_reader.get_next_line());
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
    auto  buffer_data_cursor = buffer_data.data();
    vector3  lo_corner =
        !compute_boundary ? vector3_zero() : vector3(std::numeric_limits<float_32_bit>::max(),
                                                     std::numeric_limits<float_32_bit>::max(),
                                                     std::numeric_limits<float_32_bit>::max());
    vector3  hi_corner =
        !compute_boundary ? vector3_zero() : vector3(std::numeric_limits<float_32_bit>::min(),
                                                     std::numeric_limits<float_32_bit>::min(),
                                                     std::numeric_limits<float_32_bit>::min());
    {
        TMPROF_BLOCK();

        if (has_integer_components)
        {
            for (natural_32_bit i = 0U; i < num_elements; ++i)
                for (natural_8_bit j = 0U; j < num_components; ++j)
                {
                    line_reader.read_next_line_as_primitive_type<natural_32_bit>(buffer_data_cursor);
                    buffer_data_cursor += sizeof(natural_32_bit);
                }
        }
        else
        {
            for (natural_32_bit  i = 0U; i < num_elements; ++i)
            {
                vector3  point;
                for (natural_8_bit  j = 0U; j < num_components; ++j)
                {
                    line_reader.read_next_line_as_primitive_type<float_32_bit>(buffer_data_cursor);
                    if (compute_boundary)
                    {
                        point(j) = *(float_32_bit const*)buffer_data_cursor;
                        if (point(j) < lo_corner(j))
                            lo_corner(j) = point(j);
                        if (point(j) > hi_corner(j))
                            hi_corner(j) = point(j);

                    }
                    buffer_data_cursor += sizeof(float_32_bit);
                }
            }
        }
    }

    spatial_boundary const  boundary{ length(hi_corner - lo_corner), lo_corner, hi_corner };

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
        GLuint const  id, 
        natural_8_bit const  num_components_per_primitive,
        natural_32_bit const  num_primitives,
        natural_8_bit const  num_bytes_per_component,
        bool const  has_integral_components,
        std::unique_ptr<std::vector<natural_8_bit> >&  data_ptr,
        spatial_boundary const* const  boundary
        )
{
    initialise(
        id,
        num_components_per_primitive, num_primitives, num_bytes_per_component, has_integral_components,
        data_ptr,
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
    vector3  lo_corner;
    vector3  hi_corner;
    if (do_compute_boundary)
    {
        lo_corner = vector3(std::numeric_limits<float_32_bit>::max(), std::numeric_limits<float_32_bit>::max(), std::numeric_limits<float_32_bit>::max());
        hi_corner = vector3(std::numeric_limits<float_32_bit>::min(), std::numeric_limits<float_32_bit>::min(), std::numeric_limits<float_32_bit>::min());
        for (auto const& point : data)
        {
            for (natural_32_bit j = 0U; j < 3U; ++j)
            {
                if (point.at(j) < lo_corner(j))
                    lo_corner(j) = point.at(j);
                if (point.at(j) > hi_corner(j))
                    hi_corner(j) = point.at(j);
            }
        }
    }
    else
    {
        lo_corner = vector3_zero();
        hi_corner = vector3_zero();
    }
    spatial_boundary const  boundary{ length(hi_corner - lo_corner), lo_corner, hi_corner };
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

    std::unique_ptr< std::vector<natural_8_bit> >  data_ptr;
    data_ptr = data_begin == nullptr ? nullptr :
                                       std::unique_ptr< std::vector<natural_8_bit> >(
                                                new std::vector<natural_8_bit>(data_begin, data_end)
                                                );
    initialise(
        id,
        num_components_per_primitive,
        num_primitives,
        num_bytes_per_component,
        has_integral_components,
        data_ptr,
        boundary
        );
}


void  buffer_file_data::initialise(
        GLuint const  id,
        natural_8_bit const  num_components_per_primitive,
        natural_32_bit const  num_primitives,
        natural_8_bit const  num_bytes_per_component,
        bool const  has_integral_components,
        std::unique_ptr<std::vector<natural_8_bit> >&  data_ptr,
        spatial_boundary const* const  boundary
        )
{
    ASSUMPTION(data_ptr != nullptr && data_ptr->size() == num_bytes_per_component * num_components_per_primitive * num_primitives);

    m_id = id;
    m_num_primitives = num_primitives;
    m_num_components_per_primitive = num_components_per_primitive;
    m_num_bytes_per_component = num_bytes_per_component;
    m_has_integral_components = has_integral_components;
    m_data_ptr.swap(data_ptr);
    m_boundary = boundary == nullptr ? nullptr :
                                       std::unique_ptr<spatial_boundary>(new spatial_boundary(*boundary));

    ASSUMPTION(m_num_components_per_primitive != 0U);
    ASSUMPTION(m_num_primitives > 0U);
    ASSUMPTION((m_has_integral_components && m_num_bytes_per_component == (natural_8_bit)sizeof(natural_32_bit)) ||
               (!m_has_integral_components && m_num_bytes_per_component == sizeof(float_32_bit)));
}


void  buffer_file_data::create_gl_buffer() const
{
    if (id() != 0U)
        return;

    TMPROF_BLOCK();

    glGenBuffers(1U, &m_id);
    ASSUMPTION(m_id != 0U);

    GLenum const  target = has_integral_components() ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
    glBindBuffer(target, m_id);
    glBufferData(
            target,
            (GLsizeiptr)m_data_ptr->size(),
            (GLvoid const*)m_data_ptr->data(),
            GL_STATIC_DRAW
            );
    INVARIANT(glGetError() == 0U);
}


void  buffer_file_data::destroy_gl_buffer() const
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glDeleteBuffers(1U, &m_id);
    INVARIANT(glGetError() == 0U);
}


bool  buffer_file_data::make_current(natural_32_bit const  start_location, bool const  use_per_instance) const
{
    TMPROF_BLOCK();

    create_gl_buffer();

    glBindBuffer(GL_ARRAY_BUFFER, id());
    for (natural_32_bit  location_shift = 0U; location_shift * 4U < num_components_per_primitive(); ++location_shift)
    {
        natural_32_bit const  location = start_location + location_shift;
        natural_32_bit const  num_components = std::min(4U, num_components_per_primitive() - location_shift * 4U);
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(
                location,
                num_components,
                GL_FLOAT,
                GL_FALSE,
                num_components_per_primitive() * sizeof(float_32_bit),
                (void*)(location_shift * (4U * sizeof(float_32_bit)))
                );
        glVertexAttribDivisor(location, use_per_instance ? 1 : 0);

        INVARIANT(glGetError() == 0U);
    }

    return true;
}


}}

namespace gfx { namespace detail {


buffers_binding_data::buffers_binding_data(
        async::finalise_load_on_destroy_ptr const  finaliser,
        std::filesystem::path const&  index_buffer_path,
        std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, std::filesystem::path> const&  paths
        )
    : m_id(0U)
    , m_index_buffer()
    , m_num_indices_per_primitive(0U)
    , m_buffers()
    , m_ready(false)
{
    TMPROF_BLOCK();

    async::finalise_load_on_destroy_ptr const  buffers_finaliser =
        async::finalise_load_on_destroy::create(
                [this](async::finalise_load_on_destroy_ptr const  finaliser) {
                    initialise(0U, m_index_buffer, 0U, m_buffers);
                    },
                finaliser
                );

    m_index_buffer.insert_load_request(index_buffer_path.string(), buffers_finaliser);
    for (auto const& elem : paths)
        m_buffers[elem.first].insert_load_request(elem.second, buffers_finaliser);
}


buffers_binding_data::buffers_binding_data(
        async::finalise_load_on_destroy_ptr const  finaliser,
        natural_8_bit const  num_indices_per_primitive, // 1 (points), 2 (lines), or 3 (triangles)
        std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, std::filesystem::path> const&  paths
        )
    : m_id(0U)
    , m_index_buffer()
    , m_num_indices_per_primitive(0U)
    , m_buffers()
    , m_ready(false)
{
    TMPROF_BLOCK();

    async::finalise_load_on_destroy_ptr const  buffers_finaliser =
        async::finalise_load_on_destroy::create(
                [this, num_indices_per_primitive](async::finalise_load_on_destroy_ptr const  finaliser) {
                    initialise(0U, buffer(), num_indices_per_primitive, m_buffers);
                    },
                finaliser
                );

    for (auto const& elem : paths)
        m_buffers[elem.first].insert_load_request(elem.second, buffers_finaliser);
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
    ASSUMPTION(m_buffers.count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION) == 1UL);
    ASSUMPTION(m_buffers.at(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION).has_boundary());
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

    glGenVertexArrays(1U, &m_id);
    ASSUMPTION(m_id != 0U);
    INVARIANT(glGetError() == 0U);
}


void  buffers_binding_data::destroy_gl_binding()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glDeleteVertexArrays(1U, &m_id);
    INVARIANT(glGetError() == 0U);
}


}}

namespace gfx {


bool  buffers_binding::ready(
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
        std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, GLint> const&  locations
#endif
        ) const
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
        glBindVertexArray(id());
        INVARIANT(glGetError() == 0U);

        if (get_index_buffer().loaded_successfully())
            get_index_buffer().create_gl_buffer();

        for (auto const& location_and_buffer : get_buffers())
            if (location_and_buffer.second.make_current(
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
                    locations.at(location_and_buffer.first),
#else
                    value(location_and_buffer.first),
#endif
                    false) == false)
                return false;

        mutable_this->set_ready();
    }

    return true;
}


bool  buffers_binding::make_current(
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
        std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, GLint> const&  locations
#endif
        ) const
{
    TMPROF_BLOCK();

    if (!ready(
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
            locations
#endif
            ))
        return false;

    glBindVertexArray(id());
    INVARIANT(glGetError() == 0U);

    if (get_index_buffer().empty())
    {
        detail::current_draw::set_index_buffer_id(0U);
        detail::current_draw::set_num_components_per_primitive(get_num_indices_per_primitive());
        detail::current_draw::set_num_primitives(
            get_buffers().at(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION).num_primitives()
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


bool  make_current(
        buffers_binding const&  binding
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
        ,std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, GLint> const&  locations
#endif
        )
{
    return binding.make_current(
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
            locations
#endif
            );
}


}
