#include <qtgl/detail/buffer_cache.hpp>
#include <qtgl/detail/resource_loader.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <functional>

namespace qtgl { namespace detail {


static bool  buffers_props_equal(buffer_properties_ptr const  props0, buffer_properties_ptr const  props1)
{
    return *props0 == *props1;
}

static size_t  buffers_props_hasher(buffer_properties_ptr const  props)
{
    return hasher_of_buffer_properties(*props);
}

static size_t  boost_path_hasher(boost::filesystem::path const&  path)
{
    std::size_t seed = 0ULL;
    boost::hash_combine(seed,path.string());
    return seed;
}


buffer_cache&  buffer_cache::instance()
{
    static buffer_cache  bc;
    return bc;
}

buffer_cache::buffer_cache()
    : m_cached_buffers(10ULL,&qtgl::detail::boost_path_hasher)
    , m_pending_buffers()
    , m_failed_loads(10ULL,&qtgl::detail::boost_path_hasher)
    , m_mutex()
{}

void  buffer_cache::receiver(buffer_properties_ptr const  props,
                             buffer_data_ptr const  data,
                             std::string const&  error_message
                             )
{
    TMPROF_BLOCK();

    ASSUMPTION(props.operator bool());

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_pending_buffers.push_back(std::make_tuple(props,data,error_message));
}

void buffer_cache::clear()
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_pending_buffers.clear();
    m_cached_buffers.clear();
    m_failed_loads.clear();
}

void  buffer_cache::insert_load_request(boost::filesystem::path const&  buffer_file)
{
    TMPROF_BLOCK();

    if (buffer_file.empty())
        return;
    if (!find(buffer_file).expired())
        return;
    {
        std::lock_guard<std::mutex> const  lock(m_mutex);
        if (m_failed_loads.count(buffer_file) != 0ULL)
            return;
    }

    resource_loader::instance().insert_buffer_request(
                buffer_file,
                std::bind(&buffer_cache::receiver,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3)
                );
}

void  buffer_cache::process_pending_buffers()
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    while (!m_pending_buffers.empty())
    {
        buffer_properties_ptr const  buffer_props = std::get<0>(m_pending_buffers.back());
        if (m_cached_buffers.count(buffer_props->buffer_file()) == 0ULL &&
            m_failed_loads.count(buffer_props->buffer_file()) == 0ULL)
        {
            buffer_data_ptr const buffer_data = std::get<1>(m_pending_buffers.back());
            std::string&  error_message = std::get<2>(m_pending_buffers.back());

            buffer_ptr const  buffer =
                    error_message.empty() ? buffer::create(*buffer_data,buffer_props,error_message) :
                                            buffer_ptr();
            if (error_message.empty())
                m_cached_buffers.insert({buffer_props->buffer_file(),buffer});
            else
                m_failed_loads.insert({buffer_props->buffer_file(),m_pending_buffers.back()});
        }
        m_pending_buffers.pop_back();
    }
}

std::weak_ptr<buffer const>  buffer_cache::find(boost::filesystem::path const&  shader_file)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    auto const  it = m_cached_buffers.find(shader_file);
    if (it == m_cached_buffers.cend())
        return {};
    return it->second;
}

void  buffer_cache::cached(std::vector<buffer_properties_ptr>&  output)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    for (auto const&  path_buffer : m_cached_buffers)
        output.push_back(path_buffer.second->properties());
}

void  buffer_cache::failed(std::vector< std::pair<buffer_properties_ptr,std::string> >&  output)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    for (auto const&  path_info : m_failed_loads)
        output.push_back({std::get<0>(path_info.second),std::get<2>(path_info.second)});
}



}}
