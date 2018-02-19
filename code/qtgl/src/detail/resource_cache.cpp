#include <qtgl/detail/resource_cache.hpp>

namespace qtgl { namespace detail { namespace async {


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
    std::lock_guard<std::mutex> const  lock(mutex());
    m_worker_finished = true;
    m_queue.clear();
    m_resource_just_being_loaded.clear();
}


void  resource_load_planner::insert_load_request(
        key_type const&  key,
        resource_loader_type const&  loader,
        resource_load_priority_type const  priority
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!key.empty());

    std::lock_guard<std::mutex> const  lock(mutex());
    m_queue.push(queue_value_type{priority,key,loader});
    start_worker_if_not_running();
}


void  resource_load_planner::cancel_load_request(key_type const&  key)
{
    TMPROF_BLOCK();

    while (true)
    {
        std::lock_guard<std::mutex> const  lock(mutex());
        if (resource_just_being_loaded() != key)
            break;
    }

    std::lock_guard<std::mutex> const  lock(mutex());
    for (queue_storage_type::iterator  it = m_queue.begin(); it != m_queue.end(); ++it)
        if (std::get<1>(*it) == key)
        {
            *it = queue_value_type{ std::get<0>(*it), "", std::get<2>(*it) };
            break;
        }
}


resource_load_planner::resource_load_planner()
    : m_queue()
    , m_mutex()
    , m_resource_just_being_loaded()
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
            std::lock_guard<std::mutex> const  lock(mutex());
            if (m_queue.empty())
                done = true;
            else
            {
                done = false;

                task = m_queue.top();
                m_queue.pop();

                m_resource_just_being_loaded = std::get<1>(task);
            }
        }
        if (done)
            break;

        if (!std::get<1>(task).empty())
            std::get<2>(task)();

        std::lock_guard<std::mutex> const  lock(mutex());
        m_resource_just_being_loaded.clear();
    }

    m_worker_finished = true;
}


resource_holder_type::resource_holder_type()
    : m_ref_count(0ULL)
    , m_resource_ptr(nullptr)
    , m_error_message()
    , m_load_state(ASYNC_LOAD_STATE::IN_PROGRESS)
{}


ASYNC_LOAD_STATE  resource_holder_type::get_load_state(key_type const&  key) const
{
    if (m_load_state == ASYNC_LOAD_STATE::IN_PROGRESS)
    {
        std::lock_guard<std::mutex> const  lock(resource_load_planner::instance().mutex());
        if (resource_load_planner::instance().resource_just_being_loaded() != key)
        {
            if (resource_ptr() != nullptr)
                m_load_state = ASYNC_LOAD_STATE::FINISHED_SUCCESSFULLY;
            else if (!m_error_message.empty())
                m_load_state = ASYNC_LOAD_STATE::FINISHED_WITH_ERROR;
        }
    }
    return m_load_state;
}


resource_cache&  resource_cache::instance()
{
    static resource_cache  cache;
    return cache;
}


void  resource_cache::erase_resource(key_type const&  key)
{
    TMPROF_BLOCK();

    auto const  it = m_cache.find(key);
    if (it == m_cache.end())
        return;
    m_cache.erase(it);
}


resource_handle::resource_handle(resources_cache_type::value_type* const  data_ptr)
    : m_data_ptr(data_ptr)
{
    ASSUMPTION(m_data_ptr != nullptr);
    m_data_ptr->second.inc_ref_count();
}


}}}
