#include <utility/async_resource_load.hpp>


namespace async { namespace detail {


std::string  key_type::generate_unique_id()
{
    static natural_64_bit  s_fresh_key_id = 0ULL;
    static std::string const  s_uid_prefix(get_prefix_of_generated_fresh_unique_id());
    return s_uid_prefix + std::to_string(++s_fresh_key_id);
}

key_type const&  key_type::get_invalid_key()
{
    static key_type const  s_invalid_key(
            std::string(get_default_data_type_name()),
            std::string(get_prefix_of_generated_fresh_unique_id()) + "INVALID"
            );
    return s_invalid_key;
}


}}

namespace async { namespace detail {


resource_load_planner&  resource_load_planner::instance()
{
    static resource_load_planner planner;
    return planner;
}


resource_load_planner::~resource_load_planner()
{
    assert(m_worker_finished == true);
    assert(m_resource_just_being_loaded == key_type::get_invalid_key());
    assert(m_queue.empty());

    if (m_worker_thread.joinable()) // This must be here, because although the worker can be terminated
        m_worker_thread.join();     // we still need to join with it.
}


void  resource_load_planner::clear()
{
    TMPROF_BLOCK();

    if (m_worker_thread.joinable())
        m_worker_thread.join();
    std::lock_guard<std::mutex> const  lock(mutex());
    m_worker_finished = true;
    m_queue.clear();
    m_resource_just_being_loaded = key_type::get_invalid_key();
}


void  resource_load_planner::insert_load_request(
        key_type const&  key,
        resource_loader_type const&  loader,
        resource_load_priority_type const  priority
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(key != key_type::get_invalid_key());

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
        {
            for (queue_storage_type::iterator  it = m_queue.begin(); it != m_queue.end(); ++it)
                if (std::get<1>(*it) == key)
                {
                    *it = queue_value_type{ std::get<0>(*it), key_type::get_invalid_key(), std::get<2>(*it) };
                    break;
                }
            break;
        }
    }
}


resource_load_planner::resource_load_planner()
    : m_queue()
    , m_mutex()
    , m_resource_just_being_loaded(key_type::get_invalid_key())
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
        TMPROF_BLOCK();

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

        if (std::get<1>(task) != key_type::get_invalid_key())
        {
            TMPROF_BLOCK();
            std::get<2>(task)();
        }

        std::lock_guard<std::mutex> const  lock(mutex());
        m_resource_just_being_loaded = key_type::get_invalid_key();
    }

    m_worker_finished = true;
}


finalise_load_on_destroy::~finalise_load_on_destroy()
{
    resource_cache::instance().finalise_load(m_key, m_error_message);
}


resource_holder_type::resource_holder_type()
    : m_ref_count(0ULL)
    , m_resource_ptr(nullptr)
    , m_error_message()
    , m_load_state(LOAD_STATE::IN_PROGRESS)
{}


void  resource_holder_type::finalise_load(std::string const&  force_error_message)
{
    if (m_load_state != LOAD_STATE::IN_PROGRESS)
    {
        ASSUMPTION(m_error_message == force_error_message);
        return;
    }

    ASSUMPTION(m_error_message.empty() || m_error_message == force_error_message);

    if (!force_error_message.empty())
        m_error_message = force_error_message;

    if (!m_error_message.empty())
        m_load_state = LOAD_STATE::FINISHED_WITH_ERROR;
    else
    {
        ASSUMPTION(resource_ptr() != nullptr);
        m_load_state = LOAD_STATE::FINISHED_SUCCESSFULLY;
    }
}


resource_cache&  resource_cache::instance()
{
    static resource_cache  cache;
    return cache;
}


resources_cache_type::value_type*  resource_cache::find_resource(key_type const&  key)
{
    resources_cache_type::iterator const  it = m_cache.find(key);
    if (it != m_cache.end())
        return &*it;
    return nullptr;
}


void  resource_cache::finalise_load(key_type const&  key, std::string const&  force_error_message)
{
    TMPROF_BLOCK();

    {
        std::lock_guard<std::mutex> const  lock(mutex());
        resources_cache_type::value_type* const  resource_ptr = find_resource(key);
        if (resource_ptr == nullptr)
            return;
        resource_ptr->second->finalise_load(force_error_message);
    }
    process_notification_callbacks(key);
}


void  resource_cache::process_notification_callbacks(key_type const&  key)
{
    TMPROF_BLOCK();

    std::vector<notification_callback_type>  to_process;
    {
        std::lock_guard<std::mutex> const  lock(mutex());
        auto const  it = m_notification_callbacks.find(key);
        if (it != m_notification_callbacks.end())
        {
            to_process = it->second;
            m_notification_callbacks.erase(it);
        }
    }
    for (auto const&  callback : to_process)
        callback();
}


natural_64_bit  resource_cache::s_fresh_key_id = 0ULL;


key_type  resource_cache::generate_fresh_key()
{
    return key_type{"@generic", msgstream() << ++s_fresh_key_id};
}


resource_cache::~resource_cache()
{
    assert(m_cache.empty());
    assert(m_notification_callbacks.empty());
}


}}

namespace async {


key_type  get_statistics_of_cached_resources(statistics_of_cached_resources&  output_statistics)
{
    // TODO!
    return get_invalid_key();
}


}
