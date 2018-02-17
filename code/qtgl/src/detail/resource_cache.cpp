#include <qtgl/detail/resource_cache.hpp>

namespace qtgl { namespace detail {


resource_load_planner&  resource_load_planner::instance()
{
    static resource_load_planner planner;
    return planner;
}


resource_load_planner::~resource_load_planner()
{
    clear();
}


void  resource_load_planner::clear()
{
    TMPROF_BLOCK();

    if (m_worker_thread.joinable())
        m_worker_thread.join();
    m_worker_finished = true;
    std::lock_guard<std::mutex> const  lock(mutex_to_resources());
    m_queue.clear();
    m_pointer_to_resource_just_being_loaded = nullptr;
}


void  resource_load_planner::insert_load_request(
        boost::filesystem::path const&  path,
        pointer_to_resource_type const  pointer_to_resource,
        loader_type const&  loader,
        natural_32_bit const  priority
        )
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(mutex_to_resources());
    for (queue_storage_type::const_iterator  it = m_queue.cbegin(); it != m_queue.cend(); ++it)
        if (std::get<2>(*it) == pointer_to_resource)
            return;
    m_queue.push(queue_value_type{priority,path,pointer_to_resource,loader});
    start_worker_if_not_running();
}


void  resource_load_planner::invalidate_load_request(pointer_to_resource_type const  pointer_to_resource)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(mutex_to_resources());
    for (queue_storage_type::iterator  it = m_queue.begin(); it != m_queue.end(); ++it)
        if (std::get<2>(*it) == pointer_to_resource)
        {
            std::get<2>(*it) = nullptr;
            break;
        }
}


resource_load_planner::resource_load_planner()
    : m_queue()
    , m_mutex_to_resources()
    , m_pointer_to_resource_just_being_loaded(nullptr)
    , m_worker_thread()
    , m_worker_finished(true)
{}


void  resource_load_planner::start_worker_if_not_running()
{
    TMPROF_BLOCK();

    if (m_worker_finished)
    {
        if (m_worker_thread.joinable())
            m_worker_thread.join();
        m_worker_finished = false;
        m_worker_thread = std::thread(&resource_load_planner::worker,this);
    }
}


void  resource_load_planner::worker()
{
    TMPROF_BLOCK();

    while (true)
    {
        bool  done;
        queue_value_type  task;
        {
            std::lock_guard<std::mutex> const  lock(mutex_to_resources());
            if (m_queue.empty())
                done = true;
            else
            {
                done = false;

                task = m_queue.top();
                m_queue.pop();

                m_pointer_to_resource_just_being_loaded = std::get<2>(task);
            }
        }
        if (done)
            break;

        if (std::get<2>(task) != nullptr)
            std::get<3>(task)(std::get<1>(task),std::get<2>(task));

        std::lock_guard<std::mutex> const  lock(mutex_to_resources());
        m_pointer_to_resource_just_being_loaded = nullptr;
    }

    m_worker_finished = true;
}


}}
