#include <utility/async_resource_load.hpp>
#include <utility/config.hpp>
#include <boost/algorithm/string.hpp>


namespace async { namespace detail {


natural_64_bit  key_type::generate_next_unique_id()
{
    static std::atomic<natural_64_bit>  s_fresh_key_id{0ULL};
    return ++s_fresh_key_id;
}

std::string  key_type::generate_unique_id()
{
    static std::string const  s_uid_prefix(get_prefix_of_generated_fresh_unique_id());
    return s_uid_prefix + std::to_string(generate_next_unique_id());
}

std::string  key_type::generate_unique_custom_id(std::string  custom_prefix)
{
    boost::algorithm::replace_all(custom_prefix, "\n", "\\n");
    boost::algorithm::replace_all(custom_prefix, "\r", "\\r");
    boost::algorithm::replace_all(custom_prefix, "\t", "\\t");
    return custom_prefix + "#" + std::to_string(generate_next_unique_id());
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


std::string  to_string(key_type const&  key)
{
    std::stringstream  sstr;
    sstr << key;
    return sstr.str();
}


}}

namespace async { namespace detail {


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


finalise_load_on_destroy::finalise_load_on_destroy(
    key_type const& key,
    finalise_load_on_destroy_ptr const  parent,
    callback_function_type const& callback
)
    : m_key(key)
    , m_error_message()
    , m_parent(parent)
    , m_callback(callback)
{}


finalise_load_on_destroy::~finalise_load_on_destroy()
{
    TMPROF_BLOCK();

    if (is_terminated())
    {
        if (get_parent() != nullptr && get_parent()->get_error_message().empty())
            get_parent()->force_finalisation_as_failure(msgstream() <<
                "ERROR: Resource loading was terminated. So, terminating also load of the resource " << get_parent()->get_key() << "."
                );
        return;
    }

    if (get_key() == key_type::get_invalid_key())
    {
        if (get_error_message().empty())
        {
            TMPROF_BLOCK();

            try
            {
                if (get_parent()->get_error_message().empty())
                {
                    bool  is_resource_valid = true;
                    if (get_parent()->get_key() != key_type::get_invalid_key())
                    {
                        std::lock_guard<std::mutex> const  lock(resource_cache::instance().mutex());
                        is_resource_valid = resource_cache::instance().find_resource(get_parent()->get_key()) != nullptr;
                    }
                    if (is_resource_valid)
                        get_callback()(get_parent());
                    else
                        get_parent()->force_finalisation_as_failure(
                                msgstream() << "The resource '" << get_parent()->get_key() << "' is no longer valid."
                                );
                }
            }
            catch (std::exception const&  e)
            {
                get_parent()->force_finalisation_as_failure(e.what());
            }
        }
        else
            get_parent()->force_finalisation_as_failure(get_error_message());
    }
    else
    {
        LOAD_STATE const  load_state = resource_cache::instance().finalise_load(get_key(), get_error_message());

        std::vector<finalise_load_on_destroy_ptr>  finalisers;
        resource_cache::instance().take_finalisers(get_key(), finalisers);

        if (load_state == LOAD_STATE::FINISHED_WITH_ERROR)
        {
            std::string const  msg = msgstream() << "Load of depenedent resource '" << get_key() << "' has FAILED.";
            if (get_parent() != nullptr)
                get_parent()->force_finalisation_as_failure(msg);
            for (auto finaliser_ptr : finalisers)
                finaliser_ptr->force_finalisation_as_failure(msg);
        }
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


}}

namespace async { namespace detail {


static bool  is_in_terminated_state = false;

void terminate()
{
    const bool do_clear = !is_terminated();
    is_in_terminated_state = true;
    if (do_clear)
        resource_load_planner::instance().clear();
}

bool is_terminated()
{
    return is_in_terminated_state;
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
    terminate();

    assert(m_worker_finished == true);
    assert(get_key_of_resource_load_data(m_resource_just_being_loaded) == key_type::get_invalid_key());
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
    m_resource_just_being_loaded = get_invalid_resource_load_data();
}


void  resource_load_planner::insert_load_request(resource_load_priority_type const  priority, resource_load_data_type const&  data)
{
    TMPROF_BLOCK();

    if (is_terminated())
        return;

    std::lock_guard<std::mutex> const  lock(mutex());
    m_queue.push(queue_value_type{priority, data});
    start_worker_if_not_running();
}


resource_load_planner::resource_load_planner()
    : m_queue()
    , m_mutex()
    , m_resource_just_being_loaded(get_invalid_resource_load_data())
    , m_worker_thread()
    , m_worker_finished(true)
{}


void  resource_load_planner::start_worker_if_not_running()
{
    TMPROF_BLOCK();
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    resource_load_planner::worker();
#else
    if (m_worker_finished)
    {
        if (m_worker_thread.joinable())
            m_worker_thread.join();
        m_worker_finished = false;
        m_worker_thread = std::thread(&resource_load_planner::worker,this);
    }
#endif
}


void  resource_load_planner::worker()
{
    TMPROF_BLOCK();

    while (true)
    {
        TMPROF_BLOCK();

        bool  done;
        {
            std::lock_guard<std::mutex> const  lock(mutex());
            if (m_queue.empty())
                done = true;
            else
            {
                done = false;

                m_resource_just_being_loaded = m_queue.top().second;
                m_queue.pop();
            }
        }
        if (done)
            break;

        {
            TMPROF_BLOCK();
            auto const loaded_key = m_resource_just_being_loaded.first;
            LOG(LSL_DEBUG, "async::detail::resource_load_planner::worker(): STARTING key_type=" << loaded_key);
            perform_load_of_resource_load_data(m_resource_just_being_loaded);
            LOG(LSL_DEBUG, "async::detail::resource_load_planner::worker(): FINISHED key_type=" << loaded_key);
        }

        resource_load_data_type const  auto_delete_resource_in_the_end_of_the_interation = m_resource_just_being_loaded;

        {
            std::lock_guard<std::mutex> const  lock(mutex());
            m_resource_just_being_loaded = get_invalid_resource_load_data();
        }
    }

    m_worker_finished = true;
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
            LOG(LSL_ERROR, m_error_message);
        }
        break;
    case LOAD_STATE::FINISHED_SUCCESSFULLY:
        if (!force_error_message.empty())
        {
            m_load_state = LOAD_STATE::FINISHED_WITH_ERROR;
            m_error_message = force_error_message;
            LOG(LSL_ERROR, m_error_message);
        }
        break;
    case LOAD_STATE::FINISHED_WITH_ERROR:
        if (!force_error_message.empty())
        {
            m_error_message += " " + force_error_message;
            LOG(LSL_ERROR, m_error_message);
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


void  resource_cache::take_finalisers(key_type const  key, std::vector<finalise_load_on_destroy_ptr>&  output)
{
    std::lock_guard<std::mutex> const  lock(mutex());
    finalisers_cache_type::iterator const  it = m_finalisers.find(key);
    if (it != m_finalisers.end())
    {
        output.swap(it->second);
        //for (auto  finaliser_ptr : it->second)
        //    output.push_back(finaliser_ptr);
        it->second.clear();
    }
}


natural_64_bit  resource_cache::s_fresh_key_id = 0ULL;


key_type  resource_cache::generate_fresh_key()
{
    return key_type{"@generic", msgstream() << ++s_fresh_key_id};
}


resource_cache::~resource_cache()
{
    terminate();
    assert(m_cache.empty());
}


}}

namespace async {


key_type  get_key_of_resource_just_being_loaded()
{
    std::lock_guard<std::mutex> const  lock(detail::resource_load_planner::instance().mutex());
    return detail::resource_load_planner::instance().resource_just_being_loaded().first;
}


}

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
        just_being_loaded = get_key_of_resource_load_data(detail::resource_load_planner::instance().resource_just_being_loaded());
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
