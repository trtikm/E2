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


std::ostream&  operator<<(std::ostream&  ostr, key_type const&  key)
{
    ostr << "async::key_type{" << key.get_data_type_name() << ", " << key.get_unique_id() << "}";
    return ostr;
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

finalise_load_on_destroy_ptr  finalise_load_on_destroy::create(
        key_type const&  key,
        finalise_load_on_destroy_ptr const  parent
        )
{
    ASSUMPTION(key != key_type::get_invalid_key());
    return  finalise_load_on_destroy_ptr(new finalise_load_on_destroy(key, parent, callback_function_type()));
}


finalise_load_on_destroy_ptr  finalise_load_on_destroy::create(
        callback_function_type const&  callback,
        finalise_load_on_destroy_ptr const  finaliser
        )
{
    ASSUMPTION(callback.operator bool() && finaliser != nullptr && finaliser->get_key() != key_type::get_invalid_key());
    return  finalise_load_on_destroy_ptr(new finalise_load_on_destroy(key_type::get_invalid_key(), finaliser, callback));
}


finalise_load_on_destroy::~finalise_load_on_destroy()
{
    TMPROF_BLOCK();

    if (get_key() == key_type::get_invalid_key())
    {
        if (get_error_message().empty())
        {
            TMPROF_BLOCK();

            try
            {
                get_callback()(get_parent());
            }
            catch (std::exception const&  e)
            {
                get_parent()->force_finalisation_as_failure(e.what());
            }
        }
        else
            get_parent()->force_finalisation_as_failure(get_error_message());
    }
    else if (resource_cache::instance().finalise_load(get_key(), get_error_message()) == LOAD_STATE::FINISHED_WITH_ERROR)
    {
        if (get_parent() != nullptr)
            get_parent()->force_finalisation_as_failure(
                    msgstream() << "Load of depenedent resource '" << get_key() << "' has FAILED."
                    );
    }
}


void  finalise_load_on_destroy::force_finalisation_as_failure(std::string const&  force_error_message)
{
    std::size_t const  index = force_error_message.find("ERROR: ");
    std::string const  error_message_addon = index == 0UL ? force_error_message.substr(7UL) : force_error_message;
    if (m_error_message.empty())
        m_error_message = "ERROR: " + error_message_addon;
    else
        m_error_message += " " + error_message_addon;
}



resource_holder_type::resource_holder_type()
    : m_ref_count(0ULL)
    , m_resource_ptr(nullptr)
    , m_error_message()
    , m_load_state(LOAD_STATE::IN_PROGRESS)
{}


void  resource_holder_type::finalise_load(std::string const&  force_error_message)
{
    switch (m_load_state)
    {
    case LOAD_STATE::IN_PROGRESS:
        if (force_error_message.empty())
        {
            ASSUMPTION(resource_ptr() != nullptr);
            m_load_state = LOAD_STATE::FINISHED_SUCCESSFULLY;
        }
        else
        {
            m_load_state = LOAD_STATE::FINISHED_WITH_ERROR;
            m_error_message = force_error_message;
            LOG(error, m_error_message);
        }
        break;
    case LOAD_STATE::FINISHED_SUCCESSFULLY:
        if (!force_error_message.empty())
        {
            m_load_state = LOAD_STATE::FINISHED_WITH_ERROR;
            m_error_message = force_error_message;
            LOG(error, m_error_message);
        }
        break;
    case LOAD_STATE::FINISHED_WITH_ERROR:
        if (!force_error_message.empty())
        {
            m_error_message += " " + force_error_message;
            LOG(error, m_error_message);
        }
        break;
    default: UNREACHABLE();
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


LOAD_STATE  resource_cache::finalise_load(key_type const  key, std::string const&  error_message)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(mutex());
    resources_cache_type::value_type* const  resource_ptr = find_resource(key);
    if (resource_ptr == nullptr)
        return LOAD_STATE::FINISHED_SUCCESSFULLY;
    resource_ptr->second->finalise_load(error_message);
    return resource_ptr->second->get_load_state();
}


natural_64_bit  resource_cache::s_fresh_key_id = 0ULL;


key_type  resource_cache::generate_fresh_key()
{
    return key_type{"@generic", msgstream() << ++s_fresh_key_id};
}


resource_cache::~resource_cache()
{
    assert(m_cache.empty());
}


}}

namespace async {


key_type  get_statistics_of_cached_resources(statistics_of_cached_resources&  output_statistics)
{
    TMPROF_BLOCK();

    {
        std::lock_guard<std::mutex> const  lock(detail::resource_cache::instance().mutex());
        for (auto const&  cache_elem : detail::resource_cache::instance().get_cache())
            output_statistics.insert({
                cache_elem.first,
                { cache_elem.second->get_load_state(), cache_elem.second->ref_count(), cache_elem.second->error_message() }
                });
    }

    key_type  just_being_loaded = get_invalid_key();
    {
        std::lock_guard<std::mutex> const  lock(detail::resource_cache::instance().mutex());
        just_being_loaded = detail::resource_load_planner::instance().resource_just_being_loaded();
    }

    // This is mecessary, because loading is on a different 'worker' thread and so many other
    // resources could be loaded between the filling-in the 'output_statistics' map and acquiring
    // the 'just_being_loaded' key. So, the next line preserves the consistency (in a price of
    // some inaccuracy).
    if (just_being_loaded != get_invalid_key())
    {
        auto  it = output_statistics.find(just_being_loaded);
        if (it == output_statistics.end())
            output_statistics.insert({ just_being_loaded, cached_resource_info(LOAD_STATE::IN_PROGRESS, 1ULL, "") });
        else
            output_statistics.at(just_being_loaded) = cached_resource_info(LOAD_STATE::IN_PROGRESS, 1ULL, "");
    }

    return just_being_loaded;
}


}
