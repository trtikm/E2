#include <qtgl/detail/resource_loader.hpp>
#include <qtgl/detail/texture_cache.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <tuple>

namespace qtgl { namespace detail {


resource_loader&  resource_loader::instance()
{
    static resource_loader  rl;
    return rl;
}

resource_loader::resource_loader()
    : m_worker_thread()
    , m_worker_finished(true)
    , m_mutex()
    , m_texture_requests()
{}

void  resource_loader::start_worker_if_not_running()
{
    if (m_worker_finished)
    {
        m_worker_finished = false;
        m_worker_thread = std::thread(&resource_loader::worker,this);
    }
}

void  resource_loader::clear()
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_texture_requests.clear();
    if (m_worker_thread.joinable())
        m_worker_thread.join();
}

void  resource_loader::insert(texture_properties_ptr const  props, texture_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_texture_requests.push_back({props,receiver});
    start_worker_if_not_running();
}

bool  resource_loader::fetch(texture_properties_ptr&  output_props, texture_receiver_fn&  output_receiver)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (m_texture_requests.empty())
        return false;
    std::tie(output_props,output_receiver) = m_texture_requests.front();
    m_texture_requests.pop_front();
    return true;
}

void  resource_loader::worker()
{
    TMPROF_BLOCK();

    while (true)
    {
        bool  done = true;

        // Loading textures
        {
            texture_properties_ptr  props;
            texture_receiver_fn  receiver;
            while (fetch(props,receiver))
            {
                receiver(load_texture_image_file(props->image_file()),props);
                done = false;
            }
        }

        if (done)
            break;
    }

    m_worker_finished = true;
}


}}
