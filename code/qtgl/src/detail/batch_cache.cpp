#include <qtgl/detail/batch_cache.hpp>
#include <qtgl/detail/resource_loader.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <functional>

namespace qtgl { namespace detail {


static size_t  boost_path_hasher(boost::filesystem::path const&  path)
{
    std::size_t seed = 0ULL;
    boost::hash_combine(seed,path.string());
    return seed;
}


batch_cache&  batch_cache::instance()
{
    static batch_cache  bc;
    return bc;
}

batch_cache::batch_cache()
    : m_cached_batches(10ULL,&qtgl::detail::boost_path_hasher)
    , m_pending_batches()
    , m_failed_loads(10ULL,&qtgl::detail::boost_path_hasher)
    , m_mutex()
{}

void  batch_cache::receiver(boost::filesystem::path const&  path,
                            std::shared_ptr<batch const> const  data,
                            std::string const&  error_message
                            )
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_pending_batches.push_back(std::make_tuple(path,data,error_message));
}

void batch_cache::clear()
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_cached_batches.clear();
    m_pending_batches.clear();
    m_failed_loads.clear();
}

void  batch_cache::insert_load_request(boost::filesystem::path const&  batch_file)
{
    TMPROF_BLOCK();

    if (batch_file.empty())
        return;
    if (find(batch_file).operator bool())
        return;
    {
        std::lock_guard<std::mutex> const  lock(m_mutex);
        if (m_failed_loads.count(batch_file) != 0ULL)
            return;
    }

    resource_loader::instance().insert_batch_request(
                batch_file,
                std::bind(&batch_cache::receiver,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3)
                );
}

void  batch_cache::process_pending_batches()
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    while (!m_pending_batches.empty())
    {
        std::string const&  error_message = std::get<2>(m_pending_batches.back());
        if (error_message.empty())
        {
            boost::filesystem::path const&  batch_file = std::get<0>(m_pending_batches.back());
            if (m_cached_batches.count(batch_file) == 0ULL)
            {
                std::shared_ptr<batch const> const batch_data = std::get<1>(m_pending_batches.back());
                INVARIANT(batch_data.operator bool());
                m_cached_batches.insert({batch_file,batch_data});
            }
        }
        else
        {
            boost::filesystem::path const&  batch_file = std::get<0>(m_pending_batches.back());
            if (m_failed_loads.count(batch_file) == 0ULL)
                m_failed_loads.insert({batch_file,error_message});
        }
        m_pending_batches.pop_back();
    }
}

batch_ptr  batch_cache::find(boost::filesystem::path const&  shader_file)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    auto const  it = m_cached_batches.find(shader_file);
    if (it == m_cached_batches.cend())
        return {};
    return it->second;
}

void  batch_cache::cached(std::vector<boost::filesystem::path>&  output)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    for (auto const&  path_batch : m_cached_batches)
        output.push_back(path_batch.first);
}

void  batch_cache::failed(std::vector< std::pair<boost::filesystem::path,std::string> >&  output)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    for (auto const&  path_error : m_failed_loads)
        output.push_back(path_error);
}



}}
