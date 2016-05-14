#include <qtgl/detail/vertex_program_cache.hpp>
#include <qtgl/detail/resource_loader.hpp>
#include <qtgl/shader_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <functional>

namespace qtgl { namespace detail {


bool  program_props_equal(vertex_program_properties_ptr const  props0, vertex_program_properties_ptr const  props1)
{
    return *props0 == *props1;
}

size_t  program_props_hasher(vertex_program_properties_ptr const  props)
{
    return hasher_of_vertex_program_properties(*props);
}

size_t  boost_path_hasher(boost::filesystem::path const&  path)
{
    std::size_t seed = 0ULL;
    boost::hash_combine(seed,path.string());
    return seed;
}


vertex_program_cache&  vertex_program_cache::instance()
{
    static vertex_program_cache  tc;
    return tc;
}

vertex_program_cache::vertex_program_cache()
    : m_cached_programs(10ULL,&qtgl::detail::boost_path_hasher)
    , m_pending_programs()
    , m_props_to_pathnames(10ULL,&qtgl::detail::program_props_hasher,&qtgl::detail::program_props_equal)
    , m_failed_loads(10ULL,&qtgl::detail::boost_path_hasher)
    , m_mutex()
    , m_dummy_program(vertex_program_generators::transform_3D_vertices::create())
{}

void  vertex_program_cache::receiver(boost::filesystem::path const&  shader_file,
                                     source_code_lines_ptr const  source_code_lines,
                                     std::string const&  error_message)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_pending_programs.push_back(std::make_tuple(shader_file,source_code_lines,error_message));
}

void vertex_program_cache::clear(bool const  props_to_files, bool const destroy_also_dummy_program)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_pending_programs.clear();
    m_cached_programs.clear();
    if (props_to_files)
        m_props_to_pathnames.clear();
    if (destroy_also_dummy_program)
        m_dummy_program.reset();
}

void  vertex_program_cache::insert_load_request(boost::filesystem::path const&  shader_file)
{
    TMPROF_BLOCK();

    if (shader_file.empty())
        return;
    if (!find(shader_file).expired())
        return;
    {
        std::lock_guard<std::mutex> const  lock(m_mutex);
        if (m_failed_loads.count(shader_file) != 0ULL)
            return;
    }
    if (shader_file == vertex_program_generators::transform_3D_vertices::imaginary_shader_file())
        return;
    if (!boost::filesystem::is_regular_file(shader_file))
        return;

    resource_loader::instance().insert(
                shader_file,
                std::bind(&vertex_program_cache::receiver,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3)
                );
}

bool  vertex_program_cache::insert_load_request(vertex_program_properties_ptr const  props)
{
    boost::filesystem::path  shader_file = find_shader_file(props);
    if (shader_file.empty())
        return false;
    insert_load_request(shader_file);
    return true;
}

bool  vertex_program_cache::insert(boost::filesystem::path const&  shader_file, vertex_program_ptr const  program)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    return m_cached_programs.insert({shader_file,program}).second;
}

void  vertex_program_cache::process_pending_programs()
{
    TMPROF_BLOCK();

    while (!m_pending_programs.empty())
    {
        boost::filesystem::path const& shader_file = std::get<0>(m_pending_programs.back());
        source_code_lines_ptr const source_code_lines = std::get<1>(m_pending_programs.back());
        std::string&  error_message = std::get<2>(m_pending_programs.back());

        vertex_program_ptr const  program =
                error_message.empty() ? vertex_program::create(*source_code_lines,error_message) :
                                        vertex_program_ptr();
        if (error_message.empty())
        {
            m_cached_programs.insert({shader_file,program});
            m_props_to_pathnames.insert({program->properties(),shader_file});
        }
        else
            m_failed_loads.insert({shader_file,m_pending_programs.back()});

        m_pending_programs.pop_back();
    }
}

std::weak_ptr<vertex_program const>  vertex_program_cache::find(boost::filesystem::path const&  shader_file)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    process_pending_programs();
    auto const  it = m_cached_programs.find(shader_file);
    if (it == m_cached_programs.cend())
        return {};
    return it->second;
}

std::weak_ptr<vertex_program const>  vertex_program_cache::find(vertex_program_properties_ptr const  props)
{
    TMPROF_BLOCK();

    boost::filesystem::path  shader_file = find_shader_file(props);
    if (shader_file.empty())
        return {};
    return find(shader_file);
}

bool  vertex_program_cache::associate_properties_with_pathname(vertex_program_properties_ptr const  props,
                                                               boost::filesystem::path const&  shader_file)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    auto const  it = m_props_to_pathnames.find(props);
    if (it == m_props_to_pathnames.end())
        return m_props_to_pathnames.insert({props,shader_file}).second;
    return shader_file == it->second;
}

boost::filesystem::path  vertex_program_cache::find_shader_file(vertex_program_properties_ptr const  props) const
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    auto const  it = m_props_to_pathnames.find(props);
    if (it == m_props_to_pathnames.end())
        return {};
    return it->second;
}


}}
